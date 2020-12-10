#include <Arduino.h>
#define SIG_PIN 22
#define LED_BUILTIN 2

volatile int interruptCounter;
int totalInterruptCounter;
hw_timer_t * timer = NULL;
long cnt=500000;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t lastIsrAt = 0;

void blink() {
  digitalWrite(SIG_PIN, !digitalRead(SIG_PIN));
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  lastIsrAt = millis();
  if(digitalRead(SIG_PIN)>0){
        Serial.print(lastIsrAt);
        Serial.print(":");
        Serial.println(totalInterruptCounter);
    }
 
}

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  blink();
  portEXIT_CRITICAL_ISR(&timerMux);
}
void setup() {
  pinMode(SIG_PIN, OUTPUT);
  Serial.begin(115200);
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, cnt, true);
  timerAlarmEnable(timer);
  pinMode(LED_BUILTIN, OUTPUT);
}
void loop() {
  uint32_t  isrTime = 0;
  if (interruptCounter > 0) {
    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
//    timerAlarmWrite(timer, cnt, true);
    portEXIT_CRITICAL(&timerMux);
    totalInterruptCounter++;
    
    isrTime = lastIsrAt;
//    if(lastIsrAt>10000){lastIsrAt=0;}
//    if(digitalRead(SIG_PIN)>0){
//        Serial.print(isrTime);
//        Serial.print(":");
//        Serial.println(totalInterruptCounter);
//    }
    
    cnt-=10;
    
  }
}
