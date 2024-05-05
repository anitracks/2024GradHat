/* motorTest
 *
 * This is to test the vibration motors on the 2023 Grad hat v0.1 board.
 *
 * Seth McNeill
 * 2024 January 18
 */

#define SW4 7
#define SW5 8
#define LMOT1 10
#define LMOT2 11
#define RMOT1 12
#define RMOT2 13
#define TMOT1  0
#define TMOT2  1
#define BUZZ   4

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("Starting...");

  // setup pins
  pinMode(SW4, INPUT);
  pinMode(SW5, INPUT);
  pinMode(LMOT1, OUTPUT);
  pinMode(LMOT2, OUTPUT);
  pinMode(RMOT1, OUTPUT);
  pinMode(RMOT2, OUTPUT);
  pinMode(TMOT1, OUTPUT);
  pinMode(TMOT2, OUTPUT);
  pinMode(BUZZ, OUTPUT);
}

unsigned long runTime = 1000;  // how long to run/stop for
int mode = 0;
bool isRunning = false;

void loop() {
  static unsigned long lastStart = 0;
  unsigned long curTime = millis();
  if(curTime - lastStart > runTime) {
    if(isRunning) {
      tone(BUZZ, 750);
      analogWrite(LMOT1, 0);
      analogWrite(LMOT2, 255);
      analogWrite(RMOT1, 255);
      analogWrite(RMOT2, 0);
      analogWrite(TMOT1, 0);
      analogWrite(TMOT2, 255);
      isRunning = false;
    } else {
      tone(BUZZ, 500);
      analogWrite(LMOT1, 255);
      analogWrite(LMOT2, 0);
      analogWrite(RMOT1, 0);
      analogWrite(RMOT2, 255);
      analogWrite(TMOT1, 255);
      analogWrite(TMOT2, 0);
      isRunning = true;
    }
    lastStart = curTime;
  }
 /*
  if(digitalRead(SW4)) {
    analogWrite(LMOT1, 0);
    analogWrite(LMOT2, 255);
  } else {
    analogWrite(LMOT1, 0);
    analogWrite(LMOT2, 0);
  }
  if(digitalRead(SW5)) {
    analogWrite(RMOT1, 255);
    analogWrite(RMOT2, 0);
  } else {
    analogWrite(RMOT1, 0);
    analogWrite(RMOT2, 0);
  }
*/
}
