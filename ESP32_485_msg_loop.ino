#include <RS485_non_blocking.h>

constexpr int BITRATE = 115200;
//constexpr SoftwareSerialConfig swSerialConfig = SWSERIAL_8E1;
//constexpr bool invert = false;
constexpr int BLOCKSIZE = 128; // use fractions of 256

SoftwareSerial serialIUT;
HardwareSerial& logger(Serial);

size_t fWrite (const byte what)
{
  return serialIUT.write (what);  
}

int fAvailable ()
{
  return serialIUT.available();
}
 
int fRead ()
{
  return serialIUT.read();
}
 
void fTxFlush ()
{
#ifdef SoftSer
    ;   //do nothing, Tx software serial emulation is blocking
#else 
    serialIUT.flush();
#endif
}

void fRxFlush()
{
  // TODO - this is commented for HALF-DUPLEX simulation
  //while(fAvailable())
  // fRead();
}

void 
f485_receive_mode()
{
  // change line dir
  delay(1);
  fRxFlush();
}

void f485_transmit_mode()
{
    // change line dir
  delay(1);
  fTxFlush();
}



//this is channel to send/receive packets over serial if. The comm to serial is via fRead, fWrite,...
RS485 msgChannel (fRead, fAvailable, fWrite, BLOCKSIZE);   //RS485 myChannel (read_func, available_func, write_func, msg_len);

void setup() {

  logger.begin(BITRATE,SERIAL_8N1,);
  Serial1.begin(BITRATE,SERIAL_8N1, 21, 22);  // re-routing RxD to  GPIO21 and TxD to GPIO22
  Serial2.begin(BITRATE,SERIAL_8N1,);
}
    logger.begin(9600);
    serialIUT.begin(IUTBITRATE, swSerialConfig, D5, D6, invert, 2 * BLOCKSIZE);
    msgChannel.begin ();      
    logger.println("Loopback example for EspSoftwareSerial+485");
}


const byte msg [] = "Hello world";
#define TRM_INTERVAL  1000  //1 sec
#define REPLY_TIMEOUT  200  //200 msec

void loop ()
{
  // ---------------- transmitter ------------------------------
  char inBuf[BLOCKSIZE];
  char outBuf[BLOCKSIZE]= "hello world";
  static unsigned long last_transmission = 0;
  static int waiting_for_reply = 0;


  
  // ----------- master simulation -------------------------------------------
  //logger.println("Transmitter:");
  
  if (waiting_for_reply)
  {
    if (msgChannel.update ())
    {
      // msg received
      logger.print ("Message received: ");
      logger.write (msgChannel.getData (), msgChannel.getLength ()); 
      logger.println ();
      waiting_for_reply = 0;
      f485_transmit_mode();
      // process message
    }
    else if((unsigned long)(millis() - last_transmission) > REPLY_TIMEOUT) {
        // reply not received
        f485_transmit_mode();
        waiting_for_reply = 0;
        // TODO - signal error somehow
        logger.println ("Reply timeout");
    }
    Serial.print( "!" );
  }     // if (waiting_for_reply)
  else if( (unsigned long)(millis() - last_transmission) > TRM_INTERVAL){  // check if it is time for the next comm
    Serial.println( "\nTime to transmit -------------------------------" );
    f485_transmit_mode();
    msgChannel.sendMsg (msg, sizeof (msg));
    last_transmission = millis();    // mark the transmit time so we can calculate the time for the next transmission and check for reply timeout
    fTxFlush ();                     // make sure the data are transmitted properly befor swithching the line direction
    f485_receive_mode();
    logger.println("MSG transmitted, receive timeout started");
    waiting_for_reply = 1;
  }
  else {
    Serial.print( "." );
  }
/*
  // ----------- slave simulation -------------------------------------------
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
