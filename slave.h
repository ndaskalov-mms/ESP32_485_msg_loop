
//
//
// ----------- slave simulation -------------------------------------------
//
//
void slave() {
    lprintf("Slave loop\n");
	memcpy(errorsDB_backup, errorsDB, sizeof(errorsDB_backup));   // backup error DB
	retCode = check4msg(SlaveMsgChannel, slaveAdr, NO_TIMEOUT);             // message if available will stored in global rcvMSG variable
	if(retCode == MSG_READY) {				                            // ERR_OK (0)- no message, ERR_RCV_MSG (<0) -parsing error, MSG_READY (1)- message present          
     slaveProcessCmd(rcvMsg);
	}		// if retCode
	else if(retCode != ERR_OK)                 
        ErrWrite(ERR_WARNING, "Slave rcv cmd err"); 
	// no message available for processing 
	// do something usefull like take a nap or read ADC and process the zones info
	// or collect some errors info to be send as status to MASTER some day
	convertZones(SzoneDB, SLAVE_ZONES_CNT, SzoneResult);
	
	// find out if some new errors occured while receiving/processing the message
	if(memcmp(errorsDB, errorsDB_backup, sizeof(errorsDB))) 
		printNewErrors();
}
