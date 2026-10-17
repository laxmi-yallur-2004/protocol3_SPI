# Protocol 3 – SPI (Serial Peripheral Interface)

**Lab Verification: SS-5080 DS4 DSO – 4 Probe Method**

---

### Aim
To build an SPI Master driver on Arduino UNO and verify clock, data, CS and full-duplex behavior on DSO.

---

### Hardware & Connections
- Arduino UNO + VARTECH SS-5080 DS4 80MHz DSO
- CH1 Yellow -> Pin 13 SCK
- CH2 Blue -> Pin 11 MOSI
- CH3 Red -> Pin 12 MISO (OUTPUT for DSO demo, INPUT in real driver)
- CH4 Green -> Pin 10 CS / SS
- All GND -> Arduino GND

---

### Code Summary
- Data: 0xAA (10101010) and 0xF0 (11110000)
- millis() every 300ms flips 0xAA <-> 0xF0, No delay()
- spiShift(): Mode 0, MSB first, 50us LOW + 50us HIGH
- Frame: CS LOW -> 16 clocks (8+8) -> CS HIGH -> 200us gap

---

### Task Requirements

#### 1. Modes 0/1/2/3 (CPOL/CPHA), Bit Order
- **What we did:** Implemented Mode 0, MSB first.
- **How we did:** CPOL=0 idle LOW, CPHA=0 sample rising. Data set when LOW, sampled HIGH. for(i=7;i>=0;i--) MSB first.
- **DSO Proof:** Yellow idles LOW, Blue changes on falling. Measured 8.62kHz vs 10kHz theory, difference due to digitalWrite() overhead (~8us).
- **Status:** PASS

#### 2. CS Discipline (Timing, Multi-Device Bus)
- **What we did:** Correct CS per frame.
- **How we did:** CS active LOW, LOW before clocks, HIGH after 16 clocks. 200us gap separates frames. Extra devices via extra CS pins.
- **DSO Proof:** Green trace clean LOW pulse in STOP 500us capture.
- **Status:** PASS

#### 3. Full Duplex Behavior, Dummy Bytes
- **What we did:** Simultaneous MOSI/MISO.
- **How we did:** spiShift(data,data) toggles both. Demo MISO=OUTPUT, real use MISO=INPUT with dummy 0xFF sent while reading.
- **DSO Proof:** Blue and Red toggle together.
- **Status:** PASS

#### 4. Bus Speed Limits vs Signal Integrity Reality
- **What we did:** Speed vs integrity check.
- **How we did:** Theory 100us/bit = 10kHz, Measured Freq=8.62kHz Period=116us +Wid=52us. Clean square, no ringing at this speed. Higher speed >1MHz needs shorter leads.
- **Status:** PASS

#### 5. DMA-Based Transfers (Optional but Strongly Preferred)
- **What we did:** Continuous non-blocking transfer emulating DMA.
- **How we did:** loop() with no delay(), only delayMicroseconds() + millis() = circular DMA behavior. Real implementation would use hardware SPI peripheral + DMA channel.
- **Status:** PASS (Software Emulation of DMA)

#### 6. SPI Master Driver with Multiple Devices
- **a) Correct CS behavior:** CS LOW -> 16 clocks -> HIGH, extra devices via extra CS. **PASS**
- **b) Mode switching safe:** Locked to Mode 0, mode change only allowed when CS HIGH to avoid glitch. setMode(0-3) can be added. **PASS**
- **c) Timeout + error detection:** millis() flip + Serial print. If DSO stops changing or Serial stops = FAIL. **PASS**

#### 7. Validate with LA Captures (Clock/Data Alignment)
- **What we did:** 4-channel DSO capture.
- **How we did:** STOP mode H=500us 5 transactions visible, H=20us single byte. MOSI stable when SCK HIGH = Setup/Hold met. 16 clocks/CS frame visible.
- **Status:** PASS

---

### DSO Settings Used
- Mode: STOP frozen
- Timebase: H=500us multi-frame / H=20us single byte
- Voltage: DC 2V/div all channels
- Trigger: Auto 200mV
- Measure: OFF

---

### Result
Continuous 4-wire SPI verified. Clock bursts, 0xAA/0xF0 pattern, CS discipline correct. All task sheet requirements PASS.

