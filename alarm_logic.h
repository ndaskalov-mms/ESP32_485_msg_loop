#include "loadSafeConfTest/loadStore.h"
//
// initializes master's zoneDB with data from master's MzoneDB and SzonesDB as defined in compile time
// zoneDB contains MAX_SLAVE+1 arrays of struct ZONE. Each array corresponds to one slave, and one is for master.
// 
void setAlarmZonesDefaults() {
//
    memset((void*)&zonesDB, 0, sizeof(zonesDB));       // clear all data, just in case
    // copy master zone defaults first
    for(int j=0; j<MASTER_ZONES_CNT; j++) {     // for each zone
        sprintf(zonesDB[i][j].zoneName, "Zone_%d", j);
        zonesDB[MASTER_ADDRESS][j].boardID = MASTER_ADDRESS;     
        zonesDB[MASTER_ADDRESS][j].zoneID = j;
        zonesDB[MASTER_ADDRESS][j].gpio = MzoneDB[j].gpio;
        zonesDB[MASTER_ADDRESS][j].mux = MzoneDB[j].mux;
        zonesDB[MASTER_ADDRESS][j].zoneDefs = 0;
        zonesDB[MASTER_ADDRESS][j].zonePartition = NO_PARTITION; 
        zonesDB[MASTER_ADDRESS][j].zoneOptions = BYPASS_EN  | FORCE_EN;   
        zonesDB[MASTER_ADDRESS][j].zoneExtOpt = 0;    
    }
    // then init defaults for all slaves
    for(int i = SLAVE_ADDRESS1; i <= MAX_SLAVES; i++) {         // for each slave board 
        for(int j=0; j<MSLAVE_ZONES_CNT; j++) {     // for each zone
            sprintf(zonesDB[i][j].zoneName, "Zone_%d", j);
            zonesDB[i][j].boardID = i;     
            zonesDB[i][j].zoneID = j;
            zonesDB[i][j].zoneDefs = 0;
            zonesDB[i][j].zonePartition = NO_PARTITION; 
            zonesDB[i][j].zoneOptions = BYPASS_EN  | FORCE_EN;   
            zonesDB[i][j].zoneExtOpt = 0;    
        }
    }
}
//
//
//
void setAlarmPgmsDefaults() {
//
    memset((void*)&zonesDB, 0, sizeof(pgmDB));       // clear all data
        // copy master zone defaults first
    for(int j=0; j<MASTER_PGM_CNT; j++) {     // for each zone
        sprintf(zonesDB[i][j].pgmName, "PGM_%d", j);
        zonesDB[MASTER_ADDRESS][j].boardID = MASTER_ADDRESS;     
        zonesDB[MASTER_ADDRESS][j].pgmID = j;
        zonesDB[MASTER_ADDRESS][j].gpio = MpgmDB[j].gpio;
        zonesDB[MASTER_ADDRESS][j].iValue = MpgmDB[j].iValue;
    }
    for(int i = SLAVE_ADDRESS; i <= MAX_SLAVES; i++) {         // for each board 
        for(int j=0; j<MAX_PGM_CNT; j++) {     // for each zone
            sprintf(zonesDB[i][j].zoneName, "PGM_%d", j);
            zonesDB[i][j].boardID = i;     
            zonesDB[i][j].pgmID = j;
        }
    }
}
}
//
//
// Initialize zones, pgm, parttitons, etc data storage in case there is no saved copy on storage
// uses globals zonesDB, pgmDB, etc
//
void setAlarmDefaults() {
//    
   setAlarmZonesDefaults();
   setAlarmPgmsDefaults();
}    

//
//  print alarm zones data
//  parms: struct ZONE DB[]  - (pointer) to array of ZONE  containing the zones to be printed
//
void printAlarmZones(int startBoard, int endBoard) { 
    for (int j = startBoard; j <= endBoard; j++) {
        logger.printf("       boardID zoneID    gpio   mux zoneStat zoneDefs zonePart zoneOpt zoneExtOpt zoneName\n");
        for (int i = 0; i < MAX_ZONES_CNT; i++) {                        // iterate
           logger.printf ("Zone data: %2d\t%2d\t%2d\t%2d\t%2d\t%2d\t%2d\t%2d\t%2d%16s\n",zonesDB[j][i].boardID, zonesDB[j][i].zoneID, zonesDB[j][i].gpio,\
                                                                                         zonesDB[j][i].mux, zonesDB[j][i].zoneABstat, zonesDB[j][i].zoneDefs,\
                                                                                         zonesDB[j][i].zonePartition, zonesDB[j][i].zoneOptions, zonesDB[j][i].zoneExtOpt,\
                                                                                         zonesDB[j][i].zoneName);
        }
    }
}
//
//  initAlarm() - tries to load complete alarm zones data from storage
//  if successful, sets global flag alarmDataValid to true, this way enables operation.
//  if not successful read, initialize empty zones database (zonesDB) and sets global flag alarmDataValid to false
//  this flag will be checked in loop function and only MQTT channel will be enabled and the system will wait for alarm zones, part, etc data
//
int initAlarm() {
   if(!readConfig(configFileName))   {          //read config file  
        setAlarmDefaults();
#ifdef ENABLE_CONFIG_CREATE
        if(!saveConfig(configFileName))          // check if we can create the file, maybe this is the first ride TODO - make it to check only once
#endif
            return false;
   }
   return true;                                 // we got the database 
}
