/*********************************************************************************
 * @copyright (c) 2024 University of Applied Sciences and Arts Western Switzerland
 * All rights reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential
 *********************************************************************************
 * Project : HEIA-FR / Autonomous Metrology in Underground Sites
 * @file   : bluesync.c
 * @date   : 10.12.2024
 ********************************************************************************/

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/mesh.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>

#include <math.h>

#include <zephyr/sys/util.h>

#if defined(CONFIG_BLUESYNC_USED_IN_MESH)
#include <zephyr/bluetooth/mesh.h>
#endif

#include <bluesync/bluesync.h>
#include "bluesync.h"
#include "bs_state_machine.h"
#include "bluesync_bitfields.h"
#include "local_time.h"


#if defined(CONFIG_BLUESYNC_TEST_BABBLESIM_SUPPORT)
#include "statistic/bluesync_statistic.h"
#endif

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(bluesync, CONFIG_BLUESYNC_LOG_LEVEL);

#define BT_UNIT_MS_TO_TICKS(_ms) ((_ms) * 8 / 5)
#define BLUESYNC_BT_MIN_ADV_INT BT_UNIT_MS_TO_TICKS(30)
#define BLUESYNC_BT_MAX_ADV_INT BT_UNIT_MS_TO_TICKS(60)
#define BLUESYNC_BT_SCAN_INT BT_UNIT_MS_TO_TICKS(60)
#define BLUESYNC_BT_SCAN_WIND_SIZE BT_UNIT_MS_TO_TICKS(60)

//bluesync node param

static struct bluesync_param param = {
	.timeslot_index = 0,
	.local = {
		.timer_ticks = {0},
		.bitfield = {0},
	},
	.local_mutex = Z_MUTEX_INITIALIZER(param.local_mutex),
	.local_history = {
		[0 ... BURST_WINDOWS_SIZE-1] = {
			.timer_ticks = {0},
			.bitfield = {0},
		}
	},
	.local_head = 0,
    .local_count = 0,
	.local_history_mutex = Z_MUTEX_INITIALIZER(param.local_history_mutex),

	.rcv = {
		.timer_ticks = {0},
		.bitfield = {0},
	},
	.rcv_mutex = Z_MUTEX_INITIALIZER(param.rcv_mutex),
	.rcv_history = {
		[0 ... BURST_WINDOWS_SIZE-1] = {
			.timer_ticks = {0},
			.bitfield = {0},
		}
	},
	.rcv_head = 0,
    .rcv_count = 0,
	.rcv_history_mutex = Z_MUTEX_INITIALIZER(param.rcv_history_mutex),

	.current_round_id = 0xFF,
	.adv_param = BT_LE_ADV_PARAM_INIT(
		BT_LE_ADV_OPT_EXT_ADV |
		BT_LE_ADV_OPT_USE_IDENTITY |
		BT_LE_ADV_OPT_CODED ,
		BLUESYNC_BT_MIN_ADV_INT,
		BLUESYNC_BT_MAX_ADV_INT,
		NULL
	),
	.scan_param = BT_LE_SCAN_PARAM_INIT(
		BT_LE_SCAN_TYPE_PASSIVE,
		BT_LE_SCAN_OPT_CODED | 
		BT_LE_SCAN_OPT_NO_1M,
		BLUESYNC_BT_SCAN_INT,
		BLUESYNC_BT_SCAN_WIND_SIZE
	),
	.mutex = Z_MUTEX_INITIALIZER(param.mutex),
};

// Define the stack space for the thread
K_THREAD_STACK_DEFINE(bluesync_thread_stack, CONFIG_BLUESYNC_THREAD_STACK_SIZE);

K_MSGQ_DEFINE(bluesync_rx_msgq, sizeof(struct bluesync_msg_client), 17, 1);
K_SEM_DEFINE(bluesync_end_sync_sem, 0, 1);
K_SEM_DEFINE(bluesync_start_new_sync_sem, 0, 1);
K_SEM_DEFINE(bluesync_role_assign_sem, 0, 1);

