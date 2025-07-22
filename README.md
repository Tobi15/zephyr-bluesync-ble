# BlueSync – BLE Time Synchronization for Zephyr 🚀

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)

**BlueSync** is a lightweight, modular time synchronization system for [Zephyr RTOS](https://zephyrproject.org/), designed to coordinate clocks across wireless sensor nodes using Bluetooth Low Energy (BLE) broadcasts or Mesh messaging.

This module is inspired by research in wireless time synchronization (such as BlueSync) and provides a robust framework for aligning clocks using linear regression techniques.

In order to use this synchronization with BLE Mesh, you must use this Zephyr repository [Custom Zephyr RTOS](https://github.com/Tobi15/sdk-zephyr/tree/v3.5.99-ncs1-hbi-coded-phy)

---

## ✨ Features

- ⏱️ **Precise logical time tracking** with slope/offset correction  
- 📡 **BLE mesh or advertisement-based sync** supported  
- 🧠 **Linear regression** across burst windows  
- 🧪 **Hardware test hooks** (buttons) and **BabbleSim support**  
- 📦 Easily integrated as a Zephyr external module

---

## 📁 Directory Structure

```
zephyr-bluesync/
├── include/              # Public API headers
│   └── bluesync/
│       └── bluesync.h
├── src/                  # Core source files
|   ├── statistic/            # Optional BabbleSim support
│   |   └── statistic.c
│   ├── bluesync.c
│   ├── local_time.c
│   ├── bs_state_machine.c
│   ├── bluesync_bitfields.c
│   └── ...
├── zephyr/
│   └── Kconfig           # Configuration options
├── CMakeLists.txt
├── module.yml
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
  url: https://github.com/yourname/zephyr-bluesync
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
