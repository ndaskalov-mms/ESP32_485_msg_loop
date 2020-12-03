/*
 * errors.h
 * contains all errors handling related staff
 * all possible errors have assigned error codes (enum errorID). There is static build errors database errorsDB, every record of
 * is made of struct ERROR, consisting errorID (used for look-up, shall be one of defined in enum errorID) and counter (errorCnt) 
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

enum errorID {
  ERR_OK = 0,                           // no error
  ERR_INV_PAYLD_LEN = 1,                    // send/rcv  message payload issue (too long or doesn't match message code payload size)
  ERR_TRM_MSG = 2,                          // something wrong happened when transmitting, the specific error shall be reported already at the point of contact
  ERR_RCV_MSG = 3,                          // something wrong happened when receiving, the specific error shall be reported already at the point of contact
  ERR_BAD_CMD = 4,                          // unknown command
  ERR_BAD_DST = 5,                          // unknown destination
  ERR_RS485_BUF_OVERFLOW = -1,                     // RS485 class receive buffer overflow
  ERR_RS485_INV_BYTE_CODE = -2,                    // RS485 byte encodding error detected
  ERR_RS485_BAD_CRC = -3,                          // RS485 crc error
  ERR_RS485_TIMEOUT = -4,                          // RS485 timeout waiting for ETX when STX is received
  ERR_RS485_FORCE_SCREW = -5 ,                     // RS485 intentionally generated for testing purposes
  ERR_RS485_NO_CALLBACK = -6,                      // RS485 has no read/write/available callback 
  ERR_RS485_DEBUG = -7,                            // used for debug prints
};

// Errors friendly names for UI
char  t_ERR_OK[] =         "";
char  t_ERR_INV_PAYLD_LEN[] = "Send/rcv  message payload issue (too long or doesn't match message code payload size)";
char  t_ERR_TRM_MSG[]       = "Message transmit error";
char  t_ERR_RCV_MSG[]     = "Message Receive error";  
char  t_ERR_BAD_CMD[]       = "Unknown command";
char  t_ERR_BAD_DST[]     = "Unknown destination";
char  t_ERR_RS485_BUF_OVERFLOW[] =  "RS485 class receive buffer overflow";
char  t_ERR_RS485_INV_BYTE_CODE[] = "RS485 byte encodding error detected";
char  t_ERR_RS485_BAD_CRC[] =       "RS485 crc error";
char  t_ERR_RS485_TIMEOUT[] =     "RS485 timeout waiting for ETX when STX is received";
char  t_ERR_RS485_FORCE_SCREW[] = "RS485 intentionally generated for testing purposes";
char  t_ERR_RS485_NO_CALLBACK[] =   "RS485 has no read/write/available callback";
char  t_ERR_RS485_DEBUG[] =         "Debug print:";

//
// errors storage for reporting purposes
//
//errors title ecord structure in errorsDBtitles
struct t_ERROR {
  int errorID;
  char * err_title;
};
// errors records structure for errorsDB
struct ERROR {
  int errorID;
  unsigned long errorCnt;
};
//
// Running errors are collected in errorsDB structure which keeps a count for each error type
// 
struct ERROR errorsDB[] = {{ERR_INV_PAYLD_LEN, 0}, {ERR_TRM_MSG, 0}, {ERR_RCV_MSG, 0}, {ERR_BAD_CMD, 0}, {ERR_BAD_DST, 0}, \
              {ERR_RS485_BUF_OVERFLOW, 0}, {ERR_RS485_INV_BYTE_CODE, 0}, {ERR_RS485_BAD_CRC, 0}, {ERR_RS485_TIMEOUT, 0},\
              {ERR_RS485_FORCE_SCREW, 0}, {ERR_RS485_NO_CALLBACK, 0}, {ERR_RS485_DEBUG, 0}} ;
// there is a backup cope of the DB, used to find a new errors after specific action (send/receive message)
// before the action errorDB is mem-copied to errorDB_backup and after using mem-compare we find if new errors occured
struct ERROR errorsDB_backup[] = {{ERR_INV_PAYLD_LEN, 0}, {ERR_TRM_MSG, 0}, {ERR_RCV_MSG, 0}, {ERR_BAD_CMD, 0}, {ERR_BAD_DST, 0}, \
              {ERR_RS485_BUF_OVERFLOW, 0}, {ERR_RS485_INV_BYTE_CODE, 0}, {ERR_RS485_BAD_CRC, 0}, {ERR_RS485_TIMEOUT, 0},\
              {ERR_RS485_FORCE_SCREW, 0}, {ERR_RS485_NO_CALLBACK, 0}, {ERR_RS485_DEBUG, 0}} ;
// print titles are kept separately in order to minimize copy/compare cycles
struct t_ERROR errorsDBtitles[] = {{ERR_INV_PAYLD_LEN, t_ERR_INV_PAYLD_LEN}, {ERR_TRM_MSG, t_ERR_TRM_MSG}, {ERR_RCV_MSG, t_ERR_RCV_MSG},\
                {ERR_BAD_CMD, t_ERR_BAD_CMD}, {ERR_BAD_DST, t_ERR_BAD_DST}, {ERR_RS485_BUF_OVERFLOW, t_ERR_RS485_BUF_OVERFLOW},\
                {ERR_RS485_INV_BYTE_CODE, t_ERR_RS485_INV_BYTE_CODE}, {ERR_RS485_BAD_CRC, t_ERR_RS485_BAD_CRC},\
                {ERR_RS485_TIMEOUT, t_ERR_RS485_TIMEOUT}, {ERR_RS485_FORCE_SCREW, t_ERR_RS485_FORCE_SCREW},\
                {ERR_RS485_NO_CALLBACK, t_ERR_RS485_NO_CALLBACK}, {ERR_RS485_DEBUG, t_ERR_RS485_DEBUG}} ;

int findErrorEntry(int err_code, struct ERROR errorsArray[]) {
  logger.printf("Looking for record for error code   %d \n", err_code);
  for (int i = 0; i < sizeof(errorsDB)/sizeof(struct ERROR); i++) {
    //logger.printf("Looking at index  %d out of  %d:\n", err_code, sizeof(errors)/sizeof(struct ERROR)-1);
    if(errorsArray[i].errorID == err_code) {
      logger.printf("Found error index %d\n", i);
      return  i;
    }
  }
  logger.printf("Error index not found!!!!!!!\n");
  return -1;
}

void printErrorsDB() {
  logger.printf("Printing errorsDB\n");
  for (int i = 0; i < sizeof(errorsDBtitles)/sizeof(struct t_ERROR); i++)     // print title first
        logger.printf("%ld\t- %s \n", errorsDB[i].errorCnt, errorsDBtitles[i].err_title);
}

void ErrWrite (int err_code, char* what)           // callback to dump info to serial console from inside RS485 library
{
  int index = 0;
  // update errors struct here
  switch (err_code)
  {
    //case  ERR_DEBUG:                            
    //  logger.print (*what);
    //  break;
    case ERR_OK:    
      logger.printf (what);
      break;                 
    case ERR_INV_PAYLD_LEN:
    case ERR_BAD_CMD:
    case ERR_TRM_MSG:
    case ERR_RCV_MSG:
    case  ERR_RS485_BUF_OVERFLOW:                     // RS485 class receive buffer overflow
    case  ERR_RS485_FORCE_SCREW:                      // RS485 intentionally generated for testing purposes
    case  ERR_RS485_INV_BYTE_CODE:                    // RS485 byte encodding error detected
    case  ERR_RS485_BAD_CRC:                          // RS485 crc error
    case  ERR_RS485_TIMEOUT:                          // RS485 timeout waiting for ETX when STX is received
    case ERR_RS485_NO_CALLBACK:                      
      index = findErrorEntry(err_code, errorsDB);
      errorsDB[index].errorCnt++;
      logger.printf (what);
      break;
    default:
      logger.printf ("Invalid error code %d received in errors handling callback callback", err_code);
      break;
  }

}

void ErrWrite (int err_code, char* formatStr, int arg)   {        // format str is printf-type one
        char tmpBuf[256];                         
        sprintf(tmpBuf,formatStr, arg);                           // finalyze the string according to format specs (printf type)
        ErrWrite(err_code, tmpBuf);                               // process the error
}
