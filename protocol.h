/*
 * protocol.h
 */
//
// check for timeout waiting for receive message
// the transmission time is captured in RS485 class sending function and is retrieved by getLastTransmitTime
// returns 1 if timeout, 0 if not
int checkTimeout(RS485& Channel, unsigned long timeout) {
    return ((unsigned long)(millis() - Channel.getLastTransmitTime ()) > timeout);
}
// compose message containing:
// first byte:  upper 4 bits - command code 
//              lower 4 bits  - destination ID
// the rest:    payload, up to MAX_PAYLOAD_SIZE
// params:
//      cmd       - command to be send;   range 0 - 15; Commands IDs are 0-7, when MS bit (0x8) means reply to particular command
//      dst       - destiantion board ID; range 0 - 15; , 0 is Master, 0xF: broadcat
//      payload[]  - poiner to buffer containing the payload of the command
//      out_buf[]   - buffer where to store the composed message
//      payload_len - len of the payload to be send
// return: lenght of the composed message or 0 if error
//
int compose_msg(byte cmd, byte dest, byte *payload, byte *out_buf, int payload_len) {
  int index = 0;
  
  // first byte is COMMMAND/REPLY code (4 MS BITS) combined with the destination address (4 ls BITS)
  out_buf[index++] = ((cmd << 4) | (dest & 0x0F));
  // next comes the payload
  if ((payload_len + index) > MAX_MSG_LENGHT) {
     return ERR_INV_PAYLD_LEN;						// all errors are negative numbers
  }
  // copy  
  for (int i =0; i< payload_len; i++) 
    out_buf[index++] = payload[i];
  return index;           							// return number of bytes to transmit
}
//
// send message using 485 interface. Message parameters are transferred as separate arguments
// params:
//      RS485& trmChannel     - reference to instanse of RS485 class, defined in RS485 lib to handle frame (STX, ETX, CRC) level send/receive staff
//      HardwareSerial& uart  - reference to instance of HardwareSerial class defined in Arduino core to handle UART level stall
//      cmd       - one byte command to be send;   range 0 - 15; Commands IDs are 0-7, when MS bit (0x8) means reply to particular command
//      dst       - one byte destiantion board ID; range 0 - 15; , 0 is Master, 0xF: broadcat
//      payload[]  - poiner to buffer containing the payload of the command
//      len       - len of the payload to be send
// returns:   ERR_INV_PAYLD_LEN - payload larger than buffer
//            ERR_RS485         - error in RS485 lib (sendMsg, no write callback)
//            ERR_OK            - Alles in ordung   
// other:     reflects error occured in global errors struct as well         
 
int SendMessage(RS485& trmChannel, HardwareSerial& uart, byte cmd, byte dst, byte *payload, int len ) {
    byte tmpBuf[MAX_MSG_LENGHT];
    int tmpLen;              // lenght of data to transmit, shall be less than MAX_MSG_LENGHT
    byte err = ERR_OK;
    if(!(tmpLen = compose_msg(cmd, dst, payload, tmpBuf, len))) {
      ErrWrite (ERR_INV_PAYLD_LEN, "SendMessage: error composing message -  too long???\n");   // must be already reported by compose_msg
      return ERR_INV_PAYLD_LEN;
    }
    if(cmd == FREE_CMD) 
      logger.printf("SendMsg: sending message LEN = %d, CMD|DST = %x, subCMD = %x, payload len = %d, PAYLOAD: %s\n ",\
                                                tmpLen,    tmpBuf[0],    tmpBuf[1],       tmpBuf[2],    &tmpBuf[3]);
    else
      LogMsg("SendMsg: sending message LEN = %d, CMD|DST = %x, PAYLOAD: ", tmpLen, tmpBuf[0], &tmpBuf[1]);
    uartTrmMode(uart);                  	// switch line dir to transmit_mode;
    if(!trmChannel.sendMsg (tmpBuf, tmpLen)) { // send fail. The only error which can originate for RS485 lib in sendMsg fuction isfor missing write callback
      err = ERR_TRM_MSG;
      ErrWrite (ERR_TRM_MSG, "SendMessage trasmit error: no write callback probably\n"); 
      }
	uartFlush(uart);                        // make sure the data are transmitted properly before revercing the line direction
    uartRcvMode(uart);                      // switch line dir to receive_mode;
    uartFlush(uart);                        // clean-up garbage due to switching, flushes both Tx and Rx
    ErrWrite (ERR_INFO, "Transmitted, going back to listening mode\n");
    return err;
}

