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
 * File: local_time.c
 * Description: Private API for managing the local time
 *
 * Project: BlueSync - BLE Time Sync for Zephyr
 * Repository: https://github.com/Tobi15/zephyr-bluesync-ble
 */

#include <zephyr/kernel.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define BLUESYNC_TICK_RATE_HZ 32768

#if defined(CONFIG_BLUESYNC_TEST_BABBLESIM_SUPPORT)
// BabbleSim or POSIX simulation environment
#define us_to_ticks(us)        ((uint64_t)(((double)(us)) * BLUESYNC_TICK_RATE_HZ / 1e6 + 0.5))
#define ticks_to_us(ticks)     ((uint64_t)(((double)(ticks)) * 1e6 / BLUESYNC_TICK_RATE_HZ))
#else
// Real hardware (use Zephyr-provided tick macros)
#define us_to_ticks(us)        ((uint64_t)(((double)(us)) * 32768.0 / 1e6 + 0.5))
#define ticks_to_us(ticks)     ((uint64_t)(((double)(ticks)) * 1e6 / 32768.0))
#endif


#define LOCAL_FREQ_HZ BLUESYNC_TICK_RATE_HZ

struct local_time {
	uint64_t uptime_ref_ticks;
	uint64_t epoch_ref_ticks;
	uint64_t epoch_ref_us;
	bool epoch_ref_valid;
	double curent_offset_ticks; // Offset correction factor
	double curent_slope_ticks;  // Drift correction factor
	struct k_mutex mutex;
};

static struct local_time local = {
	.uptime_ref_ticks = 0,
	.epoch_ref_ticks = 0,
	.epoch_ref_valid = false,
	.curent_offset_ticks = 0.0,
	.curent_slope_ticks = 1.0,
	.mutex = Z_MUTEX_INITIALIZER(local.mutex),
};

uint64_t get_logical_time_ticks_(int64_t uptime_ticks) {
    uint64_t delta_ticks;
    double slope, offset;

    k_mutex_lock(&local.mutex, K_FOREVER);
    {
        if (uptime_ticks == -1) {
            delta_ticks = k_uptime_ticks() - local.uptime_ref_ticks;
        } else {
            delta_ticks = (uint64_t)uptime_ticks - local.uptime_ref_ticks;
        }

        slope = local.curent_slope_ticks;
        offset = local.curent_offset_ticks;
    }
    k_mutex_unlock(&local.mutex);

    double corrected = (double)delta_ticks * slope + offset;

    if (local.epoch_ref_valid) {
        corrected += local.epoch_ref_ticks;
    }

    return (uint64_t)round(corrected);
}

uint64_t convert_uptime_ticks_to_est_master_ticks(int64_t uptime_ticks) {
	return get_logical_time_ticks_(uptime_ticks);
}

uint64_t get_logical_time_us(void) {
	uint64_t logical_ticks = get_logical_time_ticks_(-1);
	return ticks_to_us(logical_ticks);
}

uint64_t get_logical_time_ticks() {
	return get_logical_time_ticks_(-1);
}

void apply_timer_sync(double new_slope, double new_offset) {

	k_mutex_lock(&local.mutex, K_FOREVER);
	{
		local.curent_slope_ticks = new_slope;
		local.curent_offset_ticks = new_offset;
	}
    k_mutex_unlock(&local.mutex);
}


void set_new_epoch_unix_ref(uint64_t epoch_ref_us){
	uint64_t epoch_ref_ticks = us_to_ticks(epoch_ref_us);

	k_mutex_lock(&local.mutex, K_FOREVER);
	{
		local.uptime_ref_ticks = k_uptime_ticks();
		local.epoch_ref_ticks = epoch_ref_ticks;
		local.epoch_ref_us = epoch_ref_us;
		local.epoch_ref_valid = true;
	}
	k_mutex_unlock(&local.mutex);
}


uint64_t ticks_to_us_unix_time(uint64_t logical_tick) {
	k_mutex_lock(&local.mutex, K_FOREVER);


	int64_t delta_ticks = (int64_t)logical_tick - (int64_t)local.epoch_ref_ticks;
	int64_t delta_us = (int64_t)ticks_to_us(delta_ticks);
	uint64_t result = local.epoch_ref_us + delta_us;

	k_mutex_unlock(&local.mutex);
	return result;
}

uint64_t get_current_unix_time_us(void) {
	return ticks_to_us_unix_time(get_logical_time_ticks());
}


int64_t get_uptime_ticks_with_epoch(){
	int64_t curent_uptime_ticks =  k_uptime_ticks();
	uint64_t ticks = 0;

	k_mutex_lock(&local.mutex, K_FOREVER);
	{
		ticks = local.epoch_ref_ticks + (curent_uptime_ticks - local.uptime_ref_ticks);
	}
	k_mutex_unlock(&local.mutex);
	return ticks;
}

double get_current_slope_ticks(){
	return local.curent_slope_ticks;
}

double get_current_offset_ticks(){
	return local.curent_offset_ticks;
}

uint64_t uncompress_time(uint32_t compress_32bit_timestamp){
	uint64_t current_time_us = get_logical_time_us();

	uint64_t uncompressed_timestamp = (current_time_us & 0xFFFFFFFF00000000ULL) | 
									  ((uint64_t) compress_32bit_timestamp & 0x00000000FFFFFFFFULL);


	if (uncompressed_timestamp > current_time_us){
		uncompressed_timestamp -= 0x100000000ULL;
	}

	return uncompressed_timestamp;
}