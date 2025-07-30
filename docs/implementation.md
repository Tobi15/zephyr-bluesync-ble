# Implementation of BlueSync in Zephyr RTOS

The BlueSync protocol is implemented as a lightweight module in Zephyr RTOS using BLE Extended Advertising and hardware timers. It enables precise time synchronization across BLE Mesh sensor nodes with minimal overhead.

## BLE Communication

- **Authority node** uses `bt_le_ext_adv_start()` to send sync packets.
- **Client nodes** use `bt_le_scan_start()` with filters to receive sync messages.
- Packets are sent as **non-connectable extended advertisements**, minimizing power and connection setup overhead.

## Timestamping

- Reception timestamps are captured using **RTC** or **TIMER2** at the radio callback level.
- Timestamps are stored in microseconds, based on a 32.768 kHz or higher resolution timer.
- `k_uptime_ticks()` or hardware timer APIs are used, depending on platform.

## Linear Regression

Each client performs linear regression to compute the best-fit slope and offset between master and local clocks.

- Inputs: `N` pairs of `(local_time, master_time)`
- Computation:

  ```c
  slope  = cov(x, y) / var(x);
  offset = mean(y) - slope * mean(x);

- If regression is successful (low error), client applies the calculated:
  - Offset: directly adjusts logical time.
  - Slope: stored for future drift correction.
- If regression fails:
  - Client resets sync state.
  - Returns to SCAN_WAIT_FOR_SYNC.

## Logical Time Correction
To get synchronized time on a client node:
```
uint64_t get_logical_time_us() {
    return slope * local_time() + offset;
}
```
- `local_time()` is obtained from RTC or uptime ticks.
- Offset and slope are applied dynamically without modifying the actual hardware clock.

## Multi-Hop Propagation
- After a successful sync update, a client rebroadcasts the sync packet.
- Downstream nodes use this rebroadcast to perform the same sync routine.
- This creates a chained propagation down the linear network.

## Integration with Zephyr
- Timers: Zephyr’s `counter` driver is used to timestamp sync events.
- Bluetooth: Uses Zephyr’s `bt_le_ext_adv` and `bt_le_scan` APIs.
- Shell interface (optional): For triggering sync or printing sync stats.
- Kconfig options: Allow enabling/disabling sync features, adjusting sample count, etc.

## Logging and Debug
- Verbose logging with `CONFIG_BLUESYNC_LOG_LEVEL`
- Each sync round stores metadata:
  - `round_id`
  - `offset`, `slope`
  - Sync quality metrics (e.g., error, sample count)

Logs can be dumped to CSV or shell output for validation.