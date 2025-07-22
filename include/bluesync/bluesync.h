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
 * Description: Public API for the BlueSync time synchronization module.
 *
 * Project: BlueSync - BLE Time Sync for Zephyr
 * Repository: https://github.com/Tobi15/zephyr-bluesync-ble
 */


#ifndef BLUESYNC_H
#define BLUESYNC_H

#include <zephyr/kernel.h>

typedef enum {
	BLUESYNC_NONE_ROLE = 0,
	BLUESYNC_AUTHORITY_ROLE = 1,
	BLUESYNC_CLIENT_ROLE = 2
} bluesync_role_t;


/**
 * @brief Initializes the BlueSync time synchronization module.
 *
 * This function sets up internal data structures, mutexes, and state
 * variables required for BlueSync to operate. It should be called once
 * at startup before any synchronization or logical time queries are made.
 */
void bluesync_init();

/**
 * @brief Sets the operational role of the BlueSync node.
 *
 * This function configures the role of the device within the BlueSync
 * synchronization system. Roles define whether the device acts as a time authority
 * or as a time client within the network.
 *
 * A **time authority** is the unique source of reference time and distributes it
 * throughout the network. A **time client** receives synchronization from the authority
 * or other clients and can forward the synchronized time further once it is synchronized.
 *
 * @param role The role to assign to the device. Must be one of the values defined in
 *             the @ref bluesync_role_t enumeration:
 *             - BLUESYNC_AUTHORITY_ROLE
 *             - BLUESYNC_CLIENT_ROLE
 */
void bluesync_set_role(bluesync_role_t role);

/**
 * @brief Initiates a new BlueSync synchronization round as the time authority.
 *
 * This function is called by the node acting as the time authority to start
 * a network-wide synchronization round. It broadcasts timing information to
 * all clients, allowing them to synchronize their local clocks based on the
 * authority's reference time.
 *
 * This function should only be called by devices assigned the
 * @ref BLUESYNC_AUTHORITY_ROLE.
 */
void bluesync_start_net_sync();

/**
 * @brief Initiates a new BlueSync synchronization round as the time authority.
 *
 * This function is called by the node acting as the time authority to start
 * a network-wide synchronization round. It broadcasts timing information to
 * all clients, allowing them to synchronize their local clocks based on the
 * authority's reference time.
 *
 * This function should only be called by devices assigned the
 * @ref BLUESYNC_AUTHORITY_ROLE.
 * @param unix_epoch_ms : unix epoch reference in us
 */
void bluesync_start_net_sync_with_unix_epoch_us(uint64_t unix_epoch_us);

/**
 * @brief Get the current time which is based on the UNIX reference. 
 * 
 * @return return unix timestamp in microsecond in uint64_t
 */
uint64_t get_current_unix_time_us(void);

#endif /* BLUESYNC_H */
