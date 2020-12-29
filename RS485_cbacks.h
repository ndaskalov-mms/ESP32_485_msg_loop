/*
 Callbacks and helper functions for RS485 non-blocking library
 Licence: Released for public use.
 RS485 non-blocking requires callbacks for read, write, etc ops
 Here are the definitions for Master and Slave channes
 

*/
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
