#include <esp_littlefs.h>
#include <lfs.h>
#include <lfs_util.h>
#include <LITTLEFS.h>
#include <littlefs_api.h>
#define SPIFFS LITTLEFS

// define maximal configuration per board. IF MODIFIED, COMPILE AND CHANGE SIMULTANEOUSLY BOTH MASTER AND SLAVES!!!!
#define MAX_ZONES_CNT      18    // Limited by ADC channels, 18 on SLAVE, less on MASTER. System voltages are read via mux and includded
#define SLAVE_ZONES_CNT   18    
#define MASTER_ZONES_CNT  12    
#define MAX_PGM_CNT       8     // 8 on MASTER, 2 on SLAVE
#define MASTER_PGM_CNT    8   
#define SLAVE_PGM_CNT     2
#define MAX_SLAVES 1
#define MASTER

#define FORMAT_LITTLEFS_IF_FAILED true

#include "C:\Users\Niki\Documents\GitHub\ESP32_485_msg_loop\zonestest\gpio-def.h"


struct CONFIG_t {
  byte  version;
  byte  zoneConfigs[sizeof(zonesDB)];
  byte  pgmConfigs[sizeof(pgmDB)];
} alarmConfig, valConfig;


constexpr int BITRATE = 115200;
constexpr int LOG_BITRATE = 115200;
HardwareSerial& logger(Serial);
//
// init storage
// return: true on succsess WARNING - if storage is not formatted, formats it
//         fail on failure
//
int storageSetup() {
//
    logger.println("Inizializing FS...");
    if (SPIFFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println(F("done."));
    }else{
        Serial.println(F("fail."));
    }

}

int safeConfig() {
    // To remove previous test
    // SPIFFS.remove(F("/testCreate.txt"));
 
    File testFile = SPIFFS.open(F("/alarmConfig.bin"), "w");
    
    if (testFile){
        Serial.println("Write file content!");
        testFile.write((byte*) &alarmConfig, sizeof(alarmConfig));
        testFile.close();
        }
    else{
        Serial.println("Problem on create file!");
        return false;
        }
    // readback and verify
    testFile = SPIFFS.open(F("/alarmConfig.bin"), "r");
    if (testFile){
        Serial.println("Read file content!");
        if(int rlen = testFile.read((byte*) &valConfig, sizeof(valConfig)) != sizeof(valConfig))
          Serial.printf("Problem on reading file! Read %d expected %d\n",rlen, sizeof(valConfig) );
        testFile.close();
        for(int i=0; i<sizeof(valConfig); i++) {
          Serial.printf("Index %d content %d", i, ((byte*) &valConfig)[i]);
          Serial.println("");
         }
        }
     else
        Serial.println("Problem on read file!");
}
void setup() {
  // put your setup code here, to run once:
  logger.begin(LOG_BITRATE,SERIAL_8N1);
  logger.printf("\n\nStarting setup\n\n");
  alarmConfig.version = 10;
  alarmConfig.zoneConfigs[0] = 1;
  alarmConfig.zoneConfigs[sizeof(zonesDB)-1] = 99;
  alarmConfig.pgmConfigs[0] = 100;
  alarmConfig.pgmConfigs[sizeof(pgmDB)-1] = 200;
  logger.printf("size of zonesDB=%d\n",sizeof(zonesDB));
  logger.printf("size of pgmDB=%d\n",sizeof(pgmDB));
  storageSetup();
  safeConfig();
}

void loop() {
  // put your main code here, to run repeatedly:

}
