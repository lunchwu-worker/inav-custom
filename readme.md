# INAV - navigation capable flight controller

# F411 PSA

> INAV no longer accepts targets based on STM32 F411 MCU.

> INAV 7 was the last INAV official release available for F411 based flight controllers. INAV 8 is not officially available for F411 boards and the team has not tested either. Issues that can't be reproduced on other MCUs may not be fixed and the targets for F411 targets may eventually be completelly removed from future releases.

# ICM426xx IMUs PSA

> The filtering settings for the ICM426xx has changed to match what is used by Ardupilot and Betaflight in INAV 7.1. When upgrading from older versions you may need to recalibrate the Accelerometer and if you are not using INAV's default tune you may also want to check if the tune is still good.

# M7, M6 and older UBLOX GPS units PSA

> INAV 8.0 will mark those GPS as deprecated and INAV 9.0.0 will require UBLOX units with Protocol version 15.00 or newer. This means that you need a GPS unit based on UBLOX M8 or newer.

> If you want to check the protocol version of your unit, it is displayed in INAV's 7.0.0+ status cli command.
> INAV 8.0.0 will warn you if your GPS is too old.
> ```GPS: HW Version: Unknown Proto: 0.00 Baud: 115200 (UBLOX Proto >= 15.0 required)```


> M8, M9 and M10 GPS are the most common units in use today, are readly available and have similar capabilities.
>Mantaining and testing GPS changes across this many UBLOX versions is a challenge and takes a lot of time. Removing the support for older devices will simplify code.