static void reset_bluesync_timestamps(bluesync_timestamps_t *elem){
	memset(&elem->bitfield, 0, sizeof(elem->bitfield));
	memset(&elem->timer_ticks, 0, sizeof(elem->timer_ticks));
}

static void add_bluesync_timestamps(bluesync_timestamps_t *elem
									,uint8_t pos 
									,int64_t ticks 
#if defined(CONFIG_BLUESYNC_TEST_BABBLESIM_SUPPORT)
									,uint64_t est_ticks
#endif
									)
{
	elem->timer_ticks[pos] = ticks;
#if defined(CONFIG_BLUESYNC_TEST_BABBLESIM_SUPPORT)
	elem->remote_est_ticks[pos] = est_ticks;
#endif
	set_bit(&elem->bitfield[0], (size_t)pos);
}

static void bluesync_reset_param(){
	param.timeslot_index = 0;

	k_mutex_lock(&param.local_mutex, K_FOREVER);
	{
		reset_bluesync_timestamps(&param.local);
	}
	k_mutex_unlock(&param.local_mutex);
	k_mutex_lock(&param.rcv_mutex, K_FOREVER);
	{
		reset_bluesync_timestamps(&param.rcv);
	}
	k_mutex_unlock(&param.rcv_mutex);
}

static bluesync_status_t calculate_lr_from_history(
    double *slope,
    double *offset,
    size_t min_nb_timestamp)
{

    double sum_x = 0, sum_y = 0;
    size_t n = 0;

    // Accumulate
    for (int burst = 0; burst < param.rcv_count; burst++) {
        bluesync_timestamps_t *rcv = &param.rcv_history[burst];
        bluesync_timestamps_t *local = &param.local_history[burst];

        uint8_t burst_bitfield[NB_BYTES_BITFIELD] = {0};
        bitwise_and_bitfields(burst_bitfield, rcv, local, NB_BYTES_BITFIELD);

        for (int i = 0; i < SLOT_NUMBER; i++) {
            if (is_bit_set(burst_bitfield, i)) {
                sum_x += (double)local->timer_ticks[i];
                sum_y += (double)rcv->timer_ticks[i];
                n++;
            }
        }
    }

    if (n == 0) {
        LOG_ERR("No valid data in history for regression.");
        return BLUESYNC_NO_VALID_DATA_STATUS;
    } else if (n < min_nb_timestamp) {
        LOG_ERR("Not enough valid samples in history (min = %zu, got = %zu)", min_nb_timestamp, n);
        return BLUESYNC_NO_ENOUGH_DATA_STATUS;
    }

    double mean_x = sum_x / n;
    double mean_y = sum_y / n;

    double sum_cov = 0.0;
    double sum_var = 0.0;

    // Now second pass: accumulate covariance and variance
    for (int burst = 0; burst < param.rcv_count; burst++) {
        bluesync_timestamps_t *rcv = &param.rcv_history[burst];
        bluesync_timestamps_t *local = &param.local_history[burst];

        uint8_t burst_bitfield[NB_BYTES_BITFIELD] = {0};
        bitwise_and_bitfields(burst_bitfield, rcv, local, NB_BYTES_BITFIELD);

        for (int i = 0; i < SLOT_NUMBER; i++) {
            if (is_bit_set(burst_bitfield, i)) {
                double x = (double)local->timer_ticks[i] - mean_x;
                double y = (double)rcv->timer_ticks[i] - mean_y;
                sum_cov += x * y;
                sum_var += x * x;
            }
        }
    }

    if (fabs(sum_var) < 1e-12) {
        LOG_ERR("Variance too small → numerical instability (%e)", sum_var);
        return BLUESYNC_DENOMINATOR_TOO_SMALL;
    }

    *slope = sum_cov / sum_var;
    *offset = mean_y - (*slope * mean_x);

    return BLUESYNC_SUCCESS_STATUS;
}

