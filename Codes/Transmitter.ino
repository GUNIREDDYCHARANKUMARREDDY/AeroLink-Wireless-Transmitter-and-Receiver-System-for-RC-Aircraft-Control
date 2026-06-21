#include <SPI.h>
#include <LoRa.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// =====================================================
// TFT PINS
// =====================================================
#define TFT_CS    5
#define TFT_RST   4
#define TFT_DC    2

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// =====================================================
// LORA PINS
// =====================================================
#define SS     13
#define RST    27
#define DIO0   26

// =====================================================
// LEFT JOYSTICK
// THROTTLE
// =====================================================
#define LEFT_X   33
#define LEFT_Y   34

// =====================================================
// RIGHT JOYSTICK
// SERVO CONTROL
// =====================================================
#define RIGHT_X  32
#define RIGHT_Y  35

// =====================================================
// VARIABLES
// =====================================================
int throttleValue = 1000;

int servo1Value = 90;

int servo2Value = 90;

bool rxConnected = false;

// =====================================================
// DEADZONE FUNCTION
// =====================================================
int applyDeadzone(int value, int center, int threshold) {

  if (abs(value - center) < threshold) {

    return center;
  }

  return value;
}

// =====================================================
// SETUP
// =====================================================
void setup() {

  Serial.begin(115200);

  // =====================================================
  // TFT INIT
  // =====================================================
  tft.initR(INITR_BLACKTAB);

  tft.fillScreen(ST77XX_BLACK);

  tft.setTextSize(1);

  tft.setTextColor(ST77XX_GREEN);

  tft.setCursor(10, 10);

  tft.println("Binding RX...");

  // =====================================================
  // SPI START
  // =====================================================
  SPI.begin(18, 19, 23, SS);

  // =====================================================
  // LoRa START
  // =====================================================
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(433E6)) {

    tft.fillScreen(ST77XX_BLACK);

    tft.setTextColor(ST77XX_RED);

    tft.setCursor(10, 20);

    tft.println("LoRa Failed");

    while (1);
  }

  LoRa.receive();

  // =====================================================
  // BIND PROCESS
  // =====================================================
  unsigned long startTime = millis();

  while (millis() - startTime < 8000) {

    LoRa.idle();

    LoRa.beginPacket();

    LoRa.print("BIND_REQUEST");

    LoRa.endPacket(true);

    LoRa.receive();

    Serial.println("Bind Request Sent");

    delay(500);

    int packetSize = LoRa.parsePacket();

    if (packetSize) {

      String response = "";

      while (LoRa.available()) {

        response += (char)LoRa.read();
      }

      if (response == "RX_CONNECTED") {

        rxConnected = true;

        break;
      }
    }

    delay(500);
  }

  // =====================================================
  // RX NOT CONNECTED
  // =====================================================
  if (!rxConnected) {

    tft.fillScreen(ST77XX_BLACK);

    tft.setTextColor(ST77XX_RED);

    tft.setCursor(10, 20);

    tft.println("RX NOT");

    tft.setCursor(10, 35);

    tft.println("CONNECTED");

    while (1);
  }

  // =====================================================
  // CONNECTED
  // =====================================================
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_GREEN);

  tft.setCursor(10, 10);

  tft.println("RX CONNECTED");

  delay(1000);

  // =====================================================
  // STATIC LABELS
  // =====================================================
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_CYAN);

  tft.setCursor(5, 5);

  tft.println("TX ACTIVE");

  tft.setTextColor(ST77XX_WHITE);

  tft.setCursor(5, 25);

  tft.println("THR:");

  tft.setCursor(5, 45);

  tft.println("SERVO1:");

  tft.setCursor(5, 65);

  tft.println("SERVO2:");
}

// =====================================================
// LOOP
// =====================================================
void loop() {

//=====================================================
//READ JOYSTICKS
//=====================================================
int joy1 = analogRead(LEFT_Y);      // GPIO34
int joy2 = analogRead(RIGHT_Y);     // GPIO35
int throttleRaw = analogRead(LEFT_X); // GPIO33

// Invert throttle
throttleValue = 4095 - throttleRaw;

// =====================================================
// SERVO1
// =====================================================
if (joy1 < 1000)
{
    servo1Value = 0;
}
else if (joy1 > 3000)
{
    servo1Value = 180;
}
else
{
    servo1Value = 90;
}

// =====================================================
// SERVO2
// =====================================================
if (joy2 < 1000)
{
    servo2Value = 0;
}
else if (joy2 > 3000)
{
    servo2Value = 180;
}
else
{
    servo2Value = 90;
}

  // =====================================================
  // DATA STRING
  // =====================================================
  String data = String(throttleValue) + "," +
                String(servo1Value) + "," +
                String(servo2Value);

  // =====================================================
  // SEND DATA
  // =====================================================
  LoRa.idle();

  LoRa.beginPacket();

  LoRa.print(data);

  LoRa.endPacket(true);

  // =====================================================
  // SERIAL MONITOR
  // =====================================================
  Serial.print("THR: ");

  Serial.print(throttleValue);

  Serial.print(" S1: ");

  Serial.print(servo1Value);

  Serial.print(" S2: ");

  Serial.println(servo2Value);

  // =====================================================
  // UPDATE TFT
  // =====================================================
  tft.fillRect(60, 25, 60, 10, ST77XX_BLACK);

  tft.fillRect(60, 45, 60, 10, ST77XX_BLACK);

  tft.fillRect(60, 65, 60, 10, ST77XX_BLACK);

  tft.setTextColor(ST77XX_YELLOW);

  tft.setCursor(60, 25);

  tft.print(throttleValue);

  tft.setCursor(60, 45);

  tft.print(servo1Value);

  tft.setCursor(60, 65);

  tft.print(servo2Value);

  delay(50);
}

