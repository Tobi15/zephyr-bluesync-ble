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
 * File: bs_state_machine.h
 * Description: Implementation of the state machine for Bluesync
 *
 * Project: BlueSync - BLE Time Sync for Zephyr
 * Repository: https://github.com/Tobi15/zephyr-bluesync-ble
 */


#ifndef ZEPHYR_BLUESYNC_SRC_BS_STATE_MACHINE_H_
#define ZEPHYR_BLUESYNC_SRC_BS_STATE_MACHINE_H_

#include "bluesync.h"

/**
 * @brief Type definition representing the states
 * 
 */
typedef enum {
	BS_NONE_STATE 			= 0,
	BS_SCAN_WAIT_FOR_SYNC 	= 1,
	BS_SYNC 				= 2,
	BS_UPDATE 				= 3,
	BS_ADV 					= 4,
	BS_STOP 				= 5,
	BS_COUNT 				= 6
} bs_sm_state_t;

/**
 * @brief Type definition representing the events
 * 
 */
typedef enum {
	EVENT_NONE 				= 0,
	EVENT_INIT 				= 1,
	EVENT_NEW_SYNC_RCV 		= 2,
    EVENT_SYNC_EXPIRED 		= 3,
	EVENT_UPDATE_SUCCESS 	= 4,
	EVENT_UPDATE_FAILED 	= 5,
	EVENT_ADV_EXPIRED 		= 6,
	EVENT_NEW_NET_SYNC 		= 7,
    EVENT_COUNT 			= 8
} bs_sm_event_t;

/**
 * @brief Struct defining the callbacks of the state machine
 * 
 */
struct bs_sm_handlers {
	/**
	 * @brief This callback is called when entering in wait_for_sync state
	 * 
	 */
	void (*bs_scan_wait_for_sync_cb)(void);

	/**
	 * @brief This callback is called when entering in sync state
	 * 
	 */
	void (*bs_sync_cb)();

	/**
	 * @brief This callback is called when entering in update state
	 * 
	 */
	void (*bs_update_cb)(void);

	/**
	 * @brief This callback is called when entering in adv state
	 * 
	 */
	void (*bs_adv_cb)(void);

	/**
	 * @brief This callback is called when entering in stop state
	 * 
	 */
	void (*bs_stop_cb)(void);
};

/**
 * @brief This struct is implemented in the bluesync.c file.
 * 
 */
extern struct bs_sm_handlers *bs_sm_handlers;

/**
 * @brief Annonce an event to the state machine
 * 
 * @param event 
 */
void bs_state_machine_run(bs_sm_event_t event);

/**
 * @brief Init the state machine by passing the list of callbacks
 * 
 * @param handlers 
 */
void bs_state_machine_init(struct bs_sm_handlers *handlers);

/**
 * @brief Indicate the role to the state machine. 
 * 
 * @param role 
 */
void bs_state_machine_set_role(bluesync_role_t role);

/**
 * @brief Get the node role
 * 
 * @return bluesync_role_t 
 */
bluesync_role_t bs_state_machine_get_role();

/**
 * @brief Get the current state
 * 
 * @return bs_sm_state_t 
 */
bs_sm_state_t bs_state_machine_get_state();

#endif /* ZEPHYR_BLUESYNC_SRC_BS_STATE_MACHINE_H_ */