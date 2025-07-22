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
 * File: bluesync_bitfields.c
 * Description: Bitfield file to keep in touch which timeslot is received
 *
 * Project: BlueSync - BLE Time Sync for Zephyr
 * Repository: https://github.com/Tobi15/zephyr-bluesync-ble
 */

#include <zephyr/kernel.h>
#include "stdio.h"
#include "bluesync_bitfields.h"


//********************BITFIELD********************************* */
void print_bitfield_as_binary(uint8_t *bitfield, size_t size) {
    printk("0x");
    for (size_t i = size; i-- > 0;) {
        printk("%02X", bitfield[i]);
    }
    printk("\n");
}

void set_bit(uint8_t *bitfield, size_t bit_index) {
    bitfield[bit_index / 8] |= BIT(bit_index % 8);
}

bool is_bit_set(uint8_t *bitfield, size_t bit_index) {
    return bitfield[bit_index / 8] & BIT(bit_index % 8);
}

void bitwise_and_bitfields(uint8_t *result, const bluesync_timestamps_t *rcv, const bluesync_timestamps_t *local, size_t num_bytes) {
    for (size_t i = 0; i < num_bytes; i++) {
        result[i] = rcv->bitfield[i] & local->bitfield[i];
    }
}

// Count the number of 1 bits in a byte
static inline uint8_t count_bits_in_byte(uint8_t byte) {
    // Using GCC built-in function if available (more efficient)
    #if defined(__GNUC__)
    return __builtin_popcount(byte);
    #else
    // Fallback method: Count bits manually
    uint8_t count = 0;
    while (byte) {
        count += byte & 1;
        byte >>= 1;
    }
    return count;
    #endif
}

// Count the total number of 1 bits in the bitfield
size_t count_set_bits(uint8_t *bitfield, size_t num_bytes) {
    size_t total_count = 0;
    for (size_t i = 0; i < num_bytes; i++) {
        total_count += count_bits_in_byte(bitfield[i]);
    }
    return total_count;
}