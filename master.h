  boardID = 0;        // TODO - only for loopback testing
  static int i;
  
  memcpy(errorsDB_backup, errorsDB, sizeof(errorsDB_backup));   // backup error DB
  
  if (waiting_for_reply)
  {
    if (err = MasterMsgChannel.update ())
    {
      // check for receive error first
      if (err < 0)                              // error receiving message
      {
        ErrWrite (ERR_RCV_MSG, "Master: error occured while receiving message, ignorring message\n");   
      }
      else                                      // no error
      {
        // msg received, no errors
        ErrWrite(ERR_OK, "Master just got message\n");
        waiting_for_reply = 0;                    // TODO - check for out-of-order messages
        uartTrmMode(MasterUART);                  // Switch back to transmit_mode
        rcvMsg = parse_msg(MasterMsgChannel);   
        if (rcvMsg.parse_err) {                   // if parse error, do nothing
          //logger.printf ("Master parse message error %d\n", rcvMsg.parse_err); // yes, do nothing
          ErrWrite(ERR_OK, "Master parse message error\n");
        } 
        else if (rcvMsg.dst == BROADCAST_ID)      // check for broadcast message
          ErrWrite(ERR_OK, "Master: Broadcast command received, skipping\n");  // do nothing
        else if (rcvMsg.dst != boardID)           // check if the destination is another board
          ;                                       // TODO - master is not supposed to get this as slaves are not talcking to each other
        else { 
          //logger.printf ("Master received CMD: %x; DEST: %x; payload len: %d; PAYLOAD: ", rcvMsg.cmd, rcvMsg.dst, rcvMsg.len);
          //logger.write (rcvMsg.payload, rcvMsg.len); logger.println("");
          LogMsg("Master received message LEN: %d, CMD: %x; DEST: %x; PAYLOAD: ",rcvMsg.len, rcvMsg.cmd, rcvMsg.dst, rcvMsg.payload);
          switch (rcvMsg.cmd) {
            case PING_RES:
              logger.printf("Master: Unsupported reply command received PING_RES\n");
              break;
            case POLL_ZONES_RES:
              logger.printf("Master: Unsupported reply command received POLL_ZONES_RES\n");
              break;
            case SET_OUTS_RES:
              logger.printf("Master: Unsupported reply command received SET_OUTPUTS_RES\n");
              break;
            case FREE_TEXT_RES:
              logger.printf("Master: reply received FREE_TEXT_RES: ");
              logger.write (rcvMsg.payload, rcvMsg.len); logger.println("");
              break;
            default:
              logger.printf("Master: invalid command received %x\n", rcvMsg.cmd);
         }  // else rcvMsg.parse_err check
        }   // else (no error)
       }    // if update
      }     // else (error check)
    else if((unsigned long)(millis() - last_transmission) > REPLY_TIMEOUT) {
        // reply not received
        waiting_for_reply = 0;
        // TODO - signal error somehow
        logger.println ("Master reply timeout");
    }   // else
  }     // if (waiting_for_reply)
  //
  //check if it is time for the next communication
  //
  else if( (unsigned long)(millis() - last_transmission) > TRM_INTERVAL){         // yes, it's time
    ErrWrite( ERR_OK, "Master:  Time to transmit -------------------------------\n" );
    if(ERR_OK != SendMessage(MasterMsgChannel, MasterUART, FREE_TEXT, SLAVE1_ADDRESS, test_msg[(++i)%3], MAX_PAYLOAD_SIZE)){
      ErrWrite(ERR_OK, "Master: Error in sendMessage");
      // MQTT send error
    }
    else {
      last_transmission = millis();    // mark the transmit time so we can calculate the time for the next transmission and check for reply timeout
      logger.println("Master MSG transmitted, receive timeout started");
      waiting_for_reply = 1;
    }
  }
  //
  // no message available for processing and it is not time to send a new one
  // do something usefull like have a nap or read MQTT
  else {
    //logger.print( "." );    // no, do nothing
  }
  // or try to collect some errors info and send via MQTT when it is availabel some sunny day
  if(memcmp(errorsDB, errorsDB_backup, sizeof(errorsDB))) {
    printErrorsDB();
  }

   
		
  
  
  
