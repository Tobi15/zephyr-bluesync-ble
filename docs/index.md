---
title: BlueSync Documentation
layout: default
---

# BlueSync for Zephyr RTOS

**BlueSync** is a lightweight and energy-efficient time synchronization protocol designed for Bluetooth Low Energy (BLE) Mesh networks.  
It enables accurate, multi-hop synchronization in resource-constrained environments such as linear sensor networks deployed in tunnels or industrial facilities.

This documentation covers the design decisions, synchronization architecture, and implementation details of BlueSync within the [Zephyr RTOS](https://zephyrproject.org/) framework.

## 📚 Documentation Sections

- [🧠 Conception](conception.md):  
  Learn about the motivation, architecture, packet format, and state machines that form the foundation of BlueSync.

- [🛠 Implementation](implementation.md):  
  Dive into the technical details of how BlueSync is integrated with Zephyr, including BLE usage, timestamping, drift correction, and more.

---

_This project is developed and maintained by [@Tobi15](https://github.com/Tobi15)._