#include "alarm-defs.h"
//
// alarm zones records structure to hold all alarm zones related info
//
struct ALARM_ZONE {
  byte  valid;					// data valid	
  byte  bypassed;			    // true if zone is bypassed
  byte  zoneStat;               // status of the zone switch. (open, close, short, line break
  byte  zoneType;				// zone type - enable, entry delay, follow, instant, stay, etc
  byte  zonePartition;          // assigned to partition X
  byte  zoneOptions;            // auto shutdown, bypass, stay, force, alarm type, intellyzone, dealyed transission
  byte  zoneExtOpt;             // zone tamper, tamper supervision, antimask, antimask supervision
  //byte	lastZoneStat;			// last zone status reported
  unsigned long reportedAt;		// the time when the zone status was reported
  char  zoneName[16];           // user friendly name
};        
//
// alarm pgms records structure to hold all alarm pgms related info
//
struct ALARM_PGM {
  byte  valid; 					// data valid	
  byte  iValue;             	// initial value
  byte  cValue;             	// current
  char  pgmName[16];           	// user friendly name
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
	byte maxSlaveBrds;				// how many slaves are installed. Run-time this value is copied to maxSlaves
	int  armRestrictions;			// all DBs are sized to MAX_SLAVES compile time, means maxSlaveBrds =< maxSlave !!!!
	byte troubleLatch;				// if trouble, latch it or not
	byte tamperBypassEn;			// true - if zone is bypassed ignore tamper; false - follow global or local tamper settings
	byte tamperOpts;				// global tamper optons, same as local - see #define ZONE_TAMPER_OPT_XXXXXX
	byte antiMaskOpt;				// global antimask optons, same as local - see #define ZONE_ANTI_MASK_SUPERVISION__XXXXXX
	byte rfSupervisionOpt;			// wireless sensors supervision see RF_SUPERVISION_XXXX
	unsigned long entryDelay1Start;	//to store the time when entry delay 1 zone opens
	unsigned long entryDelay2Start; //to store the time when entry delay 2 zone opens
};
//
// alarm partition 
//
struct ALARM_PARTITION_t {
	byte armStatus;					//  bitmask, DISARM = 0, REGULAR_ARM, FORCE_ARM, INSTANT_ARM, STAY_ARM 
	byte forceOnRegularArm;			// allways use force arm (bypass open zones) when regular arming
	byte forceOnStayArm;			// allways use force arm (bypass open zones) when stay arming
	byte followZone2entryDelay2;	// if and entry delay zone is bypassed and follow zone is opens, the alarm will be postponed by EntryDelay2 
	byte notBypassedEntyDelayZones; // used to triger follow zones to use ENTRY_DELAY2 if no more notBypassedEntyDelayZones
	byte alarmOutputEn;				// enable to triger bell or siren once alarm condition is detected in partition
	byte alarmCutOffTime;			// cut alarm output after 1-255 seconds
	byte noCutOffOnFire;			// disable cut-off for fire alarms
	byte alarmRecycleTime;			// re-enable after this time if alarm condition not fixed
	unsigned long armTime;			// arm time
	byte entryDelay1Interval;		// entry delay 1 delay in seconds 1-255
	byte entryDelay2Interval;		// entry delay 2 delay in seconds 1-255
	unsigned long entryDelay1;		// entry delay 1 delay start time in mS
	unsigned long entryDelay2;		// entry delay 2 delay start time in mS
	byte exitDelay;					// exit delay in seconds 1-255
	byte partitionName[16];			// user readable name
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
typedef struct ALARM_PARTITION_t alarmPartArr_t[MAX_PARTITION];
alarmPartArr_t partitionDB;
//
// Struct to store the all alarm configuration
//
struct CONFIG_t {
  byte  version;
  byte  zoneConfig[sizeof(zonesDB)];
  byte  pgmConfig[sizeof(pgmsDB)];
  byte  keyswConfig[sizeof(keyswDB)];
  byte  alarmOptionsConfig[sizeof(alarmGlobalOpts)];
  byte  alarmPartConfig[sizeof(partitionDB)];
  byte  csum;
} alarmConfig, tmpConfig;
//
#include "loadSafeConfTest/loadStore.h"
#include "alarmHelpers.h"
//
// set or get partition ENTRY_DELAY or ENTRY_DELAY2 timer
// params: timer - int ENTRY_DELAY1 or ENTRY_DELAY2
//		   oper  - int SET or GET
//		   partition - int index in partitionDB
// returns: ERR_OK for SET
//			> 0 if timeout set by corresponding delay in partition definition expired, 0 if not
// 
int partitionTimer(int timer, int oper, int partition) {
//
  if (oper == SET)  {                      // record the current time in milliseconds
	if (timer == ENTRY_DELAY1)
	  partitionDB[partition].entryDelay1 = millis();
	else if (timer == ENTRY_DELAY2)
	  partitionDB[partition].entryDelay2 = millis();
	else	
	  ErrWrite(ERR_CRITICAL, "partitionTimer: Invalid entry delay param for partition %d\n", partition);
	return ERR_OK;
  }	
  if (timer == ENTRY_DELAY1)			  // check if the interval exoired. Note - interval is in seconds
	return ((unsigned long)(millis() - partitionDB[partition].entryDelay1) > ((unsigned long)partitionDB[partition].entryDelay1Interval)*1000);
  else if (timer == ENTRY_DELAY2)
	return ((unsigned long)(millis() - partitionDB[partition].entryDelay2) > ((unsigned long)partitionDB[partition].entryDelay2Interval)*1000);
  else	
	ErrWrite(ERR_CRITICAL, "partitionTimer: Invalid entry delay param for partition %d\n", partition);
	return ERR_OK;
  }	
}
//
// look-up zone by name - returns board index or  zone index from name parm suplied
// params: char * zoneName name to look-up
//		   int getZoneBoard - see return codes	
// returns: board index in zonesDB if getZoneBoard == true,  zone index in zonesDB
//          ERR_CRITICAL if name is not found
//
int zoneLookup( char * name, int getZoneBoard) {
	logger.printf ("zoneLookup: looking for zone %s\n", name);
	for (int j = MASTER_ADDRESS; j <= maxSlaves; j++) {
        for(int i=0; i< (j?SLAVE_ALARM_ZONES_CNT:MASTER_ALARM_ZONES_CNT); i++) {             // for each board' zone
			if(strcmp(name, zonesDB[j][i].zoneName)) {
				logger.printf ("zoneLookup: zone found board %d index %d\n", j, i);
				return (getZoneBoard?j:i)
		}
    }
    ErrWrite(ERR_WARNING, "zoneLookup: zone %s not found\n", name);
	return ERR_WARNING;										// zone is not found
}
//
// count not bypassed entry delay zones for given partition . Needed because in case of all entry delay zones are bypassed,
// follow zones has to start by themself ENTRY_DELAY2 if followZone2entryDelay2 is true 
// params: int partIdx - index in partitionDB
// returns: bypassed entry delay zones count
int countBypassedEntryDelayZones(int partIdx) {
	for (int j = MASTER_ADDRESS; j <= maxSlaves; j++) {
		for(int i=0; i< (j?SLAVE_ALARM_ZONES_CNT:MASTER_ALARM_ZONES_CNT); i++) {  
			if((zonesDB[j][i].zonePartition == partIdx) {
				if((zonesDB[j][i].zoneType == ENTRY_DELAY1) || (zonesDB[j][i].zoneType == ENTRY_DELAY2) {
					if(!zonesDB[j][i].bypassed) 
						res++;
				}
			}
		}
    }
	return res;
}	
//
// bypassZone- baypasses zone if allowed. 
// params: int board - board the zone belongs to (MASTER_ADDRESS, SLAVE... mazSlaves)
//         int zoneIdx - zone index 0 .. MASTER/SLAVE_ZONES_MAX depends on board
// returns: false - failure, true - success
//
int bypassZone(int board,  int zoneIdx) {
	int res = 0; 
	int partIdx;
	if(!(zonesDB[board][zoneIdx].zoneOptions & BYPASS_EN))     // allowed to bypass?
		return false;										   // no, return	
	zonesDB[board][zoneIdx].bypassed = true;				   // yes, mark it as bypassed
	// count unbypassed ENTRY_DELAYX zones for this partition and update  notBypassedEntyDelayZones in partition struct
	// this is needed for follow zones to know if there is entry delay zones to follow (in case all ENTRY_DELAY zones are bypassed)
	// and if no entry delay zones and followZone2entryDelay2 in corresponding partition opts is set, follow zones will use ENTRY_DELAY2 instead
	partIdx = zonesDB[board][zoneIdx].zonePartition;		   // find zone's partition
	partitonDB[partIdx].notBypassedEntyDelayZones = countBypassedEntryDelayZones(partIdx);	// mark how many entry delay zone are not bypassed
	return true;
}	
//
// unBypassZone- un- baypasses zone
// params: int board - board the zone belongs to (MASTER_ADDRESS, SLAVE... mazSlaves)
//         int zoneIdx - zone index 0 .. MASTER/SLAVE_ZONES_MAX depends on board
// returns: none
//
void unBypassZone(int board,  int zoneIdx) {
	int res = 0; 
	int partIdx;
	zonesDB[board][zoneIdx].bypassed = false;				   // yes, mark it as un bypassed
	// count unbypassed ENTRY_DELAYX zones for this partition and update  notBypassedEntyDelayZones in partition struct
	// this is needed for follow zones to know if there is entry delay zones to follow (in case all ENTRY_DELAY zones are bypassed)
	// and if no entry delay zones and followZone2entryDelay2 in corresponding partition opts is set, follow zones will use ENTRY_DELAY2 instead
	partIdx = zonesDB[board][zoneIdx].zonePartition;		   // find zone's partition
	partitonDB[partIdx].notBypassedEntyDelayZones = countBypassedEntryDelayZones(partIdx);	// mark how many entry delay zone are not bypassed
}		
// process zone error, generates trouble or alarm
//
void processZoneError(struct ALARM_ZONE zone) {
int tamperOpt;
	//if(!(zone.zoneExtOpt & ZONE_TAMPER_OPT))	// if local otpions are specified
		//tamperOpt = 								// no, follow the global tamper settings
}	
//
// check arm restrictions
// params: byte partIxd - partition number (ID) to be used as index in partitionDB
//			   int 	action	- bitmask, DISARM = 0, REGULAR_ARM, FORCE_ARM, INSTANT_ARM, STAY_ARM, see enum  ARM_METHODS_t
//	returns: ERR_OK(0) if no restrictions found, otherwise number of zones in error conditions
//
int checkArmRestrctions(byte partIdx, int action) {
int ret = 0; 
  ErrWrite(ERR_DEBUG, "Checking arm restictions for part %d\n", partIdx);
  for(int i = 0; i <=maxSlaves; i++) {         // for each board 
     for(int j=0; j< (i?SLAVE_ALARM_ZONES_CNT:MASTER_ALARM_ZONES_CNT); j++) {             // for each board' zone
        //logger.printf("Looking at board %d zone %d\n", i, j);  
        sprintf(zonesDB[i][j].zoneName, "Zone_%d", j);
        zonesDB[i][j].zoneType = 0;
        if(zonesDB[i][j].zonePartition == partIdx) {                         // check only zones assigned to this partition
          if(zonesDB[i][j].valid && !zonesDB[i][j].bypassed)              // check if zone is no defined/in use or if bypassed
            if(zonesDB[i][j].zoneStat & ZONE_ERROR_MASK)                     // tamper or antimask error in zone?
              ret+=1;                                                         // no error, continue
          }
        }
    }
  logger.printf("Found %d alarm zones of partition %d in error condition\n", ret, partIdx);
  //printAlarmZones((byte *) zonesDB, i, i);
  return ret; 
}
//
void reportArm(int partIdx) {
  ReportMQTT(ARM_TOPIC, "Arming");
}
//
// arm partition 
// params: byte partIxd - partition number (ID) to be used as index in partitionDB
//			   int 	action	- bitmask, DISARM = 0, REGULAR_ARM, FORCE_ARM, INSTANT_ARM, STAY_ARM, see enum  ARM_METHODS_t
//
void armPartition(byte partIxd, int action)  {
   if(action == DISARM)
    ErrWrite(ERR_DEBUG, "Disarming partition %d\n", partIxd);
   else
    ErrWrite(ERR_DEBUG, "Arming partition %d\n", partIxd);                                
   switch (action) {
		case DISARM:
		case REGULAR_ARM:
		case FORCE_ARM:
		case INSTANT_ARM:
		case STAY_ARM:
	    if(action != DISARM) {
		  if(ERR_OK != checkArmRestrctions(partIxd, action)) {
		     ReportMQTT(ARM_TOPIC, "Partition not armed due to restrictions");
		     return; 
		  }
	    }
		if(partitionDB[partIxd].armStatus == action) {
			ErrWrite(ERR_DEBUG, "Partition %d already armed\n", partIxd);    
			return;
		}
		partitionDB[partIxd].armStatus = action;
		partitionDB[partIxd].armTime = millis();
		reportArm(partIxd);
		break;
	default:
		ErrWrite(ERR_WARNING, "Request to arm/disarm invalid partition %d\n", partIxd);
		break;
	}
	// fix follows partitions RECURSIVELLY
	for(int i = 0; i < MAX_PARTITION; i++) {
		if(i == partIxd)                        // skip check for current partition to avoid loops 
			continue;                             // like part 1 follows part 1
		//logger.printf("Checking partition %d if follows partition %d\n", i, partIxd);
		if(!partitionDB[i].follows[partIxd])		// follows is array of MAX_PARTITION bytes, if byte of idx i is true, 
			continue;								              // it means that this partition follows partition idx = i
		//logger.printf("Found partition %d follows partition %d\n", i, partIxd);
		armPartition(i, action); 		            // call recursively
		}
}	
//
// alarmLoop() - implement all alarm business
//
void alarmLoop() {
int cz, cb;

/*
  		for(cz = 0; cz < MASTER_ALARM_ZONES_CNT; cz++) {
				if(!(zonesDB[MASTER_ADDRESS][cz].valid || zonesDB[MASTER_ADDRESS][cz].bypassed))	// check if zone is no defined/in use or if bypassed
					continue;									// yes, continue with next one
				if(zonesDB[MASTER_ADDRESS][cz].zoneStat & ZONE_ERROR_MASK) {    // check for tampered zone or masked
					processZoneError(zonesDB[MASTER_ADDRESS][cz]);				// process zone error, generates trouble or alarm
					continue;
				}
		}
*/
//  armPartition( 0, REGULAR_ARM);
//  armPartition( 0, DISARM);

}
