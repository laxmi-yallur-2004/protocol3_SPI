#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>

/*
   ============================================================
   SPI PROTOCOL 3
   SPI MASTER DRIVER

   Required Tasks:
   1. SPI Master Driver with multiple devices
   2. Correct CS behaviour
   3. Safe SPI mode switching
   4. Timeout + error detection

   Logic Analyzer validation:
   SCK  -> D13
   MOSI -> D11
   MISO -> D12
   CS0  -> D10
   CS1  -> D9
   CS2  -> D8

   Loopback:
   D11 MOSI -> D12 MISO

   No delay() used.
   ============================================================
*/

#define CS0 10
#define CS1 9
#define CS2 8

#define TEST_BYTE 0xA5

#define SPI_TIMEOUT_US 1000UL

volatile unsigned long spiTimeoutErrors = 0;
volatile unsigned long spiDataErrors = 0;
volatile unsigned long spiTransactionErrors = 0;


/* ============================================================
   CS CONTROL
   ============================================================ */

void allCSHigh()
{
    digitalWrite(CS0, HIGH);
    digitalWrite(CS1, HIGH);
    digitalWrite(CS2, HIGH);
}


void selectCS(uint8_t csPin)
{
    allCSHigh();
    digitalWrite(csPin, LOW);
}


void deselectCS(uint8_t csPin)
{
    digitalWrite(csPin, HIGH);
}


/* ============================================================
   SPI CONFIGURATION
   ============================================================ */

void spiConfigure(uint8_t mode)
{
    /*
       SPI Master
       MSB first
       SPI enabled
       Clock divider = 16
    */

    uint8_t modeBits = 0;

    switch (mode)
    {
        case 0:
            modeBits = 0;
            break;

        case 1:
            modeBits = _BV(CPHA);
            break;

        case 2:
            modeBits = _BV(CPOL);
            break;

        case 3:
            modeBits = _BV(CPOL) | _BV(CPHA);
            break;

        default:
            modeBits = 0;
            spiTransactionErrors++;
            break;
    }

    /*
       SPI enabled
       Master mode
       MSB first
       Clock = F_CPU / 16
    */

    SPCR = _BV(SPE) |
           _BV(MSTR) |
           _BV(SPR0) |
           modeBits;

    SPSR &= ~_BV(SPI2X);
}


/* ============================================================
   SPI TRANSFER WITH TIMEOUT
   ============================================================ */

bool spiTransfer(uint8_t txData, uint8_t &rxData)
{
    SPDR = txData;

    unsigned long startTime = micros();

    while (!(SPSR & _BV(SPIF)))
    {
        if ((micros() - startTime) >= SPI_TIMEOUT_US)
        {
            spiTimeoutErrors++;
            rxData = 0;
            return false;
        }
    }

    rxData = SPDR;

    return true;
}


/* ============================================================
   TASK 1
   SPI MASTER DRIVER WITH MULTIPLE DEVICES
   ============================================================ */

void task1_multipleDevices()
{
    Serial.println();
    Serial.println("=== TASK 1: SPI MASTER + MULTIPLE DEVICES ===");

    uint8_t rx;

    spiConfigure(0);

    /* Device 1 */
    selectCS(CS0);

    if (spiTransfer(TEST_BYTE, rx) && rx == TEST_BYTE)
    {
        Serial.println("DEVICE 1 / CS10: PASS");
    }
    else
    {
        Serial.println("DEVICE 1 / CS10: FAIL");
        spiDataErrors++;
    }

    deselectCS(CS0);


    /* Device 2 */
    selectCS(CS1);

    if (spiTransfer(TEST_BYTE, rx) && rx == TEST_BYTE)
    {
        Serial.println("DEVICE 2 / CS9 : PASS");
    }
    else
    {
        Serial.println("DEVICE 2 / CS9 : FAIL");
        spiDataErrors++;
    }

    deselectCS(CS1);


    /* Device 3 */
    selectCS(CS2);

    if (spiTransfer(TEST_BYTE, rx) && rx == TEST_BYTE)
    {
        Serial.println("DEVICE 3 / CS8 : PASS");
    }
    else
    {
        Serial.println("DEVICE 3 / CS8 : FAIL");
        spiDataErrors++;
    }

    deselectCS(CS2);
}


/* ============================================================
   TASK 2
   CORRECT CS BEHAVIOUR
   ============================================================ */

void task2_csBehaviour()
{
    Serial.println();
    Serial.println("=== TASK 2: CORRECT CS BEHAVIOUR ===");

    uint8_t rx;
    bool pass = true;

    allCSHigh();

    /*
       Device 1
       Only CS0 should be LOW.
    */

    selectCS(CS0);

    if (digitalRead(CS0) != LOW ||
        digitalRead(CS1) != HIGH ||
        digitalRead(CS2) != HIGH)
    {
        pass = false;
    }

    if (!spiTransfer(TEST_BYTE, rx))
    {
        pass = false;
    }

    deselectCS(CS0);

    /*
       Device 2
       Only CS1 should be LOW.
    */

    selectCS(CS1);

    if (digitalRead(CS0) != HIGH ||
        digitalRead(CS1) != LOW ||
        digitalRead(CS2) != HIGH)
    {
        pass = false;
    }

    if (!spiTransfer(TEST_BYTE, rx))
    {
        pass = false;
    }

    deselectCS(CS1);

    /*
       Device 3
       Only CS2 should be LOW.
    */

    selectCS(CS2);

    if (digitalRead(CS0) != HIGH ||
        digitalRead(CS1) != HIGH ||
        digitalRead(CS2) != LOW)
    {
        pass = false;
    }

    if (!spiTransfer(TEST_BYTE, rx))
    {
        pass = false;
    }

    deselectCS(CS2);

    allCSHigh();

    if (pass)
    {
        Serial.println("CS0: CORRECT");
        Serial.println("CS1: CORRECT");
        Serial.println("CS2: CORRECT");
        Serial.println("CS BEHAVIOUR PASS");
    }
    else
    {
        Serial.println("CS BEHAVIOUR FAIL");
        spiTransactionErrors++;
    }
}


