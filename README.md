# vscp-demo-link-stm32f103-wiz-ip20-blinky

![](images/test-setup1.jpg)


[VSCP blinky demo](https://github.com/grodansparadis/vscp-firmware/wiki) for STM32F103C8T6 "Blue Pill" with [WIZnet IP20 module](https://wiznet.io/products/serial-to-ethernet-modules/wiz-ip20). The firmware is built with CMake and STM32CubeMX-generated code, using the GNU Arm Embedded Toolchain. It demonstrates basic GPIO control for blinking an LED and configures and sets up network connectivity. 

The Blinky demo general functionality is described [here](https://github.com/grodansparadis/vscp/wiki/Blinky).The VSCP blinky demo is the simplest possible VSCP application, toggling a LED on and off at a configurable interval and generate events from a single button. It is general demo available on different platforms and transports. This project serves as a starting point to learn more about VSCP and for developing more complex VSCP applications on STM32 microcontrollers with some sort of connectivity. The VSCP blinky demo is designed to be simple and easy to understand, making it ideal for learning how to use the VSCP protocol and develop applications for embedded systems. It is implemented on different platforms, including STM32 microcontrollers, and can be used as a reference for building your own VSCP applications.

## Hardware
- [ST-Link V2 programmer/debugger](https://www.st.com/en/development-tools/st-link-v2.html)
- [WIZ-IP20 IO Module](https://docs.wiznet.io/Product/Modules/Serial-to-Ethernet-Module/WIZ-IP20/ip20-io)
- [W55RP20-S2E Command Manual](https://docs.wiznet.io/Product/Chip/MCU/Pre-programmed-MCU/W55RP20-S2E/command-manual-en)
- [WIZ-IP20 Product Page](https://wiznet.io/products/serial-to-ethernet-modules/wiz-ip20)
- [WIZnet S2E Tool GUI Getting Started Guide](https://github.com/Wiznet/WIZnet-S2E-Tool-GUI/wiki/Getting-started-guide_en)

### Resources

* TIM2 is used as a millisecond timebase.
* TIM3 is used as a 1 µs free-running counter, extended to 32 bits via software overflow.

## Persistent Flash Storage

128 KB of internal flash is reserved for persistent storage (registers, configuration) using the last page of the 512 KB flash:

| Region  | Start        | Size  | 
|---------|-------------|-------|
| Ram     | `0x20000000` | 96 KB |
| Firmware | `0x08000000` | 384 KB | 
| Storage  | `0x08060000` |  128 KB | 

The linker script ([STM32F401RETx_FLASH.ld](firmware/STM32F401RETx_FLASH.ld)) reduces the `FLASH` region to 384 KB and defines a separate `STORAGE` region. The driver is in [Core/Inc/flash_storage.h](firmware/Core/Inc/flash_storage.h) and [Core/Src/flash_storage.c](firmware/Core/Src/flash_storage.c).

Key constraints:
- A full sector (128 KB) must be **erased** before any byte in it can be written
- Writes are **16-bit (half-word)** aligned
- Flash endurance: ~**10 000** erase cycles per sector

```c
#include "flash_storage.h"

// Erase both storage pages (required before first write)
flash_storage_erase();

// Write two half-words at offset 0
uint16_t cfg[] = { 0x0001, 0x1234 };
flash_storage_write(0, cfg, 2);

// Read back
uint16_t buf[2];
flash_storage_read(0, buf, 2);
```

### Setup

![](schema/connection%20diagram.svg)


- Connect an LED with a suitable resistor to PA5 for blinking (or use the onboard LED if available)
- Adda button to PC13 for user input (or use the onboard button if available)
- Extra debug output is available on PA2/PA3 (USART2 TX/RX) for printf debugging. Connect a serial TTL to USB adapter to PA2 and GND to view debug output on your computer. On the nucleo card the built in ST_Link debugger provides a virtual COM port for this purpose. The default baud rate is 115200, 8N1.

If you need to connect an externa ST-Link programmer/debugger, connect SWDIO to PA13, SWCLK to PA14, GND to GND and 3.3V to 3.3V. The WIZ-IP20 module is connected to the STM32F401 via UART1 (PA9/PA10) and to the network via Ethernet.


## Build the project

Check out the repository

```bash
  git clone --recurse-submodules https://github.com/grodansparadis/vscp-demo-link-stm32f103-wiz-ip20-blinky.git
```

Go to the firmware directory

```bash
cmake -B build \
  -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake \
  -DCMAKE_BUILD_TYPE=Debug

cmake --build build -j$(nproc)
```

This produces build/firmware.elf and .hex/.bin.


## Flashing the project

```bash
# OpenOCD
openocd -f interface/stlink.cfg -f target/st m32f1x.cfg \
  -c "program build/firmware.elf verify reset exit"

# ST-Link CLI (install with 'sudo apt install stlink-tools')
st-flash write build/firmware.bin 0x08000000
```

### Build Tips

- Regenerating code — CubeMX overwrites CMakeLists.txt files in cmake/stm32cubemx/ but leaves your top-level alone; keep custom code in Core/Src and your own cmake files
- VS Code — install the CMake Tools extension, point it at the toolchain file via cmake.configureSettings in settings.json
- Ninja — add -G Ninja to the cmake command for faster builds

### Testing ST_Link CLI

```bash

# Check version of st-flash
st-flash --version

# Example reply
v1.8.0

# Check connected ST-Link devices 
st-info --probe      # detects connected ST-Link

# Example reply
Found 1 stlink programmers
  version:    V2J23S9
  serial:     066EFF545454885087241237
  flash:      524288 (pagesize: 16384)
  sram:       98304
  chipid:     0x433
  dev-type:   STM32F401xD_xE
``` 

## Debugging

Use OpenOCD with GDB:

```bash
# Start OpenOCD in one terminal
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg

# In another terminal, connect with GDB
arm-none-eabi-gdb build/firmware.elf
(gdb) target remote localhost:3333
(gdb) load
(gdb) continue

# You can set breakpoints, inspect variables, etc. in GDB

# Alternatively use 'st-util' for a more user-friendly interface
# but with less features than OpenOCD
st-util
```

## Testing the firmware

As default the demo will use the static IP address 192.168.1.88. You can change this and other network settings in the **wiznet-ip20.h** file and recomile the firmware and flash new firmware. The WIZ-IP20 module must be connected to the same network as your computer. 

First test is to use a ping command to test connectivity:

```bash
ping 192.168.1.88
```

If no response is received, check the wiring and network settings. 

You can now use **telnet** to connect to the WIZ-IP20 module on port 9598. 

```bash
telnet 192.168.1.88 9598
```

You should see a prompt for the VSCP link server

```bash
akhe@fluorine:~$ telnet 192.168.1.88 9598
Trying 192.168.1.88...
Connected to 192.168.1.88.
Escape character is '^]'.
Welcome to the VSCP Blinky demo
STM32F401 + WIZnet IP20
Version: 0.0.1 - 210
Copyright (C) 2000-2026 Grodans Paradis AB
https://www.grodansparadis.com
+OK
``` 

You can now issue commands to the VSCP link server. But to get access to all commands you need to login with the default username and password. 

```bash
user vscp
password secret
```

You can now type **help** to get a list of all available commands. The VSCP link protocol is described in detail in the [VSCP link protocol specification](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_tcpiplink).

## Write registers

VSCP use events to control things like a LED. Initially the LED will blink with a 500ms blink interval. You can use the **send** command to send events to the VSCP link server. Register 4 and 5 holds the blink interval. The following example will change the blink interval of the LED to 1000ms. 

```bash
send 0,0,11,0,,0,-,16,4,0x03
send 0,0,11,0,,0,-,16,5,0xe8
```

Here we use the [write register event](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.protocol?id=type11) to write to register 4 and 5. The first event will write the value 3 to register 4 and the second event will write the value 232 (0xe8) to register 5. That is 03e8 is written to the blink interval registers, the blink interval is calculated as 1000ms = 03e8h = 1000 decimal.

Use

```bash
send 0,0,11,0,,0,-,16,4,0x01
send 0,0,11,0,,0,-,16,5,0xf4
```

to set the blink interval back to 500ms. 

What we do here is to send a VSCP event to the VSCP link server. The event is sent as a [comma separated list of numbers](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_tcpiplink?id=tcpip-send). In this case the link server forward the event to the node itself. The node will then process the event and change the blink interval.

The first number is the **head** for the VSCP frame. This is always 0 for this demo but can have bits set for different functionality.

The second byte is the **VSCP class**. This is kind of a group of events, in this case protocol related functionality. [There is a lot of events defined](https://grodansparadis.github.io/vscp-doc-spec/#/./level_i_events) for different purposes.

The third number is the **VSCP type**, in this case the [write register event](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.protocol?id=type11). 

The forth number is the **obid**, object id, which is used to identify the event. This is always 0 for this demo. It is used by higher level software to identify events.

Then we have (number four and five) the **timefield** and the **timestamp**. The timefield is always empty for modern VSCP nodes (two commas). It was previously used to supply a ISO formatted date and time in older VSCP nodes (YYYY:MM:DDTHH:MM:SS). Now the next number after the two commas, is the 64-bit timestamp, holds this information with nanosecond resolution. We can set it to 0 for this demo.

The dash is a placeholder for the [GUID](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_globally_unique_identifiers?id=globally-unique-identifiers) (globally unique identifier)- We can set any GUID here in the this demo, but in a real world scenario you will see something like **00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:01** here. 

After the GUID the data for the event is listed as a comma separated list of numbers. Each of the value numbers here can be set as decimal, binary, octal or hex values. Every event has different data format, in this case the first byte is the node id, the second the register to write and the third the value that should be written.

## Read GUID

If you cant to check the GUID of the demo node you can use the [read GUID command](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_tcpiplink?id=tcpip-read-guid) to read the GUID. The following example will read the GUID of the demo node.

```bash
getguid
```
For this demo the GUID is constructed from the 96-bit unique id stored in the STM32F401 microcontroller. It will look something like this: 

```bash
FD:00:02:00:5D:00:5E:33:33:51:0C:34:35:34:37:10
```

The last byte is the nickname for the node, in this case 0x10/16. You recognize it from the write commands above where it is data byte 0. The nickname can be changed by writing to register 0x91/145 in [the standard register space](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_register_abstraction_model?id=register_abstraction_model) of a device. The nickname is used by the VSCP link server to identify the node.

## Read registers

If you want to read the blink interval registers you can use the [read register event](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.protocol?id=type10) to read the values. The following example will read the blink interval registers 4 and 5.

```bash
send 0,0,9,0,,0,-,16,4
send 0,0,9,0,,0,-,16,5
```

Type is now 9 instead of 11 and data byte 2, the data to write is not needed. Use 


```bash
retr
```

twice to poll data from the node you will get a reply from the node with the values of the registers. The reply will look something like this:

```bash
352,0,10,0,,2369722372559,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00,4,1
+OK - Success.

352,0,10,0,,2379583825845,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00,5,244
+OK - Success.
```

Register 4 has content 1 and register 5 content 244. That is 01f4h = 500 decimal, the blink interval is 500ms.

You may also get two events when you read events from the node. 

[Class=1026, Type= 2](https://grodansparadis.github.io/vscp-doc-spec/#/./class2.information?id=type2) is a [heartbeat event](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.protocol?id=type2) that is sent every 60 seconds by the node. It is used to indicate that the node is alive and functioning. 

and

[1024,20](https://grodansparadis.github.io/vscp-doc-spec/#/./class2.protocol?id=type20) which is the high ens server capabilities event. It is sent by the node to indicate its capabilities and features. It is used by clients to determine what the node can do and how it can be used.


As you now understand and see the VSCP link protocol is very simple and easy to use. It is a great way to get started with VSCP and to learn more about the protocol. But to work with it using telnet is quite hard work. It is much easier to use a VSCP client like [VSCP Works](https://github.com/grodansparadis/vscp-works-qt) or code like the [vscp-helper-lib](https://github.com/grodansparadis/vscp-helper-lib).

Quit the telnet session with

```bash
quit
```

### VSCP Works


