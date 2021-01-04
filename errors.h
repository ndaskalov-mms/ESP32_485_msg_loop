/*
 * errors.h
 * contains all errors handling related staff
 * all possible errors have assigned error codes (enum errorID). There is static build errors database errorsDB, every record of
 * is made of struct ERROR_REC, consisting errorID (used for look-up, shall be one of defined in enum errorID) and counter (errorCnt) 
 * for this error. Copy of errorsDB is contained in errorsDB_backup.
 * before the some actions or predefined interval errorDB is mem-copied to errorDB_backup and after using mem-compare we find 
 * if new errors occured.
 * Print-friendly errors names are stored in char arrays named with errorID prefixed by t_ . Print names are 
 * organized in t_ERROR errorsDBtitles[], where each entry is struct t_ERROR, containing errorID (again for look-up) 
 * and pointer to err_title;
 * at run time, when error occure, ErrWrite callback function is called, which increases the number of errors for particular error stored 
 * in errorsDB and optionally prints the description of the error supplied by the caller to console. 
 * to find out the proper record for pardicular errorID findErrorEntry() is used, which is doing exhaustive search of the errorsDB 
 * searching a match of supplied error number and errorID stored in the errorsDB records
*/

int findErrorEntry(int err_code, struct ERROR_REC errorsArray[]);

enum errorID {
  ERR_INFO = 2,                           // just print if INFO is set
  MSG_READY = 1,                          // message received
  ERR_OK = 0,                           	// no error
  ERR_DEBUG = -1, 							          // debug print, can be enabled/disable by #define DEBUG
  ERR_WARNING = -2, 						          // warning print  can be enabled/disable by #define WARNING
  ERR_INV_PAYLD_LEN = -3,                   // send/rcv  message payload issue (too long or doesn't match message code payload size)
  ERR_TRM_MSG = -4,                         // something wrong happened when transmitting, the specific error shall be reported already at the point of contact
  ERR_RCV_MSG = -5,                         // something wrong happened when receiving, the specific error shall be reported already at the point of contact
  ERR_BAD_CMD = -6,                         // unknown command
  ERR_BAD_DST = -7,                         // unknown destination
  ERR_TIMEOUT = -8,                         // send/recv message timeout at calling function
  ERR_DB_INDEX_NOT_FND = -9,				// cannot find error in the database
  ERR_RS485_BUF_OVERFLOW = -10,            	// RS485 class receive buffer overflow
  ERR_RS485_INV_BYTE_CODE = -11,            // RS485 byte encodding error detected
  ERR_RS485_BAD_CRC = -12,                  // RS485 crc error
  ERR_RS485_TIMEOUT = -13,                  // RS485 timeout waiting for ETX when STX is received
  ERR_RS485_FORCE_SCREW = -14 ,             // RS485 intentionally generated for testing purposes
  ERR_RS485_NO_CALLBACK = -15,              // RS485 has no read/write/available callback 
  };

// Errors friendly names for UI
char  t_ERR_OK[] 					      = "";	      // print w/o title, just the string supplied by caller
char  t_ERR_DEBUG[] 				    = "";	      // print w/o title, just the string supplied by calle
char  t_ERR_WARNING[] 				  = "";	      // print w/o title, just the string supplied by calle
char  t_ERR_INV_PAYLD_LEN[] 		= "Send/rcv  message payload issue (too long or doesn't match message code payload size)";
char  t_ERR_TRM_MSG[]       		= "Message transmit error";
char  t_ERR_RCV_MSG[]     			= "Message Receive error";  
char  t_ERR_BAD_CMD[]       		= "Unknown command";
char  t_ERR_BAD_DST[]     			= "Unknown destination";
char  t_ERR_TIMEOUT[]     			= "Send/recv message timeout at calling function";
char  t_ERR_DB_INDEX_NOT_FND[]     		= "Cannot find error ID in errorsDB";
char  t_ERR_RS485_BUF_OVERFLOW[] 	= "RS485 class receive buffer overflow";
char  t_ERR_RS485_INV_BYTE_CODE[] 	= "RS485 byte encodding error detected";
char  t_ERR_RS485_BAD_CRC[] 		= "RS485 crc error";
char  t_ERR_RS485_TIMEOUT[] 		= "RS485 timeout waiting for ETX when STX is received";
char  t_ERR_RS485_FORCE_SCREW[] 	= "RS485 intentionally generated for testing purposes";
char  t_ERR_RS485_NO_CALLBACK[] 	= "RS485 has no read/write/available callback";

