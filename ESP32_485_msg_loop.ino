#include "Alarm_RS485.h"

constexpr int BITRATE = 115200;
constexpr int LOG_BITRATE = 115200;

#define MASTER
#define SLAVE

#define HEADER_SIZE 1   //STX, 1 byte
#define FOOTER_SIZE 1   //ETX, 1 byte
#define TxFIFO_SIZE 128 //ESP32 TxFIFO, we expect that whole message get's in FIFO in order not to block loop()
#define MAX_MSG_LENGHT  ((TxFIFO_SIZE - (HEADER_SIZE + FOOTER_SIZE))/2)  //message encodding doubles every nibble except STX and ETX
#define CMD_SIZE  1     // 1 byte command lenght
#define MAX_PAYLOAD_SIZE  (MAX_MSG_LENGHT - CMD_SIZE)   // shall be max 112 = ((128-(1+1))/2) - 1
#define RxBUF_SIZE 128  //no material limit

HardwareSerial& logger(Serial);
HardwareSerial& MasterUART(Serial1);
HardwareSerial& SlaveUART(Serial2);

#include "helpers.h"      // include helper functions. INCLUDE ONLY AFTER SERIAL PORT DEFINITIONS!!!!


//this is channel to send/receive packets over serial if. The comm to serial is via fRead, fWrite,...
RS485 MasterMsgChannel (Master_Read, Master_Available, Master_Write, Master_Log_Write, RxBUF_SIZE);   //RS485 myChannel (read_func, available_func, write_func, msg_len);
RS485 SlaveMsgChannel (Slave_Read, Slave_Available, Slave_Write, Slave_Log_Write, RxBUF_SIZE);   //RS485 myChannel (read_func, available_func, write_func, msg_len);

void setup() {
  logger.begin(LOG_BITRATE,SERIAL_8N1);
  // set UARTs
  MasterUART.begin(BITRATE,SERIAL_8N1, 21, 22);  // re-routing RxD to  GPIO21 and TxD to GPIO22
  SlaveUART.begin(BITRATE,SERIAL_8N1);
  // allocate data buffers and init message encoding/decoding engines (485_non_blocking library)
  MasterMsgChannel.begin ();      
  SlaveMsgChannel.begin ();  
  logger.println("Loopback example for Esp32+485");
}


byte msg [TxFIFO_SIZE] = "Hello world";
#define TRM_INTERVAL  1000  //1 sec
#define REPLY_TIMEOUT  500  //200 msec

void loop ()
{
   // ---------------- transmitter ------------------------------
  byte inBuf[RxBUF_SIZE];
  const byte outBuf[TxFIFO_SIZE]= "hello world";
  static unsigned long last_transmission = 0;
  static int waiting_for_reply = 0;
  static unsigned long master_err = 0;
  static unsigned long slave_err = 0;


  
  // ----------- master simulation -------------------------------------------
  //logger.println("Transmitter:");

  if (waiting_for_reply)
  {
    if (MasterMsgChannel.update ())
    {
      // msg received
      logger.print ("Master received message: ");
      logger.write (MasterMsgChannel.getData (), MasterMsgChannel.getLength ()); 
      logger.println ();
      waiting_for_reply = 0;
      Master_485_transmit_mode();  // seems redundant
      // process message
    }
    else if((unsigned long)(millis() - last_transmission) > REPLY_TIMEOUT) {
        // reply not received
        Master_485_transmit_mode();
        waiting_for_reply = 0;
        // TODO - signal error somehow
        logger.println ("Master reply timeout");
    }
    //logger.print( "!" );
  }     // if (waiting_for_reply)

  else if( (unsigned long)(millis() - last_transmission) > TRM_INTERVAL){  // check if it is time for the next comm
    logger.println( "\nMaster:  Time to transmit -------------------------------" );
    Master_485_transmit_mode();
    MasterMsgChannel.sendMsg (msg, sizeof (msg));
    last_transmission = millis();    // mark the transmit time so we can calculate the time for the next transmission and check for reply timeout
    // TODO for same channel loopback use Master_TxFlush only, otherwise use Master_Flush()
    Master_Flush ();                     // make sure the data are transmitted properly befor swithching the line direction
    Master_485_receive_mode();
    Master_Flush();         // flushes both Tx and Rx
    logger.println("Master MSG transmitted, receive timeout started");
    waiting_for_reply = 1;
  }
  else {
    //logger.print( "." );
  }
  if(master_err != MasterMsgChannel.getErrorCount()) {
      master_err = MasterMsgChannel.getErrorCount();
      logger.print("Master errors cnt now:");
      logger.println(master_err, DEC);
  }  
  // ----------- slave simulation -------------------------------------------
  // ---------------- receiver ------------------------------
  if (SlaveMsgChannel.update ())
  {
    logger.print ("\nSlave message received: ");
	  int len = SlaveMsgChannel.getLength ();
    memcpy (inBuf, SlaveMsgChannel.getData (), len); 
    logger.write ((const char *)inBuf); 
    //logger.println ();
	  for (int i=0; i < len; i++)
		inBuf[i] = toupper(inBuf[i]);
    // Process message and send reply
    logger.println( "\nSlave:  Sending reply -------------------------------" );
    Slave_485_transmit_mode();
    SlaveMsgChannel.sendMsg (inBuf, len);
    Slave_Flush ();                     // make sure the data are transmitted properly befor swithching the line direction
    Slave_485_receive_mode();
    Slave_Flush();         // flushes both Tx and Rx
    logger.println("Slave reply transmitted, going back to listening mode");
  }
  //else 
    //logger.print ("~");

  if(slave_err != SlaveMsgChannel.getErrorCount()) {
    slave_err = SlaveMsgChannel.getErrorCount();
    logger.print("Slave errors cnt:");
    logger.println(slave_err, DEC);
  }
}  // end of loop

