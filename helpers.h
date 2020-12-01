/*
 Callbacks and helper functions for RS485 non-blocking library
 Licence: Released for public use.
 RS485 non-blocking requires callbacks for read, write, etc ops
 Hear are the definitions for Master and Slave channes
 

*/
//HardwareSerial& logger(Serial);
//HardwareSerial& MasterUART(Serial1);
//HardwareSerial& SlaveUART(Serial2);

#define HEADER_SIZE 1   //STX, 1 byte, no encoding
#define FOOTER_SIZE 1   //ETX, 1 byte  no encoding
#define CRC_SIZE    2   //CRC, 1 byte but due to encoding is send as 2 bytes
#define CMD_SIZE    2   //COMMAND/REPLY + DESTINATION, 1 byte but due to encoding is send as 2 bytes
#define PAYLOAD_OFFSET  (CMD_SIZE/2) // payload offset in the received buffer 
#define TxFIFO_SIZE 128 //ESP32 TxFIFO, we expect that whole message get's in FIFO in order not to block loop()
#define MAX_MSG_LENGHT  ((TxFIFO_SIZE - (HEADER_SIZE + FOOTER_SIZE + CRC_SIZE))/2)  //message encodding doubles every nibble except STX and ETX
#define MAX_PAYLOAD_SIZE  (MAX_MSG_LENGHT - CMD_SIZE/2)   // shall be max 61 = ((128-(1+1+1))/2) - 1
#define RxBUF_SIZE 128  //no material limit
#define BROADCAST_ID    0xF
#define MASTER_ADDRESS  0
#define SLAVE1_ADDRESS  1
#define SLAVE2_ADDRESS  2
#define REPLY_OFFSET    0x8

// COMMAND CODES
// commands definition
// high nibble contains command code, while low nibble contains the recipient ID
// Master ID is always 0x0, while 0xF is reserved for broadcast message (not used so far)
// RESULT CODES - the result code is simply the command code with MS bit set:  REPLY_OFFSET = 0x80
// ----------------- PING -----------------------------------
#define PING                  0x0                  // ping 
#define PING_PAYLD_LEN        0                    // ping message has no payload
#define PING_RES              (PING|REPLY_OFFSET)  // ping reply code 
#define PING_RES_PAYLD_LEN    4                    // ping payload is 4 bytes, content not defined yet

// ----------------- POLL -----------------------------------
#define POLL_ZONES            0x1 // poll the extenders for zones status
#define POLL_PAYLD_LEN        0   // poll pmessage has no payload
#define POLL_ZONES_RES        (POLL_ZONES|REPLY_OFFSET) // poll the extenders for zones status
#define POLL_RES_PAYLD_LEN    6                         // poll payload is 6 bytes, 4 bytes zones (32 zones, 1 bit per zone, LSB is zone 1,
                                                        // 1 byte for outputs (8 outputs, LSB is OUT1) and 1 byte status

// ----------------- SET OUTPUTS -----------------------------
#define SET_OUTS              0x2                       // set output relay
#define SET_OUTS_PAYLD_LEN    1                         // SET_OUTS payload is 1 byte, 1 byte for outputs (8 outputs, LSB is OUT1)
#define SET_OUTS_RES          (SET_OUTS|REPLY_OFFSET)   // set output relay reply
#define SET_OUTS_RES_PAYLD_LEN 2                        // SET_OUTS payload is 2 bytes 1 byte for outputs (8 outputs, LSB is OUT1) and 1 byte status

// ----------------- FREE_TEXT -----------------------------
#define FREE_TEXT             0x3                        // send free text (can be binary too)
#define FREE_TEXT_PAYLD_LEN    MAX_PAYLOAD_SIZE          // FREE_TEXT payload is up to MAX_PAYLOAD_SIZE
#define FREE_TEXT_RES         (FREE_TEXT  | REPLY_OFFSET)
#define FREE_TEXT_RES_PAYLD_LEN MAX_PAYLOAD_SIZE         // FREE_TEXT_RES payload is up to MAX_PAYLOAD_SIZE

