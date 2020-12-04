  boardID = MASTER_ADDRESS;        // TODO - only for loopback testing
  int retCode;
  static int i=0;
 
  memcpy(errorsDB_backup, errorsDB, sizeof(errorsDB_backup));   // backup error DB
  
  if (waiting_for_reply)
  {
   	if((retCode = check4msg(MasterMsgChannel)) != false) {	// false means no msg, 0 means good msg, <0 means error
		    waiting_for_reply = 0;          	          	// TODO - check for out-of-order messages
        uartTrmMode(MasterUART);		              	// Switch back to transmit_mode
		if(retCode) {
          switch (rcvMsg.cmd) {
            case PING_RES:
              ErrWrite(ERR_DEBUG, "Master: Unsupported reply command received PING_RES\n");
              break;
            case POLL_ZONES_RES:
              ErrWrite(ERR_DEBUG, "Master: Unsupported reply command received POLL_ZONES_RES\n");
              break;
            case SET_OUTS_RES:
              ErrWrite(ERR_DEBUG, "Master: Unsupported reply command received SET_OUTPUTS_RES\n");
              break;
            case FREE_TEXT_RES:
              ErrWrite(ERR_DEBUG, "Master: reply received FREE_TEXT_RES: \n");
              break;
            default:
              ErrWrite(ERR_WARNING, "Master: invalid command received %x\n", rcvMsg.cmd);
			}  // switch
        }   // if retCode
     }    // if check4msg
	 else if((unsigned long)(millis() - last_transmission) > REPLY_TIMEOUT) {
        // reply not received
        waiting_for_reply = 0;
        // TODO - signal error somehow
		ErrWrite(ERR_TIMEOUT, "Master reply timeout\n");
    }   // else
  }     // if (waiting_for_reply)
  //
  //check if it is time for the next communication
  //
  else if( (unsigned long)(millis() - last_transmission) > TRM_INTERVAL){         // yes, it's time
    ErrWrite( ERR_OK, "\n\nMaster:  -------------------------------Time to transmit -------------------------------\n" );
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
  // or try to collect some errors info and send via MQTT when it is availabel some sunny day
  if(memcmp(errorsDB, errorsDB_backup, sizeof(errorsDB))) {
    printNewErrors();
  }

   
		
  
  
  
