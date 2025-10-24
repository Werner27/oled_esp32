#include "OledMenu.h"

// Konstruktor: Initialisiert interne Variablen und konfiguriert den Button-Pin
OledMenu::OledMenu(U8G2 *disp)
  : display(disp), mode(MENU_MAIN), cursorPos(0), lastPress(0), buttonHeld(false), t(0.0), graphIndex(0) {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
}

void OledMenu::begin(uint32_t cycleTimeMs) {
  display->begin();
  drawSplash();
//  drawMainMenu();

    cycleTime = cycleTimeMs;

    // Timer erstellen (FreeRTOS Software Timer)
    menuTimer = xTimerCreate(
        "MenuTimer",                     // Name
        pdMS_TO_TICKS(cycleTime),        // Zeitraum
        pdTRUE,                          // wiederkehrend
        this,                            // Timer-ID = Instanzzeiger
        [](TimerHandle_t xTimer) {        // Lambda-Callback
            OledMenu *instance = (OledMenu *)pvTimerGetTimerID(xTimer);
            if (instance) instance->handleTimer();
        }
    );

    if (menuTimer != nullptr) {
        xTimerStart(menuTimer, 0); // Timer starten
    }
}

void OledMenu::setCycleTime(uint32_t cycleTimeMs) {
    cycleTime = cycleTimeMs;
    if (menuTimer != nullptr) {
        xTimerChangePeriod(menuTimer, pdMS_TO_TICKS(cycleTime), 0);
    }
}

void OledMenu::handleTimer() {
    curStartTimer = millis();
    if (millis() - lastBlinkTime >= blinkInterval) {
        ledState = !ledState ;
        digitalWrite(LED_PIN, ledState ? HIGH : LOW );
        lastBlinkTime = millis();
    } 
    update();
    draw();
    unsigned long elapsed = millis() - curStartTimer;
//    Serial.printf("StZ %lu ms, LZ %lu ms\n", startTime, elapsed);
}


// Splashscreen mit Copyright und Smile-Logo
void OledMenu::drawSplash() {
  display->clearBuffer();
  display->setFont(u8g2_font_ncenB08_tr);
  display->drawStr(20, 30, "Copyright Werner");
  // Einfaches Smile-Logo zeichnen
  display->drawCircle(64, 48, 8);     // Gesichtskreis
  display->drawDisc(61, 46, 1);       // linkes Auge
  display->drawDisc(67, 46, 1);       // rechtes Auge
  display->drawArc(64, 49, 5, 0, 180);// Mund
  display->sendBuffer();
  delay(2000);
}

// Prüft regelmäßig den Buttonzustand (kurzer oder langer Tastendruck)
void OledMenu::update() {
  static bool wasPressed = false;            // Zustand des Buttons im letzten Zyklus
  bool pressed = (digitalRead(BUTTON_PIN) == LOW); // aktueller Zustand

  // Wenn Taste gerade gedrückt wurde (Flanke)
  if (pressed && !wasPressed) {
    lastPress = millis();
    buttonHeld = false;
  }

  // Wenn Taste länger als 1 Sekunde gehalten wird
  if (pressed && (millis() - lastPress > 500) && !buttonHeld) {
    buttonHeld = true;
    handleLongPress();                     // Langer Tastendruck erkannt
  }

  // Wenn Taste losgelassen wird, ohne lange gehalten zu sein
  if (!pressed && wasPressed && !buttonHeld) {
    handleShortPress();                    // Kurzer Tastendruck erkannt
  }
  wasPressed = pressed;                    // Zustand für nächsten Zyklus speichern
}

// Kurzer Tastendruck: Cursor bewegt sich durch das Menü oder kehrt zum Hauptmenü zurück
void OledMenu::handleShortPress() {
  if (mode == MENU_MAIN) {
    cursorPos++;
    if (cursorPos >= MENU_MODE_COUNT-1) cursorPos = 0;     // zyklisch durch die Menüpunkte
    drawMainMenu();
  }
  else if (mode == MODE_PROPERTIES) {
    cursorPos++;  
    if (cursorPos >= PROP_MODE_COUNT) cursorPos = 0;     // zyklisch durch die Menüpunkte
    drawProperties();
  } else {
    mode = MENU_MAIN;                     // Rückkehr ins Hauptmenü
    drawMainMenu();
  }
}

