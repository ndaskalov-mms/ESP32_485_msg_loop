void master() {
  static int i=0;
  logger.printf("Master loop\n");
  memcpy(errorsDB_backup, errorsDB, sizeof(errorsDB_backup)); // backup error DB
//
// first check if there is ongoing transaction, if yes, return to arduino to run some other tasks and after it will call our loop again
// wait4reply will call inside rs485 stack and check for message. If there is ready message, process it and store the results in global vars
  if(ERR_OK != wait4reply(10))	{					// is command/reply interaction in progress?? checks waiting_for_reply flag and if set, receives and process message or exits with timeout
	  return;								        // give chance to other processes to run while waiting
	}
// top priority is to send cofig data to slaves, we cannot do anything before this is done
  if(slavesSetMap) {						  // send configs to slaves, nothing we can do before this is done
	  sendSlavesConfigs();					// TODO - add support for mor slaves
    return;                       // inside wait4reply we will collect the answers from slaves and reflect in zones, pgms, etc DBs
    }  
  ErrWrite( ERR_INFO, ("Slaves configured \n"));
  // not waiting for reply, check if it is time to send new command
  //if(ERR_OK == sendFreeText(SLAVE_ADDRESS1, FREE_CMD_DATA_LEN, test_msg[(++i)%3])) // sendCmd handle and reports errors internally 
      //ErrWrite( ERR_INFO, ("Master MSG transmitted, receive timeout started\n"));
  if(isTimeFor(FREE_CMD, POLL_INTERVAL))  {// calculate the time elapsed sinse the particular command was send, yes if > interval
    ErrWrite(ERR_INFO, "\nMaster: time to transmit \n");
    //if(ERR_OK == sendFreeText(SLAVE_ADDRESS1, FREE_CMD_DATA_LEN, test_msg[(++i)%3])) // sendCmd handle and reports errors internally 
      //ErrWrite( ERR_INFO, ("Master MSG transmitted, receive timeout started\n"));
	  //if(ERR_OK == setSlavePGMs(pgmsDB[SLAVE_ADDRESS1], SLAVE_ADDRESS1)); // sendCmd handle and reports errors internally 
      //ErrWrite( ERR_INFO, ("Master MSG transmitted, receive timeout started\n"));
    //if(ERR_OK == setSlaveZones(zonesDB[SLAVE_ADDRESS1], SLAVE_ADDRESS1)); // sendCmd handle and reports errors internally 
      //ErrWrite( ERR_INFO, ("Master MSG transmitted, receive timeout started\n"));
    if(ERR_OK == getSlaveZones(SLAVE_ADDRESS1));   // sendCmd handle and reports errors internally 
      ErrWrite( ERR_INFO, ("Master MSG transmitted, receive timeout started\n"));
  }
  //
  // no message available for processing and it is not time to send a new one
  // do something usefull like have a nap or read MQTT
  else {
    logger.print( "." );                        // no, do nothing
  }
  // check for new errors and send via MQTT when it is availabel some sunny day
  if(memcmp(errorsDB, errorsDB_backup, sizeof(errorsDB))) {
    printNewErrors();
  }
}
   
		
  
  
  