#define RS485_DATA_PRESENT    1         // RS485.update returns 0 (ERR_OK) if no data, 1 (RS485_DATA_PRESENT) if data avail or negative if error
enum errorID {
  ERR_OK = 0,                           // no error
  ERR_RS485,                            // something wrong happened while sending/ receiving  message in RS485 class
  ERR_INV_PAYLD_LEN,                    // send/rcv  message payload issue (too long or doesn't match message code payload size)
  ERR_BAD_CMD,                          // unknown command
  ERR_BAD_DST,                          // unknown destination
  ERR_BUF_OVERFLOW = -1,                     // RS485 class receive buffer overflow
  ERR_INV_BYTE_CODE = -2,                    // RS485 byte encodding error detected
  ERR_BAD_CRC = -3,                          // RS485 crc error
  ERR_TIMEOUT = -4,                          // RS485 timeout waiting for ETX when STX is received
  ERR_FORCE_SCREW = -5 ,                     // RS485 intentionally generated for testing purposes
  ERR_NO_CALLBACK = -6,                      // RS485 has no read/write/available callback 
  ERR_DEBUG = -7,                            // used for debug prints
};
//
// errors storage for reporting purposes
//
struct ERRORS {
  unsigned long rs485_send = 0;         // something wrong happened while sending message in RS485 class
  unsigned long rs485_recv = 0;         // something wrong happened while receiving message in RS485 class
  unsigned long send_payload = 0;       // send message payload issue (too long?)
  unsigned long rcv_payload = 0;        // received message payload issue (doesn't match message code payload size)
  unsigned long bad_cmd = 0;            // unknown command
  unsigned long bad_dst = 0;            // unknown destination
// below erors are generated from RS485 class (lib)  
  unsigned long rs485_buf_overflow = 0; // receive buffer overflow
  unsigned long rs485_force_screw = 0;  // intentionally generated for testing purposes
  unsigned long rs485_inv_byte_code = 0;// byte encodding error detected
  unsigned long rs485_bad_crc = 0;      // crc error
  unsigned long rs485_timeout = 0;      // timeout waiting for ETX when STX is received
} errors;

struct MSG {
  byte cmd;
  byte dst;
  int  len;
  byte payload[MAX_PAYLOAD_SIZE];
  byte parse_err;
} ;


// callbacks for RS485 library interface to onboard UARTS

size_t MasterWrite (const byte what)   // callback to write byte to UART
{return MasterUART.write (what);}
int MasterAvailable ()                 // callback to check if something received
{return MasterUART.available();}
int MasterRead ()                      // callback to read received bytes
{return MasterUART.read();}
size_t SlaveWrite (const byte what)    // callback to write byte to UART
{return SlaveUART.write (what);}
int SlaveAvailable ()                  // callback to check if something received
{return SlaveUART.available();}
int SlaveRead ()                       // callback to read received bytes
{return SlaveUART.read();}              

ErrWrite(ERR_INV_PAYLD_LEN, "Error composing message -  too long???",rmsg.len );   

/*
void LogMsg(char *formatStr, int len, int cmd_dst, byte *payload) {
    logger.printf(formatStr, len, payload[0]);
    logger.write (&payload[1], len-1);
    logger.println();
}
*/
void LogMsg(char *formatStr, int len, int cmd_dst, byte *payload) {
    logger.printf(formatStr, len, payload[0]);
    logger.write (&payload[1], len-1);
    logger.println();
}

void LogMsg(char *formatStr, int len, int cmd, int dst, byte *payload) {
    logger.printf(formatStr, len, cmd, dst, payload[0]);
    logger.write (&payload[1], len-1);
    logger.println();
}

void ErrWrite (int err_code, char* what)           // callback to dump info to serial console from inside RS485 library
{
  // update errors struct here
  switch (err_code)
  {
    //case  ERR_DEBUG:                            // RS485 class receive buffer overflow
    //  logger.print (*what);
    //  break;
    case  ERR_OK:                               // RS485 class receive buffer overflow
      logger.println (what);
      break;
    case  ERR_BUF_OVERFLOW:                      // RS485 class receive buffer overflow
      logger.printf (what);
      break;
    case  ERR_FORCE_SCREW:                      // RS485 intentionally generated for testing purposes
      logger.printf (what);
      break;
    case  ERR_INV_BYTE_CODE:                    // RS485 byte encodding error detected
      logger.printf (what);
      break;
    case  ERR_BAD_CRC:                          // RS485 crc error
      logger.printf (what);
      break;
    case  ERR_TIMEOUT:                          // RS485 timeout waiting for ETX when STX is received
      logger.printf (what);
      break;

      //ERR_NO_CALLBACK
      //ERR_RS485
      //ERR_INV_PAYLD_LEN, 
      //
    default:
      logger.printf ("Invalid error code %d received in errors handling callback callback", err_code);
  }
}

void ErrWrite (int err_code, char* formatStr, int len)   {        // format str is printf-type one
        char tmpBuf[256];                         
        sprintf(tmpBuf,formatStr, rmsg.len);                      // finalyze the string according to format specs (printf type)
        ErrWrite(err_code, tmpBuf)                                // process the error
}
 
// flush transmitter only 
void uartTxFlush(HardwareSerial& uart){
  while(uart.availableForWrite()!=127) ;  // availableForWrite returns 0x7f - uart->dev->status.txfifo_cnt;
  delay(5);
  }
// flush receiver only 
void uartRxFlush(HardwareSerial& uart){
  while(uart.available())
	uart.read();
  }
// flush both
void uartFlush(HardwareSerial& uart) {
  uart.flush();
  }
// switch direction of the RS485 driver
void uartRcvMode(HardwareSerial& uart){
  // change line dir
  delay(1);
  //uartRxFlush(uart);                    // read any garbage coming from switching dir
  }
void uartTrmMode(HardwareSerial& uart){
  // change line dir
  delay(1);
  //uartTxFlush(uart);                    // ???
  }
