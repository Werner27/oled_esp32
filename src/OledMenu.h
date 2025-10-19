#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include <math.h>


#define BUTTON_PIN 5
#define ANALOG_CHANNELS 3
#define GRAPH_HEIGHT 40
#define GRAPH_OFFSET_Y 20

// ======================================================
// Signaltypen für Testmodus
// ======================================================
enum SignalType {
  SIGNAL_SINE,
  SIGNAL_SQUARE,
  SIGNAL_TRIANGLE
};

enum MenuMode {
MENU_MAIN,
MODE_MEASURE,
MODE_GRAPH,
MODE_INFO
};


class OledMenu {
public:
OledMenu(U8G2 *display);
void begin();
void update();
void draw();


private:
U8G2 *display;
MenuMode mode;
int cursorPos;
unsigned long lastPress;
bool buttonHeld;


float t;
float values[ANALOG_CHANNELS][128];
int graphIndex;


void drawMainMenu();
void drawMeasure();
void drawGraph();
void drawInfo();
void drawSplash();


void handleShortPress();
void handleLongPress();
float readAnalog(int ch);
float generateSignal(SignalType type, float freq, float amplitude, float phase);
};