// Langer Tastendruck: Aktiviert den aktuell markierten Menüpunkt
void OledMenu::handleLongPress() {
  if (mode == MENU_MAIN) {
    switch (cursorPos) {
      case 0: mode = MODE_MEASURE; break; // Messwerte anzeigen
      case 1: mode = MODE_GRAPH; break;   // Graph anzeigen
      case 2: mode = MODE_PROPERTIES; cursorPos= 0; break;   // Eigenschaften anzeigen
      case 3: mode = MODE_INFO; break;    // Info-Text anzeigen
    }
  }
  else if (mode == MODE_PROPERTIES) {
    switch (cursorPos) {
      case 0: // Aktive Kanäle einstellen
        numChannels++;
        if (numChannels > ANALOG_CHANNELS) numChannels = 1;
        break;
      case 1: // Blinkfrequenz einstellen
        blinkInterval += 200;
        if (blinkInterval > 2000) blinkInterval = 200;
        break;
      case 2: // Zurück zum Hauptmenü
        mode = MENU_MAIN;   // Rückkehr ins Hauptmenü
        cursorPos = 0;
        break;
    }
  }
  draw();
}

// Entscheidet je nach Modus, welche Anzeige gezeichnet wird
void OledMenu::draw() {
  switch (mode) {
    case MENU_MAIN: drawMainMenu(); break;
    case MODE_MEASURE: drawMeasure(); break;
    case MODE_GRAPH: drawGraph(); break;
    case MODE_PROPERTIES: drawProperties(); break;
    case MODE_INFO: drawInfo(); break;
  }
}

// Hauptmenüanzeige mit Cursor
void OledMenu::drawMainMenu() {
  const char *items[] = {"Messwert", "Graph", "Eigenschaften", "Info"};
  int itemCount = sizeof(items) / sizeof(items[0]);

  display->clearBuffer();
  display->setFont(u8g2_font_6x12_tr);
  display->drawStr(20, 10, "== Hauptmenue ==");

  for (int i = 0; i < itemCount; i++) {
    if (i == cursorPos) display->drawStr(5, 25 + i * 12, ">"); // Cursor anzeigen
    display->setCursor(15, 25 + i * 12);
    display->print(items[i]);
  }
  display->sendBuffer();
}

// Zeigt aktuelle Messwerte (digital) an
void OledMenu::drawMeasure() {
  display->clearBuffer();
  display->setFont(u8g2_font_6x12_tr);
  display->drawStr(5, 10, "== Messwerte ==");
  t += 0.1;
  for (int ch = 0; ch < numChannels; ch++) {
    float val = readAnalog(ch);            // aktuellen Messwert lesen
    display->setCursor(10, 25 + ch * 10);
    display->print("CH");
    display->print(ch + 1);
    display->print(": ");
    display->print(val, 2);                // Wert mit zwei Nachkommastellen
  }
  display->sendBuffer();
}

// Graphdarstellung der analogen Werte
void OledMenu::drawGraph() {
  // Neue Messwerte aufnehmen
  t += 0.1;
  for (int ch = 0; ch < numChannels; ch++) {
    float val = readAnalog(ch);
    values[ch][graphIndex] = map(val * 100, 0, 330, GRAPH_HEIGHT, 0);
  }
  
  // Darstellung vorbereiten
  display->clearBuffer();
  // display->setFont(u8g2_font_5x8_tr);
  // display->drawStr(0, 10, "Graph-Modus");
  display->setFont(u8g2_font_6x12_tr);
//  display->setFont(u8g2_font_ncenB08_tr);

  // Beispiel: Messwert des 1. Kanals simuliert als Sinus
  //  float value = (sin(t) + 1.0) * 1.65; // Spannung zwischen 0–3.3V
  float value = readAnalog(0);

  char title[32];
  snprintf(title, sizeof(title), "Graph-Modus  U=%.2fV", value);
  display->drawStr(0, 10, title);

  graphIndex = (graphIndex + 1) % 128; // zyklische Speicherung

  // Achsen zeichnen
  display->drawLine(0, GRAPH_OFFSET_Y + GRAPH_HEIGHT, 127, GRAPH_OFFSET_Y + GRAPH_HEIGHT);
  display->drawLine(0, GRAPH_OFFSET_Y, 0, GRAPH_OFFSET_Y + GRAPH_HEIGHT);

  // Linien zwischen aufeinanderfolgenden Punkten verbinden
  for (int ch = 0; ch < numChannels; ch++) {
    for (int i = 0; i < 127; i++) {
      int next = (i + graphIndex) % 128;
      int next2 = (next + 1) % 128;
      display->drawLine(i, GRAPH_OFFSET_Y + values[ch][next],
                        i + 1, GRAPH_OFFSET_Y + values[ch][next2]);
    }
  }
  display->sendBuffer();
}
// Anzeige der Eigenschaften (z.B. Anzahl der Kanäle)
/*
void OledMenu::drawProperties() {
    display->clearBuffer();
    display->setFont(u8g2_font_ncenB08_tr);
    display->drawStr(0, 10, "== Eigenschaften ==");

    char buf[24];
    snprintf(buf, sizeof(buf), "Kanaele: %d", numChannels);
    display->drawStr(0, 25, buf);
    display->drawStr(0, 45, "Langdruck = +1 Kanal");
    display->drawStr(0, 55, "Kurz = zurueck");

    display->sendBuffer();
}
*/
void OledMenu::drawProperties() {
  const char *items[] = {"Kanäle:", "Blinkfrq:", "Zurück:"};
  int itemCount = sizeof(items) / sizeof(items[0]);

  display->clearBuffer();
  display->setFont(u8g2_font_6x12_tr);
  display->drawStr(20, 10, "== Eigenschaften ==");

  for (int i = 0; i < itemCount; i++) {
    if (i == cursorPos) display->drawStr(5, 25 + i * 12, ">"); // Cursor anzeigen
    display->setCursor(15, 25 + i * 12);
    display->print(items[i]); 
    display->setCursor(80, 25 + i * 12);
    if (i == 0) {
      display->print(numChannels);
    } else if (i == 1) {
      display->print(blinkInterval);
      display->print(" ms");
    } 
  }
  display->sendBuffer();
}