//
// errors storage for reporting purposes
//
//errors title ecord structure in errorsDBtitles
struct t_ERROR {
  int errorID;
  char * err_title;
};
// errors records structure for errorsDB
struct ERROR_REC {
  int errorID;
  unsigned long errorCnt;
};
//
// Running errors are collected in errorsDB structure which keeps a count for each error type
// 
struct ERROR_REC errorsDB[] = {{ERR_INV_PAYLD_LEN, 0}, {ERR_TRM_MSG, 0}, {ERR_RCV_MSG, 0}, {ERR_BAD_CMD, 0}, {ERR_BAD_DST, 0}, \
              {ERR_RS485_BUF_OVERFLOW, 0}, {ERR_RS485_INV_BYTE_CODE, 0}, {ERR_RS485_BAD_CRC, 0}, {ERR_RS485_TIMEOUT, 0},\
              {ERR_RS485_FORCE_SCREW, 0}, {ERR_RS485_NO_CALLBACK, 0}, {ERR_TIMEOUT, 0}, {ERR_DB_INDEX_NOT_FND, 0}} ;
// there is a backup cope of the DB, used to find a new errors after specific action (send/receive message)
// before the action errorDB is mem-copied to errorDB_backup and after using mem-compare we find if new errors occured
// print titles are kept separately in order to minimize copy/compare cycles
//
struct ERROR_REC errorsDB_backup[sizeof(errorsDB)/sizeof(struct ERROR_REC)];  // temp DB backup in order to find the new errors for each loop()
//
struct t_ERROR errorsDBtitles[sizeof(errorsDB)/sizeof(struct ERROR_REC)] = {{ERR_INV_PAYLD_LEN, t_ERR_INV_PAYLD_LEN}, {ERR_TRM_MSG, t_ERR_TRM_MSG}, {ERR_RCV_MSG, t_ERR_RCV_MSG},\
                {ERR_BAD_CMD, t_ERR_BAD_CMD}, {ERR_BAD_DST, t_ERR_BAD_DST}, {ERR_RS485_BUF_OVERFLOW, t_ERR_RS485_BUF_OVERFLOW},\
                {ERR_RS485_INV_BYTE_CODE, t_ERR_RS485_INV_BYTE_CODE}, {ERR_RS485_BAD_CRC, t_ERR_RS485_BAD_CRC},\
                {ERR_RS485_TIMEOUT, t_ERR_RS485_TIMEOUT}, {ERR_RS485_FORCE_SCREW, t_ERR_RS485_FORCE_SCREW},\
                {ERR_RS485_NO_CALLBACK, t_ERR_RS485_NO_CALLBACK}, {ERR_TIMEOUT, t_ERR_TIMEOUT}, {ERR_DB_INDEX_NOT_FND, t_ERR_DB_INDEX_NOT_FND}} ;


