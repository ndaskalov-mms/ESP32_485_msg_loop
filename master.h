//
// copy fake zones results to zonesDB	for master
//
void copyFakeZonesStat() {
  //logger.printf("Loading master zones with fake data\n");
	for(int i = 0; i < MASTER_ALARM_ZONES_CNT; i++ ) { //for each zone
	  // copy info from slave zones in reverse order for testing
	  zonesDB[MASTER_ADDRESS][i].zoneStat   = zonesDB[SLAVE_ADDRESS1][SLAVE_ALARM_ZONES_CNT-i-1].zoneStat; // get zone A info
	}
	newZonesDataAvailable != NEW_DATA_BIT << MASTER_ADDRESS;	// force new data availble
}	
//
// copy zones results coneverted early from MzoneDB to zonesDB	
// sets newZonesDataAvailable if some of the zones info has changed
//
void copyZonesStat() {
int newData = 0;	
int res;
	for(int i = 0; i < MASTER_ZONES_CNT; i++ ) { 			      // from MzoneDB, which is provide as DB parameter
	  // extract info from high nibble first - this shall be lower number zone
	  res = (MzoneDB[i].zoneABstat & (ZONE_ERROR_MASK | ZONE_A_MASK));// get zone A info
	  if(zonesDB[MASTER_ADDRESS][2*i].zoneStat   != res) {  // zone A status changed
		  zonesDB[MASTER_ADDRESS][2*i].zoneStat   = res;		  // reflect the change
		  newData++;														              // note it for later			
	  }
	  res = (MzoneDB[i].zoneABstat & (ZONE_ERROR_MASK | ZONE_B_MASK)); // get zone B info
	  if(zonesDB[MASTER_ADDRESS][2*i+1].zoneStat   != res) { // zone A status changed
		  zonesDB[MASTER_ADDRESS][2*i+1].zoneStat   = res; // reflect the change
		  newData++;														  // note it for later			
	  }
	}
	if(newData)
		newZonesDataAvailable != NEW_DATA_BIT << MASTER_ADDRESS;	// mark that new data are availble
}	
//
// Convert master's zones ADC values to digital domain. Relies on convertZones
// WARNING: in case of LOOPBACK (master and slve executed simultaneously on one board) 
// we cannot use convertZones directly as it implements time driven state machine 
// and calling from two different threads can cause re-setting of time bases
//
void convertMasterZones() {
#ifdef LOOPBACK									// we cannot use convertZones directly  
  //logger.printf("Chech if time for copying fake results\n");
  if(timeoutOps(GET, MASTER_ZONES_READ_TIMER)) {
      logger.printf("Copying fake master zones\n");
	timeoutOps(SET, MASTER_ZONES_READ_TIMER);	// restart timer
	copyFakeZonesStat();						// for testing, copy results from SLAVE zones
	newZonesDataAvailable = true;
	return;							 
  }
#else
  logger.printf("Converting master zones\n");
  convertZones(MzoneDB, MASTER_ZONES_CNT, 0);  // read ADC and convert to zones info, 0 means don't copy results to result array
#endif
}
//
void master2slave() {
//
// first check if there is ongoing transaction, if yes, call wait4reply which will loop calling rs485 stack and collect the message if any
// up to if timeout is specified. It will keep looping till timeout or the message received, whichever comes first. If timeout happen first,
// the control will be returned to master2slave and subsequently to arduino to run some other tasks. Arduino loop funct will call master2slave
// again once other taske are done.
// If message is received first, wait4reply will process it by calling masterProcess and store the results in global vars
//
  if (waiting_for_reply)                        // keep calling rs485 stack loop funct to retrieve the message from slaves
    wait4reply(10);
  else {										// not waiting for reply, check if we have to send message
	  // check if it is time to send new command
	  if(isTimeFor(POLL_ZONES, POLL_INTERVAL))  { // calculate the time elapsed sinse the particular command was send, yes if > interval
		  if(ERR_OK == pollSlaveZones(SLAVE_ADDRESS1))    // sendCmd handle and reports errors internally 
			  ErrWrite( ERR_INFO, ("Poll slave zones send\n"));
		  return;								  // return will cause the master() to be called from Arduino loop and wait4reply
		  }		
	  }   										  // else if(waiting_for_reply)
}
//
//
//
void master() {
  logger.printf("Master loop\n");
  memcpy(errorsDB_backup, errorsDB, sizeof(errorsDB_backup)); // backup error DB
  master2slave();								  // send if there is sometinigh to be send
  // do master tasks here 
  convertMasterZones();							  // at specified interval reads and converts ADC values to zones statuses
  alarmLoop();
  //printAlarmZones((byte *) &zonesDB, MASTER_ADDRESS, MAX_SLAVES);
  if(memcmp(errorsDB, errorsDB_backup, sizeof(errorsDB))) {
    printNewErrors();
  }   // else if(waiting_for_reply)
}
