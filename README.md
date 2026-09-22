# SPI Protocol 3 — SPI Master Driver

## 1. Project Objective

This project implements an **SPI Master Driver** on an Arduino Uno using the ATmega328P hardware SPI peripheral.

The implementation covers four required tasks:

1. SPI Master Driver with multiple devices
2. Correct Chip Select (CS) behaviour
3. Safe SPI mode switching
4. Timeout and error detection

The SPI signals can be verified using a **Logic Analyzer**.

No `delay()` function is used in the code.

---

## 2. Hardware

### Arduino Board

* Arduino Uno
* ATmega328P
* CPU frequency: 16 MHz

### SPI Connections

| Arduino Pin | SPI Signal | Purpose              |
| ----------- | ---------- | -------------------- |
| D13         | SCK        | SPI Clock            |
| D11         | MOSI       | Master Out Slave In  |
| D12         | MISO       | Master In Slave Out  |
| D10         | CS0        | Device 1 Chip Select |
| D9          | CS1        | Device 2 Chip Select |
| D8          | CS2        | Device 3 Chip Select |

### Loopback Connection

For the software transfer test:

```text
D11 (MOSI) ───────── D12 (MISO)
```

This connects the transmitted SPI data back to the Arduino so that the received byte can be checked.

### USB

The Arduino USB connection is used for:

* Uploading the firmware
* Viewing Serial Monitor results

Serial communication:

```text
Baud Rate: 115200
```

---

# 3. Task 1 — SPI Master Driver with Multiple Devices

## What was built

The Arduino is configured as an **SPI Master**.

Three Chip Select lines are provided:

```text
CS0 → D10 → Device 1
CS1 → D9  → Device 2
CS2 → D8  → Device 3
```

The SPI driver supports:

* SPI Master mode
* MSB-first transmission
* SPI clock divider of 16
* SPI Modes 0, 1, 2 and 3
* SPI data transfer using the ATmega328P SPI hardware

The test byte used is:

```text
0xA5
```

Binary representation:

```text
10100101
```

With the MOSI-to-MISO loopback connection, the transmitted `0xA5` should be received back as `0xA5`.

## How it works

For each device:

```text
Select CS
     ↓
Send 0xA5
     ↓
Receive data
     ↓
Compare received data with 0xA5
     ↓
Deselect CS
```

The program tests Device 1, Device 2 and Device 3 separately.

## Expected result

```text
DEVICE 1 / CS10: PASS
DEVICE 2 / CS9 : PASS
DEVICE 3 / CS8 : PASS
```

---

# 4. Task 2 — Correct CS Behaviour

## What was implemented

The driver ensures that only the required device is selected during an SPI transaction.

Before selecting a device:

```text
CS0 = HIGH
CS1 = HIGH
CS2 = HIGH
```

When Device 1 is selected:

```text
CS0 = LOW
CS1 = HIGH
CS2 = HIGH
```

When Device 2 is selected:

```text
CS0 = HIGH
CS1 = LOW
CS2 = HIGH
```

When Device 3 is selected:

```text
CS0 = HIGH
CS1 = HIGH
CS2 = LOW
```

After the transaction, the selected CS line is returned HIGH.

## CS sequence

The required transaction sequence is:

```text
CS LOW
   ↓
SPI CLOCK + DATA
   ↓
CS HIGH
```

The function `selectCS()` first makes all CS lines HIGH and then pulls the requested CS line LOW.

This prevents multiple SPI devices from being selected at the same time.

## Software verification

The code uses `digitalRead()` to verify the CS states.

Expected result:

```text
CS0: CORRECT
CS1: CORRECT
CS2: CORRECT
CS BEHAVIOUR PASS
```

---

# 5. Task 3 — Safe SPI Mode Switching

SPI supports four clock modes.

| SPI Mode | CPOL | CPHA |
| -------- | ---: | ---: |
| Mode 0   |    0 |    0 |
| Mode 1   |    0 |    1 |
| Mode 2   |    1 |    0 |
| Mode 3   |    1 |    1 |

The driver can configure all four modes.

## Safe switching method

The current device is first deselected before changing the SPI mode.

The sequence used in the code is:

