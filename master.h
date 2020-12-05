  boardID = MASTER_ADDRESS;        // TODO - only for loopback testing
  static int i=0;
 
  memcpy(errorsDB_backup, errorsDB, sizeof(errorsDB_backup)); // backup error DB
  
  if (waiting_for_reply)   {                                  // check for message available
    if(ERR_OK != (retCode = check4msg(MasterMsgChannel, REPLY_TIMEOUT))) {    // see what we have so far at receiver
     	waiting_for_reply = 0;                                  // we got message or error
      if(retCode == MSG_READY)                                // MSG_READY means good msg, ERR_OK(0) means no msg, <0 means error 
          masterProcessMsg(rcvMsg);                           // process message
      else if (retCode < 0)                                   // error or timeout receiving msg?
  		    ErrWrite(ERR_OK, "Master receive reply error or timeout\n");
    }                                                         // if (check4msg)
  }                                                           // if (waiting)
  else if( checkTimeout(MasterMsgChannel, TRM_INTERVAL)) {    //check if it is time for the next communication
    ErrWrite( ERR_DEBUG, "\n\nMaster:  -------------------------------Time to transmit -------------------------------\n" );
    if(ERR_OK != SendMessage(MasterMsgChannel, MasterUART, FREE_TEXT, SLAVE1_ADDRESS, test_msg[(++i)%3], MAX_PAYLOAD_SIZE)){
      ErrWrite(ERR_TRM_MSG, "\n\nMaster: Error in sendMessage\n");
     }                                                         // else if
    else {
      //last_transmission = millis();    // mark the transmit time so we can calculate the time for the next transmission and check for reply timeout
      ErrWrite( ERR_OK, ("Master MSG transmitted, receive timeout started\n"));
      waiting_for_reply = 1;
    }                                                          //else
  }                                                             // else if timeout
  //
  // no message available for processing and it is not time to send a new one
  // do something usefull like have a nap or read MQTT
  else {
    //logger.print( "." );    // no, do nothing
  }
  // check for new errors and send via MQTT when it is availabel some sunny day
  if(memcmp(errorsDB, errorsDB_backup, sizeof(errorsDB))) {
    printNewErrors();
  }

   
		
  
  
  