static void bluesync_decode_msg(struct bluesync_msg_client *msg, struct net_buf_simple *buf){
	msg->client_timer_ticks = k_uptime_ticks();
	msg->rcv.round_id = net_buf_simple_pull_u8(buf);
	msg->rcv.index_timeslot = net_buf_simple_pull_u8(buf);
	msg->rcv.master_timer_ticks = net_buf_simple_pull_le64(buf);
#if defined(CONFIG_BLUESYNC_TEST_BABBLESIM_SUPPORT)
	msg->master_estimation_ticks = get_logical_time_ticks();
#endif
}

static void bluesync_store_current_burst(){
	k_mutex_lock(&param.mutex, K_FOREVER);
	{
		k_mutex_lock(&param.rcv_history_mutex, K_FOREVER);
		{
			param.rcv_history[param.rcv_head] = param.rcv;
			param.rcv_head = (param.rcv_head + 1) % BURST_WINDOWS_SIZE;
			if (param.rcv_count < BURST_WINDOWS_SIZE) {
				param.rcv_count++;
			}
		}
		k_mutex_unlock(&param.rcv_history_mutex);

		k_mutex_lock(&param.local_history_mutex, K_FOREVER);
		{
			param.local_history[param.local_head] = param.local;
			param.local_head = (param.local_head + 1) % BURST_WINDOWS_SIZE;
			if (param.local_count < BURST_WINDOWS_SIZE) {
				param.local_count++;
			}
		}
		k_mutex_unlock(&param.local_history_mutex);
	}
	k_mutex_unlock(&param.mutex);
}

static bluesync_status_t end_sync_timeslot_process() {
	LOG_DBG("method: %s", __func__);
	
#if defined(CONFIG_BLUESYNC_TEST_BABBLESIM_SUPPORT)
	statistic_bluesync_status(&param.rcv, &param.local, SLOT_NUMBER);
#endif

	// Save the burst
	bluesync_store_current_burst();

	double slope_ticks = 0.0;
	double offset_ticks = 0.0;

	bluesync_status_t err = 0;
	err = calculate_lr_from_history(&slope_ticks, &offset_ticks, SLOT_NUMBER/2);

	if (err != BLUESYNC_SUCCESS_STATUS){
		return err;
	}

	apply_timer_sync(slope_ticks, offset_ticks);

	k_mutex_lock(&param.mutex, K_FOREVER);
	{
		param.current_round_id = param.new_round_id;
	}
	k_mutex_unlock(&param.mutex);
	return BLUESYNC_SUCCESS_STATUS;
}

void drift_estimation_handler(struct k_timer *timer_id){
	k_sem_give(&bluesync_end_sync_sem);
}

static void bluesync_scan_packet_process(){
	struct bluesync_msg_client msg;
	int ret = k_msgq_get(&bluesync_rx_msgq, &msg, K_FOREVER);
    if (ret != 0) {
		LOG_ERR("No message available (err: %d)", ret);
		return;
    }

	uint8_t current_round_id = msg.rcv.round_id;
	uint8_t current_timeslot_idx = msg.rcv.index_timeslot;

	bs_sm_state_t current_state = bs_state_machine_get_state();

	if (current_state == BS_SCAN_WAIT_FOR_SYNC)
	{
		if (current_round_id == param.current_round_id){
			return;	// already synchronized to this round
		}
		k_mutex_lock(&param.mutex, K_FOREVER);
		{
			param.new_round_id = current_round_id;
		}
		k_mutex_unlock(&param.mutex);

		int remaining_slots = BLUESYNC_TIMESTAMP_ARRAY_SIZE + 1 - current_timeslot_idx;
		k_timer_start(&param.drift_estimation_timer, K_MSEC(remaining_slots * CONFIG_BLUESYNC_ADV_INT_MS), K_NO_WAIT);

		bs_state_machine_run(EVENT_NEW_SYNC_RCV);

	}
	
	if(current_timeslot_idx > SLOT_NUMBER){
		LOG_ERR("Index too large: %u (max %u)", current_timeslot_idx, SLOT_NUMBER);
		return;
	}

	// add timestamp to local set if index is between the range
	if(current_timeslot_idx >= 0  && current_timeslot_idx < SLOT_NUMBER){
		k_mutex_lock(&param.local_mutex, K_FOREVER);
		{
			add_bluesync_timestamps(&param.local 
									, current_timeslot_idx 
									, msg.client_timer_ticks
#if defined(CONFIG_BLUESYNC_TEST_BABBLESIM_SUPPORT)
									, msg.master_estimation_ticks
#endif
									);
		}
		k_mutex_unlock(&param.local_mutex);
	}

	// add timestamp to master set if index is between the range
	if(current_timeslot_idx >= 1  && current_timeslot_idx <= SLOT_NUMBER){
		k_mutex_lock(&param.rcv_mutex, K_FOREVER);
		{
			add_bluesync_timestamps(&param.rcv
									, current_timeslot_idx-1
									, msg.rcv.master_timer_ticks
#if defined(CONFIG_BLUESYNC_TEST_BABBLESIM_SUPPORT)
									, 0
#endif														
									);
		}
		k_mutex_unlock(&param.rcv_mutex);
	}
}