// send message using 485 interface. Mesagge parameters are transferred as members of struct type MSG
// params:
//      RS485& trmChannel     - reference to instanse of RS485 class, defined in RS485 lib to handle frame (STX, ETX, CRC) level send/receive staff
//      HardwareSerial& uart  - reference to instance of HardwareSerial class defined in Arduino core to handle UART level stall
//      struct MSG msg2trm    - struct with message details like CDM, DST, PAYLOAD, LEN
// returns:   ERR_INV_PAYLD_LEN - payload larger than buffer
//            ERR_RS485         - error in RS485 lib (sendMsg, no write callback)
//            ERR_OK            - Alles in ordung   
// other:     reflects error occured in global errors struct as well         
 
byte SendMessage(RS485& trmChannel, HardwareSerial& uart, struct MSG msg2trm ) {
    return (SendMessage(trmChannel, uart, msg2trm.cmd, msg2trm.dst, msg2trm.payload, msg2trm.len ));
}
// parse received message
// byte[0] CMD | DST (4 MS bits is command and lower 4 bits is destination
// byte [1] ......... byte[MAX_MSG_LENGHT] - payload
// MAX_MSG_LENGHT is calculated in a way to fit complete message in TxFIFO
// params:    reference to instance of RS485 to be used for messgae retrieve
// returns:   struct MSG with received message or parse ERR flag set in case of errors
// other:     reflects error occured in global errors struct as well
struct MSG  parse_msg(RS485& rcv_channel) {
    struct MSG rmsg;                                      // temp buffers
    byte tmpBuf[MAX_MSG_LENGHT]; 
    rmsg.parse_err = 0;                                   // clear error flags
    rmsg.len = rcv_channel.getLength();                   // command+dest (1 byte) + payload_len (command specific)
    if ((rmsg.len <  1) || (rmsg.len >  MAX_MSG_LENGHT)){ // message len issue, at least 1 byte (command+dest) // FIXME
      rmsg.parse_err = ERR_INV_PAYLD_LEN;  
      ErrWrite(ERR_INV_PAYLD_LEN, "Parse_msg: error payload len: %d exceeds buffer size\n",rmsg.len );                      
      return rmsg;                                        // error, no command code in message
    } 
    memcpy (tmpBuf, rcv_channel.getData (), rmsg.len);     // copy message in temp buf
    //LogMsg("Parse_msg: message recv: LEN = %d, CMD|DST = %x, PAYLOAD: ", rmsg.len-1, tmpBuf[0], &tmpBuf[1]);
    // extract command and destination
    rmsg.cmd = ((tmpBuf[0] >> 4) & 0x0F);                  // cmd is hihg nibble
    rmsg.dst = tmpBuf[0] & 0x0F;                           // destination is low nibble
    //LogMsg("Parse_msg: message recv: LEN = %d, CMD = %x, DST = %x, PAYLOAD: ", rmsg.len-1, rmsg.cmd, rmsg.dst, &tmpBuf[1]);
    switch (rmsg.cmd & ~(0xF0 | REPLY_OFFSET )) {          // check for valid commands and replies. clear reply bit to facilitate test
      case PING:
        if (--rmsg.len == PING_PAYLD_LEN)
          memcpy(rmsg.payload, &tmpBuf[1], rmsg.len);
        else {
          rmsg.parse_err = ERR_INV_PAYLD_LEN;
          ErrWrite(rmsg.parse_err, "Parse message: invalid payload len for PING msg received\n");
          return rmsg;
        }
        break;
      case POLL_ZONES:
        if (--rmsg.len == POLL_PAYLD_LEN)
          memcpy(rmsg.payload, &tmpBuf[1], rmsg.len);
        else { 
          rmsg.parse_err = ERR_INV_PAYLD_LEN;
          ErrWrite(rmsg.parse_err, "Parse message: invalid payload len for POLL_ZONES msg received\n");
          return rmsg;
        }
        break;
      case SET_OUTS:
        if (--rmsg.len == SET_OUTS_PAYLD_LEN)
          memcpy(rmsg.payload, &tmpBuf[1], rmsg.len);
        else { 
          rmsg.parse_err = ERR_INV_PAYLD_LEN;
          ErrWrite(rmsg.parse_err, "Parse message: invalid payload len for SET_OUTS msg received\n");
          return rmsg;
        }
        break;
      case FREE_CMD:
        if (--rmsg.len == FREE_CMD_PAYLD_LEN) {
          rmsg.len = tmpBuf[2];                                     // actual free cmd payload size
          rmsg.subCmd = tmpBuf[1]; 
          memcpy(rmsg.payload, &tmpBuf[3], rmsg.len);               // two bytes for subCmd and payload len
          //LogMsg("Parse_msg: FREE CMD recv: DATA LEN = %d, subCMD = %x, DST = %x, DATA: ", rmsg.len, rmsg.subCmd, rmsg.dst, rmsg.payload);
          }      
        else  {
          rmsg.parse_err = ERR_INV_PAYLD_LEN;
          ErrWrite(rmsg.parse_err, "Parse message: invalid payload len for FREE TEXT msg received\n");
          return rmsg;
        }
        break;
      default:
        rmsg.parse_err = ERR_BAD_CMD;
        ErrWrite(ERR_BAD_CMD, "parse_msg error bad command %x received\n",rmsg.cmd ); 
        return rmsg;        // error, no command code in message;   
    }  // switch
    return rmsg;
}

