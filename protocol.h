// compose message containing:
// first byte:  upper 4 bits - command code 
//              lower 4 bits  - destination ID
// the rest:    payload, up to MAX_PAYLOAD_SIZE
// return: lenght of the composed message or NULL if error
byte  compose_msg(byte cmd, byte dest, byte *payload, byte *out_buf, int payload_len) {
  int index = 0;
  // first byte is COMMMAND/REPLY code (4 ms BITS) combined with the destination address (4 ls BITS)
  out_buf[index++] = ((cmd << 4) | (dest & 0x0F));
  // next comes the payload
  if ((payload_len + index) > MAX_MSG_LENGHT) {
    logger.printf("Payload size %d is larger than buffer size %d, skipping", payload_len, MAX_MSG_LENGHT);
    return NULL;
  }
  // copy  
  for (int i =0; i< payload_len; i++) 
    out_buf[index++] = payload[i];
  return index;           // return number of bytes to transmit
}

// ------------------------- code ----------------------------------------------
void SendMessage(RS485& trm_channel, struct MSG msg2trm ) {
    byte tmpBuf[MAX_MSG_LENGHT];
    byte tmpLen;              // lenght of data to transmit, shall be less than MAX_MSG_LENGHT
    if(!(tmpLen = compose_msg(msg2trm.cmd, msg2trm.dst, msg2trm.payload, tmpBuf, msg2trm.len))) {
      logger.println( "\nSlave:  Error composing message -  too long???");
      errors.protocol +=1;
      return;
    }
    logger.printf("Sending message LEN = %d, CMD|DST = %x, PAYLOAD = ", tmpLen, tmpBuf[0]);
    logger.write (&tmpBuf[1], tmpLen);
    logger.println();
    Slave_485_transmit_mode();
    SlaveMsgChannel.sendMsg (tmpBuf, tmpLen);
    Slave_Flush ();                     // make sure the data are transmitted properly befor swithching the line direction
    Slave_485_receive_mode();
    Slave_Flush();         // flushes both Tx and Rx
    logger.println("Transmitted, going back to listening mode");
}

struct MSG  parse_msg(RS485& rcv_channel) {

    struct MSG rmsg;                                      // temp buffers
    byte tmpBuf[MAX_MSG_LENGHT]; 
    rmsg.parse_err = 0;                                   // clear error flags
    
    rmsg.len = rcv_channel.getLength();                   // command+dest (1 byte) + payload_len (command specific)
    if ((rmsg.len <  1) || (rmsg.len >  MAX_MSG_LENGHT)){ // message len issue, at least 1 byte (command+dest)
      errors.protocol++;
      rmsg.parse_err = INV_PAYLD_LEN;                     
      logger.printf("parse_MSG: Invalid message payload len = %d\n", rmsg.len);
      return rmsg;                                        // error, no command code in message
    } 
    memcpy (tmpBuf, rcv_channel.getData (), rmsg.len);     // copy message in temp buf
    //logger.printf("Parse message recv: LEN = %d, CMD|DST = %x, PAYLOAD = %s\n", rmsg.len, tmpBuf[0], &tmpBuf[1]);
    // extract command and destination
    rmsg.cmd = ((tmpBuf[0] >> 4) & 0x0F);                  // cmd is hih nibble
    rmsg.dst = tmpBuf[0] & 0x0F;                           // destination is low nibble
    //logger.printf("Parse message recv: LEN = %d, CMD = %x, DST = %x, PAYLOAD = %s\n", rmsg.len, rmsg.cmd, rmsg.dst, &tmpBuf[1]);
    switch (rmsg.cmd & ~(0xF0 | REPLY_OFFSET )) {          // check for valid commands and replies. clear reply bit to facilitate test
      case PING:
        if (--rmsg.len == PING_PAYLD_LEN)
          memcpy(rmsg.payload, &tmpBuf[1], rmsg.len);
        else  
          rmsg.parse_err = INV_PAYLD_LEN;
        return rmsg;
        break;
      case POLL_ZONES:
        if (--rmsg.len == POLL_PAYLD_LEN)
          memcpy(rmsg.payload, &tmpBuf[1], rmsg.len);
        else  
          rmsg.parse_err = INV_PAYLD_LEN;
        memcpy(rmsg.payload, &tmpBuf[1], rmsg.len);     
        return rmsg;
        break;
      case SET_OUTS:
        if (--rmsg.len == SET_OUTS_PAYLD_LEN)
          memcpy(rmsg.payload, &tmpBuf[1], rmsg.len);
        else  
          rmsg.parse_err = INV_PAYLD_LEN;
        memcpy(rmsg.payload, &tmpBuf[1], rmsg.len);
        return rmsg;
        break;
      case FREE_TEXT:
        if (--rmsg.len == FREE_TEXT_PAYLD_LEN)
          memcpy(rmsg.payload, &tmpBuf[1], rmsg.len);
        else  
          rmsg.parse_err = INV_PAYLD_LEN;
        memcpy(rmsg.payload, &tmpBuf[1], rmsg.len);
        return rmsg;
        break;
      default:
        logger.printf("Unknown command %d received", rmsg.cmd);
        rmsg.parse_err = BAD_CMD;
        return rmsg;        // error, no command code in message;   
    }  // switch
}
