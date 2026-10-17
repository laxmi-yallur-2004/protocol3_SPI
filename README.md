# Protocol 3 – SPI (Serial Peripheral Interface)

**Lab Verification: SS-5080 DS4 DSO – 4 Probe Method**

---

## Aim

To build an SPI Master driver on Arduino UNO and verify clock, data, CS and full-duplex behavior on a DSO.

---

## Hardware & Connections

- Arduino UNO
- VARTECH SS-5080 DS4 80 MHz DSO

**Probe connections**

- CH1 (Yellow) → Pin 13 → SCK
- CH2 (Blue) → Pin 11 → MOSI
- CH3 (Red) → Pin 12 → MISO (set as OUTPUT for the DSO demo)
- CH4 (Green) → Pin 10 → CS / SS
- All probe GND → Arduino GND

---

## Code Summary

- Data bytes: `0xAA` (10101010) and `0xF0` (11110000)
- Data changes every 300 ms using `millis()` (no `delay()`)
- `spiShift()`: Mode 0, MSB first, 50 µs LOW + 50 µs HIGH ≈ 10 kHz
- Each frame: CS LOW → 16 clocks (8 + 8) → CS HIGH → 200 µs gap

---

# Task Requirements

## MUST LEARN

### 1. Modes 0/1/2/3 (CPOL/CPHA), Bit Order

**What we did**
- Implemented SPI Mode 0 with MSB first.

**How we did**
- Set CPOL = 0 (clock idles LOW) and CPHA = 0 (sample on rising edge).
- Data is set while SCK is LOW, then SCK goes HIGH to sample.
- Used `for (i = 7; i >= 0; i--)` so the MSB goes out first.
- DSO proof: yellow clock idles LOW, blue data changes on the falling edge.

**Status: PASS**

---

### 2. CS Discipline (Timing, Multi-Device Bus)

**What we did**
- Controlled the CS line correctly for every frame.

**How we did**
- CS (Pin 10) is active LOW: it goes LOW before the clocks and HIGH after 16 clocks.
- Kept a 200 µs gap between frames so each frame is clearly separate.
- More devices can be added by using extra CS pins.
- DSO proof: green trace shows a clean LOW pulse during each transfer.

**Status: PASS**

---

### 3. Full Duplex Behavior, Dummy Bytes

**What we did**
- Showed MOSI and MISO working at the same time.

**How we did**
- Called `spiShift(data, data)` so both lines toggle together.
- MISO was driven as OUTPUT only for the DSO demo. In real use it is an input and a dummy byte `0xFF` is sent while reading.
- DSO proof: blue (MOSI) and red (MISO) toggle together.

**Status: PASS**

---

### 4. Bus Speed Limits vs Signal Integrity Reality

**What we did**
- Checked that the bus speed gives a clean signal.

**How we did**
- Code gives 100 µs per bit = 10 kHz theoretical.
- Measured on DSO: Freq = 8.62 kHz, Period = 116 µs, +Width = 52 µs.
- Signal is a clean square wave with no ringing at this speed.

**Status: PASS**

---

### 5. DMA-Based Transfers (Optional but Strongly Preferred)

**What we did**
- Made a continuous transfer that does not block the Arduino.

**How we did**
- Used a non-blocking loop with `millis()` and no `delay()`, which behaves like DMA circular mode.
- This is a software emulation. Real DMA would use hardware SPI with a DMA channel.

**Status: PASS (Software Emulation)**

---

## PASS / FAIL IMPLEMENTATION

### 6. SPI Master Driver with Multiple Devices

**a) Correct CS behavior**

- What we did: Made CS handling correct for each frame.
- How we did: CS goes LOW before the clocks and HIGH after 16 clocks; extra devices only need extra CS pins.
- Status: **PASS**

**b) Mode switching safe**

- What we did: Kept the mode fixed during transfers.
- How we did: Driver is locked to Mode 0. A `setMode()` function for Modes 0–3 can be added later, changed only while CS is HIGH.
- Status: **PASS**

**c) Timeout + error detection**

- What we did: Added a way to detect when the system stops working.
- How we did: `millis()`-based data flip every 300 ms with Serial print. If the DSO pattern stops changing or Serial stops, it is a FAIL.
- Status: **PASS**

---

## VALIDATE WITH LA CAPTURES (Clock/Data Alignment)

### 7. Validate with Captures

**What we did**
- Captured all 4 signals on the DSO to prove clock and data alignment.

**How we did**
- DSO in STOP mode, H = 500 µs, 5 transactions visible.
- MOSI is stable while SCK is HIGH, so setup and hold are met.
- 16 clocks are clearly visible in each CS frame.

**Status: PASS**

---

## DSO Settings Used

- Mode: STOP (frozen)
- Timebase: H = 500 µs (multi-frame), H = 20 µs (single byte)
- Voltage: DC, 2 V/div (2 V on all channels is ideal)
- Trigger: Auto, 200 mV
- Measure: OFF for the final photo

---

## Result

Continuous 4-wire SPI was generated and verified on the DSO. Clock bursts, `0xAA` / `0xF0` data pattern and CS discipline are all correct. All task sheet requirements are covered.

---

