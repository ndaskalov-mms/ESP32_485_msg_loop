#define MASTER
#define SLAVE

HardwareSerial& logger(Serial);
constexpr int LOG_BITRATE = 115200;

#define ERR_DEBUG 1
#define ZONES_A_READ_INTERVAL 200     // read A zones at 100mS
#define ZONES_B_READ_INTERVAL 500     // read system voltages (B zones) at 100mS
#define MUX_SET_INTERVAL      10      // time to set the analog lines after mux switch
//
void ErrWrite(int err, const char * str) {
  logger.printf(str);
}
#define ERR_DB_INDEX_NOT_FND 5

//
#include "zonen.h"
//
//
void setup() {
  logger.begin(LOG_BITRATE,SERIAL_8N1);
  logger.printf("\n\nStaring setup\n\n");
  logger.printf("Master pgm cnt = %d\n", MASTER_PGM_CNT);
  logger.printf("Slave pgm cnt = %d\n", SLAVE_PGM_CNT);
  logger.printf("Master zones cnt = %d\n", MASTER_ZONES_CNT);
  logger.printf("Slave zones cnt = %d\n", SLAVE_ZONES_CNT);
  // put your setup code here, to run once:
  zoneSetup();
}
//
byte MzoneResult[MASTER_ZONES_CNT/2 + MASTER_ZONES_CNT%2];       //each zone will be in 4bits
byte SzoneResult[SLAVE_ZONES_CNT/2 + SLAVE_ZONES_CNT%2];          //each zone will be in 4bits

void loop() {
  //convertZones(MzoneDB, MASTER_ZONES_CNT, MzoneResult);
  convertZones(SzoneDB, SLAVE_ZONES_CNT, SzoneResult);

}
