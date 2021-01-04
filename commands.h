//
// command.h - implementation for commands/replies between devices
//

//
// send command and register the transmission time
// in case of error, ErrWrite will register the err in the database and to do some global staff (like sending error over MQTT, SMTP, etc)
// params:  cmd - command code (can be with reply flag set as well
//          dst - destination address
//          * payload - pointer to payload to be send
// returns: ERR_OK on success
//          ERR_BAD_CMD on wrong command send request
//          ERR_TRM_MSG on error while sending (payload size > max, no write callback for RS485 class, etc
//
int sendCmd(byte cmd, byte dst, byte * payload) {
    int ret_code;
    if (waiting_for_reply)
      return ERR_OK;                                     // TODO not sure if it is not better to return error, otherwise we can lock-up on waiting_for_repy
    int cmd_index = findCmdEntry(cmd);                   // get index into database in order to access command parameters
    if(ERR_DB_INDEX_NOT_FND == cmd_index) {              //  the command is not found in the database
      ErrWrite(ERR_DB_INDEX_NOT_FND, "\n\nMaster: err in findCmdEntry lookup for %d cmd\n\n", cmd);
      return ERR_BAD_CMD; 
    }
    cmdDB[cmd_index].last_transmitted = millis();        // register the send time
    if (ERR_OK != (ret_code = SendMessage(MasterMsgChannel, MasterUART, cmd, dst, payload, cmdDB[cmd_index].len))) {
      ErrWrite(ERR_TRM_MSG, "\n\nMaster: Err in sendMessage sending %d cmd\n\n", cmd);
      return ERR_TRM_MSG;
    }
    else {
      waiting_for_reply = cmd;
      return ERR_OK;
    }
}
//
// send free command and register the transmission time
// in case of error, ErrWrite will register the err in the database and to do some global staff (like sending error over MQTT, SMTP, etc)
// params:  subCmd - sub command code, valid only with FREE_CMD command
//          dst - destination address
//			payloadLen - len of payload to be send
//          * payload - pointer to payload to be send
// returns: ERR_OK on success
//          ERR_BAD_CMD on wrong command send request
//          ERR_TRM_MSG on error while sending (payload size > max, no write callback for RS485 class, etc
//
int sendFreeCmd(byte subCmd, byte dst, byte payloadLen, byte * payload) {
byte tmpBuf[256];
//
	if(payloadLen > FREE_CMD_PAYLD_LEN)					// check payload size
	  payloadLen = FREE_CMD_PAYLD_LEN;					// if too long, trunkate, sendCmd will issue error later
	tmpBuf[0] = subCmd;										      // prepare the message payload, first byte is the subCmd,
	tmpBuf[1] = payloadLen;									    //  followed by actual payload len
	for (int i =0; i<payloadLen; i++) 					// and payload
		tmpBuf[i+2] = payload[i];
	return sendCmd(FREE_CMD, dst, tmpBuf);
}
//
//
//
int setSlaveZones(struct ALARM_ZONE zone[]) {
byte tmpBuf[FREE_CMD_DATA_LEN];
int j = 0; int i = 0;
//
	for(i=0; (i<SLAVE_ZONES_CNT) && (j<FREE_CMD_DATA_LEN); i++) {
		tmpBuf[j++] = zone[i].gpio;
		tmpBuf[j++] = zone[i].mux;
		tmpBuf[j++] = zone[i].zoneID;
		}
	if(DEBUG) {
			logger.printf ("Zone set data: Zone GPIO:\tMUX:\tZoneID:\n");
		    for (i = 0; i <  j; i+=3)                         // iterate
				  logger.printf ("%d %d %d   ", tmpBuf[i], tmpBuf[i+1], tmpBuf[i+2]);
			logger.printf("\n");
			}
	return sendFreeCmd(SET_ZONE_SUB_CMD, zone[0].boardID, j, tmpBuf);
}	
//
//
void setAlarmZones(byte pldBuf[]) {
  for(int i=0, j=0;j<SLAVE_ZONES_CNT;j++)  {         // TODO - add check for GPIO
    memset((void*)&SzoneDB[j], 0, sizeof(struct ZONE));
	  SzoneDB[j].gpio = pldBuf[i++];
	  SzoneDB[j].mux 	= pldBuf[i++];
	  SzoneDB[j].zoneID = pldBuf[i++];
    }
	if(DEBUG) 
		printZones(SzoneDB, SLAVE_ZONES_CNT);
}
//
// master process messages root function. It is called when message (should be reply)  is received at master
//
void masterProcessMsg(struct MSG msg) {
//
  if(waiting_for_reply != (msg.cmd & ~REPLY_OFFSET))
	  ErrWrite(ERR_WARNING, "Master: Out of order reply \n");
  switch (msg.cmd) {
    case PING_RES:
	  ErrWrite(ERR_INFO, "Master: Unsupported reply command received PING_RES\n");
      break;
    case POLL_ZONES_RES:
      ErrWrite(ERR_INFO, "Master: Unsupported reply command received POLL_ZONES_RES\n");
      break;
    case SET_OUTS_RES:
      ErrWrite(ERR_INFO, "Master: Unsupported reply command received SET_OUTPUTS_RES\n");
      break;
    case FREE_CMD_RES:
	    //ErrWrite(ERR_DEBUG, "Master: reply received for FREE_CMD cmd: \n");
  		switch(msg.subCmd) {
  			case FREE_TEXT_SUB_CMD:
  				ErrWrite(ERR_DEBUG, "Master: reply received for FREE_TEXT_SUB_CMD: ");
          logger.printf("%s\n", msg.payload);
  				break;
   			case SET_ZONE_SUB_CMD:
  				ErrWrite(ERR_DEBUG, "Master: reply received for SET_ZONE_SUB_CMD: %1x\n", msg.payload[0] );
  				break;
  			default:
  				ErrWrite(ERR_WARNING, "Master: invalid sub-command received %x\n", msg.subCmd);
  				break;
  		  }
		  break;
    default:
      ErrWrite(ERR_WARNING, "Master: invalid command received %x\n", msg.cmd);
    }  // switch
} 

