// This #include statement was automatically added by the Spark IDE.
#include "Adafruit_GPS.h"
#include "Adafruit_LIS3DH.h"
#include "GPS_Math.h"

#include <math.h>
#include "math.h"
#include <ctype.h>

#define NAME "toddbike"
#define mySerial Serial1
#define CLICKTHRESHHOLD 20
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
time_t lastIdleCheckin = 0;

#define PUBLISH_INTERVAL (60 * 1000)

// if no motion for 3 minutes, sleep! (milliseconds)
#define NO_MOTION_IDLE_SLEEP_DELAY (3 * 60 * 1000)

// lets wakeup every 6 hours and check in (seconds)
#define HOW_LONG_SHOULD_WE_SLEEP (6 * 60 * 60)

// when we wakeup from deep-sleep not as a result of motion,
// how long should we wait before we publish our location?
// lets set this to less than our sleep time, so we always idle check in.
// (seconds)
#define MAX_IDLE_CHECKIN_DELAY (HOW_LONG_SHOULD_WE_SLEEP - 60)

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

    // TODO - for testing
    Particle.function("batt", particleBattery);
    Particle.function("gps", particleGPS);
}

void initAccel() {
    accel.begin(LIS3DH_DEFAULT_ADDRESS);
    accel.setDataRate(LIS3DH_DATARATE_LOWPOWER_5KHZ); // 5kHz low-power sampling
    accel.setRange(LIS3DH_RANGE_4_G); // 4 gravities range

    // listen for single-tap events at the threshold
    // keep the pin high for 1s, wait 1s between clicks
    //uint8_t c, uint8_t clickthresh, uint8_t timelimit, uint8_t timelatency, uint8_t timewindow
    accel.setClick(1, CLICKTHRESHHOLD);//, 0, 100, 50);
}

int particleBattery(String command){
  Serial.println(fuel.getSoC());
    Particle.publish("B",
          "v:" + String::format("%.2f",fuel.getVCell()) +
          ",c:" + String::format("%.2f",fuel.getSoC()),
          16777215, PRIVATE
    );
    return 1;
}

int particleGPS(String command) {
    publishGPS();
    return 1;
}

/* ====== LOOPING ===== */
void loop() {
    unsigned long now = millis();
    checkGPS();

    if (lastIdleCheckin == 0) { lastIdleCheckin = now; }
    if (lastMotion > now) { lastMotion = now; }
    if (lastPublish > now) { lastPublish = now; }

    // we'll be woken by motion, lets keep listening for more motion.
    // if we get two in a row, then we'll connect to the internet and start reporting in.
    bool hasMotion = digitalRead(WKP);
    digitalWrite(LED, (hasMotion) ? HIGH : LOW); // flash on motion
    if (hasMotion) {
        lastMotion = now;

        if (Particle.connected() == false) {
            Serial.println("CONNECTING DUE TO MOTION!");
            Particle.connect();
        }
    }

    // it's been too long!  Lets say hey!
    if ((now - lastIdleCheckin) >= MAX_IDLE_CHECKIN_DELAY) {
        if (Particle.connected() == false) {
            Serial.println("CONNECTING DUE TO IDLE!");
            Particle.connect();
        }

        Particle.publish(NAME + String("_status"), "miss you <3");
        lastIdleCheckin = now;
    }

    // have we published recently?
    if (((now - lastPublish) > PUBLISH_INTERVAL) || (lastPublish == 0)) {
        lastPublish = now;
        publishGPS();
    }

    // if nothing's happened for a while, go to sleep
    if ((now - lastMotion) > NO_MOTION_IDLE_SLEEP_DELAY) {
        Particle.publish(NAME + String("_status"), "sleeping!");

        lastPublish = 0;
        lastMotion = 0;

        digitalWrite(D6, HIGH); // Turn off GPS power draw
        delay(10*1000); // settle down before sleep
        System.sleep(SLEEP_MODE_DEEP, HOW_LONG_SHOULD_WE_SLEEP);
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

void publishGPS() {
  // Note the minus, longitutde is backwards for western hemisphere
  String latLong = String::format("%f,-%f", convertDegMinToDecDeg(GPS.latitude), convertDegMinToDecDeg(GPS.longitude));
  Serial.println(latLong);
  Particle.publish("G", latLong, 16777215, PRIVATE);

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
