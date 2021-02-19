int checkArmRestrctions(byte partIxd, int action) {
    return ERR_OK;
}
//
void reportArm(int partIxd) {
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
		lprintf("Checking partition %d if follows partition %d\n", i, partIxd);
		if(!partitionDB[i].follows[partIxd])		// follows is array of MAX_PARTITION bytes, if byte of idx i is true, 
			continue;								              // it means that this partition follows partition idx = i
		lprintf("Found partition %d follows partition %d\n", i, partIxd);
		armPartition(i, action); 		            // call recursively
		}
}	


//
enum  ARM_RESTRICTIONS_t {
    RESTRICT_ON_SUPERVISOR_LOSS = 0x1,
    RESTRICT_ON_TAMPER          = 0x2,                                    // default ON
    RESTRICT_ON_AC_FAILURE      = 0x4,
    RESTRICT_ON_BATTERY_FAILURE = 0x8,                                    // default ON
    RESTRICT_ON_BELL         	= 0x10, 
	RESTRICT_ON_SLAVE 			= 0x20,
    RESTRICT_ON_ANTI_MASK       = 0X40,
};
