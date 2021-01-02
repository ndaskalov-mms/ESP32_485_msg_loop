  boardID = MASTER_ADDRESS;                     // TODO - only for loopback testing
  static int i=0;
  memcpy(errorsDB_backup, errorsDB, sizeof(errorsDB_backup)); // backup error DB
  
  // are we waiting for reply? MSG_READY means good msg,ERR_OK(0) means no msg,<0 means UART error or parse err
  if (waiting_for_reply)   {                    // check for message available
    if(ERR_OK != (retCode = check4msg(MasterMsgChannel, REPLY_TIMEOUT))) {    // see what we got so far at receiver
     	waiting_for_reply = 0;                    // we got something, either message or error
      if(retCode != MSG_READY)								 
		    ErrWrite(ERR_WARNING, "Master rcv reply error or timeout\n"); 	 // error	  
      else {
		    masterProcessMsg(rcvMsg);             // message, process it. 
	    }                                         // else
    }                                           // if ERR_OK
  }                                             // if (waiting)
  // not waiting for reply, check if it is time to send new command
  else if (isTimeFor(FREE_CMD, POLL_INTERVAL))  {// calculate the time elapsed sinse the particular command was send, yes if > interval
    ErrWrite(ERR_INFO, "\nMaster: time to transmit \n");
    //if(ERR_OK == sendFreeCmd(FREE_TEXT_SUB_CMD, SLAVE_ADDRESS1, FREE_CMD_DATA_LEN, test_msg[(++i)%3])) // sendCmd handle and reports errors internally 
      //ErrWrite( ERR_INFO, ("Master MSG transmitted, receive timeout started\n"));
	if(ERR_OK == setSlaveZones(zonesDB[SLAVE_ADDRESS1])); // sendCmd handle and reports errors internally 
      ErrWrite( ERR_INFO, ("Master MSG transmitted, receive timeout started\n"));
  }
  //
  // no message available for processing and it is not time to send a new one
  // do something usefull like have a nap or read MQTT
  else {
    //logger.print( "." );                        // no, do nothing
  }
  // check for new errors and send via MQTT when it is availabel some sunny day
  if(memcmp(errorsDB, errorsDB_backup, sizeof(errorsDB))) {
    printNewErrors();
  }

   
		
  
  
  
