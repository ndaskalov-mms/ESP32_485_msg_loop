  boardID = MASTER_ADDRESS;        // TODO - only for loopback testing
  int retCode;
  static int i=0;
 
  memcpy(errorsDB_backup, errorsDB, sizeof(errorsDB_backup));   // backup error DB
  
  if (waiting_for_reply)                                    // check for message available
  {
    retCode = check4msg(MasterMsgChannel);
   	if(retCode == MSG_READY) {                            	// false means no msg, 0 means good msg, <0 means error 
		    waiting_for_reply = 0;          	                  // TODO - check for out-of-order messages
        uartTrmMode(MasterUART);		              	        // Switch back to transmit_mode  
        masterProcessMsg(rcvMsg);                           // process message
     }
     else if (retCode < 0)                                  // error receiving msg?
        ErrWrite(ERR_DEBUG, "Master received wrong message\n");
     // check for message reply timeout                      // reply not received
	   else if((unsigned long)(millis() - last_transmission) > REPLY_TIMEOUT) { 
        waiting_for_reply = 0;
        uartTrmMode(MasterUART);
		    ErrWrite(ERR_TIMEOUT, "Master reply timeout\n");
     }                                                       // else
  }                                                          // if (waiting_for_reply)
  else if( (unsigned long)(millis() - last_transmission) > TRM_INTERVAL){  //check if it is time for the next communication
    ErrWrite( ERR_DEBUG, "\n\nMaster:  -------------------------------Time to transmit -------------------------------\n" );
    if(ERR_OK != SendMessage(MasterMsgChannel, MasterUART, FREE_TEXT, SLAVE1_ADDRESS, test_msg[(++i)%3], MAX_PAYLOAD_SIZE)){
      ErrWrite(ERR_TRM_MSG, "\n\nMaster: Error in sendMessage\n");
      // MQTT send error
    }
    else {
      last_transmission = millis();    // mark the transmit time so we can calculate the time for the next transmission and check for reply timeout
      ErrWrite( ERR_OK, ("Master MSG transmitted, receive timeout started\n"));
      waiting_for_reply = 1;
    }
  }
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

   
		
  
  
  
