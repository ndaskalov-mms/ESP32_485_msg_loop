/*
 Callbacks and helper functions for RS485 non-blocking library
 Licence: Released for public use.
 RS485 non-blocking requires callbacks for read, write, etc ops
 Hear are the definitions for Master and Slave channes
 

*/
//HardwareSerial& logger(Serial);
//HardwareSerial& MasterUART(Serial1);
//HardwareSerial& SlaveUART(Serial2);

#define REPLY_OFFSET  0x80
// COMMAND CODES
// commands definition
// high nibble contains command code, while low nibble contains the recipient ID
// Master ID is always 0x0, while 0xF is reserved for broadcast message (not used so far)
enum command_codes{
      PING = 0x0,             // ping 
      POLL_ZONES = 0x1,       // poll the extenders for zones status
      SET_OUTS = 0x2,         // set output relay
      FREE_TEXT = 0x3,        // send free text (can be binary too)
};  // end of enum

// RESULT CODES
// the result code is simply the command code with MS bit set:  REPLY_OFFSET = 0x80
enum result_codes{
      PING_RES = (PING | REPLY_OFFSET),     // ping 
      POLL_ZONES_RES = (POLL_ZONES | REPLY_OFFSET),      // poll the extenders for zones status
      SET_OUTS_RES = (SET_OUTS  | REPLY_OFFSET),         // set output relay
      FREE_TEXT_RES = (FREE_TEXT  | REPLY_OFFSET),
};  // end of enum

// compose message containing:
// first byte:  upper 4 bits - command code 
//              lower 4 bits  - destination ID
// the rest:    payload, up to MAX_PAYLOAD_SIZE
// return: pointer to buffer with the composed message or NULL if error
byte * compose_msg(byte cmd, byte dest, byte *payload, byte *out_buf, int payload_len) {
  int index = 0;
  // first byte is COMMMAND/REPLY code combined with the destination address 
  out_buf[index++] = ((cmd << 4) | (dest & 0x0F));
  // next comes the payload
  if ((payload_len + index) > MAX_MSG_LENGHT) {
    logger.printf("Payload size %d is larger than buffer size %d", payload_len, MAX_MSG_LENGHT);
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
