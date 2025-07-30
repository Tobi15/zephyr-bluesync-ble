# BlueSync – BLE Time Synchronization for Zephyr 🚀

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)

**BlueSync** is a lightweight, modular time synchronization system for [Zephyr RTOS](https://zephyrproject.org/), designed to coordinate clocks across wireless sensor nodes using Bluetooth Low Energy (BLE) broadcasts or Mesh messaging.

This module is inspired by research in wireless time synchronization (such as BlueSync) and provides a robust framework for aligning clocks using linear regression techniques.

In order to use this synchronization with BLE Mesh, you must use this Zephyr repository [Custom Zephyr RTOS](https://github.com/Tobi15/sdk-zephyr/tree/v3.5.99-ncs1-hbi-coded-phy)

---

## Reference

This project is based on the following work:

> A. A. Ghosh and R. Sridhar, "**BlueSync: BLE-Based Time Synchronization Using Broadcast Advertisements**", *2021 IEEE 5th World Forum on Internet of Things (WF-IoT)*, pp. 52–57, 2021.  
> DOI: [10.1109/WF-IoT51360.2021.9555832](https://doi.org/10.1109/WF-IoT51360.2021.9555832)


## ✨ Features

- ⏱️ **Precise logical time tracking** with slope/offset correction  
- 📡 **BLE mesh or advertisement-based sync** supported  
- 🧠 **Linear regression** across burst windows  
- 🧪 **Hardware test hooks** (buttons) and **BabbleSim support**  
- 📦 Easily integrated as a Zephyr external module

---

## 📁 Directory Structure

```
zephyr-bluesync-ble/
├── include/              # Public API headers
│   └── bluesync/
│       └── bluesync.h
├── src/                  # Core source files
│   ├── statistic/            # Optional BabbleSim support
│   │   │── bluesync_statistic.h
│   │   │── bluesync_statistic_bsim.h
│   │   │── bluesync_statistic_bsim.c
│   │   │── synced_time_logger.h
│   │   └── synced_time_logger.c
│   ├── bluesync.h
│   ├── bluesync.c
│   ├── local_time.h
│   ├── local_time.c
│   ├── bs_state_machine.h
│   ├── bs_state_machine.c
│   ├── bluesync_bitfields.h
│   └── bluesync_bitfields.c
├── zephyr/
│   ├── module.yml
│   └── Kconfig           # Configuration options
├── CMakeLists.txt
├── LICENSE
└── NOTICE
```

---

## 🛠️ Integration Instructions

### 1. Add to `west.yml`

In your Zephyr workspace `west.yml`, add:

```yaml
- name: bluesync
  path: modules/bluesync
  url: https://github.com/Tobi15/zephyr-bluesync
  revision: main
```

Then fetch it:

```bash
west update
```

---

### 2. Enable via `prj.conf`

Add to your application’s `prj.conf`:

```conf
CONFIG_BLUESYNC_SUPPORT=y
CONFIG_BLUESYNC_USED_IN_MESH=y
```

Optional test configs:

```conf
CONFIG_BLUESYNC_TEST_BABBLESIM_SUPPORT=y
```

---

### 3. Use in Code

For the single authority node in the network: 
```c
#include <bluesync/bluesync.h>

void main(void) {
    bluesync_init();
    bluesync_set_role(BLUESYNC_AUTHORITY_ROLE);

    //every t seconds -> perform a new synchronisation of your network
    while(1){
        uint64_t current_timestamp = get_NTP() // custom method to get your NTP time
        bluesync_start_net_sync_with_unix_epoch_us(current_timestamp);

        k_sleep(M_SECONDS(n));
    }
}
```

For the client node in the network: 
```c
#include <bluesync/bluesync.h>

void main(void) {
    bluesync_init();
    bluesync_set_role(BLUESYNC_CLIENT_ROLE);

    //every t seconds -> get the sync NTP time
    while(1){
        uint64_t current_timestamp = get_current_unix_time_us();
    }
}
```

---

## ⚙️ Configuration Options (`menuconfig`)

After enabling `CONFIG_BLUESYNC_SUPPORT`, other options become available under:

```
[*] BlueSync support  ---> 
    [*] Use BlueSync in BLE Mesh mode
    [*] Enable hardware button triggers
    ...
```

---

## 🔒 License

This project is licensed under the **Apache 2.0 License**.  
See [`LICENSE`](LICENSE) and [`NOTICE`](NOTICE) for details.

---

## 📫 Contributing

Contributions and improvements are welcome!  
By submitting a pull request, you agree to license your contribution under Apache 2.0.

---

## 🔗 Links

- Zephyr RTOS: https://zephyrproject.org/
- Conventional Commits: https://www.conventionalcommits.org/


## 📖 Documentation
Detailed documentation is available at this [`site`](https://tobi15.github.io/zephyr-bluesync-ble/)