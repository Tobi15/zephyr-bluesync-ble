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

/**
 * @brief Check if a specific bit is set in the bitfield.
 *
 * @param bitfield Pointer to the bitfield array.
 * @param bit_index Index of the bit to check.
 * @return true if the bit is set, false otherwise.
 */
bool is_bit_set(uint8_t *bitfield, size_t bit_index);

/**
 * @brief Set a specific bit in the bitfield.
 *
 * @param bitfield Pointer to the bitfield array.
 * @param bit_index Index of the bit to set.
 */
void set_bit(uint8_t *bitfield, size_t bit_index);

/**
 * @brief Perform a bitwise AND between two bitfields and store the result.
 *
 * This is typically used to find common timestamps (slots) between received and local sets.
 *
 * @param result Output buffer where the result will be stored.
 * @param rcv Pointer to the bitfield from received timestamps.
 * @param local Pointer to the bitfield from local timestamps.
 * @param num_bytes Number of bytes in each bitfield.
 */
void bitwise_and_bitfields(uint8_t *result, const bluesync_timestamps_t *rcv, const bluesync_timestamps_t *local, size_t num_bytes);

/**
 * @brief Count the number of bits set to 1 in a bitfield.
 *
 * Useful for determining how many timeslots have been received.
 *
 * @param bitfield Pointer to the bitfield array.
 * @param num_bytes Number of bytes in the bitfield.
 * @return Number of bits set to 1.
 */
size_t count_set_bits(uint8_t *bitfield, size_t num_bytes);
#endif /* ZEPHYR_BLUESYNC_SRC_BLUESYNC_BITFIELDS_H_ */