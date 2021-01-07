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
byte tmpBuf[FREE_CMD_PAYLD_LEN];
	if(payloadLen > FREE_CMD_DATA_LEN)	 {				// check payload size
		ErrWrite (ERR_INV_PAYLD_LEN, "SendFreeCmd: error sending message -  too long???\n");   // must be already reported by compose_msg
		return ERR_INV_PAYLD_LEN;
		}
	tmpBuf[FREE_CMD_SUB_CMD_OFFSET] = subCmd;										  // prepare the message payload, first byte is the subCmd,
	tmpBuf[FREE_CMD_DATA_LEN_OFFSET] = payloadLen;								//  followed by actual payload len
	for (int i =0; i<payloadLen; i++) 					                  // and payload
		tmpBuf[i+FREE_CMD_HDR_LEN] = payload[i];
	return sendCmd(FREE_CMD, dst, tmpBuf);
}
//
// sends FREE TEXT cmd
// params: dst - destintion
//         dataLen - payload len to be send
//         payload - pointer to payload buffer
// returns: see sendFreeCmd() for return codes
// 
int sendFreeText(byte dst, int dataLen,  byte payload[]) {
    return sendFreeCmd(FREE_TEXT_SUB_CMD, dst, dataLen, payload); // sendCmd handle and reports errors internally 
}
//
#ifdef MASTER
//
// request from remote slave board zones params - GPIO, mux, zone number (zoneID)
// params: int dst - destination board ID 
// returns: see sendFreeCmd() for return codes
// 
int getSlaveZones(byte dst) {
byte cmdCode=0;
	return sendFreeCmd(GET_ZONE_SUB_CMD, dst, 1, &cmdCode);
}	
//
// extracts and sets to remote slave board zones params - GPIO, mux, zone number (zoneID)
// params: zone[] - array of ALARM_ZONE structs, containing zones info (including boardID)
//                  note: boardID of zone[0] will be used for all zones setting, as it is supposed that all zones belong to the same board
//                  command payload is set of SLAVE_ZONES_CNT triplets, each one containing (Zone GPIO, MUX, ZoneID)
// returns: see sendFreeCmd() for return codes
// 
//
int setSlaveZones(struct ALARM_ZONE zone[]) {
int j = 0; int i = 0;
//
	for(i=0; (i<SLAVE_ZONES_CNT) && (j<FREE_CMD_DATA_LEN); i++) {   // extract current zone info from zone array
		tmpMsg[j++] = zone[i].gpio;                                   // and put in payload
		tmpMsg[j++] = zone[i].mux;
		tmpMsg[j++] = zone[i].zoneID;
		}
	if(DEBUG) {                                                     // debug print
			logger.printf ("Zone set data: Zone GPIO:\tMUX:\tZoneID:\n");
		    for (i = 0; i <  j; i+=3)                                 // iterate
				  logger.printf ("%d %d %d   ", tmpMsg[i], tmpMsg[i+1], tmpMsg[i+2]);
			logger.printf("\n");
			logger.printf("setSlaveZones data len: %d\n", j);
			}
	return sendFreeCmd(SET_ZONE_SUB_CMD, zone[0].boardID, j, tmpMsg);
}	
//
// master process messages root function. It is called when message (should be reply)  is received at master
// patrams: struct MSG ms - contains received message attributes
// returns: none
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
			case GET_ZONE_SUB_CMD:
  				ErrWrite(ERR_DEBUG, "Master: reply received for GET_ZONE_SUB_CMD: %1x\n", msg.payload[0] );
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
//
#endif
//
#ifdef SLAVE
//
// sends to master slave board zones params - GPIO, mux, zone number (zoneID)
// params: zone[] - array of ALARM_ZONE structs, containing zones info (including boardID)
//                  reply payload is set of SLAVE_ZONES_CNT triplets, each one containing (Zone GPIO, MUX, ZoneID)
// returns: see sendFreeCmd() for return codes
// 
int returnSlaveZones(struct ZONE zone[]) {
int j = FREE_CMD_DATA_OFFSET; int i = 0;						// index where to put the payload, spare some room for header
//															  	// use global tmpMsg
	tmpMsg[FREE_CMD_SUB_CMD_OFFSET] = GET_ZONE_SUB_CMD;       	// prepare reply payload, first byte  is the subcommand we are replying to 
	for(i=0; (i<SLAVE_ZONES_CNT) && (j<FREE_CMD_DATA_LEN); i++) { // extract current zone info from zone array
		tmpMsg[j++] = zone[i].gpio;                             // and put in payload
		tmpMsg[j++] = zone[i].mux;
		tmpMsg[j++] = zone[i].zoneID;
		}
	tmpMsg[FREE_CMD_DATA_LEN_OFFSET]  = j-FREE_CMD_DATA_OFFSET;   // recalc actual paylaod len
	if(DEBUG) {                                                     // debug print
		logger.printf ("Zone get data: Zone GPIO:\tMUX:\tZoneID:\n");
		for (i = FREE_CMD_DATA_OFFSET; i <  j; i+=3)                                 // iterate
			logger.printf ("%d %d %d   ", tmpMsg[i], tmpMsg[i+1], tmpMsg[i+2]);
		logger.printf("\n");
		logger.printf("setSlaveZones data len: %d\n", j);
		}
		return SendMessage(SlaveMsgChannel, SlaveUART, (FREE_CMD | REPLY_OFFSET), MASTER_ADDRESS, tmpMsg, j); // one byte payload only
}	
//
// extract and stores the zone params from received command in local zones DB
// params: byte pldBuf[] - the payload of received SET_ZONE_SUB_CMD 
//         payload is set of SLAVE_ZONES_CNT triplets, each one containing (Zone GPIO, MUX, ZoneID) 
// returns: none
// TODO: - add check for GPIO
//
void setAlarmZones(byte pldBuf[]) {
  for(int i=0, j=0;j<SLAVE_ZONES_CNT;j++)  {         // TODO - add check for GPIO
    memset((void*)&SzoneDB[j], 0, sizeof(struct ZONE));
    SzoneDB[j].gpio = pldBuf[i++];
    SzoneDB[j].mux  = pldBuf[i++];
    SzoneDB[j].zoneID = pldBuf[i++];
    }
  if(DEBUG) 
    printZones(SzoneDB, SLAVE_ZONES_CNT);
}
//
// replies to master on received setAlarmZones cmd
// params: int err - error code to be returned
// returns: see sendFreeCmd() for return codes
// uses global tmpMsg[]
//
int replySetAlarmZones(int err) {
	tmpMsg[FREE_CMD_SUB_CMD_OFFSET] = SET_ZONE_SUB_CMD;       // prepare reply payload, first byte  is the subcommand we are replying to 
	tmpMsg[FREE_CMD_DATA_LEN_OFFSET]  = 1;                    // second бъте is the payload len,  which is 1 byte
	tmpMsg[FREE_CMD_DATA_OFFSET]  = err;           					  // third is the aktual payload which in this cas is no error (ERR_OK)	 
	//for(int i =0; i< MAX_MSG_LENGHT; i++)
	  //logger.printf ("%2d ", tmpMsg[i]);             				// there is one byte cmd|dst
	//logger.printf("\n");
	return SendMessage(SlaveMsgChannel, SlaveUART, (FREE_CMD | REPLY_OFFSET), MASTER_ADDRESS, tmpMsg, FREE_CMD_HDR_LEN+1); // one byte payload only
}
//
// replies to master on received poll zones cmd
// the zones status is stored by convertZones in SzoneResult[]
// params: byte zoneResultArr[] result of to be returned
//		   int len - len of the result
// returns: see sendFreeCmd() for return codes
// uses global zoneInfoValid
//
int replyPollZones(byte zoneResultArr[], int len) {
int ret;
       if(zoneInfoValid == ZONE_A_VALID | ZONE_B_VALID) 				// check if all zones are read already, if not does not reply
          return SendMessage(SlaveMsgChannel, SlaveUART, (POLL_ZONES | REPLY_OFFSET), MASTER_ADDRESS, zoneResultArr, len);
}
//
// slave process messages root function. It is called when message is received at slave
// patrams: struct MSG msg - contains received message attributes
// returns: none
//
int slaveProcessCmd(struct MSG msg) {
//
int i;
  for(i =0; i< MAX_MSG_LENGHT; i++)
    tmpMsg[i] = 0;
  LogMsg("slaveProcessCmd: message recv: DATA LEN = %d, CMD = %x, DST = %x, PAYLOAD: ", msg.dataLen, msg.cmd, msg.dst, msg.payload);
  switch (msg.cmd) {
    case PING:
    ErrWrite(ERR_INFO, "Master: Unsupported command received PING_RES\n");
      break;
    case POLL_ZONES:
        ErrWrite (ERR_DEBUG,"POLL ZONES command received\n");
        // send the zones status, stored by convertZones in SzoneResult[]
		    if(ERR_OK != replyPollZones(SzoneResult, sizeof(SzoneResult)))
          ErrWrite(ERR_TRM_MSG, "Slave: Error replying to POLL ZONES cmd");
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
          if(ERR_OK != returnSlaveZones(SzoneDB)) 							// reply with OK
            ErrWrite(ERR_TRM_MSG, "Slave: Error replying to SET_ZONE_SUB_CMD");
          break;
        case GET_ZONE_SUB_CMD:
          ErrWrite(ERR_DEBUG, "Slave: received GET_ZONE_SUB_CMD\n");
          if(ERR_OK != returnSlaveZones(SzoneDB))              // reply with requested zones info
            ErrWrite(ERR_TRM_MSG, "Slave: Error replying to GET_ZONE_SUB_CMD");
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
#endif
