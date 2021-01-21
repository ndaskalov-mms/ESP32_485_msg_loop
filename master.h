void master() {
  static int i=0;
  logger.printf("Master loop\n");
  memcpy(errorsDB_backup, errorsDB, sizeof(errorsDB_backup)); // backup error DB
//
  wait4reply(2);                                 // checks waiting_for_reply flag and if set, receives and process message or exits with timeout
  logger.printf("slavesSetMap = %2x\n", slavesSetMap);
  if(slavesSetMap) 
	  sendSlavesConfigs();
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
   
		
  
  
  