static uint8_t bt_packet_buf[sizeof(uint16_t) + sizeof(struct bluesync_msg)] = {0};

static bluesync_status_t bluesync_encode_msg(struct bt_data *bt_pkt, uint8_t current_round_id, 
	uint8_t current_timeslot_idx, 
	int64_t time_ticks){

	memset(bt_packet_buf, 0, sizeof(bt_packet_buf));

	struct bluesync_msg msg = {
		.round_id = current_round_id,
		.index_timeslot = current_timeslot_idx,
		.master_timer_ticks = time_ticks
	};

	uint16_t manufacturer_id = MY_MANUFACTURER_ID;
    
    // Copy Manufacturer ID (Little Endian format)
    bt_packet_buf[0] = manufacturer_id & 0xFF;
    bt_packet_buf[1] = (manufacturer_id >> 8) & 0xFF;
    
    // Copy the struct data into the buffer
    memcpy(&bt_packet_buf[2], &msg, sizeof(struct bluesync_msg));

	bt_pkt[0].type = BT_DATA_FLAGS;
	bt_pkt[0].data_len = 1;
	bt_pkt[0].data = (uint8_t []) { BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR };

	bt_pkt[1].type = BT_DATA_MANUFACTURER_DATA;
	bt_pkt[1].data_len = sizeof(bt_packet_buf);
	bt_pkt[1].data = bt_packet_buf;

	return BLUESYNC_SUCCESS_STATUS;
}

static void bluesync_send_adv(){
	struct bt_data bt_packet[2];

	if (param.timeslot_index == 0){
		bluesync_encode_msg(bt_packet, param.current_round_id, param.timeslot_index, 0);
	} 
	else {
		bluesync_encode_msg(bt_packet, param.current_round_id, param.timeslot_index,
							param.local.timer_ticks[param.timeslot_index-1]);
	}

	int err = bt_le_ext_adv_set_data(param.adv, bt_packet, ARRAY_SIZE(bt_packet), NULL, 0);
    if (err) {
        LOG_ERR("Failed to set advertising data (err %d)", err);
        return;
    }

	struct bt_le_ext_adv_start_param start = {
		.num_events = 1,
	};

#if defined(CONFIG_BLUESYNC_USED_IN_MESH)
	bt_mesh_scan_disable();
#endif
	
	err = bt_le_ext_adv_start(param.adv, &start);
    if (err) {
        LOG_ERR("Failed to start extended advertising (err %d)", err);
        return;
    }
}

// SCANNING PART ********************************************

