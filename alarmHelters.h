//
// alarmHelpers.h - init defaults, print and other helper functions 
//
//
//  print alarm zones data
//  parms: (byte pointer) to array of ALARM_ZONE  containing the zones to be printed
//
void printAlarmZones(byte* zoneArrPtr, int startBoard, int endBoard) { 
    alarmZoneArr_t *zoneArr = (alarmZoneArr_t *)zoneArrPtr;
    for (int j = startBoard; j <= endBoard; j++) {
        logger.printf("      valid zoneStat zoneType zonePart zoneOpt zoneExtOpt zoneName\n");
        for(int i=0; i< (j?SLAVE_ALARM_ZONES_CNT:MASTER_ALARM_ZONES_CNT); i++) {             // for each board' zone
           logger.printf ("Zone data: %2d\t%2d\t%2d\t%2d\t%2d\t%2d\t%ul\t%16s\n",(*zoneArr)[j][i].valid, (*zoneArr)[j][i].zoneStat, (*zoneArr)[j][i].zoneType,\
                                                                          (*zoneArr)[j][i].zonePartition, (*zoneArr)[j][i].zoneOptions, (*zoneArr)[j][i].zoneExtOpt,\
                                                                          /*(*zoneArr)[j][i].lastZoneStat,*/ (*zoneArr)[j][i].reportedAt, (*zoneArr)[j][i].zoneName);
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
           logger.printf ("PGM data: %2d\t%2d%16s\n",(*pgmArr)[j][i].valid,(*pgmArr)[j][i].iValue, (*pgmArr)[j][i].pgmName);
        }
    }
}
//
//  print alarm keysw data
//  parms: (byte pointer) to array of ALARM_KEYSW  containing the keysw to be printed
//
void printAlarmKeysw(byte* keyswArrPtr, int maxKeysw) { 
    alarmKeyswArr_t *pgmArr = (alarmKeyswArr_t *)keyswArrPtr;
	logger.printf("      partition; type; action; boardID;	zoneID;  keyswName[16];\n");
	for (int i = 0; i < maxKeysw; i++) {                        // iterate
	   logger.printf ("KeySwitch data: %2d\t%2d\t%2d\t%2d\t%2d%16s\n",(*pgmArr)[i].partition,(*pgmArr)[i].type, (*pgmArr)[i].action, (*pgmArr)[i].boardID,\
														   (*pgmArr)[i].zoneID, (*pgmArr)[i].keyswName);
	}
}
//
// print partition options
//
void printAlarmPartOpts(byte * PartArrPtr, int maxPart) {
	alarmPartArr_t *prtArr = (alarmPartArr_t *)PartArrPtr;
  logger.printf("      	      status armTime followZone2entryDelay2              follows\n");
	for (int j = 0; j < maxPart; j++) {
		logger.printf ("Partitions status: %2d\t%d\t %u    ",(*prtArr)[j].armStatus, (*prtArr)[j].followZone2entryDelay2, (*prtArr)[j].armTime);
       for (int i = 0; i < MAX_PARTITION; i++) {                        
			  logger.printf(" %d", (*prtArr)[j].follows[i]);
		}
		logger.printf("\n");
  }
}
//
// print Alarm Global options
//
void printAlarmOpts(byte * optsPtr) {
	ALARM_GLOBAL_OPTS_t *opts = (ALARM_GLOBAL_OPTS_t *) optsPtr;
	logger.printf("Alarm Global Options armRestrictions %d,  troubleLatch %d, tamperOpts %d, antiMaskOpt %d entryDelay1Start %u, entryDelay2Start %u\n",\
				  opts->armRestrictions, opts->troubleLatch, opts->tamperOpts, opts->antiMaskOpt, opts->entryDelay1Start, opts->entryDelay2Start);
}
//
// initializes master's zonesDB with default data from master's MzoneDB and SzonesDB templates as defined in compile time
// zonesDB contains MAX_SLAVE+1 arrays of struct ALARM_ZONE. Each array corresponds to one slave, and one is for master.
// 
void setAlarmZonesDefaults(bool validFlag) {
//
	ErrWrite(ERR_DEBUG, "Setting alarm zones defaults\n");
    memset((void*)&zonesDB, 0, sizeof(zonesDB));              // clear all data, just in case
    for(int i = 0; i <= MAX_SLAVES; i++) {         // for each board 
        for(int j=0; j< (i?SLAVE_ALARM_ZONES_CNT:MASTER_ALARM_ZONES_CNT); j++) {             // for each board' zone
          sprintf(zonesDB[i][j].zoneName, "Zone_%d", j);
          zonesDB[i][j].zoneType = 0;
          zonesDB[i][j].zonePartition = PARTITION1; 
          zonesDB[i][j].zoneOptions = BYPASS_EN  | FORCE_EN;   
          zonesDB[i][j].zoneExtOpt = 0;
  		    zonesDB[i][j].valid = validFlag; 
          }
        logger.printf("Board %d alarm zones from zonesDB\n", i);
        printAlarmZones((byte *) zonesDB, i, i);
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
        pgmsDB[MASTER_ADDRESS][j].iValue = MpgmDB[j].iValue;
		pgmsDB[MASTER_ADDRESS][j].valid = validFlag;
    }
    for(int i = SLAVE_ADDRESS1; i <= MAX_SLAVES; i++) {          // for each board 
        for(int j=0; j<SLAVE_PGM_CNT; j++) {                      // for each pgm
            sprintf(pgmsDB[i][j].pgmName, "PGM_%d", j);
			pgmsDB[i][j].iValue = SpgmDB[j].iValue;
			pgmsDB[i][j].valid = validFlag;
        }
    }
}
//
// set kesy switches defaults
void setAlarmKeyswDefaults() {
//
	ErrWrite(ERR_DEBUG, "Setting keysw defaults\n");
    memset((void*)&keyswDB, 0, sizeof(keyswDB));                // clear all data
    for(int j=0; j<MAX_KEYSW_CNT; j++) {                       
        sprintf(keyswDB[j].keyswName, "KSW_%d", j);
        keyswDB[j].partition = PARTITION1;     				// this is redundant, as 0 means no partition which effective disables it
	}
}
//
// set Alarm Global Opts Defaults
//
void setAlarmGlobalOptsDefaults() {
	ErrWrite(ERR_DEBUG, "Setting Alarm Global Opts Defaults\n");
    memset((void*)&alarmGlobalOpts, 0, sizeof(alarmGlobalOpts)); // clear all data
	alarmGlobalOpts.maxSlaveBrds = MAX_SLAVES;
	alarmGlobalOpts.troubleLatch = false;
	alarmGlobalOpts.tamperOpts	= ZONE_TAMPER_OPT_DISABLED;		// follow zone settings for tamper occurs
	alarmGlobalOpts.antiMaskOpt = ZONE_ANTI_MASK_SUPERVISION_DISABLED;	// follow zone settings for anti-mask
}
//
// sets Alarm Partititons Options Defaults
//
void setAlarmPartOptsDefaults() {
	memset((void*)&partitionDB, 0, sizeof(partitionDB));                  // clear all data
#ifdef TEST_PART_FOLLOW
  logger.printf("Setting random partititon follows\t");
  for(int i = 0; i < MAX_PARTITION; i++) {
    partitionDB[i].followZone2entryDelay2 = true;
    for(int j = 0; j < MAX_PARTITION; j++) 
      partitionDB[i].follows[j] = (byte)(rand()&1);
  }
  logger.printf("Done\n");
  printAlarmPartOpts((byte*) &partitionDB, MAX_PARTITION);
  logger.printf("Done\n"); 
#endif
}
//
// Initialize zones, pgm, parttitons, etc data storage in case there is no valid CONFIG FILE on storage
// uses globals zonesDB, pgmsDB, etc
// 1. initializes zonesDB with default data from MzoneDB and SzonesDB
// 2. copy zonesDB to alarmConfig
//
void setAlarmDefaults(bool validFlag) {
//    
   setAlarmZonesDefaults(validFlag);
   setAlarmPgmsDefaults(validFlag);
   setAlarmKeyswDefaults();
   setAlarmGlobalOptsDefaults();
   setAlarmPartOptsDefaults();
   // copy zonesDB into alarmConfig 
   memcpy((byte *) &alarmConfig.zoneConfig, (byte *) zonesDB, sizeof(alarmConfig.zoneConfig));
   // printAlarmZones((byte *) &alarmConfig.zoneConfig, MASTER_ADDRESS, MAX_SLAVES);
   // copy pgmsDB into alarmConfig 
   memcpy((byte *) &alarmConfig.pgmConfig, (byte *) pgmsDB, sizeof(alarmConfig.pgmConfig)); 
   //printAlarmPgms((byte*) &alarmConfig.pgmConfig, MASTER_ADDRESS, MAX_SLAVES); 
   memcpy((byte *) &alarmConfig.keyswConfig, (byte *) keyswDB, sizeof(alarmConfig.keyswConfig)); 
   //printAlarmKeysw((byte*) &alarmConfig.keyswConfig, MAX_KEYSW_CNT); 
   memcpy((byte *) &alarmConfig.alarmOptionsConfig, (byte *) &alarmGlobalOpts, sizeof(alarmConfig.alarmOptionsConfig)); 
   printAlarmOpts((byte*) &alarmConfig.alarmOptionsConfig); 
   memcpy((byte *) &alarmConfig.alarmPartConfig, (byte *) partitionDB, sizeof(alarmConfig.alarmPartConfig)); 
   printAlarmPartOpts((byte*) &alarmConfig.alarmPartConfig, MAX_PARTITION); 
}
//
//  initAlarm() - tries to load complete alarm zones data from storage
//  if successful, sets global flag masterDataValid and remoteDataValid to true, this way enables operation.
//  if not successful read, initialize zones database (zonesDB) and sets global flags master DataValid and remoteDataValid to false
//  this flag will be checked in loop function and only MQTT channel will be enabled and the system will wait for alarm zones, part, etc data
//
void initAlarm() {
   if(FORCE_FORMAT_FS)
      formatStorage();
   memset((void*)&alarmConfig, 0, sizeof(alarmConfig));		// clear alarm config DB
   if(readConfig(configFileName))   {          //read config file  
  		logger.printf("Copying from alarmConfig to zonesDB\n");
  		memcpy((byte *) zonesDB, (byte *) &alarmConfig.zoneConfig, sizeof(zonesDB)); // config OK, copy the databases
  		memcpy((byte *) pgmsDB,  (byte *) &alarmConfig.pgmConfig,  sizeof(pgmsDB)); // and return
  	    memcpy((byte *) keyswDB, (byte *) &alarmConfig.keyswConfig, sizeof(keyswDB)); 
  		memcpy((byte *) &alarmGlobalOpts, (byte *) alarmConfig.alarmOptionsConfig, sizeof(alarmGlobalOpts)); 
  	    memcpy((byte *) &partitionDB, (byte *) alarmConfig.alarmPartConfig, sizeof(partitionDB)); 
  		//printAlarmKeysw((byte*) &keyswDB, MAX_KEYSW_CNT); 
  	    printAlarmOpts((byte*) &alarmGlobalOpts); 
  	    printAlarmPartOpts((byte*) &partitionDB, MAX_PARTITION); 
  		//printAlarmZones((byte *) &alarmConfig.zoneConfig, MASTER_ADDRESS, MAX_SLAVES);
  		//printAlarmZones((byte *) zonesDB, 0, 1);
  		return;   
	 }
   //   wrong or missing config file
   ErrWrite(ERR_WARNING, "Wrong or missing config file\n");
   if(!ENABLE_CONFIG_CREATE) { 					  // do not create config, we have to wait for data from MQTT 	
  		setAlarmDefaults(false);                  // init zones and pgms DBs with default data and set valid flag to false to all zones and pgms 
  		logger.printf("Looping for getting alarm settings from MQTT\n");
  		while(true) ;							  
  		return;									  // because dataValid flags are false, main loop will wait 	
   }
   // create config file with parms from default zones and pgms DBs, mostly used for testing 
   ErrWrite(ERR_WARNING, "Creating config file\n");
   setAlarmDefaults(true);                  	// set valid flag to all zones and pgms to true
   saveConfig(configFileName);              	// create the file, maybe this is the first ride TODO - make it to check only once
   logger.printf("Setted alarm defaults for testing\n");
}