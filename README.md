PROTOCOL 3 - SPI*
Task: SPI Master Driver with Multiple Devices*

TASK REQUIREMENTS:*
1. Correct CS behavior
2. Mode switching safe
3. Timeout + error detection
4. Validate with LA captures (clock/data alignment)

---

*WHAT WE DID & HOW WE DID:*

*1. Made Common SPI Function:*
`spi_transfer_with_timeout(csPin, spiMode, spiClock, tx, rx, len)`

*2. Correct CS Behavior:*
`CS HIGH` idle -> `LOW` in transfer -> `HIGH` after. Safe.

*3. Mode Switching Safe:*
DEV1 `MODE0 4MHz`, DEV2 `MODE3 1MHz` using `beginTransaction`

*4. Timeout + Error Detection:*
100ms timeout with `millis()` - Returns PASS/FAIL

*5. Full Duplex:*
`SPI.transfer()` with dummy `0xFF`

---

*VALIDATION RESULTS:*

*A) SERIAL MONITOR RESULT:*
DEV1 MODE0 4MHz: PASS
DEV2 MODE3 1MHz: PASS
RESULT: PASS - SPI Communication OK
*B) LOGIC ANALYZER (LA) RESULT:*
Connection: D0=SCK(13), D1=MOSI(11), D2=CS1(4), D3=CS2(10), GND=GND

LA Waveform Shows:
- D2 LOW pulse - SCK fast (4MHz) - MOSI 0x9F - D2 HIGH
- Gap (Mode Switch)
- D3 LOW pulse - SCK slow (1MHz) - MOSI 0x55 0xAA - D3 HIGH

This proves:
- CS never overlap = PASS
- Clock speed changed = PASS
- Mode0 to Mode3 switched = PASS
- Data aligned with clock = PASS

*C) FINAL PASS/FAIL:*
`RESULT: PASS - Both Serial + LA Validated`

*One Code = Both Proofs Done*
