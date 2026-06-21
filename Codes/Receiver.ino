#include <SPI.h>
#include <LoRa.h>
#include <ESP32Servo.h>

// =====================================================
// LORA PINS
// =====================================================
#define SS      13
#define RST     27
#define DIO0    26

// =====================================================
// OUTPUT PINS
// =====================================================
#define SERVO1_PIN   25
#define SERVO2_PIN   33

#define ESC_PIN      14
#define DC_PWM_PIN   12

#define FAILSAFE_TIME 1000

Servo servo1;
Servo servo2;

String receivedData = "";

int throttleValue = 1000;
int servo1Value = 90;
int servo2Value = 90;

unsigned long lastReceiveTime = 0;

void setup()
{
    Serial.begin(115200);

    // =====================================================
    // LoRa
    // =====================================================
    SPI.begin(18, 19, 23, SS);

    LoRa.setPins(SS, RST, DIO0);

    if (!LoRa.begin(433E6))
    {
        Serial.println("LoRa Failed!");
        while (1);
    }

    Serial.println("RX READY");

    LoRa.receive();

    // =====================================================
    // Servos
    // =====================================================
    servo1.attach(SERVO1_PIN);
    servo2.attach(SERVO2_PIN);

    // =====================================================
    // PWM Outputs
    // =====================================================
    ledcAttach(DC_PWM_PIN, 5000, 8);
    ledcAttach(ESC_PIN, 5000, 8);

    // =====================================================
    // Safe Start
    // =====================================================
    ledcWrite(DC_PWM_PIN, 0);
    ledcWrite(ESC_PIN, 0);

    delay(3000);

    Serial.println("System Ready");
}

void loop()
{
    int packetSize = LoRa.parsePacket();

    if (packetSize)
    {
        receivedData = "";

        while (LoRa.available())
        {
            receivedData += (char)LoRa.read();
        }

        // =====================================================
        // Bind Request
        // =====================================================
        if (receivedData == "BIND_REQUEST")
        {
            LoRa.idle();

            LoRa.beginPacket();
            LoRa.print("RX_CONNECTED");
            LoRa.endPacket(true);

            delay(200);

            LoRa.receive();
            return;
        }

        // =====================================================
        // Parse Data
        // =====================================================
        sscanf(receivedData.c_str(),
               "%d,%d,%d",
               &throttleValue,
               &servo1Value,
               &servo2Value);

        lastReceiveTime = millis();

        // =====================================================
        // Servos
        // =====================================================
        servo1.write(servo1Value);
        servo2.write(servo2Value);

        // =====================================================
        // PWM Value (0-255)
        // =====================================================
        int pwmValue = map(
            throttleValue,
            0,
            4095,
            0,
            255);

        pwmValue = constrain(
            pwmValue,
            0,
            255);

        // =====================================================
        // DC Motor PWM
        // =====================================================
        ledcWrite(DC_PWM_PIN, pwmValue);

        // =====================================================
        // ESC PWM (same as DC motor)
        // =====================================================
        ledcWrite(ESC_PIN, pwmValue);

        // =====================================================
        // Serial Monitor
        // =====================================================
        Serial.print("RAW=");
        Serial.print(throttleValue);

        Serial.print(" PWM=");
        Serial.print(pwmValue);

        Serial.print(" S1=");
        Serial.print(servo1Value);

        Serial.print(" S2=");
        Serial.println(servo2Value);

        LoRa.receive();
    }

    // =====================================================
    // FAILSAFE
    // =====================================================
    if (millis() - lastReceiveTime > FAILSAFE_TIME)
    {
        ledcWrite(DC_PWM_PIN, 0);
        ledcWrite(ESC_PIN, 0);
    }
}