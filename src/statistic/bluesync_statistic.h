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


void statistic_bluesync_status(bluesync_timestamps_t *elem_master, bluesync_timestamps_t *elem_slave, size_t size);

 /*
 * @brief Initialize the statistic for bluesync app
 *
 */
void bluesync_statistic_init();

/**
 * @brief Deinitialize the statistic for bluesync app
 *
 */
void  bluesync_statistic_deinit();


#endif /* ZEPHYR_BLUESYNC_SRC_STATISTIC_BLUESYNC_STATISTIC_H_*/