void scan_cb(const bt_addr_le_t *addr, int8_t rssi, struct net_buf_simple *buf){

	// Check if there is enough data in the buffer
	if (buf->len < 2) {
		LOG_ERR("Error: Not enough data for extended advertising");
		return;
	}
	// Parse length and type fields
	// Parse AD_ELEMENT_FLAG
	uint8_t len_ad_flag = net_buf_simple_pull_u8(buf);
	if (len_ad_flag > 0){
		net_buf_simple_pull_mem(buf, len_ad_flag);
	}
	// Parse AD_ELEMENT_MANUFACTURER_SPECIFIC
	uint8_t len = net_buf_simple_pull_u8(buf);
	if (len > buf->len) {
		LOG_ERR("Error: Buffer length mismatch");
		return;
	}
	uint8_t type = net_buf_simple_pull_u8(buf);
	if (type == BT_DATA_MANUFACTURER_DATA){
		// get manufacturer id
		net_buf_simple_pull_be16(buf);
		// Verify that the payload length matches the expected size
        if (len - 3 != sizeof(struct bluesync_msg)) {
            LOG_ERR("Error: Unexpected manufacturer data size");
            return;
        }

        struct bluesync_msg_client msg;

		bluesync_decode_msg(&msg, buf);

		int ret = k_msgq_put(&bluesync_rx_msgq, &msg, K_NO_WAIT);

		if (ret != 0 ) {
			LOG_ERR("Failed to put message in msgq");
		}

	} else {
		LOG_ERR("Error: Unsupported advertising data type (0x%02X)", type);
	}
}

#if !defined(CONFIG_BLUESYNC_USED_IN_MESH)
static void bt_scan_cb(const bt_addr_le_t *addr, int8_t rssi,
	uint8_t adv_type, struct net_buf_simple *buf){
	if (adv_type == BT_GAP_ADV_TYPE_EXT_ADV){
		scan_cb(addr, rssi, buf);
	}
}
#endif

static void bluesync_scan_start(){
#if defined(CONFIG_BLUESYNC_USED_IN_MESH)
	bt_mesh_register_aux_scan_cb(BT_GAP_ADV_TYPE_EXT_ADV, &scan_cb);
#else
	int err = bt_le_scan_start(&param.scan_param, &bt_scan_cb);
    if (err) {
        LOG_ERR("Failed to start scanning (err %d)", err);
    } else {
        LOG_DBG("Scanning started.");
    }
#endif
}

static void bluesync_scan_stop(){
#if defined(CONFIG_BLUESYNC_USED_IN_MESH)
	bt_mesh_unregister_aux_scan_cb();
#else
	int err = bt_le_scan_stop();
    if (err == -EALREADY) {
        LOG_ERR("Scanning was not active.");
    } else if (err) {
        LOG_ERR("Failed to stop scanning (err %d)", err);
    } else {
        LOG_DBG("Scanning stopped.");
    }
#endif
}

// ADVERTISING PART ******************************************

static void bluesync_adv_process() {
	for (int i = 0; i <= SLOT_NUMBER; i++){
		bluesync_send_adv();
		k_sleep(K_MSEC(CONFIG_BLUESYNC_ADV_INT_MS));
	}
}

void adv_sent_cb(struct bt_le_ext_adv *adv, struct bt_le_ext_adv_sent_info *info)
{
	k_mutex_lock(&param.mutex, K_FOREVER);
	{
		if (info->num_sent >= 1) {
			if( param.timeslot_index < SLOT_NUMBER){
				param.local.timer_ticks[param.timeslot_index] = get_logical_time_ticks();
			}
			param.timeslot_index++;
		}
	}
	k_mutex_unlock(&param.mutex);
#if defined(CONFIG_BLUESYNC_USED_IN_MESH)
	bt_mesh_scan_enable();
#endif
	
}

static struct bt_le_ext_adv_cb bt_callbacks = {
	.sent = adv_sent_cb,
};

static void bluesync_init_adv(){
	int err = bt_le_ext_adv_create(&param.adv_param, &bt_callbacks, &param.adv);
	if (err) {
		LOG_ERR("Failed to create extended advertising set (err %d)", err);
		return;
	}
}

// BLUESYNC_STATE_MACHINE CALLBACKS *********************************************

void bs_scan_wait_for_sync_handler(void){
	LOG_DBG("method: %s",__func__);
	bluesync_reset_param();

	bluesync_scan_start();
}

void bs_sync_handler(){
	LOG_DBG("method: %s",__func__);
}

void bs_update_handler(void){
	LOG_DBG("method: %s",__func__);
	bluesync_scan_stop();
	bluesync_status_t status = end_sync_timeslot_process();

	if(status != BLUESYNC_SUCCESS_STATUS){
		bs_state_machine_run(EVENT_UPDATE_FAILED);
		return;
	}

	bs_state_machine_run(EVENT_UPDATE_SUCCESS);
}

