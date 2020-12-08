  boardID = MASTER_ADDRESS;                     // TODO - only for loopback testing
  static int i=0;
  memcpy(errorsDB_backup, errorsDB, sizeof(errorsDB_backup)); // backup error DB
  
  // are we waiting for reply? MSG_READY means good msg,ERR_OK(0) means no msg,<0 means UART error or parse err
  if (waiting_for_reply)   {                    // check for message available
    if(ERR_OK != (retCode = check4msg(MasterMsgChannel, REPLY_TIMEOUT))) {    // not ERR_OK, see what we have so far at receiver
     	waiting_for_reply = 0;                    // we got message or error
      if(retCode != MSG_READY)								 
		    ErrWrite(ERR_WARNING, "Master rcv reply error or timeout\n"); 		  
      else {
		    masterProcessMsg(rcvMsg);               // process message put all reply related code here
	    }                                         // else
    }                                           // if ERR_OK
  }                                             // if (waiting)
  // not waiting for reply, check if it is time to send new command
  else if (isTimeFor(FREE_TEXT, POLL_INTERVAL))  {
    ErrWrite(ERR_INFO, "\nMaster: time to transmit \n");
    if(ERR_OK == sendCmd(FREE_TEXT, SLAVE_ADDRESS1, test_msg[(++i)%3])) // sendCmd handle and reports errors internally 
      ErrWrite( ERR_INFO, ("Master MSG transmitted, receive timeout started\n"));
  }
  //
  // no message available for processing and it is not time to send a new one
  // do something usefull like have a nap or read MQTT
  else {
    //logger.print( "." );                      // no, do nothing
  }
  // check for new errors and send via MQTT when it is availabel some sunny day
  if(memcmp(errorsDB, errorsDB_backup, sizeof(errorsDB))) {
    printNewErrors();
  }

   
		
  
  
  