```text
Device 1 selected
       ↓
SPI Mode 0 transfer
       ↓
Device 1 deselected
       ↓
All CS HIGH
       ↓
Change SPI configuration
       ↓
SPI Mode 3
       ↓
Device 2 selected
       ↓
SPI Mode 3 transfer
       ↓
Device 2 deselected
```

The test specifically checks:

```text
Device 1 → Mode 0
Device 2 → Mode 3
```

Expected result:

```text
Device 1 Mode 0: PASS
Device 2 Mode 3: PASS
SAFE MODE SWITCH PASS
```

---

# 6. Task 4 — Timeout + Error Detection

## Timeout protection

The SPI transfer function does not wait forever for the SPI hardware to complete.

The code waits for the SPI interrupt flag:

```text
SPIF
```

A timeout limit is defined:

```text
SPI_TIMEOUT_US = 1000
```

Therefore, if the SPI transfer does not complete within the defined timeout period, the function reports a timeout instead of waiting indefinitely.

The timeout counter is:

```text
spiTimeoutErrors
```

## Error counters

The code maintains three error counters:

### 1. SPI Timeout Errors

Counts SPI transfers that exceed the timeout limit.

```text
spiTimeoutErrors
```

### 2. SPI Data Errors

Counts cases where the received data does not match the expected test byte.

```text
spiDataErrors
```

### 3. SPI Transaction Errors

Counts invalid SPI configuration or CS-related transaction failures detected by the software tests.

```text
spiTransactionErrors
```

## Expected result

```text
SPI TRANSFER: PASS
Timeout protection: ENABLED
Error counters: ENABLED
Timeout Errors: 0
Data Errors: 0
Transaction Errors: 0
TIMEOUT + ERROR DETECTION PASS
```

---

# 7. SPI Configuration

The SPI hardware is configured using the ATmega328P SPI registers.

The driver configures:

```text
SPI Enabled
Master Mode
MSB First
Clock Divider = 16
```

For the Arduino Uno:

```text
F_CPU = 16 MHz
```

With clock divider 16:

```text
SPI Clock = 16 MHz / 16
          = 1 MHz
```

The SPI mode is changed using the CPOL and CPHA bits.

---

# 8. USB + Serial Monitor Verification

The USB connection is used to upload the program and observe the test results.

### Steps

1. Connect the Arduino Uno to the PC using USB.
2. Connect:

```text
D11 → D12
```

for SPI loopback.
3. Upload `SPI_Protocol_3.ino`.
4. Open Serial Monitor.
5. Set baud rate to:

```text
115200
```

6. Observe the four task results.

The tests run automatically once after reset.

There is no repeated test loop and no `delay()`.

---

# 9. Expected Serial Monitor Output

```text
========================================
 SPI PROTOCOL 3
 SPI MASTER DRIVER
 Arduino Uno / ATmega328P
========================================
Loopback: D11 MOSI -> D12 MISO
SCK : D13
CS0 : D10
CS1 : D9
CS2 : D8
No delay() used.

=== TASK 1: SPI MASTER + MULTIPLE DEVICES ===
DEVICE 1 / CS10: PASS
DEVICE 2 / CS9 : PASS
DEVICE 3 / CS8 : PASS

=== TASK 2: CORRECT CS BEHAVIOUR ===
CS0: CORRECT
CS1: CORRECT
CS2: CORRECT
CS BEHAVIOUR PASS

=== TASK 3: SAFE SPI MODE SWITCHING ===
Device 1 Mode 0: PASS
Device 2 Mode 3: PASS
SAFE MODE SWITCH PASS

=== TASK 4: TIMEOUT + ERROR DETECTION ===
SPI TRANSFER: PASS
Timeout protection: ENABLED
Error counters: ENABLED
Timeout Errors: 0
Data Errors: 0
Transaction Errors: 0
TIMEOUT + ERROR DETECTION PASS

========================================
 FINAL ERROR COUNTERS
========================================
SPI Timeout Errors       : 0
SPI Data Errors          : 0
SPI Transaction Errors   : 0

ALL SPI SOFTWARE TESTS PASS
========================================
 TEST COMPLETE
========================================
```

---

# 10. Logic Analyzer Validation

The Serial Monitor verifies the **software results**.

