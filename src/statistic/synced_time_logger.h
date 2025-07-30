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
 * File: synced_time_logger.h
 * Description: file that logs with bable sim a timestamp value every second
 *
 * Project: BlueSync - BLE Time Sync for Zephyr
 * Repository: https://github.com/Tobi15/zephyr-bluesync-ble
 */

#ifndef ZEPHYR_BLUESYNC_SRC_STATISTIC_SYNCED_TIME_LOGGER_H_
#define ZEPHYR_BLUESYNC_SRC_STATISTIC_SYNCED_TIME_LOGGER_H_

#include <stdint.h>



/**
 * @brief Initialize the timer that store regularly timestamp of the node
 *
 */
void synced_time_logger_init();

/**
 * @brief Deinitialize the timer that store regularly timestamp of the node
 *
 */
void synced_time_logger_deinit();

#endif /* ZEPHYR_BLUESYNC_SRC_STATISTIC_SYNCED_TIME_LOGGER_H_*/
