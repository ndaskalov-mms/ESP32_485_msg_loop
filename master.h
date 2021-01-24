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
	  // chack if we have something to send in priority order
	  if(slavesSetZonesMap) {					// top priority - send configs to slaves, nothing we can do before this is done
		  ErrWrite( ERR_INFO, ("Sending slaves configuration\n")); //Master process message will clear corresponding bits once confirmation reply
		  setSlavesZones();						// slavesSetZonesMap holds bitmap of slaves to be set. Will do error reporting internally
		  return;                               // inside wait4reply we will collect the answers from slaves and reflect in zones, pgms, etc DBs
		  }  									// return as we have to wait for slave reply
	  if(slavesSetPgmsMap) {					// send new PGM states if any
		  ErrWrite( ERR_INFO, ("Sending new PGMs states\n")); //Master process message will clear corresponding bits once confirmation reply
		  setSlavesPgms();						// slavesSetPgms holds bitmap of slaves with new PGMs settings  and will do error reporting
		  return;                               // inside wait4reply we will collect the answers from slaves and reflect in zones, pgms, etc DBs
		  }  									// return as we have to wait for slave reply
	  // check if it is time to send new command
	  if(isTimeFor(POLL_ZONES, POLL_INTERVAL))  { // calculate the time elapsed sinse the particular command was send, yes if > interval
		  if(ERR_OK == pollSlaveZones(SLAVE_ADDRESS1))    // sendCmd handle and reports errors internally 
			ErrWrite( ERR_INFO, ("Poll slave zones send\n"));
		  return;								  // return will cause the master() to be called from Arduino loop and wait4reply
		  }		
	  }   										  // else if(waiting_for_reply)
}


void master() {
  logger.printf("Master loop\n");
  memcpy(errorsDB_backup, errorsDB, sizeof(errorsDB_backup)); // backup error DB
  master2slave();								  // send if there is sometinigh to be send
  // check for new errors and send via MQTT when it is availabel some sunny day
  if(memcmp(errorsDB, errorsDB_backup, sizeof(errorsDB))) {
    printNewErrors();
  }   // else if(waiting_for_reply)
}

//if(ERR_OK == sendFreeText(SLAVE_ADDRESS1, FREE_CMD_DATA_LEN, test_msg[(++i)%3])) // sendCmd handle and reports errors internally 
  //ErrWrite( ERR_INFO, ("Master MSG transmitted, receive timeout started\n"));
//if(ERR_OK == sendFreeText(SLAVE_ADDRESS1, FREE_CMD_DATA_LEN, test_msg[(++i)%3])) // sendCmd handle and reports errors internally 
  //ErrWrite( ERR_INFO, ("Master MSG transmitted, receive timeout started\n"));
//if(ERR_OK == setSlavePGMs(pgmsDB[SLAVE_ADDRESS1], SLAVE_ADDRESS1)); // sendCmd handle and reports errors internally 
  //ErrWrite( ERR_INFO, ("Master MSG transmitted, receive timeout started\n"));
//if(ERR_OK == setSlaveZones(zonesDB[SLAVE_ADDRESS1], SLAVE_ADDRESS1)); // sendCmd handle and reports errors internally 
  //ErrWrite( ERR_INFO, ("Master MSG transmitted, receive timeout started\n"));		

  
  
