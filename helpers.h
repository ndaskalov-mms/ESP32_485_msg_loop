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
#define MAX_PAYLOAD_SIZE  (MAX_MSG_LENGHT - CMD_SIZE)   // shall be max 112 = ((128-(1+1))/2) - 1
#define RxBUF_SIZE 128  //no material limit
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

#define BROADCAST_ID          0xF
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


// errors
struct ERRORS {
  unsigned long rs485 = 0;
  unsigned long protocol = 0;
} comm_errors;

// compose message containing:
// first byte:  upper 4 bits - command code 
//              lower 4 bits  - destination ID
// the rest:    payload, up to MAX_PAYLOAD_SIZE
// return: pointer to buffer with the composed message or NULL if error
byte * compose_msg(byte cmd, byte dest, byte *payload, byte *out_buf, int payload_len) {
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
    
  return out_buf;
}



size_t Master_Log_Write (char * what)
{return logger.println (what);}

size_t Slave_Log_Write (char * what)
{return logger.println (what);}

size_t Master_Write (const byte what)
{return MasterUART.write (what);}

int Master_Available ()
{return MasterUART.available();}
 
int Master_Read ()
{return MasterUART.read();}
 
// flush transmitter only 
void Master_TxFlush(){
  while(MasterUART.availableForWrite()!=127) ;  // availableForWrite returns 0x7f - uart->dev->status.txfifo_cnt;
  delay(5);
}

// flush receiver only 
void Master_RxFlush(){
  while(Master_Available())
	Master_Read ();
}

// flush both
void Master_Flush()
{MasterUART.flush();}

void Master_485_receive_mode(){
  // change line dir
  delay(1);
  //Master_RxFlush();
}

void Master_485_transmit_mode(){
    // change line dir
  delay(1);
  //Master_TxFlush();
}

//---------- callbacks for slave UART channel

size_t Slave_Write (const byte what)
{return SlaveUART.write (what);}

int Slave_Available ()
{return SlaveUART.available();}
 
int Slave_Read ()
{return SlaveUART.read();}
 
// flush transmitter only 
void Slave_TxFlush(){
  while(SlaveUART.availableForWrite()!=127) ;
  delay(5);
}

// flush receiver only 
void Slave_RxFlush(){
  while(Slave_Available())   // TODO add error handling
	Slave_Read ();
}

// flush both
void Slave_Flush()
{SlaveUART.flush();}

void Slave_485_receive_mode(){
  // change line dir
  delay(1);
  Slave_RxFlush();
}

void Slave_485_transmit_mode(){
    // change line dir
  delay(1);
  Slave_TxFlush();
}