void bs_adv_handler(void){
	LOG_DBG("method: %s",__func__);

	bluesync_reset_param();
	bluesync_adv_process();
	bs_state_machine_run(EVENT_ADV_EXPIRED);
}

void bs_stop_handler(void){
	LOG_DBG("method: %s",__func__);
}

struct bs_sm_handlers handlers ={
	.bs_scan_wait_for_sync_cb = bs_scan_wait_for_sync_handler,
	.bs_sync_cb = bs_sync_handler,
	.bs_update_cb = bs_update_handler,
	.bs_adv_cb = bs_adv_handler,
	.bs_stop_cb = bs_stop_handler,
};


// BLUESYNC THREAD *********************************

void bluesync_thread_fnt(void *arg1, void *arg2, void *arg3)
{
    // Define the event array
    struct k_poll_event events[] = {
		K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SEM_AVAILABLE,
								 K_POLL_MODE_NOTIFY_ONLY, 
								 &bluesync_start_new_sync_sem),
		K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_MSGQ_DATA_AVAILABLE, 
								 K_POLL_MODE_NOTIFY_ONLY, 
								 &bluesync_rx_msgq),
		K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SEM_AVAILABLE, 
								 K_POLL_MODE_NOTIFY_ONLY, 
								 &bluesync_end_sync_sem),
	};

	bs_state_machine_init(&handlers);
	bluesync_init_adv();
	k_timer_init(&param.drift_estimation_timer, drift_estimation_handler, NULL);

	k_sem_take(&bluesync_role_assign_sem, K_FOREVER);
	bs_state_machine_run(EVENT_INIT);
    
    while (1)
    {
		k_poll(events, ARRAY_SIZE(events), K_FOREVER);

		if(events[0].state == K_POLL_STATE_SEM_AVAILABLE){
			k_sem_take(&bluesync_start_new_sync_sem, K_NO_WAIT);
			bs_state_machine_run(EVENT_NEW_NET_SYNC);
		}
		
		if(events[1].state == K_POLL_STATE_MSGQ_DATA_AVAILABLE){
			bluesync_scan_packet_process();
		}

		if(events[2].state == K_POLL_STATE_SEM_AVAILABLE){
			k_sem_take(&bluesync_end_sync_sem, K_NO_WAIT);
			bs_state_machine_run(EVENT_SYNC_EXPIRED);
		}

		// clear events
		events[0].state = K_POLL_STATE_NOT_READY;
        events[1].state = K_POLL_STATE_NOT_READY;
        events[2].state = K_POLL_STATE_NOT_READY;
    }
}

// PUBLIC ******************************************

void bluesync_init(){
	LOG_DBG("bluesync init");

	k_tid_t thread_id = k_thread_create(&param.bluesync_thread, bluesync_thread_stack,
                                      K_THREAD_STACK_SIZEOF(bluesync_thread_stack),
                                      bluesync_thread_fnt, &param, NULL, NULL,
                                      CONFIG_BLUESYNC_THREAD_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(thread_id, "bluesync_thread");
}

void bluesync_set_role(bluesync_role_t role){
	if (role != BLUESYNC_NONE_ROLE) {
		bs_state_machine_set_role(role);
		k_sem_give(&bluesync_role_assign_sem);
	}
	else {
		LOG_ERR("Wrong type given");
	}
}


void bluesync_start_net_sync(){
	if(bs_state_machine_get_role() == BLUESYNC_AUTHORITY_ROLE){
		k_mutex_lock(&param.mutex, K_FOREVER);
		{
			param.current_round_id++;
		}
		k_mutex_unlock(&param.mutex);
		k_sem_give(&bluesync_start_new_sync_sem);
	}
}

void bluesync_start_net_sync_with_unix_epoch_us(uint64_t unix_epoch_us){
	if(bs_state_machine_get_role() == BLUESYNC_AUTHORITY_ROLE){
		//set the new reference unix_epoch
		set_new_epoch_unix_ref(unix_epoch_us);

		//start a new sync in the network
		bluesync_start_net_sync();
	}
}