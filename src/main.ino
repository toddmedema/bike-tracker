#include "Adafruit_GPS.h"
#include "Adafruit_LIS3DH.h"
#include "GPS_Math.h"
#include "math.h"
#include "ctype.h"

#define NAME "toddbike"
#define mySerial Serial1
#define CLICKTHRESHHOLD 100
#define LED D7

Adafruit_GPS GPS(&mySerial);
Adafruit_LIS3DH accel = Adafruit_LIS3DH(A2, A5, A4, A3);
FuelGauge fuel;
SYSTEM_MODE(SEMI_AUTOMATIC); // manual control of cell connection
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));

int lastSecond = 0;
bool ledState = false;
unsigned long lastMotion = 0;
unsigned long lastPublish = 0;

// publish once every 1.5 minutes, so 2 publishes per cycle
#define PUBLISH_INTERVAL (90 * 1000)

// if no motion for 2 publish cycles, sleep!
#define NO_MOTION_IDLE_SLEEP_DELAY (2 * PUBLISH_INTERVAL + 20 * 1000)

// wake up to check in regularly
#define SLEEP_DURATION_SECONDS (8 * 60 * 60)

/* ===== SETUP ===== */
void setup() {
    initAccel();

    // enable the power to the gps module & set up GPS
    pinMode(D6, OUTPUT);
    digitalWrite(D6, LOW);
    GPS.begin(9600);
    mySerial.begin(9600);

    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
    Serial.begin(9600);

    //# request a HOT RESTART, in case we were in standby mode before.
    GPS.sendCommand("$PMTK101*32");
    delay(250);

    // request everything!
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_ALLDATA);
    delay(250);

    // turn off antenna updates
    GPS.sendCommand(PGCMD_NOANTENNA);
    delay(250);
}

void initAccel() {
    accel.begin(LIS3DH_DEFAULT_ADDRESS);
    accel.setDataRate(LIS3DH_DATARATE_LOWPOWER_5KHZ); // 5kHz low-power sampling
    accel.setRange(LIS3DH_RANGE_2_G); // 2 gravities range - pretty sensitive

    // listen for single-tap events at the threshold
    // keep the pin high for 1s, wait 1s between clicks
    //uint8_t c, uint8_t clickthresh, uint8_t timelimit, uint8_t timelatency, uint8_t timewindow
    accel.setClick(1, CLICKTHRESHHOLD);//, 0, 100, 50);
}

/* ====== LOOPING ===== */
void loop() {
  unsigned long now = millis();
  checkGPS();

  // make sure time tracking is in a good state
  if (lastMotion > now) { lastMotion = now; }
  if (lastPublish > now) { lastPublish = now; }

  // if motion / woken by motion, flash LED and update timestamp
  bool hasMotion = digitalRead(WKP);
  digitalWrite(LED, (hasMotion) ? HIGH : LOW); // flash on motion
  if (hasMotion) {
    lastMotion = now;
    Serial.println('MOTION');
  }

  // while, maintain a connection (won't be awake for long)
  if (Particle.connected() == false) {
    Serial.println("connecting and publishing position");
    Particle.connect();
  }

  // while awake, publish regularly
  if ((now - lastPublish) > PUBLISH_INTERVAL) {
    lastPublish = now;
    publishGPS();
  }

  // if nothing's happened for a while, publish battery state and go to sleep
  if ((now - lastMotion) > NO_MOTION_IDLE_SLEEP_DELAY) {
    String batt = String::format("%.2fv,%.2f%%", fuel.getVCell(), fuel.getSoC());
    Particle.publish(NAME + String("_b"), batt, 16777215, PRIVATE);
    Particle.publish(NAME + String("_s"), "sleeping", 16777215, PRIVATE);
    lastPublish = 0;
    lastMotion = 0;
    digitalWrite(D6, HIGH); // Turn off GPS power draw
    delay(20*1000); // settle down before sleep
    System.sleep(SLEEP_MODE_DEEP, SLEEP_DURATION_SECONDS);
  }
  delay(10);
}

// process and dump everything from the module through the library.
void checkGPS() {
  while (mySerial.available()) {
    char c = GPS.read();
    if (GPS.newNMEAreceived()) {
      GPS.parse(GPS.lastNMEA());
    }
  }
}

// publish location to Particle cloud as an event
void publishGPS() {
  // Note the minus, longitutde is backwards for western hemisphere
  String latLong = String::format("%f,-%f", convertDegMinToDecDeg(GPS.latitude), convertDegMinToDecDeg(GPS.longitude));
  Serial.println(latLong);
  Particle.publish(NAME + String("_g"), latLong, 16777215, PRIVATE);

  /* example of all the things we could log
    unsigned int msSinceLastMotion = (millis() - lastMotion);
    bool motionInTheLastMinute = (msSinceLastMotion < 60000);

    String gps_line =
          "{\"lat\":"    + String(convertDegMinToDecDeg(GPS.latitude))
        + ",\"lon\":-"   + String(convertDegMinToDecDeg(GPS.longitude))
        + ",\"a\":"     + String(GPS.altitude)
        + ",\"q\":"     + String(GPS.fixquality)
        + ",\"spd\":"   + String(GPS.speed)
        + ",\"mot\":"   + String(motionInTheLastMinute)
        + ",\"s\": "  + String(GPS.satellites)
        + ",\"vcc\":"   + String(fuel.getVCell())
        + ",\"soc\":"   + String(fuel.getSoC())
        + "}";
  */
}
