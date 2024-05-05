/* gradMain2024
 *
 * Main program for the 2024 grad board deployment
 *
 * Seth McNeill
 * 2024 February 18
 */

/*
  Functionality:
    1) Blue around outside, yellow on tassel
    2) Tilt effect with LEDs
    3) Rainbow demo run
    4) Battery level
    5) IR React/send
    6) Play school song or something similar
    7) Blue/Yellow tassel moving

*/

#include <Adafruit_NeoPixel.h>  // NeoPixels
#include "Adafruit_MAX1704X.h"  // battery monitor (requires battery and switch turned on)
#include <Adafruit_LSM6DS3TRC.h>// IMU

#define BTN1 8
#define BTN2 7
#define BUZZ 4
#define IR_XMT 2
#define IR_RCV 3
#define IMU_INT1 5
#define IMU_INT2 6
#define BAT_ALERT 15
#define NEO_PIN 17
#define NEO_COUNT 66
#define TASSEL_START 45  // need to check
#define BTN_DEBOUNCE 250
#define PICO_I2C_SDA 20
#define PICO_I2C_SCL 21

enum BoardModes {MODE_MIN, BLUE_YELLOW=MODE_MIN, COLOR_FILL, IMU, BATT, MODE_MAX};
BoardModes operator++(BoardModes& m) { // allow for preincremnting of enum type
  m = BoardModes(m+1);
  if(m == MODE_MAX) {
    m = MODE_MIN;
  }
  return(m);
}
BoardModes curMode = BLUE_YELLOW;
Adafruit_NeoPixel strip(NEO_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_MAX17048 maxlipo;
bool battMonPresent = false;  // set to true if battery monitor detected
Adafruit_LSM6DS3TRC imu;

void setup() {
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  blueYellow();
  Serial.begin(115200);
  pinMode(BUZZ, OUTPUT);
  tone(BUZZ, 1000, 200);
  delay(3000);
  Serial.println("Starting...");

  pinMode(BTN1, INPUT);
  pinMode(BTN2, INPUT);

//  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
//  strip.show();            // Turn OFF all pixels ASAP

  Wire.setSDA(PICO_I2C_SDA);
  Wire.setSCL(PICO_I2C_SCL);
  
  if (!maxlipo.begin()) {
    Serial.println(F("Couldnt find Adafruit MAX17048?\nMake sure a battery is plugged in!"));
    battMonPresent = false;
  } else {
    battMonPresent = true;
    Serial.print(F("Found MAX17048"));
    Serial.print(F(" with Chip ID: 0x")); 
    Serial.println(maxlipo.getChipID(), HEX);
    maxlipo.setAlertVoltages(2.1, 4.2);
  }

  if (!imu.begin_I2C(0x6B)) {
    // if (!lsm6ds3trc.begin_SPI(LSM_CS)) {
    // if (!lsm6ds3trc.begin_SPI(LSM_CS, LSM_SCK, LSM_MISO, LSM_MOSI)) {
    Serial.println("Failed to find LSM6DS3TR-C chip");
  } else {
    Serial.println("LSM6DS3TR-C IMU Found!");
    imu.configInt1(false, false, true); // accelerometer DRDY on INT1
    imu.configInt2(false, true, false); // gyro DRDY on INT2
  }
}

void loop() {
  static int lastBtn1 = 0;
  static unsigned long lastBtn1Time = 0;
  static unsigned long lastPrintTime = 0;
  int curBtn1 = digitalRead(BTN1);
  unsigned long curTime = millis();

  // Serial.print(curBtn1);
  // Serial.print(",");
  // Serial.print(lastBtn1);
  // Serial.print(",");
  // Serial.print(curTime - lastBtn1Time);
  // Serial.print(",");
  // Serial.println("");
  if((curBtn1 != lastBtn1) && (curTime - lastBtn1Time > BTN_DEBOUNCE) && curBtn1) {
    ++curMode;
    Serial.print(curBtn1);
    Serial.print(",");
    Serial.print(curMode);
    Serial.println("");
    lastBtn1Time = curTime;
    tone(BUZZ, 1000, 100);
  }
  lastBtn1 = curBtn1;
  switch(curMode) {
    case BLUE_YELLOW: blueYellow(); break;
    case COLOR_FILL: cycleColorFill(); break;
    case BATT: battDisp(); break;
    case IMU: imuLEDs(); break;
    default: Serial.println("Unknown mode");
  }
  // if(curTime - lastPrintTime > 1000) {
  //   Serial.println("Hello");
  // }
}

void battDisp() {
  static unsigned long lastRun = 0;
  unsigned long curTime = millis();
  #define UPDATE_PERIOD 3000  
  if(curTime - lastRun > UPDATE_PERIOD) {
    if(battMonPresent) {
      maxlipo.wake();
      strip.clear();
      float batPct = maxlipo.cellPercent();
      int splitSpot = (100.0-batPct)*(NEO_COUNT - TASSEL_START)/100 + TASSEL_START;
      Serial.print("Split spot: "); Serial.println(splitSpot);
      for(int ii = TASSEL_START; ii < splitSpot; ii++) {
        strip.setPixelColor(ii, strip.Color(100,0,0));
      }
      for(int ii = splitSpot; ii < NEO_COUNT; ii++) {
        strip.setPixelColor(ii, strip.Color(0,70,0));
      }
      Serial.print(F("Batt Voltage: ")); Serial.print(maxlipo.cellVoltage(), 3); Serial.println(" V");
      Serial.print(F("Batt Percent: ")); Serial.print(batPct, 1); Serial.println(" %");
      Serial.print(F("(Dis)Charge rate : ")); Serial.print(maxlipo.chargeRate(), 1); Serial.println(" %/hr");
      if (maxlipo.isActiveAlert()) {
        uint8_t status_flags = maxlipo.getAlertStatus();
        Serial.print(F("ALERT! flags = 0x"));
        Serial.print(status_flags, HEX);
        strip.setPixelColor(0, strip.Color(100,0,0));
        if (status_flags & MAX1704X_ALERTFLAG_SOC_CHANGE) {
          Serial.print(", SOC Change");
          strip.setPixelColor(1, strip.Color(100,0,0));
          maxlipo.clearAlertFlag(MAX1704X_ALERTFLAG_SOC_CHANGE); // clear the alert
        }
        if (status_flags & MAX1704X_ALERTFLAG_SOC_LOW) {
          Serial.print(", SOC Low");
          strip.setPixelColor(2, strip.Color(100,0,0));
          maxlipo.clearAlertFlag(MAX1704X_ALERTFLAG_SOC_LOW); // clear the alert
        }
        if (status_flags & MAX1704X_ALERTFLAG_VOLTAGE_RESET) {
          Serial.print(", Voltage reset");
          strip.setPixelColor(3, strip.Color(100,0,0));
          maxlipo.clearAlertFlag(MAX1704X_ALERTFLAG_VOLTAGE_RESET); // clear the alert
        }
        if (status_flags & MAX1704X_ALERTFLAG_VOLTAGE_LOW) {
          Serial.print(", Voltage low");
          strip.setPixelColor(4, strip.Color(100,0,0));
          maxlipo.clearAlertFlag(MAX1704X_ALERTFLAG_VOLTAGE_LOW); // clear the alert
        }
        if (status_flags & MAX1704X_ALERTFLAG_VOLTAGE_HIGH) {
          Serial.print(", Voltage high");
          strip.setPixelColor(5, strip.Color(100,0,0));
          maxlipo.clearAlertFlag(MAX1704X_ALERTFLAG_VOLTAGE_HIGH); // clear the alert
        }
        if (status_flags & MAX1704X_ALERTFLAG_RESET_INDICATOR) {
          Serial.print(", Reset Indicator");
          strip.setPixelColor(6, strip.Color(100,0,0));
          maxlipo.clearAlertFlag(MAX1704X_ALERTFLAG_RESET_INDICATOR); // clear the alert
        }
        Serial.println();
      }
    } else {
      strip.clear();
      for(int ii = TASSEL_START; ii < NEO_COUNT; ii++) {
        strip.setPixelColor(ii, strip.Color(100,0,0));
      }
      Serial.println("No battery");
    }
    strip.show();
    lastRun = curTime;
  }
}

void blueYellow() {
  static unsigned long lastRun = 0;
  unsigned long curTime = millis();
  #define UPDATE_PERIOD 3000  
  if(curTime - lastRun > UPDATE_PERIOD) {
    for(int ii = 0; ii < TASSEL_START;ii++) {
      strip.setPixelColor(ii, strip.Color(0,0,180));
    }
    for(int ii = TASSEL_START; ii < NEO_COUNT;ii++) {
      strip.setPixelColor(ii, strip.Color(100,100,0));
    }
    strip.show();
    lastRun = curTime;
  }
}

void cycleColorFill() {
  static unsigned long lastCall = 0;
  unsigned long resetInterval = 1000;
  static int colorMode = 0;
  int nColor = 3;
  bool retVal = false;
  bool resetI = false;
  unsigned long curTime = millis();
  if(curTime - lastCall > resetInterval) {
    resetI = true;
  }

  switch(colorMode) {
    case 0: retVal = colorFill(strip.Color(115,0,0), 30, resetI); break;
    case 1: retVal = colorFill(strip.Color(0,120,0), 30, resetI); break;
    case 2: retVal = colorFill(strip.Color(0,0,180), 30, resetI); break;
    default: colorMode = 0;
  }
  if(retVal) {
    colorMode++;
  }
  lastCall = curTime;
}

bool colorFill(uint32_t color, int wait, bool resetI) {
  static int i = 0;
  static unsigned long lastTime = 0;
  unsigned long curTime = millis();
  if(resetI) {
    i = 0;
    strip.clear();
  }
  if(curTime - lastTime > wait) {
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    lastTime = curTime;
    i++;
    if(i >= strip.numPixels()) {
      i = 0;
      strip.clear();
      strip.show();
      return(true);
    }
    strip.show();                          //  Update strip to match
  }
  return(false);  // still filling leds
}

void imuLEDs(void) {
  int startLEDOffset = 3;
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  imu.getEvent(&accel, &gyro, &temp);
  Serial.print("T:");
  Serial.print(temp.temperature*1.8+32.0);
  Serial.print(" deg F,");

  float theta_a = RAD_TO_DEG*atan2(accel.acceleration.x,accel.acceleration.y);
  Serial.print(theta_a);
  Serial.print(", ");

  float pct = (float)accel.acceleration.x/(float)accel.acceleration.y;
  int startLED = (int)(theta_a/6.43) + startLEDOffset;
  Serial.print(theta_a/6.43);
  Serial.print(", ");
  Serial.print(startLED % (TASSEL_START-0));
  Serial.print(", \n");
  strip.clear();
  strip.fill(strip.Color(0,70,0),0,TASSEL_START-1);
  if(startLED > 0) {
    for(int ii = startLED; ii < startLED + (TASSEL_START-1)/2; ii++){
      strip.setPixelColor(ii%(TASSEL_START-1), strip.Color(0,0,70));
    }
  } else {
    for(int ii = 0; ii < (TASSEL_START-1)/2; ii++){
      strip.setPixelColor((ii+startLED+TASSEL_START-1)%(TASSEL_START-1), strip.Color(0,0,70));
    }
  }
  strip.show();

}