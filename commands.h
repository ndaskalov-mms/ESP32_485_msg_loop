//
// command.h - implementation for commands/replies between devices
//
#ifdef MASTER
//
extern void printAlarmZones(byte *zone, int start, int end);
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
	  //logger.printf("cmd len = %d\n", len);
	  if(len == EXTRACT_LEN_FROM_PAYLOAD) 											                       // if len == 0, the len is at index 1 of the payload (FREE CMD)
		  len = payload[FREE_CMD_DATA_LEN_OFFSET]+FREE_CMD_HDR_LEN;								                   // sendMessage will validate it
    if (ERR_OK != (ret_code = SendMessage(MasterMsgChannel, MasterUART, cmd, dst, MASTER_ADDRESS,  payload, len))) {
      ErrWrite(ERR_TRM_MSG, "\n\nMaster: Err in sendMessage sending %d cmd\n\n", cmd);
      return ERR_TRM_MSG;
    }
    else {
      waiting_for_reply = cmd;
      return ERR_OK;
    }
}
//
// request from remote slave board zones conversion result
// params: int dst - destination board ID 
// returns: see sendCmd() for return codes
// 
int pollSlaveZones(byte dst) {
byte cmdCode=0;		// dummy payload
	return sendCmd(POLL_ZONES, dst, &cmdCode);
}
//
// extracts slave's zones info from POLL_ZONES command
// params: byte slave - slave board ID
//		   payload - byte [] - pointer to buffer with POLL_ZONES command result payload returned from slave
//		   int  len - payload len
//
void extractSlaveZonesInfo(int slave, byte payload[], int len) {
int res;
int newData = 0;
  for(int i = 0; i < len; i++ ) { // each byte contains result for two ADC channels or four alarm zones
	  // extract info from high nibble first - this shall be lower number zone
	  res = ((payload[i] >> ZONE_ENC_BITS) & (ZONE_ERROR_MASK | ZONE_A_MASK));  // high nibble, get zone A info
	  if(zonesDB[slave][4*i].zoneStat != res) { 								// check if zone status has changed
		  zonesDB[slave][4*i].zoneStat = res;										  // yes, store the new status
		  newData++;																              // mark it
	  }
	  res = ((payload[i] >> ZONE_ENC_BITS) & (ZONE_ERROR_MASK | ZONE_B_MASK));	// zone B
	  if(zonesDB[slave][4*i+1].zoneStat != res) {
		  zonesDB[slave][4*i+1].zoneStat = res; 
		  newData++;																              // mark it
	  }
	  res = (payload[i] & (ZONE_ERROR_MASK | ZONE_A_MASK));			// low nibble, get zone A info	
	  if(zonesDB[slave][4*i+2].zoneStat != res) {
		  zonesDB[slave][4*i+2].zoneStat = res;
		  newData++;																              // mark it
	  }
	  res = (payload[i] & (ZONE_ERROR_MASK | ZONE_B_MASK));			// zone B
	  if(zonesDB[slave][4*i+3].zoneStat != res) {
		  zonesDB[slave][4*i+3].zoneStat = res;
		  newData++;																              // mark it
	  }		
	}
	// printAlarmZones((byte *) &zonesDB, MASTER_ADDRESS, maxSlaves);
	if(newData)
		newZonesDataAvailable != NEW_DATA_BIT << slave;							// mark that new data are availble
}
//
//
// master process messages root function. It is called when message (should be reply)  is received at master
// patrams: struct MSG msg - contains received message attributes
//          global rcvMsg variable - content of the parsed message (reply)
// returns: none            TODO - add return code error or ERR_OK
//
void masterProcessMsg(struct MSG msg) {
//
int tmp;
  if(waiting_for_reply != (msg.cmd & ~REPLY_OFFSET))
	  ErrWrite(ERR_WARNING, "Master: Out of order reply \n");
  switch (msg.cmd) {
    case PING_RES:
	  ErrWrite(ERR_INFO, "Master: Unsupported reply command received PING_RES\n");
      break;
    case POLL_ZONES_RES:
	  ErrWrite(ERR_DEBUG, "Master: Reply received for POLL_ZONES_RES\n");
	  if(msg.dataLen > SZONERES_LEN)
		msg.dataLen = SZONERES_LEN;
		extractSlaveZonesInfo(msg.src, msg.payload, msg.dataLen);	// extract and copy to zonesDB, set newZonesDataAvailable in case of data changed
      break;
    case SET_OUTS_RES:
      ErrWrite(ERR_INFO, "Master: Unsupported reply command received SET_OUTPUTS_RES\n");
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
// replies to master on received poll zones cmd
// the zones status is stored by convertZones in SzoneResult[]
// params: byte zoneResultArr[] result of to be returned
//		   int len - len of the result
// returns: see sendFreeCmd() for return codes
// uses global zoneInfoValid
//
int replyPollZones(byte zoneResultArr[], int len) {
	//logger.printf("Zone info valid = %2x\n", zoneInfoValid);
       if(zoneInfoValid == (ZONE_A_VALID | ZONE_B_VALID)) 				// check if all zones are read already, if not does not reply
          return SendMessage(SlaveMsgChannel, SlaveUART, (POLL_ZONES | REPLY_OFFSET), MASTER_ADDRESS, slaveAdr, zoneResultArr, len);
	   else {
		   ErrWrite(ERR_INFO, "Poll zones received, but zones not converted yet\n");
		   return ERR_TRM_MSG;		
	   }
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
  LogMsg("slaveProcessCmd: message recv: DATA LEN = %d, CMD = %x, DST = %x, SRC = %x, PAYLOAD: ", msg.dataLen, msg.cmd, msg.dst, msg.src, msg.payload);
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
    case SET_OUTS:
      ErrWrite(ERR_INFO, "Master: Unsupported command received SET_OUTPUTS_RES\n");
      break;
    default:
      ErrWrite(ERR_WARNING, "Master: invalid command received %x\n", msg.cmd);
    }  // switch
}
#endif
