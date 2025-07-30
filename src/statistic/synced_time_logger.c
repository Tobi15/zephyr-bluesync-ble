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
#include "synced_time_logger.h"

#include <zephyr/kernel.h>

#include "posix_native_task.h"
#include "bsim_args_runner.h"

#include <stdio.h>
#include <string.h>

#include <zephyr/logging/log.h>

#include <bluesync/bluesync.h>


LOG_MODULE_REGISTER(synced_time_logger, CONFIG_APP_LOG_LEVEL);

#define SEPARATION_TOKEN ";"

#define TICKS_PER_SECOND 32768
struct k_work my_work;
static int64_t val_ref = 0;

struct msg_statistic {
	int64_t val;
	uint64_t timestamp;
};

void synced_time_logger_new_msg(struct msg_statistic *msg);

void my_work_handler(struct k_work *work)
{
	struct msg_statistic msg = {
		.timestamp = get_current_unix_time_us(),
		.val = val_ref++,
	};

	synced_time_logger_new_msg(&msg);
}

void timer_handler(struct k_timer *timer_id)
{
	k_work_submit(&my_work);
}

K_TIMER_DEFINE(aligned_timer, timer_handler, NULL);

void init_sync_overall_timer(void)
{
	k_work_init(&my_work, my_work_handler);

	uint64_t now_ticks = k_uptime_ticks();

	const uint64_t TICKS_PER_20SEC = TICKS_PER_SECOND * 20;

	// Calculate ticks to wait until the next aligned 1-second boundary
	uint64_t remainder = now_ticks % TICKS_PER_20SEC;
	uint64_t ticks_until_next = (remainder == 0) ? 0 : (TICKS_PER_20SEC - remainder);

	LOG_DBG("Now ticks: %llu", now_ticks);
	LOG_DBG("First timer in %llu ticks (aligned to 1s)", ticks_until_next);

	// Start periodic timer: delay to next aligned second, then every second
	k_timer_start(&aligned_timer, K_TICKS(ticks_until_next), K_TICKS(TICKS_PER_SECOND));
}

struct ble_node_stat {
	struct k_mutex mutex;
	FILE *file;
};

static struct ble_node_stat node;

static FILE *open_stat(char *filename, uint32_t device_number)
{
	FILE *fp;
	static char path[250];
	memset(path, 0, sizeof(path));

	snprintf(path, sizeof(path) - 1,
		 "%s%s_%i.csv",CONFIG_BLUESYNC_TEST_BABBLESIM_PATH,
		 filename, device_number);
	LOG_DBG("path : %s\n", path);
	fp = fopen(path, "w");
	__ASSERT(fp != NULL, "Cannot open file");

	return fp;
}

void synced_time_logger_init()
{
	uint32_t device_number = bsim_args_get_global_device_nbr();

	node.file = open_stat("node", device_number);

	fprintf(node.file, "val" SEPARATION_TOKEN "timestamp"
			   "\n");

	init_sync_overall_timer();
}

void synced_time_logger_new_msg(struct msg_statistic *msg)
{
	k_mutex_lock(&node.mutex, K_FOREVER);
	{
		fprintf(node.file,
			"%lld" SEPARATION_TOKEN "%llu"
			"\n",
			msg->val, msg->timestamp);
	}
	k_mutex_unlock(&node.mutex);
}

void synced_time_logger_deinit()
{
	fclose(node.file);
}
