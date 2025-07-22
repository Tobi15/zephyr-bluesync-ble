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
 * File: bluesync_bitfields.h
 * Description: Bitfield file to keep in touch which timeslot is received
 *
 * Project: BlueSync - BLE Time Sync for Zephyr
 * Repository: https://github.com/Tobi15/zephyr-bluesync-ble
 */


#ifndef ZEPHYR_BLUESYNC_SRC_BLUESYNC_BITFIELDS_H_
#define ZEPHYR_BLUESYNC_SRC_BLUESYNC_BITFIELDS_H_

#include "bluesync.h"

bool is_bit_set(uint8_t *bitfield, size_t bit_index);
void set_bit(uint8_t *bitfield, size_t bit_index);
void bitwise_and_bitfields(uint8_t *result, const bluesync_timestamps_t *rcv, const bluesync_timestamps_t *local, size_t num_bytes);
size_t count_set_bits(uint8_t *bitfield, size_t num_bytes);
#endif /* ZEPHYR_BLUESYNC_SRC_BLUESYNC_BITFIELDS_H_ */