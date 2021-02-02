#include "alarm-defs.h"
//
// alarm zones records structure to hold all alarm zones related info
//
struct ALARM_ZONE {
  byte  valid;					// data valid	
  byte  bypassed;			    // true if zone is bypassed
  byte  zoneStat;               // status of the zone switch. (open, close, short, line break
  byte  zoneDefs;				// zone type - enable, entry delay, follow, instant, stay, etc
  byte  zonePartition;          // assigned to partition X
  byte  zoneOptions;            // auto shutdown, bypass, stay, force, alarm type, intellyzone, dealyed transission
  byte  zoneExtOpt;             // zone tamper, tamper supervision, antimask, antimask supervision
  byte	lastZoneStat;			// last zone status reported
  unsigned long reportedAt;		// the time when the zone status was reported
  char  zoneName[16];           // user friendly name
};        
//
// alarm pgms records structure to hold all alarm pgms related info
//
struct ALARM_PGM {
  byte  valid; 					// data valid	
  //byte	boardID;				// the board which zones belong to. Master ID is 0	
  //byte  pgmID;                 // the number of zone by which the master will identify it. Zero based. Each ADC gpio produces one zone, but with two results  
  //byte  gpio;					// first members  are the same as struct ZONE
  byte  iValue;             // initial value
  byte  cValue;             // current
  char  pgmName[16];           // user friendly name
};
//
// keyswitch related staff
//
struct ALARM_KEYSW {
  byte  partition;				// Keyswitch can be assigned to one partition only. If == NO_PARTITION (0) the keyswitch is not defined/valid
  byte  type;					// disabled, momentary, maintained,  generate utility key on open/close, .... see enum  KEYSW_OPTS_t 
  byte  action;					// keyswitch action definition - see enum  KEYSW_ACTS_t 
  byte	boardID;				// the board of which zone will e used as keyswitch belong. Master ID is 0	
  byte  zoneID;                 // the number of zone that will e used as keyswitch
  char  keyswName[16];          // user friendly name
};  
//
struct ALARM_GLOBAL_OPTS_t {
	byte armRestrictions;
	byte troubleLatch;
	byte tamperOpts;
	byte antiMaskOpt;
	unsigned long entryDelay1Start;
	unsigned long entryDelay2Start;
};
//
// alarm partition 
//
struct PARTITION_OPTS_t {
	byte status;
	unsigned long armTime;	
	byte follows[MAX_PARTITION];
};
//
// zoneDB - database with all zones (master&slaves) info. Info from slaves are fetched via pul command over RS485
// TODO - use prep to get largest zone count
// All alarm zones zones organized as 2D array - [board][zones]. Contains data for all boards and zones in each board, incl. master
//
typedef struct ALARM_ZONE alarmZoneArr_t[MAX_SLAVES+1][MAX_ALARM_ZONES_PER_BOARD]; // every two zones here report for two contacts connected to one ADC channel
alarmZoneArr_t zonesDB;
//
//
// MASTER PGMs organized as 2D array. All pgms zones organized as 2D array - [board][pgms].
//
typedef struct ALARM_PGM alarmPgmArr_t[MAX_SLAVES+1][MAX_PGM_CNT];		        // typically master has more pgms than slave, so we use the largest denominator
alarmPgmArr_t pgmsDB;
//
// alarm keysw records structure to hold all alarm pgms related info
typedef struct ALARM_KEYSW alarmKeyswArr_t[MAX_KEYSW_CNT];		        
alarmKeyswArr_t keyswDB;
//
// alarm global options storage
struct ALARM_GLOBAL_OPTS_t  alarmGlobalOpts;
//
// alarm partition options storage
typedef struct PARTITION_OPTS_t alarmPartOpts_t[MAX_PARTITION];
alarmPartOpts_t alarmPartOpts;
//
//
// Struct to store the all alarm configuration
//
struct CONFIG_t {
  byte  version;
  byte  zoneConfig[sizeof(zonesDB)];
  byte  pgmConfig[sizeof(pgmsDB)];
  byte  keyswConfig[sizeof(keyswDB)];
  byte  alarmOptionsConfig[sizeof(alarmGlobalOpts)];
  byte  alarmPartConfig[sizeof(alarmPartOpts)];
  byte  csum;
} alarmConfig, tmpConfig;
//
#include "loadSafeConfTest/loadStore.h"
//
//  print alarm zones data
//  parms: (byte pointer) to array of ALARM_ZONE  containing the zones to be printed
//
void printAlarmZones(byte* zoneArrPtr, int startBoard, int endBoard) { 
    alarmZoneArr_t *zoneArr = (alarmZoneArr_t *)zoneArrPtr;
    for (int j = startBoard; j <= endBoard; j++) {
        logger.printf("      valid zoneStat zoneDefs zonePart zoneOpt zoneExtOpt zoneName\n");
        for (int i = 0; i < MAX_ALARM_ZONES_PER_BOARD; i++) {                        // iterate
           logger.printf ("Zone data: %2d\t%2d\t%2d\t%2d\t%2d\t%2d\t%2d\t%ul\t%16s\n",(*zoneArr)[j][i].valid, (*zoneArr)[j][i].zoneStat, (*zoneArr)[j][i].zoneDefs,\
                                                                          (*zoneArr)[j][i].zonePartition, (*zoneArr)[j][i].zoneOptions, (*zoneArr)[j][i].zoneExtOpt,\
                                                                          (*zoneArr)[j][i].lastZoneStat, (*zoneArr)[j][i].reportedAt, (*zoneArr)[j][i].zoneName);
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
	alarmPartOpts_t *prtArr = (alarmPartOpts_t *)PartArrPtr;
    logger.printf("      	status armTime follows\n");
	for (int j = 0; j <= maxPart; j++) {
		logger.printf ("Partitions status: %2d\t%u",(*prtArr)[j].status, (*prtArr)[j].armTime);
       for (int i = 0; i < MAX_PARTITION; i++) {                        // iterate
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
    // copy master zone defaults first
    for(int j=0; j<MASTER_ALARM_ZONES_CNT; j++) {     				// for each master's zone
        sprintf(zonesDB[MASTER_ADDRESS][j].zoneName, "Zone_%d", j);
        zonesDB[MASTER_ADDRESS][j].zoneDefs = 0;
        zonesDB[MASTER_ADDRESS][j].zonePartition = NO_PARTITION; 
        zonesDB[MASTER_ADDRESS][j].zoneOptions = BYPASS_EN  | FORCE_EN;   
        zonesDB[MASTER_ADDRESS][j].zoneExtOpt = 0;   
	    zonesDB[MASTER_ADDRESS][j].valid = validFlag;   	
    }
    //logger.printf("Master alarm zones from zonesDB\n");
	//printAlarmZones((byte *) zonesDB, MASTER_ADDRESS, MASTER_ADDRESS);
    // then init defaults for all slaves
    for(int i = SLAVE_ADDRESS1; i <= MAX_SLAVES; i++) {         // for each slave board 
        for(int j=0; j<SLAVE_ALARM_ZONES_CNT; j++) {     // for each zone
            sprintf(zonesDB[i][j].zoneName, "Zone_%d", j);
            zonesDB[i][j].zoneDefs = 0;
            zonesDB[i][j].zonePartition = NO_PARTITION; 
            zonesDB[i][j].zoneOptions = BYPASS_EN  | FORCE_EN;   
            zonesDB[i][j].zoneExtOpt = 0;
		    zonesDB[i][j].valid = validFlag; 
        }
        //logger.printf("Slave %d alarm zones from zonesDB\n", i);
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
    memset((void*)&keyswDB, 0, sizeof(keyswDB));                  // clear all data
    for(int j=0; j<MAX_KEYSW_CNT; j++) {                       
        sprintf(keyswDB[j].keyswName, "KSW_%d", j);
        keyswDB[j].partition = NO_PARTITION;     				// this is redundant, as 0 means no partition which effective disables it
	}
}
//
// set Alarm Global Opts Defaults
//
void setAlarmGlobalOptsDefaults() {
	ErrWrite(ERR_DEBUG, "Alarm Global Opts Defaults\n");
    memset((void*)&alarmGlobalOpts, 0, sizeof(alarmGlobalOpts));                  // clear all data
	alarmGlobalOpts.armRestrictions = 0;
	alarmGlobalOpts.troubleLatch = false;
	alarmGlobalOpts.tamperOpts	= ZONE_TAMPER_OPT_DISABLED;
	alarmGlobalOpts.antiMaskOpt = ZONE_ANTI_MASK_SUPERVISION_DISABLED;
}
//
// sets Alarm Partititons Options Defaults
//
void setAlarmPartOptsDefaults() {
	memset((void*)&alarmPartOpts, 0, sizeof(alarmPartOpts));                  // clear all data
	//for(int i = 0; i , MAX_PARTITION; i++) {
	//}
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
   memcpy((byte *) &alarmConfig.alarmPartConfig, (byte *) alarmPartOpts, sizeof(alarmConfig.alarmPartConfig)); 
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
	    memcpy((byte *) &alarmPartOpts, (byte *) alarmConfig.alarmPartConfig, sizeof(alarmPartOpts)); 
		//printAlarmKeysw((byte*) &keyswDB, MAX_KEYSW_CNT); 
	    printAlarmOpts((byte*) &alarmGlobalOpts); 
	    printAlarmPartOpts((byte*) &alarmPartOpts, MAX_PARTITION); 
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
/*
struct ALARM_ZONE {
  byte  valid;					// data valid	
  byte  bypassed;			    // true if zone is bypassed
  byte  zoneStat;               // status of the zone switch. (open, close, short, line break
  byte  zoneDefs;				// zone type - enable, entry delay, follow, instant, stay, etc
  byte  zonePartition;          // assigned to partition X
  byte  zoneOptions;            // auto shutdown, bypass, stay, force, alarm type, intellyzone, dealyed transission
  byte  zoneExtOpt;             // zone tamper, tamper supervision, antimask, antimask supervision
  char  zoneName[16];           // user friendly name
};  
//
enum ZONE_EXT_OPT_t {
    ZONE_TAMPER_OPT  = 0x1,
    ZONE_TAMPER_SUPERVISION = (0x2 | 0x4),
    ZONE_ANTI_MASK_TROUBLE  = 0x8,
    ZONE_ANTI_MASK_SUPERVISION = (0x10 | 0x20),
};
#define ZONE_TAMPER_OPT_DISABLED    0
#define ZONE_TAMPER_OPT_TROUBLE_ONLY  0x4
#define ZONE_TAMPER_OPT_ALARM_WHEN_ARMED  0x2
#define ZONE_TAMPER_OPT_ALARM  (0x4 | 0x2)
*/
//
// process zone error, generates trouble or alarm
//
void processZoneError(struct ALARM_ZONE zone) {
int tamperOpt;
	//if(!(zone.zoneExtOpt & ZONE_TAMPER_OPT))	// if local otpions are specified
		//tamperOpt = 								// no, follow the global tamper settings
}	
//
// alarmLoop() - implement all alarm business
//
void alarmLoop() {
int cz, cb;
  		for(cz = 0; cz < MASTER_ALARM_ZONES_CNT; cz++) {
				if(!(zonesDB[MASTER_ADDRESS][cz].valid || zonesDB[MASTER_ADDRESS][cz].bypassed))	// check if zone is no defined/in use or if bypassed
					continue;									// yes, continue with next one
				if(zonesDB[MASTER_ADDRESS][cz].zoneStat & ZONE_ERROR_MASK) {    // check for tampered zone or masked
					processZoneError(zonesDB[MASTER_ADDRESS][cz]);				// process zone error, generates trouble or alarm
					continue;
				}
		}
}
