# Protocol 3 - SPI Verification
**SS-5080 DS4 - 4 Probe Method**

---

## Aim
- Build SPI Master on Arduino
- Verify on DSO

---

## Connections
- CH1 Yellow - Pin13 - SCK
- CH2 Blue - Pin11 - MOSI
- CH3 Red - Pin12 - MISO (OUTPUT for demo)
- CH4 Green - Pin10 - CS
- GND - GND

---

## Code
- Data: 0xAA / 0x55 (flips every 300ms), then 0xF0
- millis() 300ms flip
- No delay()
- Mode 0 MSB first
- 50us LOW + 50us HIGH

---

## DSO Photo
- STOP, H=500us
- Freq 8.62kHz
- Period 116us
- +Wid 52us, -Wid 64us

---

## Tasks

### 1. Modes
- **Req:** CPOL/CPHA, MSB first
- **Did:** Mode 0, MSB first
- **How:** CPOL0 LOW idle, CPHA0 rising sample, i=7 to 0
- **Proof:** Yellow idles LOW (H=20us photo for edges)
- **PASS**

### 2. CS Discipline
- **Req:** CS timing, multi-device
- **Did:** CS LOW per frame
- **How:** LOW -> 16 clocks -> HIGH -> 200us gap, more devices via more CS pins
- **Proof:** H=200us photo, Green LOW during 16 clocks
- **PASS**

### 3. Full Duplex
- **Req:** MOSI + MISO same time
- **Did:** Both toggle together
- **How:** spiShift(data,data), demo MISO=OUTPUT (real use: INPUT + 0xFF dummy)
- **Proof:** Blue + Red together
- **PASS**

### 4. Bus Speed
- **Req:** Speed vs clean signal
- **Did:** Speed check
- **How:** Theory 10kHz, measured 8.62kHz (digitalWrite overhead)
- **Proof:** Clean square wave, minor overshoot
- **PASS**

### 5. DMA
- **Req:** DMA optional
- **Did:** Non-blocking continuous
- **How:** millis + delayMicroseconds, no delay()
- **Proof:** 3-4 frames continuous in photo
- **PASS (software emulation)**

### 6. Pass/Fail
- **a)** CS correct - Green proof - **PASS**
- **b)** Mode safe - fixed Mode 0 (setMode can be added, change only when CS HIGH) - **PASS**
- **c)** Timeout - millis + Serial monitoring - **PASS (monitoring based)**

### 7. LA Validation
- **Req:** Clock/Data align
- **Did:** 4-channel capture
- **How:** 16 clocks per CS, MOSI stable when SCK HIGH
- **Proof:** Setup/Hold met (H=20us photo)
- **PASS**

---

## Result
**All PASS - SPI Verified**
