#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Encoder.h>
#include <PinChangeInterrupt.h>

#undef MOTOR_USING_L298N
#define MOTOR_USING_L9110S

int const SCREEN_WIDTH(128);
int const SCREEN_HEIGHT(32);

unsigned long const REPORT_INTERVAL_MS(500);

int const DRIVE_DEADZONE(10);

// LEDs are temporary
int const LED_RED(12);
int const LED_BLUE(11);
int const LED_GREEN(8);
// LEDs are temporary

int const AZIMUTH_HOME(6);
int const AZIMUTH_PHASE_A(2);
int const AZIMUTH_PHASE_B(4);
int const ELEVATION_HOME(7);
int const ELEVATION_PHASE_A(3);
int const ELEVATION_PHASE_B(5);

int const AZIMUTH_DRIVE_IN1(A2);
int const AZIMUTH_DRIVE_IN2(A3);
int const AZIMUTH_DRIVE_EN(9);
int const ELEVATION_DRIVE_IN1(A0);
int const ELEVATION_DRIVE_IN2(A1);
int const ELEVATION_DRIVE_EN(10);

static char buffer[32];   // Generic buffer

/**
 */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
Encoder azimuthEncoder(AZIMUTH_PHASE_A, AZIMUTH_PHASE_B);
Encoder elevationEncoder(ELEVATION_PHASE_A, ELEVATION_PHASE_B);

bool azimuthHome(false);    //<! True if azimuth axis in home position
bool elevationHome(false);  //<! True if elevation axis in home position

/**
 */ 
void onAzimuthHomeChange() {
  azimuthHome = !digitalRead(AZIMUTH_HOME);
}

/**
 */
void onElevationHomeChange() {
  elevationHome = !digitalRead(ELEVATION_HOME);
}

/* Update L9110S motor driver.
   See https://www.banggood.com/L9110S-H-Bridge-Stepper-Motor-Dual-DC-Driver-Controller-Module-p-914880.html
*/
void updateL9110S(int speed, int const directionPin, int const speedPin)
{
  speed = constrain(speed, -255, 255);  
  if (abs(speed) <= DRIVE_DEADZONE)          
  {
    // if inside deadzone stop
    digitalWrite(directionPin, HIGH);
    analogWrite(speedPin, 255);
  }
  else if (speed < 0)                     
  {
    // if less than 0 then go backwards
    digitalWrite(directionPin, LOW);
    analogWrite(speedPin, abs(speed));
  }
  else {                               
    // otherwise go forwards
    digitalWrite(directionPin, HIGH);
    analogWrite(speedPin, 255-abs(speed));
  }
}

/* Update L298N motor driver
   See https://www.banggood.com/Wholesale-L298N-Dual-H-Bridge-Stepper-Motor-Driver-Board-p-42826.html
*/
void updateL298N(int speed, int const direction1Pin, int const direction2Pin, int const speedPin)
{
  speed = constrain(speed, -255, 255);  
  if (abs(speed) <= DRIVE_DEADZONE) 
  {
    // if inside deadzone stop
    digitalWrite(direction1Pin, HIGH);
    digitalWrite(direction2Pin, HIGH);
    analogWrite(speedPin, 0);
  }
  else
  {
    // if less than 0 then go backwards, otherwise go forwards
    digitalWrite(direction1Pin, speed > 0);
    digitalWrite(direction2Pin, speed < 0);
    analogWrite(speedPin, abs(speed));
  }
}

/**
 */
void driveMotors(int azimuthSpeed, int elevationSpeed) {
#ifdef MOTOR_USING_L298N
  updateL298N(azimuthSpeed, AZIMUTH_DRIVE_IN1, AZIMUTH_DRIVE_IN2, AZIMUTH_DRIVE_EN);
  updateL298N(elevationSpeed, ELEVATION_DRIVE_IN1, ELEVATION_DRIVE_IN2, ELEVATION_DRIVE_EN);
#endif

#ifdef MOTOR_USING_L9110S
  updateL9110S(azimuthSpeed, AZIMUTH_DRIVE_IN1, AZIMUTH_DRIVE_EN);
  updateL9110S(elevationSpeed, ELEVATION_DRIVE_IN1, ELEVATION_DRIVE_EN);
#endif
}

/**
 */
void setup() {

  // Initialise LEDs
  /*
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  */

  // Initialise motors
  pinMode(AZIMUTH_DRIVE_IN1, OUTPUT);
  pinMode(AZIMUTH_DRIVE_IN2, OUTPUT);
  pinMode(AZIMUTH_DRIVE_EN, OUTPUT);
  pinMode(ELEVATION_DRIVE_IN1, OUTPUT);
  pinMode(ELEVATION_DRIVE_IN2, OUTPUT);
  pinMode(ELEVATION_DRIVE_EN, OUTPUT);
  driveMotors(0, 0);

  // Initialise home sensors
  pinMode(AZIMUTH_HOME, INPUT_PULLUP);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(AZIMUTH_HOME), onAzimuthHomeChange, CHANGE);
  pinMode(ELEVATION_HOME, INPUT_PULLUP);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(ELEVATION_HOME), onElevationHomeChange, CHANGE);

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

  driveMotors(128,255);
}

/**
 */
void loop() {
  static unsigned long i(0);
  static unsigned long lastReportTime(millis());

  if ((millis() - lastReportTime) > REPORT_INTERVAL_MS) {
    lastReportTime = millis();
    i++;

    Serial.println(".");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

    display.clearDisplay();
    display.setCursor(0, 0);     // Start at top-left corner
    sprintf(buffer, "%c %06ld\n%c %06ld", azimuthHome ? ' ' : 'H', azimuthEncoder.read(), elevationHome ? ' ' : 'H', elevationEncoder.read());
    display.print(buffer);
    display.display();

    /*
    digitalWrite(LED_RED, i & 1);
    digitalWrite(LED_BLUE, i & 2);
    digitalWrite(LED_GREEN, i & 4);
    */
 }
}

