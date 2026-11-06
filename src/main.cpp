// Lab 10 – Event-Driven Programming with Push Buttons and a Touch Sensor
#include <Arduino.h>

// ---------- Pin numbers ----------
#define BUTTON_A   32   // polled button
#define BUTTON_B   33   // interrupt button
#define TOUCH_PIN  27   // touch sensor wire
#define GREEN_LED  25
#define RED_LED    26
#define BUZZER     13

// ---------- Global variables ----------
int lastStateA = HIGH;
int pressCountA = 0;
unsigned long lastChangeA = 0;
const unsigned long DEBOUNCE_TIME = 50;

volatile bool buttonBFlag = false;
volatile unsigned long lastInterruptB = 0;
int pressCountB = 0;

const int TOUCH_THRESHOLD = 752;   

typedef void (*Callback)();
Callback buttonACallback;
Callback buttonBCallback;
Callback touchCallback;

volatile bool touchFlag = false;
volatile unsigned long lastTouch = 0;
bool armed = false;
bool alarmOn = false;
unsigned long alarmStart = 0;
const unsigned long ALARM_TIME = 3000;

// ---------- Event handlers and callbacks ----------
void onButtonAPressed(){
  pressCountA++;
  Serial.print("Button A pressed. Count = ");
  Serial.println(pressCountA);
}

void onButtonBPressed(){
  pressCountB++;
  Serial.print("Button B pressed (interrupt). Count = ");
  Serial.println(pressCountB);
}

void alarmOff(){
  alarmOn = false;
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);
}

void startAlarm(){
  alarmOn = true;
  alarmStart = millis();
  digitalWrite(RED_LED, HIGH);
  digitalWrite(BUZZER, HIGH);
}


void toggleArmed(){
  armed = !armed;
  if (armed == true)
  {
    digitalWrite(GREEN_LED, HIGH);
    Serial.println("System ARMED");
  }
  else
  {
    digitalWrite(GREEN_LED, LOW);
    alarmOff();
    Serial.println("System DISARMED");
  }
}

void silenceAlarm(){
  alarmOff();
  Serial.println("Alarm silenced");
}

void onTouchDetected(){
  if (armed == true){
    Serial.println("TOUCH! Alarm ON");
    startAlarm();
  }
}

// ---------- Interrupt Service Routines (ISRs) ----------
void IRAM_ATTR buttonB_ISR(){
  if (millis() - lastInterruptB > 200){
    lastInterruptB = millis();
    buttonBFlag = true;   // FIX 3: was false
  }
}

void IRAM_ATTR touch_ISR(){
  if (millis() - lastTouch > 1000){
    lastTouch = millis();
    touchFlag = true;
  }
}

void setup(){
  Serial.begin(115200);
  delay(500);
  Serial.println("Lab 10: Event-Driven Programming");

  // Inputs and outputs
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Interrupts
  attachInterrupt(digitalPinToInterrupt(BUTTON_B), buttonB_ISR, FALLING);
  touchAttachInterrupt(TOUCH_PIN, touch_ISR, TOUCH_THRESHOLD);

  // Callbacks
  buttonACallback = toggleArmed;
  buttonBCallback = silenceAlarm;
  touchCallback = onTouchDetected;
}

void loop(){
  // Button A (polling)
  int stateA = digitalRead(BUTTON_A);
  if (stateA != lastStateA && millis() - lastChangeA > DEBOUNCE_TIME){
    lastChangeA = millis();
    lastStateA = stateA;
    if (stateA == LOW){
      buttonACallback();
    }
  }

  // Button B (interrupt)
  if (buttonBFlag == true){
    buttonBFlag = false;
    buttonBCallback();
  }

  // Touch sensor (interrupt)
  if (touchFlag == true){
    touchFlag = false;
    touchCallback();   // FIX 6: was missing ()
  }

  // Timer
  if (alarmOn == true && millis() - alarmStart >= ALARM_TIME){
    alarmOff();
    Serial.println("Alarm timed out");
  }
}
