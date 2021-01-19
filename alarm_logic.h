#include "loadSafeConfTest/loadStore.h"
//
//  print alarm zones data
//  parms: (byte pointer) to array of ALARM_ZONE  containing the zones to be printed
//
void printAlarmZones(byte* zoneArrPtr, int startBoard, int endBoard) { 
    alarmZoneArr_t *zoneArr = (alarmZoneArr_t *)zoneArrPtr;
    for (int j = startBoard; j <= endBoard; j++) {
        logger.printf("      valid boardID zoneID    gpio   mux zoneStat zoneDefs zonePart zoneOpt zoneExtOpt zoneName\n");
        for (int i = 0; i < MAX_ZONES_CNT; i++) {                        // iterate
           logger.printf ("Zone data: %2d\t%2d\t%2d\t%2d\t%2d\t%2d\t%2d\t%2d\t%2d\t%2d%16s\n",(*zoneArr)[j][i].valid, (*zoneArr)[j][i].boardID,(*zoneArr)[j][i].zoneID, (*zoneArr)[j][i].gpio,\
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
    alarmPgmArr_t *pgmArr = (alarmPgmArr_t *)pgmArrPtr;
    for (int j = startBoard; j <= endBoard; j++) {
        logger.printf("       valid boardID pgmID    gpio  iniVal pgmName\n");
        for (int i = 0; i < MAX_PGM_CNT; i++) {                        // iterate
           logger.printf ("PGM data: %2d\t%2d\t%2d\t%2d\t%2d%16s\n",(*pgmArr)[j][i].valid,(*pgmArr)[j][i].boardID, (*pgmArr)[j][i].pgmID, (*pgmArr)[j][i].gpio,\
                                                               (*pgmArr)[j][i].iValue, (*pgmArr)[j][i].pgmName);
        }
    }
}
//
// initializes master's zonesDB with default data from master's MzoneDB and SzonesDB templates as defined in compile time
// zonesDB contains MAX_SLAVE+1 arrays of struct ALARM_ZONE. Each array corresponds to one slave, and one is for master.
// 
void setAlarmZonesDefaults(bool validFlag) {
//
	ErrWrite(ERR_DEBUG, "Setting zones defaults\n");
    memset((void*)&zonesDB, 0, sizeof(zonesDB));              // clear all data, just in case
    // copy master zone defaults first
    for(int j=0; j<MASTER_ZONES_CNT; j++) {     				// for each master's zone
        sprintf(zonesDB[MASTER_ADDRESS][j].zoneName, "Zone_%d", j);
        zonesDB[MASTER_ADDRESS][j].boardID = MASTER_ADDRESS;     
        zonesDB[MASTER_ADDRESS][j].zoneID = MzoneDB[j].zoneID;
        zonesDB[MASTER_ADDRESS][j].gpio = MzoneDB[j].gpio;
        zonesDB[MASTER_ADDRESS][j].mux = MzoneDB[j].mux;
        zonesDB[MASTER_ADDRESS][j].zoneDefs = 0;
        zonesDB[MASTER_ADDRESS][j].zonePartition = NO_PARTITION; 
        zonesDB[MASTER_ADDRESS][j].zoneOptions = BYPASS_EN  | FORCE_EN;   
        zonesDB[MASTER_ADDRESS][j].zoneExtOpt = 0;   
	    zonesDB[MASTER_ADDRESS][j].valid = validFlag;   	
    }
    //logger.printf("Master zones from zonesDB\n");
	//printAlarmZones((byte *) zonesDB, MASTER_ADDRESS, MASTER_ADDRESS);
    // then init defaults for all slaves
    for(int i = SLAVE_ADDRESS1; i <= MAX_SLAVES; i++) {         // for each slave board 
        for(int j=0; j<SLAVE_ZONES_CNT; j++) {     // for each zone
            sprintf(zonesDB[i][j].zoneName, "Zone_%d", j);
            zonesDB[i][j].boardID = i;     
            zonesDB[i][j].zoneID = SzoneDB[j].zoneID;
			zonesDB[i][j].gpio 	 = SzoneDB[j].gpio;
			zonesDB[i][j].mux    = SzoneDB[j].mux;
            zonesDB[i][j].zoneDefs = 0;
            zonesDB[i][j].zonePartition = NO_PARTITION; 
            zonesDB[i][j].zoneOptions = BYPASS_EN  | FORCE_EN;   
            zonesDB[i][j].zoneExtOpt = 0;
		    zonesDB[i][j].valid = validFlag; 
        }
        //logger.printf("Slave %d zones from zonesDB\n", i);
        //printAlarmZones((byte *) zonesDB, i, i);
    }
}
//
// Copy default valuse from MpgmDB and SpgmDB into master's pgmsDB
//
void setAlarmPgmsDefaults(bool validFlag) {
//
	ErrWrite(ERR_DEBUG, "Setting pgms defaults\n");
    memset((void*)&pgmsDB, 0, sizeof(pgmsDB));                  // clear all data
        // copy master pgm defaults first
    for(int j=0; j<MASTER_PGM_CNT; j++) {                       // for each pgm
        sprintf(pgmsDB[MASTER_ADDRESS][j].pgmName, "PGM_%d", j);
        pgmsDB[MASTER_ADDRESS][j].boardID = MASTER_ADDRESS;     
        pgmsDB[MASTER_ADDRESS][j].pgmID = MpgmDB[j].rNum;
        pgmsDB[MASTER_ADDRESS][j].gpio =  MpgmDB[j].gpio;
        pgmsDB[MASTER_ADDRESS][j].iValue = MpgmDB[j].iValue;
		pgmsDB[MASTER_ADDRESS][j].valid = validFlag;
    }
    for(int i = SLAVE_ADDRESS1; i <= MAX_SLAVES; i++) {          // for each board 
        for(int j=0; j<SLAVE_PGM_CNT; j++) {                      // for each pgm
            sprintf(pgmsDB[i][j].pgmName, "PGM_%d", j);
            pgmsDB[i][j].boardID = i;   
            pgmsDB[i][j].pgmID = SpgmDB[j].rNum;
			pgmsDB[i][j].gpio = SpgmDB[j].gpio;
			pgmsDB[i][j].iValue = SpgmDB[j].iValue;
			pgmsDB[i][j].valid = validFlag;
        }
    }
}
//
// Initialize zones, pgm, parttitons, etc data storage in case there is no valid CONFIG FILE on storage
// uses globals zonesDB, pgmsDB, etc
// 1. initializes zonesDB with default data from MzonesDB and SzonesDB
// 2. copy zonesDB to alarmConfig
//
void setAlarmDefaults(bool validFlag) {
//    
   setAlarmZonesDefaults(validFlag);
   setAlarmPgmsDefaults(validFlag);
   // copy zonesDB into alarmConfig 
   memcpy((byte *) &alarmConfig.zoneConfigs, (byte *) zonesDB, sizeof(alarmConfig.zoneConfigs));
   // printAlarmZones((byte *) &alarmConfig.zoneConfigs, MASTER_ADDRESS, MAX_SLAVES);
   // copy pgmsDB into alarmConfig 
   memcpy((byte *) &alarmConfig.pgmConfigs, (byte *) pgmsDB, sizeof(alarmConfig.pgmConfigs)); 
   //printAlarmPgms((byte*) &alarmConfig.pgmConfigs, MASTER_ADDRESS, MAX_SLAVES); 
}
//
//  initAlarm() - tries to load complete alarm zones data from storage
//  if successful, sets global flag masterDataValid and remoteDataValid to true, this way enables operation.
//  if not successful read, initialize zones database (zonesDB) and sets global flags master DataValid and remoteDataValid to false
//  this flag will be checked in loop function and only MQTT channel will be enabled and the system will wait for alarm zones, part, etc data
//
void initAlarm() {
//   masterDataValid = false;                  	// bad or missing config file - maybe it is first run or storage is garbage
//   remoteDataValid = false;                 	// slaves data not fetched yet	
   if(FORCE_FORMAT_FS)
      formatStorage();
   memset((void*)&alarmConfig, 0, sizeof(alarmConfig));		// clear alarm config DB
   if(readConfig(configFileName))   {          //read config file  
		logger.printf("Copying from alarmConfig to zonesDB\n");
		memcpy((byte *) zonesDB, (byte *) &alarmConfig.zoneConfigs, sizeof(zonesDB)); // config OK, copy the databases
		memcpy((byte *) pgmsDB,  (byte *) &alarmConfig.pgmConfigs,  sizeof(pgmsDB)); // and return
//		masterDataValid = true;                  		// master data fetched successfully from storage
//		remoteDataValid = true;                 		// slaves data fetched successfully from storage	
		//printAlarmZones((byte *) &alarmConfig.zoneConfigs, MASTER_ADDRESS, MAX_SLAVES);
		//printAlarmZones((byte *) zonesDB, 0, 1);
		return;   
	    }
   //   wrong or missing config file
   ErrWrite(ERR_WARNING, "Wrong or missing config file\n");
   if(!ENABLE_CONFIG_CREATE) { 					  // do not create config, we have to wait for data from MQTT 	
		setAlarmDefaults(false);                  // init zones and pgms DBs with default data and set valid flag to false to all zones and pgms to true  
		return;									  // because dataValid flags are false, main loop will wait 	
        }
   // create config file with parms from default zones and pgms DBs, mostly used for testing 
   ErrWrite(ERR_WARNING, "Creating config file\n");
   setAlarmDefaults(true);                  	// set valid flag to all zones and pgms to true
   saveConfig(configFileName);              	// create the file, maybe this is the first ride TODO - make it to check only once
//   masterDataValid = true;                  	// master data fetched successfully from storage
//   remoteDataValid = true;                 		// slaves data fetched successfully from storage	
}

