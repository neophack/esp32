
#include <Arduino.h>
#define SIG_PIN 22
#define led 2
#define button 13
int state = LOW;
volatile long cnt = 16667;
int lut[1000] = {0};
int ind = 0;
int range = 0;
int ind30 = 0;
int isinit = 0;
int lasterrcam = 0;
volatile int64_t lastIsrAt = 0;
volatile int64_t lastIntpAt = 0;
volatile int64_t currIsrAt = 0;
volatile int interruptCounter;
int totalInterruptCounter;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  //  interruptCounter++;
  blink();
  portEXIT_CRITICAL_ISR(&timerMux);
}


void setup() {
  // put your setup code here, to run once:
  pinMode(led, OUTPUT);
  pinMode(SIG_PIN, OUTPUT);
  attachInterrupt(button, use, RISING);
  Serial.begin(115200);
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, cnt, true);

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(led, state);
}
void use() {

  currIsrAt = esp_timer_get_time();
  int errpps = (currIsrAt - lastIsrAt) % 1000000;
  int errcam = (currIsrAt - lastIntpAt) % 1000000;
  
  if(round((currIsrAt - lastIsrAt) / 1000000.)==1&&lastIsrAt!=0){
    cnt=(currIsrAt - lastIsrAt)/60;
    Serial.println(cnt);
  }

  if (isinit && errpps > 1000 && errpps > 1000000 - 1000) { //filter error pps
    return;
  }
  else {
    if (isinit == 0) {
      if (ind == 200) {//init after 200s
        isinit = 1;
      }
      if (ind == 0) {
        timerAlarmEnable(timer);
      }
    }
  }
  portENTER_CRITICAL_ISR(&timerMux);
  lastIsrAt = currIsrAt;

  ind++;
  if (ind == 1000) {
    ind = 0;
  }

  if (range < 1000) {
    range++;
  }
  Serial.println(cnt-abs(errcam - cnt )  );
  if (cnt-abs(lasterrcam - cnt )  < 100) {
    state = !state;
  }
  if (errcam >cnt) { //
    lut[ind] = 0;
//    Serial.println("short");
  } else {
    lut[ind] = 1;
//    Serial.println("long");
  }
  lasterrcam=errcam;
  portEXIT_CRITICAL(&timerMux);
//  Serial.println((uint32_t)lastIsrAt);
//  Serial.println(errpps);
//  Serial.println(errcam);
//  Serial.println("************************");
}


void blink() {
  digitalWrite(SIG_PIN, !digitalRead(SIG_PIN));

  if (digitalRead(SIG_PIN) > 0) {
    lastIntpAt = esp_timer_get_time();
    timerAlarmWrite(timer, cnt + lut[ind30], true);
    ind30++;
    if (ind30 > range) {
      if (range < 200) {
        ind30 = 0;
      } else {
        ind30 = 100;//discard init lut
      }
    }
  }


}
