	// ----------- slave simulation -------------------------------------------
	boardID = SLAVE_ADDRESS1;        // Slave destination ---------   TODO - only for loopback testing
	memcpy(errorsDB_backup, errorsDB, sizeof(errorsDB_backup));   // backup error DB
	retCode = check4msg(SlaveMsgChannel, NO_TIMEOUT);                      // message if available will stored in global rcvMSG variable
	if(retCode == MSG_READY) {				                            // false means no msg, 0 means good msg, <0 means error
		switch (rcvMsg.cmd) {                                       // process command received
		  case PING:
  			ErrWrite (ERR_WARNING, "Unsupported command received PING\n");
  			break;
		  case POLL_ZONES:
  			ErrWrite (ERR_WARNING,"Unsupported command received POLL_ZONES\n");
  			break;
		  case SET_OUTS:
  			ErrWrite (ERR_WARNING,"Unsupported command received SET_OUTPUTS\n");
  			break;
		  case FREE_TEXT:
  			ErrWrite (ERR_DEBUG,"SLAVE: FREE TEXT command received\n");
  			// return the same payload converted to uppercase
  			byte tmp_msg [MAX_PAYLOAD_SIZE];
  			for (int i=0; i < rcvMsg.len; i++)
  			  tmp_msg[i] = toupper(rcvMsg.payload[i]);
  			if(ERR_OK != SendMessage(SlaveMsgChannel, SlaveUART, (FREE_TEXT | REPLY_OFFSET), MASTER_ADDRESS, tmp_msg, rcvMsg.len))
  			  ErrWrite(ERR_TRM_MSG, "Slave: Error in sendMessage\n");
  			break;
		  default:
			  ErrWrite (ERR_WARNING, "Slave: invalid command received %x\n", rcvMsg.cmd);
		}  	// switch
	}		// if check...
	// no message available for processing 
	// do something usefull like take a nap or read ADC and process the zones info
	// or collect some errors info to be send as status to MASTER some day
	
	// find out if some new errors occured while receiving/processing the message
	if(memcmp(errorsDB, errorsDB_backup, sizeof(errorsDB))) 
		printNewErrors();
