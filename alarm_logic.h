#include "storage.h"
//
// initializes master's zoneDB with data from master's MzoneDB and SzonesDB as defined in compile time
// zoneDB contains MAX_SLAVE+1 arrays of struct ZONE. Each array corresponds to one slave, and one is for master.
// 
void initAlarmZones() {
//
    memset((void*)&zonesDB, 0, sizeof(zonesDB));       // clear all data
    for(int i = MASTER_ADDRESS; i <= MAX_SLAVES; i++) {         // for each board incl master (+1 to account master as well)
        for(int j=0; j<MAX_ZONES_CNT; j++) {     // for each zone
            if((i==MASTER_ADDRESS) && (j==MASTER_ZONES_CNT))          // skip unused zones at master (??)
                break;
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
void initAlarmPgms() {
}
//
//
// Initialize zones, pgm, parttitons, etc data storage in case there is no saved copy on storage
// uses globals zonesDB, pgmDB, etc
//
void setAlarmDefaults() {
//    
   initAlarmZones();
   initAlarmPgms();
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
int loadAlarmDataStorage(struct ALARM_ZONE zonesDB[][MAX_ZONES_CNT]) {
    return false;
}
//
//  initAlarm() - tries to load complete alarm zones data from storage
//  if successful, sets global flag alarmDataValid to true, this way enables operation.
//  if not successful read, initialize empty zones database (zonesDB) and sets global flag alarmDataValid to false
//  this flag will be checked in loop function and only MQTT channel will be enabled and the system will wait for alarm zones, part, etc data
//
int initAlarm() {
    if (loadAlarmDataStorage(zonesDB))           // Check 
        return true;
    setAlarmDefaults();

    return false;
}
