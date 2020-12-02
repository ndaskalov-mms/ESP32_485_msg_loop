  // ----------- slave simulation -------------------------------------------
  boardID = 1;        // Slave destination ---------   TODO - only for loopback testing
  if (err = SlaveMsgChannel.update ())
  {
    // check for receive error first
    if (err < 0)                                // error receiving message
    {
      ErrWrite (ERR_RCV_MSG, "Slave: error occured while receiving message, ignorring message\n");   
    }
    else                                        // no error
    {
      rcvMsg = parse_msg(SlaveMsgChannel);      // parse received message
      if (rcvMsg.parse_err) {                   // if parse error, do nothing
        ErrWrite (ERR_OK,"Slave parse message error\n"); // yse, do nothing
      } 
      else if (rcvMsg.dst == BROADCAST_ID)      // check for broadcast message
        logger.printf("Broadcast command received, skipping\n");  // do nothing
      else if (rcvMsg.dst != boardID)           // check if the destination is another board
        ;                                       // yes, do nothing
      else { 
        logger.printf ("Slave received CMD: %x; DEST: %x; payload len: %d; PAYLOAD: ", rcvMsg.cmd, rcvMsg.dst, rcvMsg.len);
        logger.write (rcvMsg.payload, rcvMsg.len); logger.println("");
        switch (rcvMsg.cmd) {
          case PING:
            logger.printf("Unsupported command received PING\n");
            break;
          case POLL_ZONES:
            logger.printf("Unsupported command received POLL_ZONES\n");
            break;
          case SET_OUTS:
            logger.printf("Unsupported command received SET_OUTPUTS\n");
            break;
          case FREE_TEXT:
            // logger.printf("Command received FREE_TEXT\n");
            // return the same payload converted to uppercase
            byte tmp_msg [MAX_PAYLOAD_SIZE];
            for (int i=0; i < rcvMsg.len; i++)
              tmp_msg[i] = toupper(rcvMsg.payload[i]);
            if(ERR_OK != SendMessage(SlaveMsgChannel, SlaveUART, (FREE_TEXT | REPLY_OFFSET), MASTER_ADDRESS, tmp_msg, rcvMsg.len))
              ErrWrite(ERR_OK, "Master: Error in sendMessage");
            break;
          default:
            logger.printf("Slave: invalid command received %x\n", rcvMsg.cmd);
        }
      } // else
    }   // check for error return from .update  
  } // if update()
  // no message available for processing 
  // do something usefull like have a nap or read ADC and process the zones info
  // or collect some errors info to be send as status to MASTER some day
  /*
  if(slave_err != SlaveMsgChannel.getErrorCount()) {
    //slave_err = SlaveMsgChannel.getErrorCount();
    logger.print("Slave errors cnt:");
    logger.println(slave_err, DEC);
  }
  */
