#include "loadSafeConfTest/loadStore.h"
//
//  print alarm zones data
//  parms: (byte pointer) to array of ALARM_ZONE  containing the zones to be printed
//
void printAlarmZones(byte* zoneArrPtr, int startBoard, int endBoard) { 
    alarmZonePtr_t *zoneArr = (alarmZonePtr_t *)zoneArrPtr;
    for (int j = startBoard; j <= endBoard; j++) {
        logger.printf("       boardID zoneID    gpio   mux zoneStat zoneDefs zonePart zoneOpt zoneExtOpt zoneName\n");
        for (int i = 0; i < MAX_ZONES_CNT; i++) {                        // iterate
           logger.printf ("Zone data: %2d\t%2d\t%2d\t%2d\t%2d\t%2d\t%2d\t%2d\t%2d%16s\n",(*zoneArr)[j][i].boardID, (*zoneArr)[j][i].zoneID, (*zoneArr)[j][i].gpio,\
                                                                                         (*zoneArr)[j][i].mux, (*zoneArr)[j][i].zoneABstat, (*zoneArr)[j][i].zoneDefs,\
                                                                                         (*zoneArr)[j][i].zonePartition, (*zoneArr)[j][i].zoneOptions, (*zoneArr)[j][i].zoneExtOpt,\
                                                                                         (*zoneArr)[j][i].zoneName);
        }
    }
}
//
//  print alarm pgms data
//  parms: (byte pointer) to array of ALARM_PGM  containing the zones to be printed
//
void printAlarmPgms(byte* pgmArrPtr, int startBoard, int endBoard) { 
    alarmPgmPtr_t *pgmArr = (alarmPgmPtr_t *)pgmArrPtr;
    for (int j = startBoard; j <= endBoard; j++) {
        logger.printf("       boardID pgmID    gpio  iniVal pgmName\n");
        for (int i = 0; i < MAX_PGM_CNT; i++) {                        // iterate
           logger.printf ("PGM data: %2d\t%2d\t%2d\t%2d%16s\n",(*pgmArr)[j][i].boardID, (*pgmArr)[j][i].pgmID, (*pgmArr)[j][i].gpio,\
                                                               (*pgmArr)[j][i].iValue, (*pgmArr)[j][i].pgmName);
        }
    }
}
//
// initializes master's zoneDB with data from master's MzoneDB and SzonesDB as defined in compile time
// zoneDB contains MAX_SLAVE+1 arrays of struct ZONE. Each array corresponds to one slave, and one is for master.
// 
void setAlarmZonesDefaults() {
//
	ErrWrite(ERR_DEBUG, "Setting zones defaults\n");
    memset((void*)&zonesDB, 0, sizeof(zonesDB));              // clear all data, just in case
    // copy master zone defaults first
    for(int j=0; j<MASTER_ZONES_CNT; j++) {     				// for each master's zone
        sprintf(zonesDB[MASTER_ADDRESS][j].zoneName, "Zone_%d", j);
        zonesDB[MASTER_ADDRESS][j].boardID = MASTER_ADDRESS;     
        zonesDB[MASTER_ADDRESS][j].zoneID = j;
        zonesDB[MASTER_ADDRESS][j].gpio = MzoneDB[j].gpio;
        zonesDB[MASTER_ADDRESS][j].mux = MzoneDB[j].mux;
        zonesDB[MASTER_ADDRESS][j].zoneDefs = 0;
        zonesDB[MASTER_ADDRESS][j].zonePartition = NO_PARTITION; 
        zonesDB[MASTER_ADDRESS][j].zoneOptions = BYPASS_EN  | FORCE_EN;   
        zonesDB[MASTER_ADDRESS][j].zoneExtOpt = 0;    
    }
    //logger.printf("Master zones from zonesDB\n");
	//printAlarmZones((byte *) zonesDB, MASTER_ADDRESS, MASTER_ADDRESS);
    // then init defaults for all slaves
    for(int i = SLAVE_ADDRESS1; i <= MAX_SLAVES; i++) {         // for each slave board 
        for(int j=0; j<SLAVE_ZONES_CNT; j++) {     // for each zone
            sprintf(zonesDB[i][j].zoneName, "Zone_%d", j);
            zonesDB[i][j].boardID = i;     
            zonesDB[i][j].zoneID = j;
            zonesDB[i][j].zoneDefs = 0;
            zonesDB[i][j].zonePartition = NO_PARTITION; 
            zonesDB[i][j].zoneOptions = BYPASS_EN  | FORCE_EN;   
            zonesDB[i][j].zoneExtOpt = 0;    
        }
        //logger.printf("Slave %d zones from zonesDB\n", i);
        //printAlarmZones((byte *) zonesDB, i, i);
    }
}
//
//
//
void setAlarmPgmsDefaults() {
//
	ErrWrite(ERR_DEBUG, "Setting pgms defaults\n");
    memset((void*)&pgmsDB, 0, sizeof(pgmsDB));                  // clear all data
        // copy master pgm defaults first
    for(int j=0; j<MASTER_PGM_CNT; j++) {                       // for each pgm
        sprintf(pgmsDB[MASTER_ADDRESS][j].pgmName, "PGM_%d", j);
        pgmsDB[MASTER_ADDRESS][j].boardID = MASTER_ADDRESS;     
        pgmsDB[MASTER_ADDRESS][j].pgmID = j;
        pgmsDB[MASTER_ADDRESS][j].gpio = MpgmDB[j].gpio;
        pgmsDB[MASTER_ADDRESS][j].iValue = MpgmDB[j].iValue;
    }
    for(int i = SLAVE_ADDRESS1; i <= MAX_SLAVES; i++) {          // for each board 
        for(int j=0; j<MAX_PGM_CNT; j++) {                      // for each pgm
            sprintf(pgmsDB[i][j].pgmName, "PGM_%d", j);
            pgmsDB[i][j].boardID = i;   
            pgmsDB[i][j].pgmID = j;
        }
    }
}
//
//