// Einfacher Info-Bildschirm
void OledMenu::drawInfo() {
  display->clearBuffer();
  display->setFont(u8g2_font_6x12_tr);
  display->drawStr(10, 25, "Dies ist ein kleiner");
  display->drawStr(10, 40, "Info-Text.");
  display->sendBuffer();
}

// Liest analoge Werte oder erzeugt Testsignale
float OledMenu::readAnalog(int ch) {
#ifdef TESTMODE
  // Im Testmodus werden künstliche Signale erzeugt (z.B. Sinus)
  // Um eine Sinuswelle um 120 Grad zu verschieben, müssen Sie diesen Winkel in Bogenmaß umrechnen.
  // Bogenmaß = Grad * (π / 180)
  // phase = 120 * (π / 180) = 2.094 radians
  float phase = ch * 2.094; // Phasenverschiebung zwischen den Kanälen (120°)
  SignalType type;
  switch (ch) {
  case 0: type = SIGNAL_SINE; break;
  case 1: type = SIGNAL_SINE; break; // type = SIGNAL_SQUARE
  case 2: type = SIGNAL_SINE; break; // type = SIGNAL_TRIANGLE
  default: return analogRead(15) * (3.3 / 4095.0);  //Pin GPIO15 lesen
  }
  return generateSignal(type, 0.1, 3.3, phase); // Testsignal erzeugen
#else
  // Im Normalbetrieb reale Analogwerte lesen
  int pin;
  switch (ch) {
    case 0: pin = 4; break;
    case 1: pin = 0; break;
    case 2: pin = 2; break;
    case 3: pin = 15; break;
  }
  return analogRead(pin) * (3.3 / 4095.0); // Spannung berechnen
#endif
}

// Erzeugt Testsignale (Sinus, Rechteck, Dreieck) basierend auf Zeitvariable t
float OledMenu::generateSignal(SignalType type, float freq, float amplitude, float phase) {
/*
  // Delta-Zeit zur realistischeren Signalerzeugung nutzen
  static unsigned long lastTime = millis();
  unsigned long now = millis();
  float deltaT = (now - lastTime) / 1000.0; // vergangene Zeit in Sekunden
  lastTime = now;
  t += deltaT; // Zeitvariable erhöhen
*/
  switch (type) {
    case SIGNAL_SINE:
      // Sinuswelle erzeugen
      return amplitude * (sin(2 * M_PI * freq * t + phase) * 0.5 + 0.5);
    case SIGNAL_SQUARE:
      // Rechteckwelle mit 50% Tastverhältnis
      return (fmod(t * freq, 1.0) < 0.5) ? amplitude : 0.0;
    case SIGNAL_TRIANGLE:
      // Dreieckwelle: linearer Anstieg/Abfall
      return amplitude * fabs(fmod(t * freq, 1.0) * 2 - 1);
  }
  return 0;
}


