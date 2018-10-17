
#include "Thread.h"
#include "ThreadController.h"

const int btn_alarm = D4;
const int red_led = D7;
const int buzina = D6;
const int piscas = D5;

const int sensorPower = D3;
const int trig = D2;
const int echo = D1;

int intruder = 0;

long duration;
int distance;


int btn_alrm_prev_state = HIGH;
int btn_alrm_current_state;

int alrmState = 0;

unsigned long keyPrevMillis = 0;
unsigned long keyPressCount = 0;
const unsigned long keySampleIntervalMs = 25;
byte longKeyPressCountMax = 120;    // 120 * 25 = 3000 ms
byte mediumKeyPressCountMin = 20;    // 20 * 25 = 500 ms

static const unsigned long LED_REFRESH_INTERVAL = 5000; // ms
static unsigned long lastRefreshTime = 0;

ThreadController cpu;
Thread led_alrm;
Thread btn;
Thread sensor;
Thread thintruder;

void setup() {

  Serial.begin(115200);
  setBotoes();
  setLeds();
  setSensor();
  setBuzina();

  thintruder.setInterval(500);
  thintruder.onRun(checkIntruder);

  led_alrm.setInterval(5000);
  led_alrm.onRun(checkAlrmState);

  btn.setInterval(25);
  btn.onRun(btnCheck);

  sensor.setInterval(25);
  sensor.onRun(checkSensor);

  cpu.add(&thintruder);
  cpu.add(&led_alrm);
  cpu.add(&btn);
  cpu.add(&sensor);
}

void checkSensor() {
  if(alrmState == 1) {
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
  
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
  
    duration = pulseIn(echo, HIGH);
  
    distance = duration * 0.034/2;
    if(distance < 40) {
      intruder = 1;
    }
    Serial.println(distance);
    delay(500);
  }
}


void btnCheck() {
  if(millis() - keyPrevMillis >= keySampleIntervalMs) {
    keyPrevMillis = millis();

    btn_alrm_current_state = digitalRead(btn_alarm);

    if(btn_alrm_prev_state == HIGH && btn_alrm_current_state == LOW) {
      keyPressed();
    }
    else if(btn_alrm_prev_state == LOW && btn_alrm_current_state == HIGH) {
      keyReleased();
    }
    else if(btn_alrm_current_state == LOW) {
      keyPressCount++;
    }
    btn_alrm_prev_state = btn_alrm_current_state;
  }
}

void loop() {
  cpu.run();
}

void checkAlrmState() {
  if(alrmState == 1 && intruder == 0) {
      ledBlink(100);
  }
}

void checkIntruder() {
  if(intruder == 1) {
    alrmState = 0;
    digitalWrite(buzina, !digitalRead(buzina));
    ledBlink(100);
  }
}

void keyPressed() {
  keyPressCount = 0;
}

void keyReleased() {
  if (keyPressCount >= longKeyPressCountMax && alrmState == 0) {
    alrmActivate();
  }
  else {
    if (keyPressCount < mediumKeyPressCountMin) {
      alrmDeactivate();
    }
  }
}

void alrmActivate() {
  intruder = 0;
  for(int i = 0; i < 3; i++) {
    ledBlink(300);
  }
  delay(1000);
  for(int i = 15; i > 0; i--) {
    ledBlink(100 * i);
  }
  alrmState = 1;
  delay(100);
}

void alrmDeactivate() {
  alrmState = 0;
  intruder = 0;
  digitalWrite(buzina, HIGH);
  for(int i = 0; i < 3; i++) {
    ledBlink(300);
  }
}

void ledBlink(int ms) {
    digitalWrite(red_led, HIGH);
    delay(ms);
    digitalWrite(red_led, LOW);
    delay(ms);
}

void setLeds() {
  pinMode(red_led, OUTPUT);
}

void setBuzina() {
  pinMode(buzina, OUTPUT);
  digitalWrite(buzina, HIGH);
}

void setBotoes() {
  pinMode(btn_alarm, INPUT);
}

void setSensor() {
  pinMode(sensorPower, OUTPUT);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
}