![INAV](http://static.rcgroups.net/forums/attachments/6/1/0/3/7/6/a9088858-102-inav.png)

# PosHold, Navigation and RTH without compass PSA

Attention all drone pilots and enthusiasts,

Are you ready to take your flights to new heights with INAV 7.1? We've got some important information to share with you.

INAV 7.1 brings an exciting update to navigation capabilities. Now, you can soar through the skies, navigate waypoints, and even return to home without relying on a compass. Yes, you heard that right! But before you launch into the air, there's something crucial to consider.

While INAV 7.1 may not require a compass for basic navigation functions, we strongly advise you to install one for optimal flight performance. Here's why:

🛰️ Better Flight Precision: A compass provides essential data for accurate navigation, ensuring smoother and more precise flight paths.

🌐 Enhanced Reliability: With a compass onboard, your drone can maintain stability even in challenging environments, low speeds and strong wind.

🚀 Minimize Risks: Although INAV 7.1 can get you where you need to go without a compass, flying without one may result in a bumpier ride and increased risk of drift or inaccurate positioning.

Remember, safety and efficiency are paramount when operating drones. By installing a compass, you're not just enhancing your flight experience, but also prioritizing safety for yourself and those around you.

So, before you take off on your next adventure, make sure to equip your drone with a compass. It's the smart choice for smoother flights and better navigation.

Fly safe, fly smart with INAV 7.1 and a compass by your side!

# INAV Community

* [INAV Discord Server](https://discord.gg/peg2hhbYwN)
* [INAV Official on Facebook](https://www.facebook.com/groups/INAVOfficial)

## Downloads

### INAV Configurator

**Get the latest version:** **[Download INAV Configurator](https://github.com/iNavFlight/inav-configurator/releases/latest)** - Available for Windows, macOS, and Linux

The INAV Configurator is the official desktop application for configuring your INAV flight controller. Choose your platform from the Assets section on the releases page.

### INAV Firmware

**Get the latest firmware:** **[Download INAV Firmware](https://github.com/iNavFlight/inav/releases/latest)**

Download the latest INAV flight controller firmware. Flash it to your flight controller using the configurator.

## Features

* Runs on the most popular F4, AT32, F7 and H7 flight controllers
* On Screen Display (OSD) - both character and pixel style
* DJI OSD integration: all elements, system messages and warnings
* Outstanding performance out of the box
* Position Hold, Altitude Hold, Return To Home and Waypoint Missions
* Excellent support for fixed wing UAVs: airplanes, flying wings
* Blackbox flight recorder logging
* Advanced gyro filtering
* Fully configurable mixer that allows to run any hardware you want: multirotor, fixed wing, rovers, boats and other experimental devices
* Multiple sensor support: GPS, Pitot tube, sonar, lidar, temperature, ESC with BlHeli_32 telemetry
* Logic Conditions, Global Functions and Global Variables: you can program INAV with a GUI
* SmartAudio and IRC Tramp VTX support
* Telemetry: SmartPort, FPort, MAVlink, LTM, CRSF
* Multi-color RGB LED Strip support
* And many more!

For a list of features, changes and some discussion please review consult the releases [page](https://github.com/iNavFlight/inav/releases) and the documentation.

## Tools

### INAV Configurator

Official tool for INAV can be downloaded [here](https://github.com/iNavFlight/inav-configurator/releases). It can be run on Windows, MacOS and Linux machines and standalone application.

### INAV Blackbox Explorer

Tool for Blackbox logs analysis is available [here](https://github.com/iNavFlight/blackbox-log-viewer/releases)

### INAV Blackbox Tools

Command line tools (`blackbox_decode`, `blackbox_render`) for Blackbox log conversion and analysis [here](https://github.com/iNavFlight/blackbox-tools).

### Telemetry screen for EdgeTX and OpenTX

Users of EdgeTX and OpenTX radios (Taranis, Horus, Jumper, Radiomaster, Nirvana) can use INAV OpenTX Telemetry Widget screen. Software and installation instruction are available here: [https://github.com/iNavFlight/OpenTX-Telemetry-Widget](https://github.com/iNavFlight/OpenTX-Telemetry-Widget)

### OSD layout Copy, Move, or Replace helper tool

[Easy INAV OSD switcher tool](https://www.mrd-rc.com/tutorials-tools-and-testing/useful-tools/inav-osd-switcher-tool/) allows you to easily switch your OSD layouts around in INAV. Choose the from and to OSD layouts, and the method of transfering the layouts.

## Installation

See: https://github.com/iNavFlight/inav/blob/master/docs/Installation.md

## Documentation, support and learning resources
* [INAV 5 on a flying wing full tutorial](https://www.youtube.com/playlist?list=PLOUQ8o2_nCLkZlulvqsX_vRMfXd5zM7Ha)
* [INAV on a multirotor drone tutorial](https://www.youtube.com/playlist?list=PLOUQ8o2_nCLkfcKsWobDLtBNIBzwlwRC8)
* [Fixed Wing Guide](docs/INAV_Fixed_Wing_Setup_Guide.pdf)
* [Autolaunch Guide](docs/INAV_Autolaunch.pdf)
* [Modes Guide](docs/INAV_Modes.pdf)
* [Wing Tuning Masterclass](docs/INAV_Wing_Tuning_Masterclass.pdf)
* [Official documentation](https://github.com/iNavFlight/inav/tree/master/docs)
* [Official Wiki](https://github.com/iNavFlight/inav/wiki)
* [Video series by Paweł Spychalski](https://www.youtube.com/playlist?list=PLOUQ8o2_nCLloACrA6f1_daCjhqY2x0fB)
* [Target documentation](https://github.com/iNavFlight/inav/tree/master/docs/boards)

## Contributing

Contributions are welcome and encouraged.  You can contribute in many ways:

* Documentation updates and corrections.
* How-To guides - received help?  help others!
* Bug fixes.
* New features.
* Telling us your ideas and suggestions.
* Buying your hardware from this [link](https://inavflight.com/shop/u/bg/)

A good place to start is the Discord channel, Telegram channel or Facebook group. Drop in, say hi.

Github issue tracker is a good place to search for existing issues or report a new bug/feature request:

https://github.com/iNavFlight/inav/issues

https://github.com/iNavFlight/inav-configurator/issues

Before creating new issues please check to see if there is an existing one, search first otherwise you waste peoples time when they could be coding instead!

## Developers

Please refer to the development section in the [docs/development](https://github.com/iNavFlight/inav/tree/master/docs/development) folder.

Nightly builds are available for testing on the following links:

https://github.com/iNavFlight/inav-nightly/releases

https://github.com/iNavFlight/inav-configurator-nightly/releases

## INAV Releases
https://github.com/iNavFlight/inav/releases

---

## 本地编译指南 (SPEEDYBEEF405WING)

本节记录在 Linux (WSL2 Ubuntu 22.04) 环境下从源码编译 **SPEEDYBEEF405WING** 固件的完整步骤。

### 环境要求

| 依赖 | 最低版本 | 说明 |
|------|----------|------|
| cmake | 3.13+ | 构建系统生成器 |
| make / ninja | - | 构建管理器 |
| ruby | 2.7+ | 生成 settings_generated 文件 |
| gcc | 9+ | 本机编译器（用于 host 工具） |
| arm-none-eabi-gcc | 9.3.1 / 13.2.1 | ARM 交叉编译工具链 |

### 安装依赖 (Ubuntu/Debian)

```bash
sudo apt update && sudo apt install -y git make cmake ruby gcc
```

> 如果网络不畅，可配置代理：
> ```bash
> export http_proxy="http://127.0.0.1:10808"
> export https_proxy="http://127.0.0.1:10808"
> ```

### 克隆仓库

```bash
git clone https://github.com/iNavFlight/inav.git
cd inav
```

### 配置 CMake

```bash
mkdir -p build && cd build

# 使用 INAV 内置工具链（自动下载 ARM GCC 13.2.1）
cmake ..

# 或者使用系统/已有工具链并跳过版本检查
cmake -DCOMPILER_VERSION_CHECK=OFF ..
```

### 编译目标固件

```bash
# 编译单个目标
make -j$(nproc) SPEEDYBEEF405WING

# 编译完成后，固件位于 build 目录
ls -l inav_*_SPEEDYBEEF405WING.hex
```

### 常见问题

#### 1. 工具链下载超时
如果自动下载 ARM 工具链超时，可手动下载并解压到 `tools/` 目录，或使用 `-DCOMPILER_VERSION_CHECK=OFF` 跳过版本检查，使用系统已有的 `arm-none-eabi-gcc`。

#### 2. `a label can only be part of a statement` 编译错误
此问题出现在 `src/main/sensors/battery.c` 中，原因是 C89/C90 标准不允许在 `case` 标签后直接声明变量。解决方法是将变量声明包裹在代码块 `{}` 中：

```c
case VOLTAGE_SENSOR_SMARTPORT:
    {
        int16_t *smartportVoltageData = smartportMasterGetVoltageData();
        // ...
    }
    break;
```

#### 3. 缺少 ruby
```bash
sudo apt install -y ruby
```

### 刷写固件

使用 **INAV Configurator** 选择本地固件文件：
```
build/inav_9.0.0_SPEEDYBEEF405WING.hex
```

或通过命令行工具 `dfu-util` / `stm32flash` 刷写。

---


