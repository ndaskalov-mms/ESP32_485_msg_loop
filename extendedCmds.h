/*
//   slavesSetZonesMap = prepareSlavesSetMap(MAX_SLAVES); // init bitmap to track if zones data are loaded to slaves
//   slavesSetPgmsMap =  prepareSlavesSetMap(MAX_SLAVES); // init bitmap to track if pgms data are loaded to slaves
//
// create bitmap to track if config data are loaded to slaves: bit 0 cooresponds to slave 1, bit 1 oslave 2, ....
// bit set means the config data are not sent to slave board
//
byte prepareSlavesSetMap(int max_slaves) {
byte tmp = 0;
	for(int i = 0; i< max_slaves; i++) 
		tmp |= 1<<i;
	return tmp;
}
//
// markSlaveAsSet in slaves bitmap tracking if config data are loaded to slaves: bit 0 cooresponds to slave 1, bit 1 oslave 2, ....
// bit set means the config data are not sent to slave board, cleared means config data loaded successfully
//
byte markSlaveAsSet(int slave, byte slaveMap) {
		slaveMap = slaveMap & ~(1<<(--slave));
		//lprintf("SlaveMap = %d\n", slaveMap);
		return slaveMap;
} 
//
// returns slave adress of the first board not set in the slaveSetMap
// bit set means the config data are not sent to slave board, cleared means config data loaded successfully
//
byte bitmap2slaveIdx(byte slaveMap, int max_slaves) {
		for(int i =0; i< max_slaves; i++) {
			if(slaveMap & (1<<i))
				return ++i;
		}
		return 0;
} 
*/

