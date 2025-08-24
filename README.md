# ![status](https://github.com/krjakbrjak/gpio-sysfs-simulator/actions/workflows/build-tests.yml/badge.svg)

# GPIO Sysfs Simulator with libfuse

## Overview

This project provides a GPIO Sysfs simulator implemented in C++ using the libfuse library. It creates a user-space filesystem that emulates the Linux GPIO sysfs interface, allowing you to simulate GPIO pin operations on a desktop or development machine. This is especially useful for development, testing, and experimentation when you do not have access to actual embedded hardware.

## Features

- **Filesystem Operations:** Implements the key filesystem operations required for GPIO sysfs simulation:
  - `getattr`: Retrieve file and directory attributes.
  - `readdir`: List the contents of directories.
  - `read`: Read data from files.
  - `write`: Write data to files.
  - `poll`: Notify the kernel when files are modified, enabling event-driven monitoring.

- **Asynchronous Monitoring:** Supports asynchronous monitoring of pin states using the `epoll` system call. When files such as `value` or `direction` are modified, the kernel is notified, allowing clients to react in real time.

- **Dynamic Pin Management:** Writing a pin number to `/export` creates new entries in the simulated filesystem, mimicking the behavior of the Linux kernel. Writing a pin number to `/unexport` removes those entries.

## Getting Started

### Prerequisites

- C++ compiler
- CMake
- Meson (required for building libfuse)

Install Meson and Ninja (example for Debian-based systems):

```sh
sudo apt install ninja-build meson
```

### Build the Project

```sh
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=$(pwd)/build/INSTALLDIR
cmake --build build --parallel
```

### Run the Simulator

```sh
export LD_LIBRARY_PATH=$(pwd)/build/INSTALLDIR/lib/x86_64-linux-gnu
$(pwd)/build/INSTALLDIR/bin/gpio-sysfs-simulator <mount-point>
```

This command mounts the simulated GPIO sysfs filesystem at `<mount-point>`. You can then use standard sysfs operations to export/unexport pins and manipulate their state.

## Examples

**Asynchronous Monitoring:**  
Use the provided [client.py](./examples/client.py) script or your own code with `epoll` to monitor changes in the simulated GPIO sysfs:

```sh
python examples/client.py --pins 1 2 3 4 -m <mount-point>
```
