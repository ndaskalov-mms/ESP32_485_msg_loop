/*
 Callbacks and helper functions for RS485 non-blocking library
 Licence: Released for public use.
 RS485 non-blocking requires callbacks for read, write, etc ops
 Hear are the definitions for Master and Slave channes
 

*/
//HardwareSerial& logger(Serial);
//HardwareSerial& MasterUART(Serial1);
//HardwareSerial& SlaveUART(Serial2);

size_t Master_Write (const byte what)
{
  return MasterUART.write (what);  
}

int Master_Available ()
{
  return MasterUART.available();
}
 
int Master_Read ()
{
  return MasterUART.read();
}
 
void Master_TxFlush()
{
    MasterUART.flush();
}

void Master_RxFlush()
{
  while(Master_Available())
  Master_Read ();
}

void Master_485_receive_mode()
{
  // change line dir
  delay(1);
  Master_RxFlush();
}

void Master_485_transmit_mode()
{
    // change line dir
  delay(1);
  Master_TxFlush();
}

//---------- callbacks for slave UART channel

size_t Slave_Write (const byte what)
{
  return SlaveUART.write (what);  
}

int Slave_Available ()
{
  return SlaveUART.available();
}
 
int Slave_Read ()
{
  return SlaveUART.read();
}
 
void Slave_TxFlush()
{
  SlaveUART.flush();

}

void Slave_RxFlush()
{
  while(Slave_Available())
  Slave_Read ();
}

void 
Slave_485_receive_mode()
{
  // change line dir
  delay(1);
  Slave_RxFlush();
}

void Slave_485_transmit_mode()
{
    // change line dir
  delay(1);
  Slave_TxFlush();
}

