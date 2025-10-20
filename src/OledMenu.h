#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include <math.h>


#define BUTTON_PIN 5                 // GPIO für Menütaster
#define ANALOG_CHANNELS 4            // Anzahl der analogen Kanäle
#define GRAPH_HEIGHT 40              // Höhe des Graphenbereichs
#define GRAPH_OFFSET_Y 20            // Y-Versatz für Graphendarstellung

// Signaltypen für Testmodus
enum SignalType {
  SIGNAL_SINE,
  SIGNAL_SQUARE,
  SIGNAL_TRIANGLE
};

// Zustände bzw. Modi des Menüs
enum MenuMode {
  MENU_MAIN,    // Hauptmenü
  MODE_MEASURE, // Digitalanzeige der Messwerte
  MODE_GRAPH,   // Graphanzeige
  MODE_INFO     // Infotextanzeige
};

class OledMenu {
public:
  OledMenu(U8G2 *display);           // Konstruktor mit Referenz auf Displayobjekt
  void begin();                      // Initialisierung des Displays und Startbildschirm
  void update();                     // Verarbeitung von Tasteneingaben und Zeitsteuerung
  void draw();                       // Darstellung je nach Modus

private:
  U8G2 *display;                     // Referenz auf das Displayobjekt
  MenuMode mode;                     // Aktueller Modus
  int cursorPos;                     // Position des Cursors im Hauptmenü
  unsigned long lastPress;           // Zeitpunkt des letzten Tastendrucks
  bool buttonHeld;                   // Flag, ob Taste lange gedrückt wurde

  float t;                           // Zeitvariable für Signalberechnung
  float values[ANALOG_CHANNELS][128];// Puffer für Graphdaten
  int graphIndex;                    // Index für Graphenaktualisierung

  // Methoden zur Anzeige und Steuerung
  void drawMainMenu();
  void drawMeasure();
  void drawGraph();
  void drawInfo();
  void drawSplash();

  // Methoden für Tasterlogik
  void handleShortPress();
  void handleLongPress();

  // Methoden für Signalverarbeitung
  float readAnalog(int ch);
  float generateSignal(SignalType type, float freq, float amplitude, float phase);
};