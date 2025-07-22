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
 * File: bs_state_machine.c
 * Description: Implementation of the state machine for Bluesync
 *
 * Project: BlueSync - BLE Time Sync for Zephyr
 * Repository: https://github.com/Tobi15/zephyr-bluesync-ble
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <bluesync/bluesync.h>
#include "bluesync.h"
#include "bs_state_machine.h"




LOG_MODULE_REGISTER(bs_state_machine, CONFIG_BLUESYNC_LOG_LEVEL);

struct bs_sm_param {
	bluesync_role_t role;
	bs_sm_state_t current_state;
	struct bs_sm_handlers *handlers;
	struct k_mutex mutex;
};

static struct bs_sm_param sm_param = {
	.role = BLUESYNC_NONE_ROLE,
	.current_state = BS_NONE_STATE,
	.handlers = NULL,
	.mutex = Z_MUTEX_INITIALIZER(sm_param.mutex)
};

bs_sm_state_t transition(bluesync_role_t role, bs_sm_state_t current_state, bs_sm_event_t event) {
    switch (current_state) {
		case BS_SCAN_WAIT_FOR_SYNC:
			if (event == EVENT_NEW_SYNC_RCV){
				LOG_DBG("Transition: BS_SCAN_WAIT_FOR_SYNC -> BS_SYNC");
                return BS_SYNC;
			}
			break;
		case BS_SYNC:
			if (event == EVENT_SYNC_EXPIRED){
				LOG_DBG("Transition: BS_SYNC -> BS_UPDATE");
                return BS_UPDATE;
			}
			break;
		case BS_UPDATE:
			if (event == EVENT_UPDATE_SUCCESS){
				LOG_DBG("Transition: BS_UPDATE -> BS_ADV");
                return BS_ADV;
			}
			else if (event == EVENT_UPDATE_FAILED){
				LOG_DBG("Transition: BS_UPDATE -> BS_SCAN_WAIT_FOR_SYNC");
                return BS_SCAN_WAIT_FOR_SYNC;
			}
			break;
		case BS_ADV:
			if (event == EVENT_ADV_EXPIRED){
				if (role == BLUESYNC_AUTHORITY_ROLE){
					LOG_DBG("Transition: BS_ADV -> BS_STOP");
                	return BS_STOP;
				}
				else if (role == BLUESYNC_CLIENT_ROLE){
					LOG_DBG("Transition: BS_ADV -> BS_WAIT_SYNC");
                	return BS_SCAN_WAIT_FOR_SYNC;
				}
			}
			break;
		case BS_STOP:
			if (event == EVENT_NEW_NET_SYNC){
				LOG_DBG("Transition: BS_STOP -> BS_ADV");
                return BS_ADV;
			}
			break; 
		default:
			if (event == EVENT_INIT){
				if (role == BLUESYNC_AUTHORITY_ROLE){
					LOG_DBG("Transition: INIT -> BS_STOP");
                	return BS_STOP;
				}
				else if (role == BLUESYNC_CLIENT_ROLE){
					LOG_DBG("Transition: INIT -> BS_WAIT_SYNC");
                	return BS_SCAN_WAIT_FOR_SYNC;
				}
			}
			break;
    }

    return current_state; // No state change
}

void bs_state_machine_run(bs_sm_event_t event) {
	if (sm_param.handlers == NULL){
		LOG_ERR("Error: no handlers set...");
		return;
	}

	k_mutex_lock(&sm_param.mutex, K_FOREVER);
	{
		sm_param.current_state = transition(sm_param.role, sm_param.current_state, event);
	}
	k_mutex_unlock(&sm_param.mutex);
     

    // Call the appropriate handler for the current state
    switch (sm_param.current_state) {
		case BS_SCAN_WAIT_FOR_SYNC:
			if (sm_param.handlers->bs_scan_wait_for_sync_cb){
				sm_param.handlers->bs_scan_wait_for_sync_cb();
			} 
            break;
		case BS_SYNC:
			if (sm_param.handlers->bs_sync_cb){
				sm_param.handlers->bs_sync_cb();
			}
            break;
		case BS_UPDATE:
			if (sm_param.handlers->bs_update_cb){
				sm_param.handlers->bs_update_cb();
			} 
            break;
		case BS_ADV:
			if (sm_param.handlers->bs_adv_cb){
				sm_param.handlers->bs_adv_cb();
			} 
            break;
        case BS_STOP:
			if (sm_param.handlers->bs_stop_cb){
				sm_param.handlers->bs_stop_cb();
			} 
            break;
        default:
            break;
    }
}

void bs_state_machine_init(struct bs_sm_handlers *handlers){
	k_mutex_lock(&sm_param.mutex, K_FOREVER);
	{
		sm_param.handlers = handlers;
	}
	k_mutex_unlock(&sm_param.mutex);
}

void bs_state_machine_set_role(bluesync_role_t role){
	LOG_DBG("set role %d",role );
	k_mutex_lock(&sm_param.mutex, K_FOREVER);
	{
		
		sm_param.role = role;
		switch (sm_param.role)
		{
		case BLUESYNC_AUTHORITY_ROLE:
			sm_param.current_state = BS_STOP;
			break;
		
		case BLUESYNC_CLIENT_ROLE:
			sm_param.current_state = BS_SCAN_WAIT_FOR_SYNC;
			break;

		default:
			sm_param.current_state = BS_NONE_STATE;
			break;
		}
	}
	k_mutex_unlock(&sm_param.mutex);
}

bluesync_role_t bs_state_machine_get_role(){
	return sm_param.role;
}

bs_sm_state_t bs_state_machine_get_state(){
	return sm_param.current_state;
}
