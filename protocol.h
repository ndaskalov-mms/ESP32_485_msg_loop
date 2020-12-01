/*
 * protocol.h
 */


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
byte  compose_msg(byte cmd, byte dest, byte *payload, byte *out_buf, int payload_len) {
  int index = 0;
  char tmpBuf[256];
  
  // first byte is COMMMAND/REPLY code (4 MS BITS) combined with the destination address (4 ls BITS)
  out_buf[index++] = ((cmd << 4) | (dest & 0x0F));
  // next comes the payload
  if ((payload_len + index) > MAX_MSG_LENGHT) {
    //sprintf(tmpBuf, "Payload size %d is larger than buffer size %d, message IS NOT SEND", payload_len, MAX_MSG_LENGHT);
    ErrWrite (ERR_INV_PAYLD_LEN, "Payload size %d is larger than buffer size", payload_len); 
    //logger.printf("ComposeMsg: Payload size %d is larger than buffer size %d, message IS NOT SEND", payload_len, MAX_MSG_LENGHT);
    return 0;
  }
  // copy  
  for (int i =0; i< payload_len; i++) 
    out_buf[index++] = payload[i];
  return index;           // return number of bytes to transmit
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
    byte tmpBuf[MAX_MSG_LENGHT];
    byte tmpLen;              // lenght of data to transmit, shall be less than MAX_MSG_LENGHT
    byte err = ERR_OK;        // means no error
    if(!(tmpLen = compose_msg(msg2trm.cmd, msg2trm.dst, msg2trm.payload, tmpBuf, msg2trm.len))) {
      ErrWrite (ERR_INV_PAYLD_LEN, "SendMsg - Error composing message -  too long???");   
      return ERR_INV_PAYLD_LEN;
    }
    LogMsg("SendMsg: sending message LEN = %d, CMD|DST = %x, PAYLOAD: ", tmpLen, tmpBuf[0], &tmpBuf[1]);
    uartTrmMode(uart);                         // switch line dir to transmit_mode;
    if(!trmChannel.sendMsg (tmpBuf, tmpLen)) {  // send fail. The only error which can originate for RS485 lib in sendMsg fuction is
      err = ERR_RS485;
      ErrWrite (ERR_NO_CALLBACK, "RS485.SendMsg error: no write callback");  
      }                                        // RS485 class is not configured properly, but we need to restore the line dir
    uartFlush(uart);                           // make sure the data are transmitted properly before reversing the line direction
    uartRcvMode(uart);                         // switch line dir to receive_mode;
    uartFlush(uart);                           // clean-up garbage due to switching, flushes both Tx and Rx
    ErrWrite (ERR_OK, "Transmitted, going back to listening mode\n");
    return err;
}

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
 
byte SendMessage(RS485& trmChannel, HardwareSerial& uart, byte cmd, byte dst, byte *payload, int len ) {
    byte tmpBuf[MAX_MSG_LENGHT];
    byte tmpLen;              // lenght of data to transmit, shall be less than MAX_MSG_LENGHT
    byte err = ERR_OK;
    if(!(tmpLen = compose_msg(cmd, dst, payload, tmpBuf, len))) {
      ErrWrite (ERR_INV_PAYLD_LEN, "Error composing message -  too long???");   
      return ERR_INV_PAYLD_LEN;
    }
    LogMsg("SendMsg: sending message LEN = %d, CMD|DST = %x, PAYLOAD: ", tmpLen, tmpBuf[0], &tmpBuf[1]);
    uartTrmMode(uart);                         // switch line dir to transmit_mode;
    if(!trmChannel.sendMsg (tmpBuf, tmpLen)) {  // send fail. The only error which can originate for RS485 lib in sendMsg fuction isfor missing write callback
      err = ERR_RS485;
      ErrWrite (ERR_NO_CALLBACK, "RS485.SendMsg error: no write callback"); 
      }
    uartFlush(uart);                           // make sure the data are transmitted properly before revercing the line direction
    uartRcvMode(uart);                         // switch line dir to receive_mode;
    uartFlush(uart);                           // clean-up garbage due to switching, flushes both Tx and Rx
    ErrWrite (ERR_OK, "Transmitted, going back to listening mode\n");
    return err;
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
    if ((rmsg.len <  1) || (rmsg.len >  MAX_MSG_LENGHT)){ // message len issue, at least 1 byte (command+dest)
      errors.rcv_payload += 1;
      rmsg.parse_err = ERR_INV_PAYLD_LEN;  
      ErrWrite(ERR_INV_PAYLD_LEN, "parse_msg error payload %d exceeds buffer",rmsg.len );                      
      return rmsg;                                        // error, no command code in message
    } 
    memcpy (tmpBuf, rcv_channel.getData (), rmsg.len);     // copy message in temp buf
    LogMsg("Parse_meg: message recv: LEN = %d, CMD|DST = %x, PAYLOAD: ", rmsg.len, tmpBuf[0], &tmpBuf[1]);
    // extract command and destination
    rmsg.cmd = ((tmpBuf[0] >> 4) & 0x0F);                  // cmd is hihg nibble
    rmsg.dst = tmpBuf[0] & 0x0F;                           // destination is low nibble
    LogMsg("Parse_meg: message recv: LEN = %d, CMD = %x, DST = %x, PAYLOAD: ", rmsg.len, rmsg.cmd, rmsg.dst, &tmpBuf[1]);
    switch (rmsg.cmd & ~(0xF0 | REPLY_OFFSET )) {          // check for valid commands and replies. clear reply bit to facilitate test
      case PING:
        if (--rmsg.len == PING_PAYLD_LEN)
          memcpy(rmsg.payload, &tmpBuf[1], rmsg.len);
        else {
          rmsg.parse_err = ERR_INV_PAYLD_LEN;
          errors.rcv_payload += 1;
          return rmsg;
        }
        break;
      case POLL_ZONES:
        if (--rmsg.len == POLL_PAYLD_LEN)
          memcpy(rmsg.payload, &tmpBuf[1], rmsg.len);
        else { 
          rmsg.parse_err = ERR_INV_PAYLD_LEN;
          errors.rcv_payload += 1;
          return rmsg;
        }
        break;
      case SET_OUTS:
        if (--rmsg.len == SET_OUTS_PAYLD_LEN)
          memcpy(rmsg.payload, &tmpBuf[1], rmsg.len);
        else { 
          rmsg.parse_err = ERR_INV_PAYLD_LEN;
          errors.rcv_payload += 1;
          return rmsg;
        }
        break;
      case FREE_TEXT:
        if (--rmsg.len == FREE_TEXT_PAYLD_LEN) 
          memcpy(rmsg.payload, &tmpBuf[1], rmsg.len);
        else  {
          rmsg.parse_err = ERR_INV_PAYLD_LEN;
          errors.rcv_payload += 1;
          return rmsg;
        }
        break;
      default:
        rmsg.parse_err = ERR_BAD_CMD;
        errors.bad_cmd += 1;
        ErrWrite(ERR_BAD_CMD, "parse_msg error bad command %x received",rmsg.cmd ); 
        return rmsg;        // error, no command code in message;   
    }  // switch
    return rmsg;
}
