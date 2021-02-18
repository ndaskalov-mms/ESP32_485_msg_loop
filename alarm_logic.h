#include "alarm-defs.h"
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
	for (int brd = MASTER_ADDRESS; brd <= maxSlaves; brd++) {
        for(int zn=0; zn< (brd?SLAVE_ALARM_ZONES_CNT:MASTER_ALARM_ZONES_CNT); zn++) {             // for each board' zone
			if(!zonesDB[brd][zn].valid)
				continue;
			if(strcmp(name, zonesDB[brd][zn].zoneName)) {
				logger.printf ("zoneLookup: zone found board %d index %d\n", brd, zn);
				return (getZoneBoard?brd:zn)
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
	for (int brd = MASTER_ADDRESS; brd <= maxSlaves; brd++) {
		for(int zn=0; i< (brd?SLAVE_ALARM_ZONES_CNT:MASTER_ALARM_ZONES_CNT); zn++) {  
			if((zonesDB[brd][zn].zonePartition == partIdx) {
				if(zonesDB[brd][zn].valid) {
					if((zonesDB[brd][zn].zoneType == ENTRY_DELAY1) || (zonesDB[brd][zn].zoneType == ENTRY_DELAY2) {
						if(!zonesDB[brd][zn].bypassed) 
							res++;
					}
				}
			}
		}
    }
	return res;
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
  for(int brd = 0; brd <=maxSlaves; brd++) {         // for each board 
     for(int zn=0; zn< (brd?SLAVE_ALARM_ZONES_CNT:MASTER_ALARM_ZONES_CNT); brd++) {             // for each board' zone
        //logger.printf("Looking at board %d zone %d\n", i, j);  
        sprintf(zonesDB[brd][zn].zoneName, "Zone_%d", j);
        zonesDB[brd][zn].zoneType = 0;
        if(zonesDB[brd][zn].zonePartition == partIdx) {                         // check only zones assigned to this partition
          if(zonesDB[brd][zn].valid && !zonesDB[brd][zn].bypassed)              // check if zone is no defined/in use or if bypassed
            if(zonesDB[brd][zn].zoneStat & ZONE_ERROR_MASK)                     // tamper or antimask error in zone?
              ret+=1;                                                         // no error, continue
          }
        }
    }
  logger.printf("Found %d alarm zones of partition %d in error condition\n", ret, partIdx);
  //printAlarmZones((byte *) zonesDB, i, i);
  return ret; 
}
//
//void reportArm(int partIdx) {
//  ReportMQTT(ARM_TOPIC, "Arming");
//}
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
// bypassZone- baypasses zone if allowed. 
// params: int board - board the zone belongs to (MASTER_ADDRESS, SLAVE... mazSlaves)
//         int zoneIdx - zone index 0 .. MASTER/SLAVE_ZONES_MAX depends on board
// returns: false - failure, true - success
//
int bypassZone(int board,  int zoneIdx) {
	int res = 0; 
	int partIdx;
	if(!zonesDB[board][zoneIdx].valid) {
		PublishMQTT(ZONES_BYPASS_STATUS_TOPIC, zonesDB[board][zoneIdx].zoneName, INVALID_ZONE_PAYLOAD);
		return false;	
	}
	if(!(zonesDB[board][zoneIdx].zoneOptions & BYPASS_EN)) {   // allowed to bypass?
		PublishMQTT(ZONES_BYPASS_STATUS_TOPIC, zonesDB[board][zoneIdx].zoneName, BYPASS_DISABLE_PAYLOAD);
		return false;										   // no, return	
	}
	zonesDB[board][zoneIdx].bypassed = true;				   // yes, mark it as bypassed
	// count unbypassed ENTRY_DELAYX zones for this partition and update  notBypassedEntyDelayZones in partition struct
	// this is needed for follow zones to know if there is entry delay zones to follow (in case all ENTRY_DELAY zones are bypassed)
	// and if no entry delay zones and followZone2entryDelay2 in corresponding partition opts is set, follow zones will use ENTRY_DELAY2 instead
	partIdx = zonesDB[board][zoneIdx].zonePartition;		   // find zone's partition
	partitonDB[partIdx].bypassedZonesCnt++;					   // update statistics
	partitonDB[partIdx].notBypassedEntyDelayZones = countBypassedEntryDelayZones(partIdx);	// mark how many entry delay zone are not bypassed
	newZonesDataAvailable != NEW_DATA_BIT << board;
	PublishMQTT(ZONES_BYPASS_STATUS_TOPIC, zonesDB[board][zoneIdx].zoneName, BYPASS_PAYLOAD);
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
	partitonDB[partIdx].bypassedZonesCnt--;					   // update statistics
	partitonDB[partIdx].notBypassedEntyDelayZones = countBypassedEntryDelayZones(partIdx);	// mark how many entry delay zone are not bypassed
	newZonesDataAvailable != NEW_DATA_BIT << board;
	PublishMQTT(ZONES_BYPASS_STATUS_TOPIC, zonesDB[board][zoneIdx].zoneName, UNBYPASS_PAYLOAD)
}
//
// process zone error, generates trouble or alarm
// if alarmGlobalOpts.tamperBypassEn == true - if zone is bypassed ignore tamper;
//										false - follow global or local tamper settings
// zone global tamper options:	alarmGlobalOpts.tamperOpts, bitmask, same as local - see #define ZONE_TAMPER_OPT_XXXXXX
// zone local taper options:    zoneDB[zone].zoneExtOpt, see below  
//  	ZONE_FOLLOW_PANEL_ONTAMPER  	== true - follow gloabl tamper options, false - follow zone tamper options
//  	ZONE_TAMPER_OPT = (0x2 | 0x4) - bitmask zone tamper opions bits - see below ZONE_TAMPER_OPT_XXX
//  	ZONE_FOLLOW_GLOBAL_ON_ANTIMASK  == true - follow gloabl antimask options, false - follow zone antimask options
//  	ZONE_ANTIMASK_OPT = (0x20 | 0x40) - bitmask zone antimask options bits - see below ZONE_ANTI_MASK_SUPERVISION_XXX
// ZONE_TAMPER_OPT_XXXXXX:										
//		ZONE_TAMPER_OPT_DISABLED  =  0	 
//		ZONE_TAMPER_OPT_TROUBLE_ONLY = 1,     
//		ZONE_TAMPER_OPT_ALARM_WHEN_ARMED  = 2,      
//		ZONE_TAMPER_OPT_ALARM  = 3,     
//
//		TODO - where to check RF errors??
//      TODO - add antimask support, currently both open line and shorted line are treated as one
//
void processZoneError(struct ALARM_ZONE zone) {
int tamperOpt;
//
	if(zone.bypassed && alarmGlobalOpts.tamperBypassEn) {			// ignore tamper on bypassed zone as prescribed
		partitionDB[zone.zonePartition].ignorredTamperZonesCnt++;
		return;									
	}
	if(zone.zoneExtOpt & ZONE_FOLLOW_PANEL_ONTAMPER)				// findout global or local options to follow
		tamperOpt = alarmGlobalOpts.tamperOpts;						// follow the global tamper settings
	else
		tamperOpt = zone.zoneExtOpt;								// follow the global tamper settings
	switch (tamperOpt) {
		case 
}	
//
// alarmLoop() - implement all alarm business
//
void alarmLoop() {
//
    if(timeoutOps(GET, ALARM_LOOP_TIMER) || newZonesDataAvailable)) 					// run the loop on spec intervals
			return;																		// or when something changed
	timeoutOps(SET, ALARM_LOOP_TIMER);													// restart timer
	cleanRTdata();																		// clear all statistics
	for (int brd = MASTER_ADDRESS; brd <= maxSlaves; brd++) {							// loop over all zones in all boards
	    for(int zn=0; zn< (brd?SLAVE_ALARM_ZONES_CNT:MASTER_ALARM_ZONES_CNT); zn++) {   // for each board' zone
			if(!zonesDB[brd][zn].valid)										 			// invalid zone, continue
				continue;	
			if(zonesDB[brd][zn].zoneStat & ZONE_ERROR_MASK) {                    		// tamper or antimask error in zone?
				processZoneError(zonesDB[brd][zn]);
			}
		}
    }

			if(strcmp(name, zonesDB[j][i].zoneName)) {
				logger.printf ("zoneLookup: zone found board %d index %d\n", j, i);
				return (getZoneBoard?j:i)
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