// from protocol.h
//
// obsolete functions to set /get zones/pgms GPIO, etc
//
/*
#ifdef MASTER
// -----------------add to masterProcess to support FREE CMD --------------------------
    case FREE_CMD_RES:
	    //ErrWrite(ERR_DEBUG, "Master: reply received for FREE_CMD cmd: \n");
  		switch(msg.subCmd) {
  			case FREE_TEXT_SUB_CMD:
  				ErrWrite(ERR_DEBUG, "Master: reply received for FREE_TEXT_SUB_CMD: ");
                lprintf("%s\n", msg.payload);
  				break;
   			case SET_ZONE_SUB_CMD:
			case GET_ZONE_SUB_CMD:
  				ErrWrite(ERR_DEBUG, "Master: reply received for SET/GET_ZONE_SUB_CMD\n");
				slavesSetZonesMap = markSlaveAsSet(msg.src, slavesSetZonesMap);  // mark we got reply from this slave, so we continue with others
  				break;
			case SET_PGM_SUB_CMD:
			case GET_PGM_SUB_CMD:
			  ErrWrite(ERR_DEBUG, "Master: reply received for SET_PGM_SUB_CMD or GET_PGM_SUB_CMD: \n");
			  slavesSetPgmsMap = markSlaveAsSet(msg.src, slavesSetPgmsMap);  // mark we got reply from this slave, so we continue with others
			  break;
			default:
			  ErrWrite(ERR_WARNING, "Master: invalid sub-command received %x\n", msg.subCmd);
			  break;
			  }
		  break;
// -----------------------------------------
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
// request from remote slave board zones params - GPIO, mux, zone number (zoneID)
// params: int dst - destination board ID 
// returns: see sendFreeCmd() for return codes
// 
int getSlaveZones(byte dst) {
byte cmdCode=0;		// dummy payload
	return sendFreeCmd(GET_ZONE_SUB_CMD, dst, 1, &cmdCode);
}	
//
// extracts and sets to remote slave board zones params - GPIO, mux, zone number (zoneID)
// params: zone[] - array of ALARM_ZONE structs, containing zones info 
//		   dst	-   slave board ID
// command payload is set of SLAVE_ZONES_CNT triplets, each one containing (Zone GPIO, MUX, ZoneID)
// returns: see sendFreeCmd() for return codes
// 
//
extern void printAlarmZones(byte *zone, int start, int end);

int sendSlaveZones(struct ALARM_ZONE zone[], int dst) {
int j = 0; int i = 0;
//
	//printAlarmZones((byte *) zone, 0, 0);
	for(i=0; (i<SLAVE_ZONES_CNT) && (j<FREE_CMD_DATA_LEN); i++) {   // extract current zone info from zone array
		tmpMsg[j++] = zone[i].gpio;                                   // and put in payload
		tmpMsg[j++] = zone[i].mux;
		tmpMsg[j++] = zone[i].zoneID;
		}
	if(DEBUG) {                                                     // debug print
			lprintf ("Zone set data: Zone GPIO:\tMUX:\tZoneID:\n");
		    for (i = 0; i <  j; i+=3)                                 // iterate
				  lprintf ("%d %d %d   ", tmpMsg[i], tmpMsg[i+1], tmpMsg[i+2]);
			lprintf("\n");
			lprintf("setSlaveZones data len: %d\n", j);
			}
	return sendFreeCmd(SET_ZONE_SUB_CMD, dst, j, tmpMsg);
}	
//
// extracts and sets to remote slave board zones params - GPIO, mux, zone number (zoneID)
// params: pgm[] - array of PGM structs, containing PGMs info 
//		   dst	-   slave board ID
// command payload is set of SLAVE_ZONES_CNT triplets, each one containing (Zone GPIO, MUX, ZoneID)
// returns: see sendFreeCmd() for return codes
// 
//
int sendSlavePGMs(struct ALARM_PGM pgm[], int dst) {
int j = 0; int i = 0;
//
	for(i=0; (i<SLAVE_PGM_CNT) && (j<FREE_CMD_DATA_LEN); i++) {   // extract current PGM info from all pgms array
		tmpMsg[j++] = pgm[i].gpio;                                   // and put in payload
		tmpMsg[j++] = pgm[i].pgmID;
		tmpMsg[j++] = pgm[i].iValue;
		}
	if(DEBUG) {                                                     // debug print
			lprintf ("PGM set data: GPIO:\tID:\tinit value:\n");
		    for (i = 0; i <  j; i+=3)                                 // iterate
				  lprintf ("%d %d %d   ", tmpMsg[i], tmpMsg[i+1], tmpMsg[i+2]);
			lprintf("\n");
			lprintf("setSlavePGMs data len: %d\n", j);
			}
	return sendFreeCmd(SET_PGM_SUB_CMD, dst, j, tmpMsg);
}	
//
// void setSlavesZones() - sends zones, pgms, etc config data to all slaves in the slavesSetZonesMap
// nonblocking function, shall be called periodically from loop() in order to check the results and move to next slave
// config data are read from global zonesDB, pgmsDB, etc. Progress is tracked by clearing corresponding bits in global slavesSetZonesMap
// 
void setSlavesZones() {
static byte curSlave;
	//lprintf("Zones curSlave = %d\n", curSlave);
	if(!slavesSetZonesMap) 						// check if some board needs configuration
		return;								// all boards set
	if (waiting_for_reply)                  // system is busy
		return;       
	lprintf("slavesSetZonesMap = %2x\n", slavesSetZonesMap);
	curSlave = bitmap2slaveIdx(slavesSetZonesMap, MAX_SLAVES); 
	ErrWrite(ERR_DEBUG, "Sending zones config data to slave %d\n", curSlave);
	if(ERR_OK == sendSlaveZones(zonesDB[curSlave], curSlave))  // sendCmd handle and reports errors internally 
      ErrWrite( ERR_DEBUG, "Zones config sent to %d, receive timeout started\n",curSlave);
	else 
	  ErrWrite( ERR_INFO, "Zones config sent to %d failure\n", curSlave);
	curSlave++;
	if(curSlave > MAX_SLAVES)
		curSlave = SLAVE_ADDRESS1;
}	
//
// void setSlavesPgms() - sends pgms config data to all slaves in the slavesSetPgmsMap
// nonblocking function, shall be called periodically from loop() in order to check the results and move to next slave
// config data are read from global zonesDB, pgmsDB, etc. Progress is tracked by clearing corresponding bits in global slavesSetPgmsMap
// 
void setSlavesPgms() {
static byte curSlave;
	lprintf("PGMs curSlave = %d\n", curSlave);
	if(!slavesSetPgmsMap) 						// check if some board needs configuration
		return;								// all boards set
	if (waiting_for_reply)                  // system is busy
		return;       
	lprintf("slavesSetPgmsMap = %2x\n", slavesSetPgmsMap);
	curSlave = bitmap2slaveIdx(slavesSetPgmsMap, MAX_SLAVES); 
	ErrWrite(ERR_DEBUG, "Sending PGMs config data to slave %d\n", curSlave);
	if(ERR_OK == sendSlavePGMs(pgmsDB[curSlave], curSlave))  // sendCmd handle and reports errors internally 
      ErrWrite( ERR_DEBUG, "PGMs config sent to %d, receive timeout started\n",curSlave);
	else 
	  ErrWrite( ERR_INFO, "PGMs config sent to %d failure\n", curSlave);
	curSlave++;
	if(curSlave > MAX_SLAVES)
		curSlave = SLAVE_ADDRESS1;
}
#endif
//
#ifdef SLAVE
//---------------------  add to slaveProcess to support FREE CMD -----------------------------
    case FREE_CMD:
      ErrWrite(ERR_DEBUG, "Slave: received FREE_CMD cmd: \n");
      switch(msg.subCmd) {
        case FREE_TEXT_SUB_CMD:
          ErrWrite(ERR_DEBUG, "Slave: received for FREE_TEXT_SUB_CMD: ");
          lprintf("%s\n", msg.payload);
          break;
        case SET_ZONE_SUB_CMD:
          ErrWrite(ERR_DEBUG, "Slave: received SET_ZONE_SUB_CMD\n");
		      setAlarmZones(msg.payload);
          if(ERR_OK != returnSlaveZones(SzoneDB)) 							// reply with all zones info after set
            ErrWrite(ERR_TRM_MSG, "Slave: Error replying to SET_ZONE_SUB_CMD");
          break;
        case GET_ZONE_SUB_CMD:
          ErrWrite(ERR_DEBUG, "Slave: received GET_ZONE_SUB_CMD\n");
          if(ERR_OK != returnSlaveZones(SzoneDB))              // reply with requested zones info
            ErrWrite(ERR_TRM_MSG, "Slave: Error replying to GET_ZONE_SUB_CMD");
          break;
        case SET_PGM_SUB_CMD:
          ErrWrite(ERR_DEBUG, "Slave: received SET_PGM_SUB_CMD\n");
          setAlarmPgms(msg.payload);
          if(ERR_OK != returnSlavePGMs(SpgmDB))               // reply with all zones info after set
            ErrWrite(ERR_TRM_MSG, "Slave: Error replying to SET_ZONE_SUB_CMD");
          break;
		default:
		  ErrWrite(ERR_WARNING, "Slave: invalid sub-command received %x\n", msg.subCmd);
		  break;
// -----------------------------------------
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
		lprintf ("Zone get data: Zone GPIO:\tMUX:\tZoneID:\n");
		for (i = FREE_CMD_DATA_OFFSET; i <  j; i+=3)                                 // iterate
			lprintf ("%d %d %d   ", tmpMsg[i], tmpMsg[i+1], tmpMsg[i+2]);
		lprintf("\n");
		lprintf("returnSlaveZones data len: %d\n", j);
		}
		return SendMessage(SlaveMsgChannel, SlaveUART, (FREE_CMD | REPLY_OFFSET), MASTER_ADDRESS, slaveAdr, tmpMsg, j); // one byte payload only
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
  //if(DEBUG) 
    //printZones(SzoneDB, SLAVE_ZONES_CNT);
}
//
// replies to master on received setAlarmZones cmd
// params: int err - error code to be returned
// returns: see sendFreeCmd() for return codes
// uses global tmpMsg[]
// not used so far. Currently as reply all zones data are send - see returnSlaveZones
//
int replySetAlarmZones(int err) {
	tmpMsg[FREE_CMD_SUB_CMD_OFFSET] = SET_ZONE_SUB_CMD;       // prepare reply payload, first byte  is the subcommand we are replying to 
	tmpMsg[FREE_CMD_DATA_LEN_OFFSET]  = 1;                    // second byte is the payload len,  which is 1 byte
	tmpMsg[FREE_CMD_DATA_OFFSET]  = err;           			  // third is the aktual payload which in this case is no error (ERR_OK)	 
	//for(int i =0; i< MAX_MSG_LENGHT; i++)
	  //lprintf ("%2d ", tmpMsg[i]);             				
	//lprintf("\n");
	return SendMessage(SlaveMsgChannel, SlaveUART, (FREE_CMD | REPLY_OFFSET), MASTER_ADDRESS, slaveAdr, tmpMsg, FREE_CMD_HDR_LEN+1); // one byte payload only
}


//
// extract and stores the pgm params from received command in local pgm DB
// params: byte pldBuf[] - the payload of received SET_PGM_SUB_CMD 
//         payload is set of SLAVE_PGM_CNT triplets, each one containing (PGM GPIO, ID (rNum), iValue) 
// returns: none
// TODO: - add check for GPIO
//
void setAlarmPgms(byte pldBuf[]) {
  for(int i=0, j=0;j<SLAVE_PGM_CNT;j++)  {         // TODO - add check for GPIO
    memset((void*)&SpgmDB[j], 0, sizeof(struct PGM));
    SpgmDB[j].gpio = pldBuf[i++];
    SpgmDB[j].rNum  = pldBuf[i++];
    SpgmDB[j].iValue = pldBuf[i++];
    }
  if(DEBUG) 
	  printPGMs(SpgmDB, SLAVE_PGM_CNT);
}
//
// sends to master slave board pgm params - GPIO, rNum (ID), initial value (iValue)
// params: pgm[] - array of PGM structs, containing PGMs info 
//                  reply payload is set of SLAVE_PGM_CNT triplets, each one containing (PGM GPIO, rNum, iValue)
// returns: see sendFreeCmd() for return codes
// 
int returnSlavePGMs(struct PGM pgm[]) {
int j = FREE_CMD_DATA_OFFSET; int i = 0;						// index where to put the payload, spare some room for header
//															  	// use global tmpMsg
	tmpMsg[FREE_CMD_SUB_CMD_OFFSET] = GET_PGM_SUB_CMD;       	// prepare reply payload, first byte  is the subcommand we are replying to 
	for(i=0; (i<SLAVE_PGM_CNT) && (j<FREE_CMD_DATA_LEN); i++) { // extract current zone info from zone array
		tmpMsg[j++] = pgm[i].gpio;                             // and put in payload
		tmpMsg[j++] = pgm[i].rNum;
		tmpMsg[j++] = pgm[i].iValue;
		}
	tmpMsg[FREE_CMD_DATA_LEN_OFFSET]  = j-FREE_CMD_DATA_OFFSET;   // recalc actual paylaod len
	if(DEBUG) {                                                     // debug print
		lprintf ("PGM get data:  GPIO:\trNum:\tiValue:\n");
		for (i = FREE_CMD_DATA_OFFSET; i <  j; i+=3)                                 // iterate
			lprintf ("%d %d %d   ", tmpMsg[i], tmpMsg[i+1], tmpMsg[i+2]);
		lprintf("\n");
		lprintf("returnSlavePGMs data len: %d\n", j);
		}
		return SendMessage(SlaveMsgChannel, SlaveUART, (FREE_CMD | REPLY_OFFSET), MASTER_ADDRESS, slaveAdr,  tmpMsg, j); // one byte payload only
}
#endif

// from master.h
byte slavesSetZonesMap = 0xFF;			  // bitmap tracking if zones data are pending to  be send slaves: bit 0 cooresponds to slave 1, bit 1 oslave 2,
byte slavesSetPgmsMap = 0xFF;			  // bitmap tracking if pgms data are pending to  be send  to slaves: bit 0 cooresponds to slave 1, bit 1 oslave 2,

/*
//if(ERR_OK == sendFreeText(SLAVE_ADDRESS1, FREE_CMD_DATA_LEN, test_msg[(++i)%3])) // sendCmd handle and reports errors internally 
  //ErrWrite( ERR_INFO, ("Master MSG transmitted, receive timeout started\n"));
//if(ERR_OK == sendFreeText(SLAVE_ADDRESS1, FREE_CMD_DATA_LEN, test_msg[(++i)%3])) // sendCmd handle and reports errors internally 
  //ErrWrite( ERR_INFO, ("Master MSG transmitted, receive timeout started\n"));
//if(ERR_OK == setSlavePGMs(pgmsDB[SLAVE_ADDRESS1], SLAVE_ADDRESS1)); // sendCmd handle and reports errors internally 
  //ErrWrite( ERR_INFO, ("Master MSG transmitted, receive timeout started\n"));
//if(ERR_OK == setSlaveZones(zonesDB[SLAVE_ADDRESS1], SLAVE_ADDRESS1)); // sendCmd handle and reports errors internally 
  //ErrWrite( ERR_INFO, ("Master MSG transmitted, receive timeout started\n"));		

   master2slave  
    // chack if we have something to send in priority order
    //if(slavesSetZonesMap) {         // top priority - send configs to slaves, nothing we can do before this is done
    //    ErrWrite( ERR_INFO, ("Sending slaves configuration\n")); //Master process message will clear corresponding bits once confirmation reply
    //    setSlavesZones();           // slavesSetZonesMap holds bitmap of slaves to be set. Will do error reporting internally
    //    return;                               // inside wait4reply we will collect the answers from slaves and reflect in zones, pgms, etc DBs
    //    }                   // return as we have to wait for slave reply
    //if(slavesSetPgmsMap) {          // send new PGM states if any
    //  ErrWrite( ERR_INFO, ("Sending new PGMs states\n")); //Master process message will clear corresponding bits once confirmation reply
    //  setSlavesPgms();            // slavesSetPgms holds bitmap of slaves with new PGMs settings  and will do error reporting
    //  return;                               // inside wait4reply we will collect the answers from slaves and reflect in zones, pgms, etc DBs
    //  }                   // return as we have to wait for slave reply  
//
// waits for message reply up to timeout milliseconds of REPLY_TIMEOUT, whichever comes first
// returns: int ERR_OK -            if message processed 
//              error code -       otherwise
//
void waitReply() {
  int retCode;
  // are we waiting for reply? MSG_READY means good msg,ERR_OK(0) means no msg,<0 means UART error or parse err
  if (!waiting_for_reply)                        // check for message available
    t1.yield(&master);                              // not waiting for message
//    
  while (ERR_OK == (retCode = check4msg(MasterMsgChannel, MASTER_ADDRESS, REPLY_TIMEOUT))) {  // ERR_OK means nothing received so far, otherwise will be error or MSG_READY
    t1.delay();                                   // no message yet, yield all other processes
	lprintf("waitReply() delay\n");
    }											// end while
  if(retCode != MSG_READY)  {                   // we got something, either message or error             
    ErrWrite(ERR_WARNING, "Master rcv reply error or timeout\n");    // error   
    }
  else {
    masterProcessMsg(rcvMsg);                   // message, process it. 
    }     
  waiting_for_reply = 0;                      // stop waiting in case of error
  lprintf("Yielding to master loop\n");
  t1.yield(&master);     												
}
*/
