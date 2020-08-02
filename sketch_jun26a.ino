#include <Math.h>
int ledPins[] = {22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37};
int muxPins[] = {38, 39, 40, 41};
int muxRead = 0;
int statusLED = 11;


double values[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int sendCounter[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int sendErrorThreshold = 34;

long previousMillis = 0;
long interval = 300;

void setup() {
  for (int i = 0; i < 16; i++) {
    analogWrite(ledPins[i], 255);
    delay(90);
  }
  for (int i = 0; i < 16; i++) {
    analogWrite(ledPins[i], 0);
    delay(90);
  }
  for (int i = 255; i > 0; i--) {
    analogWrite(statusLED, i);
    delay(5);
  }
  Serial.begin(115200);
  digitalWrite(38, 1);
  digitalWrite(39, 0);
  digitalWrite(40, 0);
  digitalWrite(41, 0);
  while (true) {
    analogRead(muxRead);
    Serial.println(analogRead(muxRead));
    delay(100);
  }
}

void loop() {
  unsigned long currentMillis = millis();

  for (int i = 0; i < 15; i++) {
    setMuxAddress((byte) i);
    analogRead(muxRead);
    double value = analogRead(muxRead);

    if (abs(values[i] - value) >= 4) {
      if (value <= 5) {
        value = 0;
      }
      if (value >= 1020) {
        value = 1023;
      }

      digitalWrite(ledPins[i], HIGH);
      values[i] = value;
      value = value / 1023;
      value = value * 100;
      byte coarse = getCoarseValue(value);
      byte fine = getFineValue(value);

      sendFader((byte) i, 0x01, coarse, fine);
      delay(20);
    } else {
      sendCounter[i]--;
      if (sendCounter[i] < 0) {
        sendCounter[i] = 1;
      }
    }

    analogWrite(ledPins[i], 0);
    if (currentMillis - previousMillis > interval) {
      if (abs(sendCounter[i] - sendErrorThreshold) >= sendErrorThreshold) {
        if (sendCounter[i] != 0) {
          analogWrite(statusLED, 150);
          sendCounter[i]--;
        }
      } else {
        analogWrite(statusLED, 0);
      }
    }

  }
  delay(30);
}


void sendFader(int i, byte page, byte coarse_value, byte fine_value) {
  byte fader = (byte) i;
  byte message[] = {
    0xF0, 0x7F, //Begin Command (DO NOT CHANGE)

    0x7F, //Device ID. Use 00-6F for an idividual ID, 70-7E as 1-15 Group ID, and 7F for Broadcast

    0x02, //Sets it as MSC Message ( DO NOT CHANGE)

    0x7F, //Command Format 01: General Light, 02: Moving Light, 7F: All
    0x06, //Command: 06: Set

    fader, // Fader. 00 is Fader 1, 01 is Fader 2, ... 0E (Value 14) is Fader 15
    page, // Page number. 01 is Page 1, 02 is Page 2 ...
    fine_value, // Fader Fine Value. (Raw value is 76)
    coarse_value, // Fader Course Value. (Raw value is 57)

    0x00,// Time in Hours
    0x00, // Time in Minutes
    0x00, // Time in Seconds
    0x7F, // Time in Frame
    0x00, // Time in Fraction



    0xF7 //End MSC Message (DO NOT CHANGE)
  };
  Serial.write(message, sizeof(message));
  sendCounter[i]++;
}

float getCoarseValue(int number) {
  float f = number * 1.27;
  int coarseValue = (int) f;
  return coarseValue;
}

int getFineValue(int number) {
  if (number == 100) {
    return 127;
  }
  number = number * 1.27;
  int I = (int)number;
  number -= (float) I;
  int fineValue = number * 127;
  return fineValue;
}

void setMuxAddress(byte n)
{
  for (byte i = 0; i < 4; i++) {
    digitalWrite(muxPins[i], n & 1);
    n /= 2;
  }
}