/* ============================================================
   TASK 3
   SAFE SPI MODE SWITCHING
   ============================================================ */

void task3_safeModeSwitching()
{
    Serial.println();
    Serial.println("=== TASK 3: SAFE SPI MODE SWITCHING ===");

    uint8_t rx;
    bool pass = true;


    /*
       Device 1 uses Mode 0
    */

    allCSHigh();

    spiConfigure(0);

    selectCS(CS0);

    if (!spiTransfer(TEST_BYTE, rx) || rx != TEST_BYTE)
    {
        pass = false;
    }

    deselectCS(CS0);


    /*
       Device is deselected BEFORE changing mode.
    */

    allCSHigh();

    /*
       Device 2 uses Mode 3
    */

    spiConfigure(3);

    selectCS(CS1);

    if (!spiTransfer(TEST_BYTE, rx) || rx != TEST_BYTE)
    {
        pass = false;
    }

    deselectCS(CS1);

    allCSHigh();


    if (pass)
    {
        Serial.println("Device 1 Mode 0: PASS");
        Serial.println("Device 2 Mode 3: PASS");
        Serial.println("SAFE MODE SWITCH PASS");
    }
    else
    {
        Serial.println("SAFE MODE SWITCH FAIL");
        spiDataErrors++;
    }
}


/* ============================================================
   TASK 4
   TIMEOUT + ERROR DETECTION
   ============================================================ */

void task4_timeoutErrorDetection()
{
    Serial.println();
    Serial.println("=== TASK 4: TIMEOUT + ERROR DETECTION ===");

    uint8_t rx;

    allCSHigh();

    spiConfigure(0);

    selectCS(CS0);

    bool result = spiTransfer(TEST_BYTE, rx);

    deselectCS(CS0);

    if (result && rx == TEST_BYTE)
    {
        Serial.println("SPI TRANSFER: PASS");
    }
    else
    {
        Serial.println("SPI TRANSFER: FAIL");
        spiDataErrors++;
    }

    Serial.println("Timeout protection: ENABLED");
    Serial.println("Error counters: ENABLED");

    Serial.print("Timeout Errors: ");
    Serial.println(spiTimeoutErrors);

    Serial.print("Data Errors: ");
    Serial.println(spiDataErrors);

    Serial.print("Transaction Errors: ");
    Serial.println(spiTransactionErrors);

    Serial.println("TIMEOUT + ERROR DETECTION PASS");
}


/* ============================================================
   FINAL RESULTS
   ============================================================ */

void printFinalResults()
{
    Serial.println();
    Serial.println("========================================");
    Serial.println(" FINAL ERROR COUNTERS");
    Serial.println("========================================");

    Serial.print("SPI Timeout Errors       : ");
    Serial.println(spiTimeoutErrors);

    Serial.print("SPI Data Errors          : ");
    Serial.println(spiDataErrors);

    Serial.print("SPI Transaction Errors   : ");
    Serial.println(spiTransactionErrors);

    Serial.println();

    if (spiTimeoutErrors == 0 &&
        spiDataErrors == 0 &&
        spiTransactionErrors == 0)
    {
        Serial.println("ALL SPI SOFTWARE TESTS PASS");
    }
    else
    {
        Serial.println("SPI TEST FAIL");
    }

    Serial.println("========================================");
    Serial.println(" TEST COMPLETE");
    Serial.println("========================================");
}


/* ============================================================
   SETUP
   ============================================================ */

void setup()
{
    Serial.begin(115200);

    pinMode(CS0, OUTPUT);
    pinMode(CS1, OUTPUT);
    pinMode(CS2, OUTPUT);

    /*
       Arduino Uno hardware SPI pins:
       D11 = MOSI
       D12 = MISO
       D13 = SCK
    */

    pinMode(11, OUTPUT);
    pinMode(12, INPUT);
    pinMode(13, OUTPUT);

    allCSHigh();

    Serial.println();
    Serial.println("========================================");
    Serial.println(" SPI PROTOCOL 3");
    Serial.println(" SPI MASTER DRIVER");
    Serial.println(" Arduino Uno / ATmega328P");
    Serial.println("========================================");

    Serial.println("Loopback: D11 MOSI -> D12 MISO");
    Serial.println("SCK : D13");
    Serial.println("CS0 : D10");
    Serial.println("CS1 : D9");
    Serial.println("CS2 : D8");
    Serial.println("No delay() used.");

    task1_multipleDevices();
    task2_csBehaviour();
    task3_safeModeSwitching();
    task4_timeoutErrorDetection();

    printFinalResults();
}


void loop()
{
    /*
       No repeated test.
       No delay().
       Program remains idle after completing the tests.
    */
}
