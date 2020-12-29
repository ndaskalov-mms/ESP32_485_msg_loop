
//
//
// ----------- slave simulation -------------------------------------------
//
//
	boardID = SLAVE_ADDRESS1;        // Slave destination ---------   TODO - only for loopback testing
	memcpy(errorsDB_backup, errorsDB, sizeof(errorsDB_backup));   // backup error DB
	retCode = check4msg(SlaveMsgChannel, NO_TIMEOUT);             // message if available will stored in global rcvMSG variable
	if(retCode == MSG_READY) {				                            // ERR_OK (0)- no message, ERR_RCV_MSG (<0) -parsing error, MSG_READY (1)- message present          
		switch (rcvMsg.cmd) {                                       // process command received
		  case PING:
  			ErrWrite (ERR_WARNING, "Unsupported cmd received PING\n");
  			break;
		  case POLL_ZONES:
  			ErrWrite (ERR_DEBUG,"POLL ZONES command received\n");
        // send the zones status, stored by convertZones in SzoneResult[]
        if(zoneInfoValid == ZONE_A_VALID | ZONE_B_VALID) {
          if(ERR_OK != SendMessage(SlaveMsgChannel, SlaveUART, (POLL_ZONES | REPLY_OFFSET), MASTER_ADDRESS, SzoneResult, sizeof(SzoneResult)));
            ErrWrite(ERR_TRM_MSG, "Slave: Error in sendMessage\n");
        }
  			break;
		  case SET_OUTS:
  			ErrWrite (ERR_WARNING,"Unsupported cmd received SET_OUTPUTS\n");
  			break;
		  case FREE_CMD:
  			ErrWrite (ERR_INFO,"SLAVE: FREE TEXT cmd received\n");
  			// return the same payload converted to uppercase
  			byte tmp_msg [MAX_PAYLOAD_SIZE];
  			for (int i=0; i < rcvMsg.len; i++)
  			  tmp_msg[i] = toupper(rcvMsg.payload[i]);
  			if(ERR_OK != SendMessage(SlaveMsgChannel, SlaveUART, (FREE_CMD | REPLY_OFFSET), MASTER_ADDRESS, tmp_msg, rcvMsg.len))
  			  ErrWrite(ERR_TRM_MSG, "Slave: Error in sendMessage\n");
  			break;
		  default:
			  ErrWrite (ERR_WARNING, "Slave: invalid command received %x\n", rcvMsg.cmd);
		}  	// switch
	}		// if retCode
	else if(retCode != ERR_OK)                 
        ErrWrite(ERR_WARNING, "Slave rcv cmd err"); 
	// no message available for processing 
	// do something usefull like take a nap or read ADC and process the zones info
	// or collect some errors info to be send as status to MASTER some day
	convertZones(SzoneDB, SLAVE_ZONES_CNT, SzoneResult);
	
	// find out if some new errors occured while receiving/processing the message
	if(memcmp(errorsDB, errorsDB_backup, sizeof(errorsDB))) 
		printNewErrors();
