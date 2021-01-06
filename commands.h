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
    int ret_code, len;
    if (waiting_for_reply)
      return ERR_OK;                                     // TODO not sure if it is not better to return error, otherwise we can lock-up on waiting_for_repy
    int cmd_index = findCmdEntry(cmd);                   // get index into database in order to access command parameters
    if(ERR_DB_INDEX_NOT_FND == cmd_index) {              //  the command is not found in the database
      ErrWrite(ERR_DB_INDEX_NOT_FND, "\n\nMaster: err in findCmdEntry lookup for %d cmd\n\n", cmd);
      return ERR_BAD_CMD; 
    }
    cmdDB[cmd_index].last_transmitted = millis();        // register the send time
	  len = cmdDB[cmd_index].len;                          // get payload len from cmd db 
	  if(!len) 											                       // if len == 0, the len is at index 1 of the payload (FREE CMD)
		  len = payload[FREE_CMD_DATA_LEN_OFFSET]+FREE_CMD_HDR_LEN;								                   // sendMessage will validate it
    if (ERR_OK != (ret_code = SendMessage(MasterMsgChannel, MasterUART, cmd, dst, payload, len))) {
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
	if(payloadLen > FREE_CMD_DATA_LEN)	 {				// check payload size
		ErrWrite (ERR_INV_PAYLD_LEN, "SendFreeCmd: error sending message -  too long???\n");   // must be already reported by compose_msg
		return ERR_INV_PAYLD_LEN;
		}
	tmpBuf[FREE_CMD_SUB_CMD_OFFSET] = subCmd;										      // prepare the message payload, first byte is the subCmd,
	tmpBuf[FREE_CMD_DATA_LEN_OFFSET] = payloadLen;									    //  followed by actual payload len
	for (int i =0; i<payloadLen; i++) 					// and payload
		tmpBuf[i+FREE_CMD_HDR_LEN] = payload[i];
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
	logger.printf("setSlaveZones data len: %d\n", j);
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
int i;
  for(i =0; i< MAX_MSG_LENGHT; i++)
    tmpMsg[i] = 0;
  LogMsg("slaveProcessCmd: message recv: DATA LEN = %d, CMD = %x, DST = %x, PAYLOAD: ", msg.dataLen, msg.cmd, msg.dst, msg.payload);
  LogMsg("Parse_msg: FREE CMD recv: Total LEN: %d, CMD: %2x, DST = %x, subCMD = %2x, DATA LEN %d, DATA: ", rmsg.len, rmsg.cmd, rmsg.dst, rmsg.subCmd,  rmsg.dataLen, rmsg.payload);
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
          i = 0;
          tmpMsg[i++] = rcvMsg.subCmd;                           // prepare reply payload, first byte  is the subcommand we are replying to 
          tmpMsg[i++]  = 1;                                      // second бъте is the payload len,  which is 1 byte
          tmpMsg[i++]  = ERR_OK;           					   // third is the aktual payload which in this cas is no error (ERR_OK)	 
          //for(int i =0; i< MAX_MSG_LENGHT; i++)
              //logger.printf ("%2d ", tmpMsg[i]);             // there is one byte cmd|dst
          //logger.printf("\n");
          if(ERR_OK != SendMessage(SlaveMsgChannel, SlaveUART, (FREE_CMD | REPLY_OFFSET), MASTER_ADDRESS, tmpMsg, i))
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
