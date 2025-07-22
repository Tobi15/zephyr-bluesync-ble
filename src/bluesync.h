/*
 * Copyright 2025 Tobias Moullet
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * File: bluesync.h
 * Description: Private API for the BlueSync time synchronization module.
 *
 * Project: BlueSync - BLE Time Sync for Zephyr
 * Repository: https://github.com/Tobi15/zephyr-bluesync-ble
 */


#ifndef TIME_SYNC_BLUESYNC_H_
#define TIME_SYNC_BLUESYNC_H_

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>

#define SLOT_NUMBER CONFIG_BLUESYNC_SLOTS_IN_BURST
#define NB_BYTES_BITFIELD (SLOT_NUMBER + 7) / 8
#define BLUESYNC_TIMESTAMP_ARRAY_SIZE SLOT_NUMBER +1

#define BURST_WINDOWS_SIZE CONFIG_BLUESYNC_BURST_WINDOWS_SIZE

#define MY_MANUFACTURER_ID  0x1234

typedef enum {
    BLUESYNC_SUCCESS_STATUS = 0,
    BLUESYNC_NO_VALID_DATA_STATUS = -1,
	BLUESYNC_NO_ENOUGH_DATA_STATUS = -2,
	BLUESYNC_DENOMINATOR_TOO_SMALL = -3,
    BLUESYNC_TIMEOUT_STATUS = -4,
    BLUESYNC_INVALID_PARAM_STATUS = -5,
    BLUESYNC_BUSY_STATUS = -6,
	BLUESYNC_BUF_WRONG_SIZE = -7
} bluesync_status_t;


typedef struct {
	uint8_t bitfield[NB_BYTES_BITFIELD];
	uint64_t timer_ticks[SLOT_NUMBER];
	//uint64_t remote_est_ticks[SLOT_NUMBER];
} bluesync_timestamps_t;

struct bluesync_param { 
	// curent timeslot index
	uint8_t timeslot_index;

	//########### RCV ###############
	// curent rcv burst
	bluesync_timestamps_t rcv;
	struct k_mutex rcv_mutex;

	// Ring buffer for received bursts
    bluesync_timestamps_t rcv_history[BURST_WINDOWS_SIZE];
    uint8_t rcv_head;
    uint8_t rcv_count;
	struct k_mutex rcv_history_mutex;

	//########## LOCAL ###############
	// curent local burst
	bluesync_timestamps_t local;
	struct k_mutex local_mutex;

	// Ring buffer for local bursts
    bluesync_timestamps_t local_history[BURST_WINDOWS_SIZE];
    uint8_t local_head;
    uint8_t local_count;
	struct k_mutex local_history_mutex;

	//########## BLE #################
	struct bt_le_ext_adv *adv;
	struct bt_le_adv_param adv_param;
	struct bt_le_scan_param scan_param;

	struct k_work_delayable bluesync_adv_delayed_work;

	struct k_timer drift_estimation_timer;
	// Worker used to perform slave synchronisation
	struct k_work end_sync_timeslot_worker;

	// round id with which node is synchronized with
	uint8_t current_round_id;
	uint8_t new_round_id;

	struct k_mutex mutex;
	struct k_thread bluesync_thread;
};

/**
 * @brief Message structure used in BlueSync time synchronization exchanges.
 *
 * This structure represents a synchronization message exchanged between nodes
 * in the BlueSync protocol. It contains timing and coordination information
 * used during a synchronization round.
 */
struct bluesync_msg {
	uint8_t round_id;
	uint8_t index_timeslot;
	uint64_t master_timer_ticks;
}__packed;


/**
 * @brief Encapsulated BlueSync message received by the client during synchronization.
 *
 * This structure wraps the original message received from the master and adds
 * the client's own timing information. It is used by the client to
 * compute synchronization parameters (e.g., offset and drift) during a sync round.
 */
struct bluesync_msg_client {
	struct bluesync_msg rcv;
	uint64_t client_timer_ticks;
};

#endif /* TIME_SYNC_BLUESYNC_H_ */