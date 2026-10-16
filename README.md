# PROTOCOL 3: SPI Master Driver with Multiple Devices

## Task Requirements

- Correct CS behavior
- Mode switching safe
- Timeout + error detection
- Validate with LA captures (clock/data alignment)

## What We Did

- Made one common SPI function: `spi_transfer_with_timeout(csPin, spiMode, spiClock, tx, rx, len)`
- CS behavior: CS is HIGH when idle, goes LOW during the transfer, then goes HIGH again
- Mode switching: DEV1 uses MODE0 at 4 MHz, DEV2 uses MODE3 at 1 MHz, using `beginTransaction`
- Timeout + error detection: 100 ms timeout using `millis()`, returns PASS or FAIL
- Full duplex: `SPI.transfer()` with dummy byte `0xFF`

## Validation Results

### A) Serial Monitor Result

- DEV1 MODE0 4MHz: PASS
- DEV2 MODE3 1MHz: PASS
- RESULT: PASS - SPI Communication OK

### B) Logic Analyzer (LA) Result

Connections:

- D0 = SCK (pin 13)
- D1 = MOSI (pin 11)
- D2 = CS1 (pin 4)
- D3 = CS2 (pin 10)
- GND = GND

LA waveform shows:

- D2 goes LOW, SCK runs fast (4 MHz), MOSI sends 0x9F, D2 goes HIGH
- Gap (mode switch)
- D3 goes LOW, SCK runs slow (1 MHz), MOSI sends 0x55 0xAA, D3 goes HIGH

This proves:

- CS never overlap = PASS
- Clock speed changed = PASS
- Mode0 to Mode3 switched = PASS
- Data aligned with clock = PASS

### C) Final Result

- RESULT: PASS - Both Serial + LA Validated
- One code = both proofs done
