#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include <math.h>

#define LED_PIN 18                   // GPIO für Blink-LED
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

#define MENU_MODE_COUNT 5  // Anzahl der Menüpunkte
// Zustände bzw. Modi des Menüs
enum MenuMode {
  MENU_MAIN,        // Hauptmenü
  MODE_MEASURE,     // Digitalanzeige der Messwerte
  MODE_GRAPH,       // Graphanzeige
  MODE_PROPERTIES,  // Eigenschaften
  MODE_INFO         // Infotext
};

#define PROP_MODE_COUNT 3  // Anzahl der Menüpunkte
// Zustände bzw. Modi des Menüs
enum PropMode {
  MODE_BLINK_FRQ,   // Blinkfrequenz einstellen
  MODE_CHANNEL,     // Aktive Kanäle einstellen
  MODE_BACK,        // Zurück zum Hauptmenü
};

class OledMenu {
public:
  OledMenu(U8G2 *display);           // Konstruktor mit Referenz auf Displayobjekt
  void begin(uint32_t cycleTimeMs);  // Initialisierung des Displays und Startbildschirm und Startet Timer mit Zykluszeit
  void setCycleTime(uint32_t cycleTimeMs); // Ändert Zykluszeit im Betrieb
  void update();                     // Verarbeitung von Tasteneingaben und Zeitsteuerung
  void draw();                       // Darstellung je nach Modus

private:
  static void onTimer(TimerHandle_t xTimer); // Timer-Callback
  void handleTimer(); // Wird vom Callback aufgerufen

  // Led-Blinksteuerung
  unsigned long curStartTimer;       // Startzeitpunkt des Timers
  unsigned long lastBlinkTime;       // Letzte Änderung der Blink-LED-Zeit
  unsigned long blinkInterval = 800; // Blinkintervall in ms
  bool ledState;                     // Aktueller Zustand der Blink-LED

  U8G2 *display;                     // Referenz auf das Displayobjekt
  TimerHandle_t menuTimer;           // FreeRTOS-Software-Timer
  uint32_t cycleTime;                // Aktuelle Zykluszeit in ms

  MenuMode mode;                     // Aktueller Modus
  PropMode modeProp = MODE_CHANNEL;                 // Aktueller Eigenschaftenmodus
  int cursorPos;                     // Position des Cursors im Hauptmenü
  unsigned long lastPress;           // Zeitpunkt des letzten Tastendrucks
  bool buttonHeld;                   // Flag, ob Taste lange gedrückt wurde
  float t;                           // Zeitvariable für Signalberechnung
  int graphIndex;                    // Index für Graphenaktualisierung

  int numChannels = 1;                   // Neu: Anzahl der aktiven Kanäle
  float values[ANALOG_CHANNELS][128];// Puffer für Graphdaten

  // Methoden zur Anzeige und Steuerung
  void drawMainMenu();
  void drawMeasure();
  void drawGraph();
  void drawProperties(); 
  void drawInfo();
  void drawSplash();

  // Methoden für Tasterlogik
  void handleShortPress();
  void handleLongPress();

  // Methoden für Signalverarbeitung
  float readAnalog(int ch);
  float generateSignal(SignalType type, float freq, float amplitude, float phase);
};