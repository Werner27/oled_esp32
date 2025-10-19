// ===================== main.cpp =====================
#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "OledMenu.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);
OledMenu menu(&display);

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  analogReadResolution(12);
  menu.begin();
}

void loop() {
  menu.update();
  menu.draw();
  delay(10);
}
