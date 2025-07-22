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
 * File: local_time.h
 * Description: Private API for managing the local time
 *
 * Project: BlueSync - BLE Time Sync for Zephyr
 * Repository: https://github.com/Tobi15/zephyr-bluesync-ble
 */


 
#ifndef ZEPHYR_BLUESYNC_SRC_LOCAL_TIME_H_
#define ZEPHYR_BLUESYNC_SRC_LOCAL_TIME_H_
#include <stdint.h>

/**
 * @brief Get the logical time us value. 
 * This is the corrected us value after synchronisation.
 * This represents the unix epoch timestamp in us of 
 * the overall mesh network. Basically, it converts logical ticks
 * into us.
 * 
 * @return uint64_t 
 */
uint64_t get_logical_time_us(void);

/**
 * @brief Get the logical time ticks value.
 * This is the corrected ticks value after synchronisation.
 * This represents the unix epoch timestamp in ticks of 
 * the overall mesh network.
 * 
 * @return uint64_t 
 */
uint64_t get_logical_time_ticks(void);

/**
 * @brief Convert a ticks value into an estimation in ticks of 
 * the overall unix epoch timestamp. 
 * 
 * @param uptime_ticks : value that should be concerted 
 * @return uint64_t 
 */
uint64_t convert_uptime_ticks_to_est_master_ticks(uint64_t uptime_ticks);

/**
 * @brief Get the current slope ticks value
 * 
 * @return double 
 */
double get_current_slope_ticks();

/**
 * @brief Get the current offset ticks value
 * 
 * @return double 
 */
double get_current_offset_ticks();

/**
 * @brief Get the uptime ticks with epoch value.
 * This method concerns only the authority Role. 
 * This method will return the unix epoch timestamp
 * based on the elapsed time (uptime - ref) and add
 * the overall reference.
 * 
 * @return int64_t 
 */
int64_t get_uptime_ticks_with_epoch();

/**
 * @brief Set the new epoch unix ref value.
 * Before the authority node starts a new network synchronisation,
 * it should update the epoch reference.
 * 
 * @param epoch_ref_ms 
 */
void set_new_epoch_unix_ref(uint64_t epoch_ref_us);

/**
 * @brief Apply the synchronisation parameters. Once the LR is made, 
 * The result of it gives a drift value (slope) and the difference between
 * itself and the reference (offset). This method permits to set these 
 * parameters in order to have the proper correction when getting timestamps 
 * values. 
 * 
 * @param slope_timer 
 * @param offset_timer_us 
 */
void apply_timer_sync(double slope_timer, double offset_timer_us);

/**
 * @brief Convert a uint32_t timestamps into a uint64_t values. 
 * This should be used in the sink node. In the Mesh network, pakcet
 * payload is limited. Therefore only uint32_t timestamp can be used. 
 * When the sink received this kind of value, it can reconstruct the 
 * correct timestamps value. 
 * 
 * @param compress_32bit_timestamp 
 * @return uint64_t 
 */
uint64_t uncompress_time(uint32_t compress_32bit_timestamp);

#endif /* ZEPHYR_BLUESYNC_SRC_LOCAL_TIME_H_ */