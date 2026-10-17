#define SCK_PIN  13  // CH1 Yellow - SCK
#define MOSI_PIN 11  // CH2 Blue - MOSI
#define MISO_PIN 12  // CH3 Pink - MISO (we drive it for demo)
#define CS_PIN   10  // CH4 Green - CS/SS

byte data1 = 0xAA; // 10101010 - Easy to see on DSO
byte data2 = 0xF0; // 11110000 - Easy to see
unsigned long lastMillis = 0;

void setup() {
  pinMode(SCK_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  pinMode(MISO_PIN, OUTPUT); // For DSO demo we make MISO OUTPUT
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  digitalWrite(SCK_PIN, LOW);
  Serial.begin(9600);
  Serial.println("4-PROBE NO DELAY READY");
}

// Fast shift - No delay() in loop, only microsec for clock
void spiShift(byte mosiData, byte misoData) {
  for (int i=7; i>=0; i--) {
    digitalWrite(SCK_PIN, LOW);
    digitalWrite(MOSI_PIN, (mosiData>>i)&1);
    digitalWrite(MISO_PIN, (misoData>>i)&1);
    delayMicroseconds(50);
    digitalWrite(SCK_PIN, HIGH);
    delayMicroseconds(50);
  }
  digitalWrite(SCK_PIN, LOW);
}

void loop() {
  // Change data every 300ms - Non-blocking
  if (millis() - lastMillis >= 300) {
    lastMillis = millis();
    data1 = ~data1; // Flip 0xAA to 0x55 - DSO shows change
    Serial.print("Sending: 0x");
    Serial.println(data1, HEX);
  }

  // CONTINUOUS SPI - No delay() - DSO sees non-stop
  digitalWrite(CS_PIN, LOW);   // CS LOW = Transfer start
  spiShift(data1, data1);      // 8 clocks - CH1 + CH2 + CH3 toggle
  spiShift(data2, data2);      // 8 clocks - 16 clocks per frame
  digitalWrite(CS_PIN, HIGH);  // CS HIGH = End
  
  delayMicroseconds(200); // Tiny gap to see CS - Not delay()
}
