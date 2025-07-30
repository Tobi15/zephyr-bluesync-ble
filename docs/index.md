---
title: BlueSync Documentation
layout: default
---

# BlueSync for Zephyr RTOS

**BlueSync** is a lightweight and energy-efficient time synchronization protocol designed for Bluetooth Low Energy (BLE) Mesh networks.  
It enables accurate, multi-hop synchronization in resource-constrained environments such as linear sensor networks deployed in tunnels or industrial facilities.

This documentation describes the protocol design and implementation within the [Zephyr RTOS](https://zephyrproject.org/) framework.

## 📚 Documentation Sections

- [🧠 Conception](conception.md):  
  Learn about the motivation, architecture, packet format, and state machines that form the foundation of BlueSync.

- [🛠 Implementation](implementation.md):  
  Dive into the technical details of how BlueSync is integrated with Zephyr, including BLE usage, timestamping, drift correction, and more.

---

## API Documentation

The full API reference generated with Doxygen is available here:

[View BlueSync API Docs](api/index.html)
[View BlueSync KConfig Docs](kconfig.md)

## Reference

This project is based on the following work:

> A. A. Ghosh and R. Sridhar, "**BlueSync: BLE-Based Time Synchronization Using Broadcast Advertisements**", *2021 IEEE 5th World Forum on Internet of Things (WF-IoT)*, pp. 52–57, 2021.  
> DOI: [10.1109/WF-IoT51360.2021.9555832](https://doi.org/10.1109/WF-IoT51360.2021.9555832)

---

_This project is developed and maintained by [@Tobi15](https://github.com/Tobi15)._