/*  
    char inBuf[BLOCKSIZE];
    char outBuf[BLOCKSIZE]= "hello world";
    //logger.println("Transmitting:");

    //serialIUT.flush();
    //serialIUT.write(outBuf, 12);
    //void RS485::sendMsg (const byte * data, const byte length)
    RS485.sendMsg (outBuf, 12);


    
    logger.println("waiting for receive");
    delay (1000);
    
    int avail = serialIUT.available();
    
    for (int i = 0; i <= avail; ++i)
    {
        unsigned char r;
        r = serialIUT.read();
        if(r)
           inBuf[i] = r;
        inBuf[i+1] = 0;

    }
    
    logger.println(inBuf);
}
*/


/*
 * 
 
  //unsigned long mill = 0;
  //unsigned long diff = 0;
 diff = (unsigned long)(millis() - time_now);
  //sprintf( outBuf, "diff=%04X\t%ld", diff, diff);
  //Serial.println( outBuf );
  //sprintf( outBuf, "trm_now=%04X\t%ld", trm_now, trm_now );
  //Serial.println( outBuf );
  //mill = millis();
  //sprintf( outBuf, "millis=%04X\t%ld", mill, mill );
  //Serial.println( outBuf );
  //sprintf( outBuf, "diff=%04X\t%ld", ((unsigned long)(millis() - time_now)), (unsigned long)(millis() - time_now) );

  if( diff > trm_intvl){  // check if it is time for the next comm
    Serial.println( "\nTime to transmit -------------------------------" );
    msgChannel.sendMsg (msg, sizeof (msg));
    trm_now = millis();              // mark the transmit time so we can check for timeout
    time_now = millis();              // mark the transmit time so we can check for timeout
    //sprintf( outBuf, "trm_now=%04X\t%ld", trm_now, trm_now );
    //Serial.println( outBuf );
    //mill = millis();
    //sprintf( outBuf, "millis=%04X\t%ld", mill, mill );
    //Serial.println( outBuf );
    //sprintf( outBuf, "diff=%04X\t%ld", ((unsigned long)(millis() - time_now)), (unsigned long)(millis() - time_now) );
    //Serial.println( outBuf );
    logger.println("transmitted, waiting for receive");
  }
  else {
    Serial.print( "." );
  }
  
  // ---------------- receiver ------------------------------
  if (msgChannel.update ())
  {
    logger.print ("Message received: ");
    logger.write (msgChannel.getData (), msgChannel.getLength ()); 
    logger.println ();
  }
  else 
    logger.print ("~");

 */