int ErrWrite (int err_code, char * what)           // callback to dump info to serial console from inside RS485 library
{
  int index = 0;
  //logger.printf ("error code %d received in errors handling callback\n------------------------", err_code);
  // update errors struct here
  switch (err_code)
  {
    //case  ERR_DEBUG:                            
    //  logger.print (*what);
    //  break;
    case ERR_OK:    
      if(SERIAL_LOG)
		  logger.printf (what);
	  if(MQTT_LOG)
		  ReportUpstream(LOG_TOPIC, what);
      break;    
  case ERR_DEBUG:    
      if(DEBUG) {
		  if(SERIAL_LOG)
			logger.printf (what);
		  if(MQTT_LOG)
			ReportUpstream(DEBUG_TOPIC, what);
	      }
      break;
  case ERR_INFO:    
	  if(INFO) {
		  if(SERIAL_LOG)
			logger.printf (what);
		  if(MQTT_LOG)
			ReportUpstream(INFO_TOPIC, what);
		  }
      break;
  case ERR_WARNING:    
      if(WARNING) {
		  if(SERIAL_LOG)
			logger.printf (what);
		  if(MQTT_LOG)
			ReportUpstream(WARNING_TOPIC, what);
	      }
      break; 
	case ERR_INV_PAYLD_LEN:
	case ERR_BAD_CMD:
	case ERR_TRM_MSG:
	case ERR_RCV_MSG:
	case ERR_RS485_BUF_OVERFLOW:                     // RS485 class receive buffer overflow
	case ERR_RS485_FORCE_SCREW:                      // RS485 intentionally generated for testing purposes
	case ERR_RS485_INV_BYTE_CODE:                    // RS485 byte encodding error detected
	case ERR_RS485_BAD_CRC:                          // RS485 crc error
	case ERR_RS485_TIMEOUT:                          // RS485 timeout waiting for ETX when STX is received
	case ERR_RS485_NO_CALLBACK:    
	case ERR_TIMEOUT:
	case ERR_DB_INDEX_NOT_FND:                       // TODO risk of endless loop  ???
    index = findErrorEntry(err_code, errorsDB);
    errorsDB[index].errorCnt++;
	  if(ERROR) {
		  if(SERIAL_LOG)
			  logger.printf (what);
		  if(MQTT_LOG)
			  ReportUpstream(ERROR_TOPIC, what);
      }
    break;
   default:
    if(SERIAL_LOG) {
		  logger.printf ("-----------------Invalid error code %d received in errors handling callback\n------------------------", (int)err_code);
  		if(what)
            logger.printf("%s\n", what);
  	  }
      if(MQTT_LOG) {
        char tmpBuf[256];                         
        sprintf(tmpBuf,"Invalid err code %d for CMD %d rcvd in err handling callback\n", err_code, waiting_for_reply);                           
        ReportUpstream(ERROR_TOPIC,tmpBuf);
        if(what)
          ReportUpstream(ERROR_TOPIC, what);
	      }
	  break;
  }
  return err_code;
}
int ErrWrite (int err_code, char* formatStr, int arg)   {        // format str is printf-type one
        char tmpBuf[256];                         
        sprintf(tmpBuf,formatStr, arg);                           // finalyze the string according to format specs (printf type)
        return ErrWrite(err_code, tmpBuf);                               // process the error
}

int findErrorEntry(int err_code, struct ERROR_REC errorsArray[]) {
  //ErrWrite(ERR_DEBUG, "Looking for record for error code   %d \n", err_code);
  for (int i = 0; i < sizeof(errorsDB)/sizeof(struct ERROR_REC); i++) {
    //logger.printf("Looking at index  %d out of  %d:\n", err_code, sizeof(errorsDB)/sizeof(struct ERROR_REC)-1);
    if(errorsArray[i].errorID == err_code) {
      //ErrWrite(ERR_DEBUG,"Found error index %d\n", i);
      return  i;
    }
  }
  ErrWrite(ERR_INFO, "Error index not found!!!!!!!\n");
  return ERR_DB_INDEX_NOT_FND;
}

void printErrorsDB() {
  ErrWrite(ERR_DEBUG,"Printing errorsDB\n");
  for (int i = 0; i < sizeof(errorsDBtitles)/sizeof(struct t_ERROR); i++)     // print title first
        logger.printf("ErrorID = %01x hex %d Dec; count = %ld; title = %s \n", errorsDB[i].errorID, errorsDB[i].errorID, errorsDB[i].errorCnt, errorsDBtitles[i].err_title);
}


void printNewErrors() {
  ErrWrite(ERR_DEBUG,"Printing new errors \n");
  for (int i = 0; i < sizeof(errorsDBtitles)/sizeof(struct t_ERROR); i++)  {   // print title first
        if(errorsDB[i].errorCnt != errorsDB_backup[i].errorCnt)
          logger.printf("%ld\t- %s \n", errorsDB[i].errorCnt, errorsDBtitles[i].err_title);     
  }
}
