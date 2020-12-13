
HardwareSerial& logger(Serial);
constexpr int LOG_BITRATE = 115200;

#define ERR_DEBUG 1
#define ZONES_READ_THROTTLE false
#define ZONES_READ_INTERVAL 100     // read all zones at 100mS

#include "zonen-pgm-test.h"

unsigned long lastRead = 0;

void setup() {
  logger.printf("Starting setup\n");
  logger.begin(LOG_BITRATE,SERIAL_8N1);
  logger.printf("\n\nStaring setup\n\n");
  // put your setup code here, to run once:
  zoneSetup();
}

void loop() {
  convertZones();
}
