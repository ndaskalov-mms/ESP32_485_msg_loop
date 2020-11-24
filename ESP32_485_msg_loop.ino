#include "Alarm_RS485.h"

#define MASTER
#define SLAVE

constexpr int BITRATE = 115200;
constexpr int LOG_BITRATE = 115200;
HardwareSerial& logger(Serial);
HardwareSerial& MasterUART(Serial1);
HardwareSerial& SlaveUART(Serial2);

#include "helpers.h"      // include helper functions. INCLUDE ONLY AFTER SERIAL PORT DEFINITIONS!!!!
#define TRM_INTERVAL  1000  //1 sec
#define REPLY_TIMEOUT  500  //200 msec

// ------------------------- variacles definition -----------------------------
byte inBuf[RxBUF_SIZE];
byte outBuf[MAX_MSG_LENGHT] = "";
unsigned long last_transmission = 0;
int waiting_for_reply = 0;
unsigned long master_err = 0;
unsigned long slave_err = 0;
byte test_msg [MAX_PAYLOAD_SIZE] = "5Hello world;6Hello world;7Hello world;8Hello world;9Hello0";
//this is channel to send/receive packets over serial if. The comm to serial is via fRead, fWrite,...
RS485 MasterMsgChannel (Master_Read, Master_Available, Master_Write, Master_Log_Write, RxBUF_SIZE);   //RS485 myChannel (read_func, available_func, write_func, msg_len);
RS485 SlaveMsgChannel (Slave_Read, Slave_Available, Slave_Write, Slave_Log_Write, RxBUF_SIZE);   //RS485 myChannel (read_func, available_func, write_func, msg_len);

// ------------------------- code ----------------------------------------------
void SlaveSendMessage(byte cmd, byte dest, byte *payload, byte *out_buf, int payload_len) {
    // byte * compose_msg(byte cmd, byte dest, byte *payload, byte *out_buf, int payload_len)
    if(!compose_msg(cmd, dest, payload, out_buf, payload_len))
      logger.println( "\nMaster:  Error composing message -  too long???");
    logger.println( "\nSlave:  Sending reply -------------------------------" );
    Slave_485_transmit_mode();
    SlaveMsgChannel.sendMsg (out_buf, payload_len);
    Slave_Flush ();                     // make sure the data are transmitted properly befor swithching the line direction
    Slave_485_receive_mode();
    Slave_Flush();         // flushes both Tx and Rx
    logger.println("Slave reply transmitted, going back to listening mode");
}

void setup() {
  logger.begin(LOG_BITRATE,SERIAL_8N1);
  // set UARTs
  MasterUART.begin(BITRATE,SERIAL_8N1, 21, 22);  // re-routing RxD to  GPIO21 and TxD to GPIO22
  SlaveUART.begin(BITRATE,SERIAL_8N1);
  // allocate data buffers and init message encoding/decoding engines (485_non_blocking library)
  MasterMsgChannel.begin ();      
  SlaveMsgChannel.begin ();  
  logger.println("Loopback example for Esp32+485");
  logger.printf("MAX_MSG_LENGHT = %d", MAX_MSG_LENGHT  );
}

void loop ()
{
#ifdef MASTER
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
      // process message here
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
    // byte * compose_msg(byte cmd, byte dest, byte *payload, byte *out_buf, int payload_len)
    if(!compose_msg(FREE_TEXT, MASTER_ADDRESS, test_msg, outBuf, MAX_PAYLOAD_SIZE))
      logger.println( "\nMaster:  Error composing message -  too long???");
    Master_485_transmit_mode();
    MasterMsgChannel.sendMsg (outBuf, MAX_MSG_LENGHT);
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
#endif
#ifdef SLAVE
  // ----------- slave simulation -------------------------------------------
  // ---------------- receiver ------------------------------
  if (SlaveMsgChannel.update ())
  {
    logger.print ("\nSlave message received: ");
	  int len = SlaveMsgChannel.getLength ();
    memcpy (inBuf, SlaveMsgChannel.getData (), len); 

    // slave process received message
    byte cmd = ((inBuf[0] >> 4) & 0x0F);
    byte dest = inBuf[0] & 0x0F;
    
    logger.printf ("Slave received CMD: %x; DEST: %x; payload len: %d; PAYLOAD: ", cmd, dest, len);
    logger.write (&inBuf[PAYLOAD_OFFSET], len);

    // Process message and send reply
    // PING = 0x0,             // ping 
    // POLL_ZONES = 0x1,       // poll the extenders for zones status
    // SET_OUTS = 0x2,         // set output relay
    // FREE_TEXT = 0x3         // send unformatted payload up to MAX_PAYLOAD_SIZE
    switch (cmd) {
      case PING:
        logger.printf("Unsupported command received PING\n");
        break;
      case POLL_ZONES:
        logger.printf("Unsupported command received POLL_ZONES\n");
        break;
      case SET_OUTS:
        logger.printf("Unsupported command received SET_OUTPUTS\n");
        break;
      case FREE_TEXT:
        logger.printf("Command received FREE_TEXT\n");
        // return the same payload converted to uppercase
        for (int i=0; i < len; i++)
          inBuf[i] = toupper(inBuf[i]);
        SlaveSendMessage (FREE_TEXT_RES, dest, inBuf, outBuf, len);
        break;
      default:
        logger.printf("Invalid command received %d\n", cmd);
    }
  } // if update()

  if(slave_err != SlaveMsgChannel.getErrorCount()) {
    slave_err = SlaveMsgChannel.getErrorCount();
    logger.print("Slave errors cnt:");
    logger.println(slave_err, DEC);
  }
#endif
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
