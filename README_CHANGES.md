# Changes in vscp-demo-link-stm32f401-wiz-ip20-blinky

Every change that goes into a release is listed here, newest release first.
Add each change under **Unreleased** when it is made. When a release is
tagged, rename **Unreleased** to the version and date, and start a new
**Unreleased** section above it.

## Unreleased

- Events sent by a link client were never freed (out of memory after about 1500 events); `rcvloop` sends '+OK' every second (it was every 1000 seconds); vscp-firmware updated (`clrall` frees the queued events)
- The decision matrix works: events from clients are run through it (only the protocol handler got them), its registers 0x60-0x7F can be read and written, and it uses the Level I rows of the MDF (new vscp-firmware with fixed Level II matrix and `bDmLevel1Rows`)
- Settings are kept: flash was rewritten without erasing, so only the first save worked. The configuration is now saved as appended records (erase only when the sector is full), one second after the last change and at once when a session ends; the old layout is loaded once
- Nickname and user id are saved and loaded; the node GUID (from the MCU id) is set for the firmware too (sent events had an all-zero GUID)
- Status register writes turn the LED off as well as on; start blink (action 5) keeps the interval (it was overwritten from the parameter and the next byte); the LED actions need control bit 7; on/off/toggle stop blinking
- CLASS1.INFORMATION ON/OFF events when the LED is turned on/off, CLASS1.CONTROL TURN-OFF on button release, alarm bit 7 when the millisecond counter wraps
- Button events are made in the main loop (they were allocated and queued in the interrupt handler); control bit 6 (enable button) is respected
- Defaults as in the MDF: blink interval 500 ms, control 0xCE (millisecond counter off)
- New transports, chosen with `-DBLINKY_MODE=`: TCP client, UDP, multicast and MQTT (VSCP JSON events), besides the TCP server. The WIZ-IP20 remote host is set with RH (RI is the reconnect interval), and the MQTT settings use the right commands (QC, QO) and the node GUID in the topics
- VSCP frame CRCs were always 0 (`crcInit()` was never called): sent frames had no valid CRC and received frames were not checked
- vscp-firmware updated to master (`src/` layout); JSON support only in the MQTT build
- README: Blinky functionality, transports, flash storage
- README and docs corrected for the STM32F401: title, clone URL, OpenOCD target (stm32f4x), broken image and repository links, MDF location
- Removed files left over from the STM32F103 project that the build does not use (F1 HAL driver and CMSIS device files, backup files, EWARM project, F103 linker script, startup file and CubeMX files)
- Fixed typo in readme (2e860a6)
- Added infor about VSCP works and other tools (7ca92cf)
- Fixed problems with user quit (a47be33)
- Updated flash size for STM32F401RE (5b9866e)
- Added test setup to front page (c86dbf0)
- Added test setup images (5ab3328)
- Added images (af0b8c8)
- Added connection diagram (ce7e425)
- ADD: Event transmit Statistics (bd7beca)
- Added watchdog (ae5261e)
- Added LED control when writing status register (8f647a5)
- Added register handling (0a05192)
- ADD: Added register defines (966909f)
- ADD: Interrupt handler for user button (4c19b78)
- Updated firmware lib to latest (697f367)
- Formatting (89d613c)
- Test (a6ebbcc)
- heartbeat fixed (f508e11)
- Convert vscp-firmware from tracked files to submodule (a9037d6)
- Ported from STM32F103 project (0986189)
- Initial commit (9ea3cfd)

No release has been tagged yet.