int slaveProcessCmd(struct MSG msg) {
//
  for(int i =0; i< MAX_MSG_LENGHT; i++)
    tmpMsg[i] = 0;
  LogMsg("slaveProcessCmd: message recv: LEN = %d, CMD = %x, DST = %x, PAYLOAD: ", msg.len-1, msg.cmd, msg.dst, msg.payload);
  switch (msg.cmd) {
    case PING:
    ErrWrite(ERR_INFO, "Master: Unsupported command received PING_RES\n");
      break;
    case POLL_ZONES:
        ErrWrite (ERR_DEBUG,"POLL ZONES command received\n");
        // send the zones status, stored by convertZones in SzoneResult[]
        if(zoneInfoValid == ZONE_A_VALID | ZONE_B_VALID) {
          if(ERR_OK != SendMessage(SlaveMsgChannel, SlaveUART, (POLL_ZONES | REPLY_OFFSET), MASTER_ADDRESS, SzoneResult, sizeof(SzoneResult)));
            ErrWrite(ERR_TRM_MSG, "Slave: Error in sendMessage\n");
        }
        break;
      break;
    case SET_OUTS:
      ErrWrite(ERR_INFO, "Master: Unsupported command received SET_OUTPUTS_RES\n");
      break;
    case FREE_CMD:
      ErrWrite(ERR_DEBUG, "Slave: received FREE_CMD cmd: \n");
      switch(msg.subCmd) {
        case FREE_TEXT_SUB_CMD:
          ErrWrite(ERR_DEBUG, "Slave: received for FREE_TEXT_SUB_CMD: ");
          logger.printf("%s\n", msg.payload);
          break;
        case SET_ZONE_SUB_CMD:
          ErrWrite(ERR_DEBUG, "Slave: received SET_ZONE_SUB_CMD\n");
		  setAlarmZones(msg.payload);
          tmpMsg[0] = rcvMsg.subCmd;                           // prepare reply payload, first byte  is the subcommand we are replying to 
          tmpMsg[1]  = 1;                                      // second бъте is the payload len,  which is 1 byte
          tmpMsg[2]  = ERR_OK;           					   // third is the aktual payload which in this cas is no error (ERR_OK)	 
          //for(int i =0; i< MAX_MSG_LENGHT; i++)
              //logger.printf ("%2d ", tmpMsg[i]);             // there is one byte cmd|dst
          //logger.printf("\n");
          if(ERR_OK != SendMessage(SlaveMsgChannel, SlaveUART, (FREE_CMD | REPLY_OFFSET), MASTER_ADDRESS, tmpMsg, FREE_CMD_PAYLD_LEN))
            ErrWrite(ERR_TRM_MSG, "Slave: Error in sendMessage\n");
          break;
        default:
          ErrWrite(ERR_WARNING, "Slave: invalid sub-command received %x\n", msg.subCmd);
          break;
        }
      break;
    default:
      ErrWrite(ERR_WARNING, "Master: invalid command received %x\n", msg.cmd);
    }  // switch
}

/*
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
        ErrWrite (ERR_INFO,"SLAVE: FREE CMD cmd received\n");
        // return the same payload converted to uppercase
        byte tmp_msg [MAX_PAYLOAD_SIZE];
        for (int i=0; i < rcvMsg.dataLen; i++)
          tmp_msg[i+2] = toupper(rcvMsg.payload[i]); 
        tmp_msg[0] = rcvMsg.subCmd;
        tmp_msg[1]  = rcvMsg.len;
        //logger.printf(" ---- rcvMsg.len %d\n", rcvMsg.len);
        //for(int i =0; i< MAX_MSG_LENGHT; i++)
        //    logger.printf ("%2d ", tmp_msg[i]);                // there is one byte cmd|dst
        //logger.printf("\n");
          if(ERR_OK != SendMessage(SlaveMsgChannel, SlaveUART, (FREE_CMD | REPLY_OFFSET), MASTER_ADDRESS, tmp_msg, rcvMsg.len))
            ErrWrite(ERR_TRM_MSG, "Slave: Error in sendMessage\n");
          break;
        default:
          ErrWrite (ERR_WARNING, "Slave: invalid command received %x\n", rcvMsg.cmd);
    }   // switch

*/
