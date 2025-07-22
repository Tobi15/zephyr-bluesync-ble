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
 * File: bluesync_statistic_bsim.c
 * Description: File implementing the required method to use babblesim
 *
 * Project: BlueSync - BLE Time Sync for Zephyr
 * Repository: https://github.com/Tobi15/zephyr-bluesync-ble
 */

#include "bluesync_statistic.h"

#include <zephyr/kernel.h>

#include <posix_native_task.h>
//#include "posix_native_task.h"
#include <bsim_args_runner.h>
//#include "bsim_args_runner.h"

#include <stdio.h>
#include <string.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(bluesync_statistic_bsim, CONFIG_BLUESYNC_LOG_LEVEL);

#define SEPARATION_TOKEN ";"

struct node_stat {
	struct k_mutex mutex;
	FILE *file;
};

static struct node_stat node;

struct bluesync_msg_client_statistic {
	uint8_t idx;
	uint64_t master_timer_ticks;
	uint64_t client_timer_ticks;
	bool valid;
};

void statistic_bluesync_status(bluesync_timestamps_t *elem_master, bluesync_timestamps_t *elem_slave, size_t size) {

	uint8_t result_bitfield[NB_BYTES_BITFIELD] = {0};
		
	// Compute bitfield indicating timestamps available in both set
	bitwise_and_bitfields(result_bitfield, elem_master, elem_slave, NB_BYTES_BITFIELD);

    for (size_t i = 0; i < size; i++) {
		struct bluesync_msg_client_statistic stat = {
			.idx = i,
			.master_timer_ticks = elem_master->timer_ticks[i],
			.client_timer_ticks = elem_slave->timer_ticks[i],
			.valid = is_bit_set(result_bitfield, i),
		};

		bluesync_statistic_packet_status(&stat);
    }
}


static FILE *open_stat(char *filename, uint32_t device_number)
{
	FILE *fp;
	static char path[250];
	memset(path, 0, sizeof(path));

	snprintf(path, sizeof(path) - 1,
		 "/workdir/my-workspace-hbi/hbi-node/tests/time_sync/mesh_with_bluesync/bsim_utils/sim_output/%s_%i.csv", filename,
		 device_number);

	fp = fopen(path, "w");
	__ASSERT(fp != NULL, "Cannot open file");

	return fp;
}



void bluesync_statistic_init()
{
	uint32_t device_number = bsim_args_get_global_device_nbr();

	node.file = open_stat("node_status", device_number);

	fprintf(node.file, "timeslot_idx" SEPARATION_TOKEN "rcv_time" SEPARATION_TOKEN
				   "local_time" SEPARATION_TOKEN "valid"
				   "\n");
}

void bluesync_statistic_packet_status(struct bluesync_msg_client_statistic *packet_status){
	k_mutex_lock(&node.mutex, K_FOREVER);
	{
		fprintf(node.file,
			"%i" SEPARATION_TOKEN "%llu" SEPARATION_TOKEN "%llu" SEPARATION_TOKEN "%u"
			"\n",
			packet_status->idx, packet_status->master_timer_ticks, packet_status->client_timer_ticks,
			packet_status->valid);
	}
	k_mutex_unlock(&node.mutex);
}

void bluesync_statistic_deinit()
{
	fclose(node.file);
}

/* Automatically close the opened files by calling hbi_api_gateway_deinit() */
NATIVE_TASK(bluesync_statistic_deinit, ON_EXIT, 501);
