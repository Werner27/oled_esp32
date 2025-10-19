#include "OledMenu.h"

OledMenu::OledMenu(U8G2 *disp)
  : display(disp), mode(MENU_MAIN), cursorPos(0), lastPress(0), buttonHeld(false), t(0.0), graphIndex(0) {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void OledMenu::begin() {
  display->begin();
  drawSplash();
  drawMainMenu();
}

void OledMenu::drawSplash() {
  display->clearBuffer();
  display->setFont(u8g2_font_ncenB08_tr);
  display->drawStr(20, 30, "Copyright Werner");
  // Einfaches Smile-Logo
  display->drawCircle(64, 48, 8);
  display->drawDisc(61, 46, 1);
  display->drawDisc(67, 46, 1);
  display->drawArc(64, 49, 5, 0, 180);
  display->sendBuffer();
  delay(2000);
}

void OledMenu::update() {
  static bool wasPressed = false;
  bool pressed = (digitalRead(BUTTON_PIN) == LOW);

  if (pressed && !wasPressed) {
    lastPress = millis();
    buttonHeld = false;
  }

  if (pressed && (millis() - lastPress > 1000) && !buttonHeld) {
    buttonHeld = true;
    handleLongPress();
  }

  if (!pressed && wasPressed && !buttonHeld) {
    handleShortPress();
  }

  wasPressed = pressed;
}

void OledMenu::handleShortPress() {
  if (mode == MENU_MAIN) {
    cursorPos++;
    if (cursorPos > 2) cursorPos = 0;
    drawMainMenu();
  } else {
    mode = MENU_MAIN;
    drawMainMenu();
  }
}

void OledMenu::handleLongPress() {
  if (mode == MENU_MAIN) {
    switch (cursorPos) {
      case 0: mode = MODE_MEASURE; break;
      case 1: mode = MODE_GRAPH; break;
      case 2: mode = MODE_INFO; break;
    }
  }
  draw();
}

void OledMenu::draw() {
  switch (mode) {
    case MENU_MAIN: drawMainMenu(); break;
    case MODE_MEASURE: drawMeasure(); break;
    case MODE_GRAPH: drawGraph(); break;
    case MODE_INFO: drawInfo(); break;
  }
}

void OledMenu::drawMainMenu() {
  display->clearBuffer();
  display->setFont(u8g2_font_6x12_tr);
  display->drawStr(20, 10, "== Hauptmenue ==");

  const char *items[] = {"Messwert", "Graph", "Info"};
  for (int i = 0; i < (sizeof(items) / sizeof(items[0])); i++) {
    if (i == cursorPos) display->drawStr(5, 25 + i * 12, ">");
    display->setCursor(15, 25 + i * 12);
    display->print(items[i]);
  }
  display->sendBuffer();
}

void OledMenu::drawMeasure() {
  display->clearBuffer();
  display->setFont(u8g2_font_6x12_tr);
  display->drawStr(5, 10, "== Messwerte ==");

  for (int ch = 0; ch < ANALOG_CHANNELS; ch++) {
    float val = readAnalog(ch);
    display->setCursor(10, 25 + ch * 10);
    display->print("CH");
    display->print(ch + 1);
    display->print(": ");
    display->print(val, 2);
  }
  display->sendBuffer();
}

void OledMenu::drawGraph() {
  t += 0.1;
  for (int ch = 0; ch < ANALOG_CHANNELS; ch++) {
    float val = readAnalog(ch);
    values[ch][graphIndex] = map(val * 100, 0, 330, GRAPH_HEIGHT, 0);
  }
  graphIndex = (graphIndex + 1) % 128;

  display->clearBuffer();
  display->setFont(u8g2_font_5x8_tr);
  display->drawStr(0, 10, "Graph-Modus");

  display->drawLine(0, GRAPH_OFFSET_Y + GRAPH_HEIGHT, 127, GRAPH_OFFSET_Y + GRAPH_HEIGHT);
  display->drawLine(0, GRAPH_OFFSET_Y, 0, GRAPH_OFFSET_Y + GRAPH_HEIGHT);

  for (int ch = 0; ch < ANALOG_CHANNELS; ch++) {
    for (int i = 0; i < 127; i++) {
      int next = (i + graphIndex) % 128;
      int next2 = (next + 1) % 128;
      display->drawLine(i, GRAPH_OFFSET_Y + values[ch][next],
                        i + 1, GRAPH_OFFSET_Y + values[ch][next2]);
    }
  }
  display->sendBuffer();
}

void OledMenu::drawInfo() {
  display->clearBuffer();
  display->setFont(u8g2_font_6x12_tr);
  display->drawStr(10, 25, "Dies ist ein kleiner");
  display->drawStr(10, 40, "Info-Text.");
  display->sendBuffer();
}

float OledMenu::readAnalog(int ch) {
#ifdef TESTMODE
  // Um eine Sinuswelle um 120 Grad zu verschieben, müssen Sie diesen Winkel in Bogenmaß umrechnen.
  // Bogenmaß = Grad * (π / 180)
  // phase = 120 * (π / 180) = 2.094 radians
  float phase = ch * 2.094;
  SignalType type;
  switch (ch)
  {
  case 0:
    type = SIGNAL_SINE;
    break;
  case 1:
    // type = SIGNAL_SQUARE;
    type = SIGNAL_SINE;
    break;
  case 2:
    // type = SIGNAL_TRIANGLE;
    type = SIGNAL_SINE;
    break;
  default:
    return 0.0;
  } 
  return generateSignal(type, 0.1, 3.3, phase); 

#else
  int pin;
  switch (ch) {
    case 0: pin = 4; break;
    case 1: pin = 0; break;
    case 2: pin = 2; break;
    case 3: pin = 15; break;
  }
  return analogRead(pin) * (3.3 / 4095.0);
#endif
}
// ======================================================
// Signalerzeugung (nur Testmodus)
// ======================================================
float OledMenu::generateSignal(SignalType type, float freq, float amplitude, float phase) {
//  t += 0.05;
  switch (type) {
    case SIGNAL_SINE:
      return amplitude * (sin(2 * M_PI * freq * t + phase) * 0.5 + 0.5);
    case SIGNAL_SQUARE:
      return (fmod(t * (freq), 1.0) < 0.5) ? amplitude*0.5 : 0;
    case SIGNAL_TRIANGLE:
      return amplitude * fabs(fmod(t * freq+0.1, 1.0) * 2 - 1);
  }
  return 0;
}