A Logic Analyzer is required to verify the actual SPI electrical signals and clock/data relationship.

## Logic Analyzer connections

| Arduino | Logic Analyzer |
| ------- | -------------- |
| D13     | SCK / CH0      |
| D11     | MOSI / CH1     |
| D12     | MISO / CH2     |
| D10     | CS0 / CH3      |
| D9      | CS1 / CH4      |
| D8      | CS2 / CH5      |
| GND     | GND            |

## Signals to check

### Clock

Check D13:

```text
SCK → D13
```

There should be SPI clock pulses during an SPI transfer.

### MOSI

Check D11:

```text
MOSI → D11
```

The test byte is:

```text
0xA5 = 10100101
```

### MISO

Check D12:

```text
MISO → D12
```

Because of the loopback:

```text
MOSI → MISO
```

the received data should correspond to the transmitted data.

### Chip Select

Check D10, D9 and D8.

Only the selected CS line should become LOW.

For example:

```text
Device 1:

CS0 = LOW
CS1 = HIGH
CS2 = HIGH
```

The Logic Analyzer should show the SPI clock/data activity while CS0 is LOW.

---

# 11. Clock/Data Alignment Validation

The Logic Analyzer is used to verify the relationship between:

```text
CS
SCK
MOSI
MISO
```

A valid transaction should look conceptually like:

```text
CS
────────────┐                  ┌────────
            └──────────────────┘
             LOW              HIGH

SCK
      _   _   _   _   _   _   _   _
_____| |_| |_| |_| |_| |_| |_| | |____

MOSI
      <------ 8 transmitted bits ------>

MISO
      <------ 8 received bits --------->
```

The Logic Analyzer should confirm:

* CS becomes active before the SPI transfer.
* SPI clock pulses occur while CS is active.
* MOSI contains the transmitted data.
* MISO contains the returned data.
* CS returns HIGH after the transaction.
* Clock/data timing corresponds to the configured SPI mode.

The Serial Monitor alone cannot prove the physical clock/data alignment. That requires the Logic Analyzer waveform.

---

# 12. Error Handling

The firmware does not wait indefinitely for an SPI transfer.

If the SPI completion flag is not detected within the timeout:

```text
Timeout Error
     ↓
spiTimeoutErrors++
     ↓
Transfer returns FAIL
```

If the received data does not match the expected byte:

```text
Data Error
     ↓
spiDataErrors++
```

If an invalid SPI mode or transaction condition is detected:

```text
Transaction Error
     ↓
spiTransactionErrors++
```

This provides basic fault detection and prevents an SPI transfer from becoming an unlimited blocking wait.

---

# 13. Verification Evidence

The project can be demonstrated using two types of evidence.

## Software evidence

Serial Monitor shows:

```text
DEVICE 1 / CS10: PASS
DEVICE 2 / CS9 : PASS
DEVICE 3 / CS8 : PASS

CS BEHAVIOUR PASS

SAFE MODE SWITCH PASS

TIMEOUT + ERROR DETECTION PASS

ALL SPI SOFTWARE TESTS PASS
```

## Hardware evidence

A Logic Analyzer capture should show:

```text
CS
SCK
MOSI
MISO
```

and confirm the actual SPI transaction, CS timing, clock pulses, data and clock/data alignment.

Recommended evidence:

1. Serial Monitor screenshot
2. Logic Analyzer waveform screenshot

---

# 14. Final Result

The SPI Protocol 3 firmware implements the four required areas:

| Requirement                   | Implementation                             |
| ----------------------------- | ------------------------------------------ |
| SPI Master + Multiple Devices | CS0, CS1 and CS2 with SPI master transfers |
| Correct CS Behaviour          | Only one CS selected at a time             |
| Safe SPI Mode Switching       | Device deselected before changing SPI mode |
| Timeout + Error Detection     | Bounded SPI wait and error counters        |

The Arduino Serial Monitor provides the software test result.

The Logic Analyzer provides physical signal-level verification of:

```text
CS + SCK + MOSI + MISO
```

Therefore, the complete validation consists of both **software verification** and **Logic Analyzer signal verification**.

---

## Files

```text
SPI_Protocol_3.ino
README.md
```

No additional SPI tasks are included in this implementation.
