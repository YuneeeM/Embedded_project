#include <SoftwareSerial.h>
SoftwareSerial BTSerial(7, 8);

#include <Wire.h>
#include "paj7620.h"
#include <LiquidCrystal_I2C.h>

#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_GFX.h>

#define PIN_NEO 2
#define NUMLED 12

#define PIN_NEOMATRIX 6
#define NUMPIXELS 64

Adafruit_NeoPixel NeoPixel = Adafruit_NeoPixel(NUMLED, PIN_NEO, NEO_GRBW + NEO_KHZ800);

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, PIN_NEOMATRIX,
                                               NEO_MATRIX_TOP + NEO_MATRIX_RIGHT + NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
                                               NEO_GRB + NEO_KHZ800);

const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255)
};

char mode;
unsigned long time_pre, time_cur;

int ledBright = 255;
int soundSensor = A0;
int count = 0;
int threshold = 500;

int x = matrix.width();
int pass = 0;

uint8_t data = 0, data1 = 0, error;

void Gesture() {
  if (!error) {
    paj7620ReadReg(0x43, 1, &data);
    if (data == GES_RIGHT_FLAG) {
      paj7620ReadReg(0x43, 1, &data);
      Serial.println("Right");

      matrix.clear();
      matrix.drawCircle(4, 4, 3, matrix.Color(255, 0, 0));
      matrix.show();
      delay(100);
    } else if (data == GES_LEFT_FLAG) {
      paj7620ReadReg(0x43, 1, &data);
      Serial.println("Left");

      matrix.clear();
      matrix.drawRect(1, 1, 6, 6, matrix.Color(0, 255, 0));
      matrix.show();
      delay(100);
    } else if (data == GES_UP_FLAG) {
      paj7620ReadReg(0x43, 1, &data);
      Serial.println("UP");

      matrix.clear();
      matrix.drawLine(1, 1, 6, 6, matrix.Color(100, 0, 255));
      matrix.show();
      delay(100);
    } else if (data == GES_DOWN_FLAG) {
      paj7620ReadReg(0x43, 1, &data);
      Serial.println("Down");

      matrix.clear();
      matrix.drawLine(6, 1, 1, 6, matrix.Color(255, 255, 255));
      matrix.show();
      delay(100);
    }
    delay(100);
  }
}


void Sign() {
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.setTextColor(colors[0]);

  matrix.fillScreen(0);
  matrix.setCursor(x, 0);
  matrix.print(F("Cheers!"));
  if (--x < -64) {
    x = matrix.width();
    if (++pass >= 5) pass = 0;
    matrix.setTextColor(colors[pass]);
  }
  matrix.show();
  delay(100);
}


void Sound() {
  int i;
  uint32_t colorValue;

  //사운드 센서 감지
  if (analogRead(soundSensor) >= threshold) {
    // 감지된 소리의 크기가 500 이상일 때
    if (count % 3 == 0) {
      colorValue = NeoPixel.Color(255, 255, 255, 0);

      for (i = 0; i < NUMLED; i++) {
        NeoPixel.setPixelColor(i, colorValue);
        NeoPixel.show();
        delay(80);
      }
    } else if (count % 3 == 1) {
      colorValue = NeoPixel.Color(255, 0, 255, 0);

      for (i = 0; i < NUMLED; i++) {
        NeoPixel.setPixelColor(i, colorValue);
        NeoPixel.show();
        delay(80);
      }
    } else if (count % 3 == 2) {
      colorValue = NeoPixel.Color(255, 255, 255, 255);

      for (i = 0; i < NUMLED; i++) {
        NeoPixel.setPixelColor(i, colorValue);
        NeoPixel.show();
        delay(80);
      }
    }
    if (count < 6)
      count++;
    else
      count = 0;
    delay(200);
  } else
    delay(1);

  return;
}

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);

  matrix.begin();
  matrix.clear();
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.setTextColor(colors[0]);

  NeoPixel.setBrightness(ledBright);
  NeoPixel.begin();
  NeoPixel.show();

  pinMode(2, OUTPUT);
  pinMode(soundSensor, INPUT);
}


void loop() {
  if (BTSerial.available()) {
    mode = BTSerial.read();

    switch (mode) {
      case '1':
        Serial.println(mode);

        matrix.begin();
        matrix.setTextWrap(false);

        matrix.setCursor(1, 1);
        matrix.setTextColor((matrix.Color(255, 255, 255)));
        matrix.print("G");
        matrix.show();

        error = paj7620Init();
        if (error) {
          Serial.print("INIT ERROR,CODE:");
          Serial.println(error);
        } else {
          Serial.println("INIT OK");
        }
        Serial.println("Please input your gestures:\n");

        error = paj7620ReadReg(0x43, 1, &data);

        for (int i = 0; i < 100; i++) {
          Gesture();
        }
        Serial.println("hi");
        matrix.clear();
        break;

      case '2':
        Serial.println(mode);
        matrix.clear();

        for (int i = 0; i < 100; i++) {
          Sign();
        }
        break;

      case '3':
        Serial.println(mode);

        if(BTSerial.read() == 'a') {
          int ledBright = BTSerial.parseInt();
          analogWrite(2, ledBright);

          Serial.print(ledBright);
          Serial.print('\n');
        }

        for (int i = 0; i < 50; i++) {
          Sound();
        }
        break;
    }
  }
}
