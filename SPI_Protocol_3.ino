#include <SPI.h>
#define CS_DEV1 4
#define CS_DEV2 10
#define TIMEOUT_MS 100

enum { SPI_FAIL = 0, SPI_PASS = 1 };

int spi_transfer_with_timeout(uint8_t csPin, uint8_t spiMode, uint32_t spiClock, uint8_t *tx, uint8_t *rx, uint8_t len) {
  digitalWrite(csPin, HIGH);
  SPI.beginTransaction(SPISettings(spiClock, MSBFIRST, spiMode));
  digitalWrite(csPin, LOW);
  unsigned long start = millis();
  for (uint8_t i = 0; i < len; i++) {
    if (millis() - start > TIMEOUT_MS) {
      digitalWrite(csPin, HIGH);
      SPI.endTransaction();
      return SPI_FAIL;
    }
    uint8_t txByte = tx? tx[i] : 0xFF;
    uint8_t rxByte = SPI.transfer(txByte);
    if (rx) rx[i] = rxByte;
  }
  digitalWrite(csPin, HIGH);
  SPI.endTransaction();
  return SPI_PASS;
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  pinMode(CS_DEV1, OUTPUT);
  pinMode(CS_DEV2, OUTPUT);
  digitalWrite(CS_DEV1, HIGH);
  digitalWrite(CS_DEV2, HIGH);
}

void loop() {
  static unsigned long lastRun = 0;
  if (millis() - lastRun < 2000) return;
  lastRun = millis();

  uint8_t tx1[] = {0x9F};
  uint8_t rx1[3] = {0};
  uint8_t tx2[] = {0x55, 0xAA};
  uint8_t rx2[2] = {0};

  int r1 = spi_transfer_with_timeout(CS_DEV1, SPI_MODE0, 4000000, tx1, rx1, 1);
  int r2 = spi_transfer_with_timeout(CS_DEV2, SPI_MODE3, 1000000, tx2, rx2, 2);

  if (r1 == SPI_PASS) Serial.println("DEV1 MODE0 4MHz: PASS");
  else Serial.println("DEV1 MODE0 4MHz: FAIL");

  if (r2 == SPI_PASS) Serial.println("DEV2 MODE3 1MHz: PASS");
  else Serial.println("DEV2 MODE3 1MHz: FAIL");

  if (r1 == SPI_PASS && r2 == SPI_PASS) Serial.println("RESULT: PASS - SPI Communication OK");
  else Serial.println("RESULT: FAIL");

  Serial.println("---");
}
