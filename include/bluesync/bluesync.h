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
 * Project: BlueSync - BLE Time Sync for Zephyr
 * Repository: https://github.com/Tobi15/zephyr-bluesync-ble
 */

/**
 * @file bluesync.h
 * @brief Public API for the BlueSync time synchronization module.
 *
 * This file defines the core API functions for initializing and managing
 * time synchronization using the BlueSync protocol in a BLE Mesh environment.
 */

/**
 * @defgroup bluesync_api BlueSync Public API
 * @brief Functions and definitions to use the BlueSync time synchronization system.
 *
 * This group contains the public interface of the BlueSync module.
 * @{
 */

#ifndef BLUESYNC_H
#define BLUESYNC_H

#include <zephyr/kernel.h>

/**
 * @brief Defines the operational roles for a BlueSync node.
 */
typedef enum {
	BLUESYNC_NONE_ROLE = 0,        /**< No role assigned. */
	BLUESYNC_AUTHORITY_ROLE = 1,   /**< Acts as the time authority (reference clock). */
	BLUESYNC_CLIENT_ROLE = 2       /**< Acts as a time-synchronized client. */
} bluesync_role_t;

/**
 * @brief Initializes the BlueSync time synchronization module.
 *
 * This function sets up internal data structures, mutexes, and state
 * variables required for BlueSync to operate. It should be called once
 * at startup before any synchronization or logical time queries are made.
 */
void bluesync_init(void);

/**
 * @brief Sets the operational role of the BlueSync node.
 *
 * This function configures the role of the device within the BlueSync
 * synchronization system. Roles define whether the device acts as a time authority
 * or as a client.
 *
 * @param role Role to assign to the device.
 *             Must be one of @ref bluesync_role_t:
 *             - BLUESYNC_AUTHORITY_ROLE
 *             - BLUESYNC_CLIENT_ROLE
 */
void bluesync_set_role(bluesync_role_t role);

/**
 * @brief Starts a BlueSync synchronization round as the time authority.
 *
 * This should be called by the device with the BLUESYNC_AUTHORITY_ROLE to
 * broadcast synchronization data to clients.
 */
void bluesync_start_net_sync(void);

/**
 * @brief Starts a synchronization round using a known UNIX epoch time.
 *
 * This function is used by the authority node to synchronize the network based on
 * an external UNIX time reference.
 *
 * @param unix_epoch_us UNIX epoch timestamp in microseconds.
 */
void bluesync_start_net_sync_with_unix_epoch_us(uint64_t unix_epoch_us);

/**
 * @brief Gets the current synchronized UNIX time.
 *
 * This returns the current time as synchronized with the authority node,
 * expressed in microseconds since the UNIX epoch.
 *
 * @return Current synchronized UNIX timestamp in microseconds.
 */
uint64_t get_current_unix_time_us(void);

#endif /* BLUESYNC_H */

/** @} */ // end of bluesync_api
