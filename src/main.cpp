#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "OledMenu.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE); // OLED-Displayobjekt
OledMenu menu(&display); // Menüobjekt mit Displayreferenz

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);                 // I2C-Pins für ESP32
  analogReadResolution(12);           // 12-Bit ADC-Auflösung
  menu.begin(100);                       // Menü initialisieren
}

void loop() {
  // Hauptloop bleibt leer, da Menü über Timer läuft 
}
 
