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
int setSlaveZone(struct ALARM_ZONE zone) {
byte tmpBuf[32];
	tmpBuf[0] = zone.gpio;
	tmpBuf[1] = zone.mux;
	tmpBuf[2] = zone.zoneID;
	return sendFreeCmd(SET_ZONE_SUB_CMD, zone.boardID, SET_ZONE_DATA_LEN, tmpBuf);
}	
//
// master process messages root function. It is called when message (should be reply)  is received at master
//
void masterProcessMsg(struct MSG msg) {
  if(waiting_for_reply != msg.cmd)
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
  					switch(msg.subCmd)
  			case SET_ZONE_SUB_CMD:	
  				ErrWrite(ERR_DEBUG, "Master: reply received for SET_ZONE_SUB_CMD: %1x\n", msg.payload[0] );
  				break;
  			default:
  				ErrWrite(ERR_WARNING, "Master: invalid sub-command received %x\n", rcvMsg.subCmd);
  				break;
  		  }
		  break;
    default:
      ErrWrite(ERR_WARNING, "Master: invalid command received %x\n", rcvMsg.cmd);
    }  // switch
} 
