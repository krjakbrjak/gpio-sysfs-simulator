# GPIO Sysfs Simulator with libfuse

## Overview

This project implements a GPIO Sysfs simulator using the libfuse library in C++. The primary goal is to create a filesystem that mimics the behavior of the GPIO sysfs interface, allowing users to simulate GPIO pin interactions on a desktop environment. This is particularly useful for development, testing, and experimentation when the actual embedded device is not readily available.

## Features

* Filesystem Hooks: The simulator implements five essential filesystem hooks:
  * getattr: Retrieve file attributes.
  * readdir: List directory contents.
  * read: Read from files.
  * write: Write to files.
  * poll: Send notifications to the kernel when files are modified.

* Asynchronous Monitoring: Clients can use the epoll system call to asynchronously monitor the state of simulated pins. Notifications are sent to the kernel whenever files (such as value and direction) are modified, allowing for real-time monitoring.

* Dynamic Pin Management: When a pin number is written to `/export`, new entries are automatically added to the simulated filesystem, replicating the behavior of the kernel. Conversely, writing a pin number to `/unexport` removes the corresponding entries.

## Getting Started

### Install Dependencies:

Ensure that libfuse is installed on your system.

```bash
# Example on Debian-based systems
sudo apt-get install fuse3 libfuse3-dev
```

### Build the Project:

```bash
cmake -B build -S .
cmake --build build
```

### Run the Simulator:

```bash
./gpio-sysfs-simulator <mount-point>
```

Replace <mount-point> with the desired mount point for the simulated GPIO sysfs.

### Examples

* Asynchronous Monitoring:
Use the [client.py](./examples/client.py) (or epoll system call in your client code) to monitor changes in the simulated GPIO sysfs.

```bash
python examples/client.py --pins 1 2 3 4 -m <mount-point>
```
