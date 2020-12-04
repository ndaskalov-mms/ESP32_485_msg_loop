  // ----------- slave simulation -------------------------------------------
  boardID = 1;        // Slave destination ---------   TODO - only for loopback testing
  memcpy(errorsDB_backup, errorsDB, sizeof(errorsDB_backup));   // backup error DB
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
        ErrWrite (ERR_RCV_MSG,"Slave parse message error\n"); // yse, do nothing
      } 
      else if (rcvMsg.dst == BROADCAST_ID)      // check for broadcast message
        ErrWrite (ERR_DEBUG,"Broadcast command received, skipping\n");  // do nothing
      else if (rcvMsg.dst != boardID)           // check if the destination is another board
        ;                                       // yes, do nothing
      else { 
		LogMsg ("Slave received CMD: %x; DEST: %x; payload len: %d; PAYLOAD: ", rcvMsg.len, rcvMsg.cmd, rcvMsg.dst, rcvMsg.payload);
        switch (rcvMsg.cmd) {
          case PING:
            ErrWrite (ERR_WARNING, "Unsupported command received PING\n");
            break;
          case POLL_ZONES:
            ErrWrite (ERR_WARNING,"Unsupported command received POLL_ZONES\n");
            break;
          case SET_OUTS:
            ErrWrite (ERR_WARNING,"Unsupported command received SET_OUTPUTS\n");
            break;
          case FREE_TEXT:
			ErrWrite (ERR_DEBUG,"FREE TEXT command received\n");
            // return the same payload converted to uppercase
            byte tmp_msg [MAX_PAYLOAD_SIZE];
            for (int i=0; i < rcvMsg.len; i++)
              tmp_msg[i] = toupper(rcvMsg.payload[i]);
            if(ERR_OK != SendMessage(SlaveMsgChannel, SlaveUART, (FREE_TEXT | REPLY_OFFSET), MASTER_ADDRESS, tmp_msg, rcvMsg.len))
              ErrWrite(ERR_TRM_MSG, "Slave: Error in sendMessage\n");
            break;
          default:
            ErrWrite (ERR_WARNING, "Slave: invalid command received %x\n", rcvMsg.cmd);
        }
      } // else
    }   // check for error return from .update  
  } // if update()
  // no message available for processing 
  // do something usefull like have a nap or read ADC and process the zones info
  // or collect some errors info to be send as status to MASTER some day
  // or try to collect some errors info and send via MQTT when it is availabel some sunny day
  if(memcmp(errorsDB, errorsDB_backup, sizeof(errorsDB))) {
    printNewErrors();
  }
