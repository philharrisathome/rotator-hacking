#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

int const SCREEN_WIDTH(128);
int const SCREEN_HEIGHT(32);

unsigned long const REPORT_INTERVAL_MS(500);

int const LED_RED(A0);
int const LED_BLUE(A1);
int const LED_GREEN(A2);

int const YAW_PHASE_A(2);
int const YAW_PHASE_B(4);

static char buffer[16];

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

void setup() {

  // Initialise LEDs
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  // Initialise encoders
  pinMode(YAW_PHASE_A, INPUT_PULLUP);
  pinMode(YAW_PHASE_B, INPUT_PULLUP);

  // Initialise display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Address 0x3C for 128x32
  display.dim(true);
  display.cp437(true);      // Use full 256 char 'Code Page 437' font
  display.setFont();        // Default font
  display.setTextSize(2);      
  display.setTextColor(SSD1306_WHITE); 
  display.setTextWrap(false);
  display.setCursor(0, 0);   
  display.clearDisplay();
  display.display();

  // Initialise serial port
  Serial.begin(115200);
  Serial.println("Hello!");
}

void loop() {
  static unsigned long i(0);
  static int oldPhaseA(0);
  static unsigned long lastReportTime(0);

  int newPhaseA = digitalRead(YAW_PHASE_A);
  if (newPhaseA && !oldPhaseA)
  {
    i += (digitalRead(YAW_PHASE_B) ? 1 : -1);
    i %= 1000000UL;
  }
  oldPhaseA = newPhaseA;

  if ((millis() - lastReportTime) > REPORT_INTERVAL_MS) {
    lastReportTime = millis();

    Serial.println(".");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

    display.clearDisplay();
    display.setCursor(0, 0);     // Start at top-left corner
    sprintf(buffer, "%06ld", i);
    display.print(buffer);
    display.display();

    digitalWrite(LED_RED, i & 1);
    digitalWrite(LED_BLUE, i & 2);
    digitalWrite(LED_GREEN, i & 4);
 }
}