//typedef int Array2D[ROW][COL]

//
// Initialize zones, pgm, parttitons, etc data storage in case there is no saved copy on storage
// uses globals zonesDB, pgmsDB, etc
//
void setAlarmDefaults() {
//    
   setAlarmZonesDefaults();
   setAlarmPgmsDefaults();
   memcpy((byte *) &alarmConfig.zoneConfigs, (byte *) zonesDB, sizeof(alarmConfig.zoneConfigs));
   //printAlarmZones((byte *) &alarmConfig.zoneConfigs, MASTER_ADDRESS, MAX_SLAVES);
   memcpy((byte *) &alarmConfig.pgmConfigs, (byte *) pgmsDB, sizeof(alarmConfig.pgmConfigs)); 
   printAlarmPgms((byte*) &alarmConfig.pgmConfigs, MASTER_ADDRESS, MAX_SLAVES); 
}    
//
//  initAlarm() - tries to load complete alarm zones data from storage
//  if successful, sets global flag alarmDataValid to true, this way enables operation.
//  if not successful read, initialize empty zones database (zonesDB) and sets global flag alarmDataValid to false
//  this flag will be checked in loop function and only MQTT channel will be enabled and the system will wait for alarm zones, part, etc data
//
int initAlarm() {
   if(FORCE_FORMAT_FS)
      formatStorage();
   if(!readConfig(configFileName))   {          //read config file  
      ErrWrite(ERR_WARNING, "Wrong or missing config file\n");
      setAlarmDefaults();                       // wrong config file
      if(ENABLE_CONFIG_CREATE){                 // create config file with default parms if enabled
        ErrWrite(ERR_WARNING, "Creating config file\n");
        saveConfig(configFileName);              // create the file, maybe this is the first ride TODO - make it to check only once
        }
      return false;
      }
   return true;                                 // we got the database 
}
