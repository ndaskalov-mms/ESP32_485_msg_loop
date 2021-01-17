
// define maximal configuration per board. IF MODIFIED, COMPILE AND CHANGE SIMULTANEOUSLY BOTH MASTER AND SLAVES!!!!
#define MAX_ZONES_CNT      18    // Limited by ADC channels, 18 on SLAVE, less on MASTER. System voltages are read via mux and includded
#define SLAVE_ZONES_CNT   18    
#define MASTER_ZONES_CNT  12    
#define MAX_PGM_CNT       8     // 8 on MASTER, 2 on SLAVE
#define MASTER_PGM_CNT    8   
#define SLAVE_PGM_CNT     2
#define MAX_SLAVES 1
#define MASTER

constexpr int BITRATE = 115200;
constexpr int LOG_BITRATE = 115200;
HardwareSerial& logger(Serial);

const char configFileName[] = "/alarmConfig.cfg";

#define ERR_OK  (0)  
#define ERR_DEBUG  (-1)  
#define ERR_CRITICAL  (-16)                        // critical error occured, 

void ErrWrite(int err, const char * str) {
  logger.printf(str);
}

#include "C:\Users\Niki\Documents\GitHub\ESP32_485_msg_loop\zonestest\gpio-def.h"

#include "loadStore.h"

//
void loadStoreSetup() {
  alarmConfig.version = 10;
  alarmConfig.zoneConfigs[0] = 1;
  alarmConfig.zoneConfigs[sizeof(zonesDB)-1] = 99;
  alarmConfig.pgmConfigs[0] = 100;
  alarmConfig.pgmConfigs[sizeof(pgmDB)-1] = 200;
  logger.printf("size of zonesDB=%d\n",sizeof(zonesDB));
  logger.printf("size of pgmDB=%d\n",sizeof(pgmDB));
  storageSetup();
}

void setup() {
  // put your setup code here, to run once:
  logger.begin(LOG_BITRATE,SERIAL_8N1);
  logger.printf("\n\nStarting setup\n\n");
  loadStoreSetup();
  if(saveConfig(configFileName)) 
    logger.printf("Write config success\n");
  else
    logger.printf("Write config fail\n");
//
  if(readConfig(configFileName)) 
    logger.printf("Read config success\n");
  else
    logger.printf("Read config fail\n");
//
}

void loop() {
  // put your main code here, to run repeatedly:

}