// 
// check and if any parce received message  
// in case of message available, parce message gunction retrieves it and stores all message components in returned stuct MSG
// assigned to global rcvMSG variable.
// parameters: none
// return: 	ERR_OK (0) in case of no message or message which is not for us
//			    ERR_RCV_MSG (negative) in case of parsing error
//          MSG_READY (1) if message present          
int check4msg(RS485& Channel, unsigned long timeout) {
	if (!(err = Channel.update ())) {                 // 0 - nothing received yet, != something is waiting                                                      
     if(timeout && ((unsigned long)(millis() - Channel.getLastTransmitTime ()) > timeout)) // check for message reply timeout            
          return ErrWrite(ERR_TIMEOUT, "Check4msg Reply timeout\n"); // timeout expired              // 0 means no timeout, wait forever
     else                                       
          return ERR_OK;                            // timeout did not expired, continue waiting              
	}
  // got message, check for receive error first
  if (err < 0) {                                  // error receiving message
		ErrWrite (ERR_RCV_MSG, "Err while rcv msg, ignorring\n"); 
		return ERR_RCV_MSG; 
  }
	// parse received message
	rcvMsg = parse_msg(Channel);      
	if (rcvMsg.parse_err) {                   
		ErrWrite (ERR_RCV_MSG,"Parse message error\n"); // parsing error
		return ERR_RCV_MSG; 
		} 
	// check for broadcast message
	if (rcvMsg.dst == BROADCAST_ID)  {    
		ErrWrite (ERR_INFO,"Broadcast command received, skipping\n");  // do nothing
		return ERR_OK;	
	}
	// check if the destination 
	if(rcvMsg.dst != boardID)           		        // check if the destination is another board
		return ERR_OK;                                // yes, do nothing
	// we got message for us
	LogMsg ("Received MSG: LEN = %d, CMD = %x, DST = %x, PAYLOAD: ",rcvMsg.len, rcvMsg.cmd, rcvMsg.dst,  rcvMsg.payload);
	return MSG_READY;								                // have message
} // check4msg

void masterProcessMsg(struct MSG msg) {
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
      ErrWrite(ERR_INFO, "Master: reply received for FREE_CMD cmd: \n");
      break;
    default:
      ErrWrite(ERR_WARNING, "Master: invalid command received %x\n", rcvMsg.cmd);
    }  // switch
} 
//
// check if it is time to send reccuring command (like POLL)
// params: cmd - command code (can be with reply flag set as well
//         timeout - timeout in milliseconds
// returns: false - on wrong command checked or tot a time yet
//          true  - time to send
int isTimeFor(byte cmd, unsigned long timeout) {
    int cmd_index = findCmdEntry(cmd);              	 // get index into database in order to access command parameters
    if(ERR_DB_INDEX_NOT_FND == cmd_index) {              //  the command is not found in the database
      ErrWrite(ERR_DB_INDEX_NOT_FND, "IsTimeFor: CMD %d not found\n", cmd);
	  return false;                                      // this is a bit ugly, but will notify the world  
    }                                                    // that we cannot send this command 
    return ((unsigned long)(millis() - cmdDB[cmd_index].last_transmitted) > timeout);
}
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
