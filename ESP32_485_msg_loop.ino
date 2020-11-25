#include "Alarm_RS485.h"              // RS485 transport implementation (library)

#define MASTER
#define SLAVE

constexpr int BITRATE = 115200;
constexpr int LOG_BITRATE = 115200;
HardwareSerial& logger(Serial);
HardwareSerial& MasterUART(Serial1);
HardwareSerial& SlaveUART(Serial2);

#include "helpers.h"                  // include helper functions. INCLUDE ONLY AFTER SERIAL PORT DEFINITIONS!!!!

#define TRM_INTERVAL  1000  //1 sec
#define REPLY_TIMEOUT  500  //200 msec

// ------------------------- global variables definition -----------------------------
byte boardID;                         // board ID: master is 0, expanders and others up to 0xE; OxF means bradcast
//byte inBuf[RxBUF_SIZE];
byte outBuf[MAX_MSG_LENGHT] = "";     // probably will be not needed
unsigned long last_transmission = 0;  // last transmission time
int waiting_for_reply = 0;
unsigned long master_err = 0;
unsigned long slave_err = 0;
byte test_msg [MAX_PAYLOAD_SIZE] = "5Hello world;6Hello world;7Hello world;8Hello world;9Hello";

//this is channel to send/receive packets over serial if. The comm to serial is via fRead, fWrite,...
RS485 MasterMsgChannel (Master_Read, Master_Available, Master_Write, Master_Log_Write, RxBUF_SIZE);   //RS485 myChannel (read_func, available_func, write_func, msg_len);
RS485 SlaveMsgChannel (Slave_Read, Slave_Available, Slave_Write, Slave_Log_Write, RxBUF_SIZE);   //RS485 myChannel (read_func, available_func, write_func, msg_len);

struct MSG rcvMsg, tmpMsg;          // temp structs for message tr/rcv

#include "protocol.h"
void setup() {
  logger.begin(LOG_BITRATE,SERIAL_8N1);
  // set UARTs
  MasterUART.begin(BITRATE,SERIAL_8N1, 21, 22);  // re-routing RxD to  GPIO21 and TxD to GPIO22
  SlaveUART.begin(BITRATE,SERIAL_8N1);
  // allocate data buffers and init message encoding/decoding engines (485_non_blocking library)
  MasterMsgChannel.begin ();      
  SlaveMsgChannel.begin ();  
  boardID = 1;      // TODO - get board ID
  logger.printf("Loopback example for Esp32+485\n");
  logger.printf("MAX_MSG_LENGHT = %d\n", MAX_MSG_LENGHT  );
  logger.printf("MAX_PAYLOAD_SIZE = %d\n", MAX_PAYLOAD_SIZE );
}

void loop ()
{
#ifdef MASTER
  boardID = 0;        // TODO - only for loopback testing
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
    if(!compose_msg(FREE_TEXT, SLAVE1_ADDRESS, test_msg, outBuf, MAX_PAYLOAD_SIZE))
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
#endif   //MASTER
#ifdef SLAVE
  // ----------- slave simulation -------------------------------------------
  boardID = 1;        // Slave destination ---------   TODO - only for loopback testing
  if (SlaveMsgChannel.update ())
  {
    //logger.print ("\nSlave message received: \n");
    rcvMsg = parse_msg(SlaveMsgChannel);   
    if (rcvMsg.parse_err) {                   // if parse error, do nothing
      logger.printf ("Slave parse message error %d\n", rcvMsg.parse_err); // yse, do nothing
    } 
    else if (rcvMsg.dst == BROADCAST_ID)      // check for broadcast message
      logger.printf("Broadcast command received, skipping\n");  // do nothing
    else if (rcvMsg.dst != boardID)           // check if the destination is another board
      ;                                       // yes, do nothing
    else { 
      logger.printf ("Slave received CMD: %x; DEST: %x; payload len: %d; PAYLOAD: ", rcvMsg.cmd, rcvMsg.dst, rcvMsg.len);
      logger.write (rcvMsg.payload, rcvMsg.len);
      logger.println("");
      switch (rcvMsg.cmd) {
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
          // logger.printf("Command received FREE_TEXT\n");
          // return the same payload converted to uppercase
          for (int i=0; i < rcvMsg.len; i++)
            tmpMsg.payload[i] = toupper(rcvMsg.payload[i]);
          tmpMsg.cmd = FREE_TEXT | REPLY_OFFSET;
          tmpMsg.len = rcvMsg.len;
          tmpMsg.dst = MASTER_ADDRESS;
          SendMessage (SlaveMsgChannel, tmpMsg);
          break;
        default:
          logger.printf("Invalid command received %d\n", rcvMsg.cmd);
      }
    } // else
  } // if update()

  if(slave_err != SlaveMsgChannel.getErrorCount()) {
    slave_err = SlaveMsgChannel.getErrorCount();
    logger.print("Slave errors cnt:");
    logger.println(slave_err, DEC);
  }
#endif
}  // end of loop
