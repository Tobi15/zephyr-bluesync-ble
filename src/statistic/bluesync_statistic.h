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
 * File: bluesync_statistic.h
 * Description: File implementing the required method to have statistic (interface)
 *
 * Project: BlueSync - BLE Time Sync for Zephyr
 * Repository: https://github.com/Tobi15/zephyr-bluesync-ble
 */

#ifndef ZEPHYR_BLUESYNC_SRC_STATISTIC_BLUESYNC_STATISTIC_H_
#define ZEPHYR_BLUESYNC_SRC_STATISTIC_BLUESYNC_STATISTIC_H_

#include <stdint.h>

#include "../bluesync.h"

/**
 * @brief Analyze and log synchronization statistics based on a series of timestamps.
 *
 * This function stores master and slave timestamp pairs into a CSV file.
 *
 * @param elem_master Pointer to the array of master timestamps.
 * @param elem_slave  Pointer to the array of local timestamps captured by the slave.
 * @param size        Number of timestamp pairs to process.
 */
void statistic_bluesync_status(bluesync_timestamps_t *elem_master, bluesync_timestamps_t *elem_slave, size_t size);

/**
 * @brief Initialize the BlueSync statistics module.
 *
 * This should be called once at system startup or before beginning a series
 * of synchronization evaluations. It opens needed files.
 */
void bluesync_statistic_init();

/**
 * @brief Deinitialize the BlueSync statistics module.
 *
 * This function cleans up and close files
 */
void  bluesync_statistic_deinit();


#endif /* ZEPHYR_BLUESYNC_SRC_STATISTIC_BLUESYNC_STATISTIC_H_*/