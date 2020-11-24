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

// ------------------------- variables definition -----------------------------
byte boardID;                         // board ID: master is 0, expanders and others up to 0xE; OxF means bradcast
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

struct MSG {
  byte cmd;
  byte dst;
  int  len;
  byte payload[MAX_PAYLOAD_SIZE];
  byte parse_err;
} ;

enum msgParseErr {
  BAD_CMD = 1,
  BAD_DST,
  INV_PAYLD_LEN,
};


struct MSG rcvMsg;

// ------------------------- code ----------------------------------------------
void SlaveSendMessage(byte cmd, byte dest, byte *payload, byte *out_buf, int payload_len) {
    // byte * compose_msg(byte cmd, byte dest, byte *payload, byte *out_buf, int payload_len)
    if(!compose_msg(cmd, dest, payload, out_buf, payload_len))
      logger.println( "\nSlave:  Error composing message -  too long???");
    logger.println( "\nSlave:  Sending reply -------------------------------" );
    Slave_485_transmit_mode();
    SlaveMsgChannel.sendMsg (out_buf, payload_len);
    Slave_Flush ();                     // make sure the data are transmitted properly befor swithching the line direction
    Slave_485_receive_mode();
    Slave_Flush();         // flushes both Tx and Rx
    logger.println("Slave reply transmitted, going back to listening mode");
}

struct MSG  parse_msg(RS485& rcv_channel) {

    struct MSG rmsg;  // MasterMsgChannel.getLength
    rmsg.parse_err = 0;
    rmsg.len = rcv_channel.getLength(); // command+dest (1 byte) + payload_len (command specific)
    byte inbuf[MAX_MSG_LENGHT];
    if ((rmsg.len <  1) || (rmsg.len >  MAX_MSG_LENGHT)){ // message len issue
      comm_errors.protocol++;
      rmsg.parse_err = INV_PAYLD_LEN;
      logger.printf("parse_MSG: Invalid message payload len = %d\n", rmsg.len);
      return rmsg;        // error, no command code in message
    }
    memcpy (inbuf, rcv_channel.getData (), rmsg.len);   // copy message in temp buf
    // extract command and destination
    rmsg.cmd = ((inBuf[0] >> 4) & 0x0F);
    rmsg.dst = inBuf[0] & 0x0F;                        // TODO - where we have to check if we are the destination for this message
    logger.printf("Parse message recv: LEN = %d, CMD|DST = %x, PAYLOAD = %s\n", rmsg.len, rmsg.dst, &inBuf[1]);
    if((rmsg.dst != BROADCAST_ID) && (rmsg.dst != boardID)) {
      logger.printf("Command for board %d received\n", rmsg.dst);
      rmsg.parse_err = BAD_DST;
      return rmsg;         // the command is for different board
    }
    
    switch (rmsg.cmd & ~(0xF0 | REPLY_OFFSET )) {        // check for valid commands and replies. clear reply bit to facilitate test
      case PING:
        memcpy(rmsg.payload, &inbuf[1], PING_PAYLD_LEN);
        return rmsg;
        break;
      case POLL_ZONES:
        memcpy(rmsg.payload, &inbuf[1], POLL_PAYLD_LEN);
        return rmsg;
        break;
      case SET_OUTS:
        memcpy(rmsg.payload, &inbuf[1], SET_OUTS_PAYLD_LEN);
        return rmsg;
        break;
      case FREE_TEXT:
        memcpy(rmsg.payload, &inbuf[1], FREE_TEXT_PAYLD_LEN);
        return rmsg;
        break;
      default:
        logger.printf("Unknown command %d received", rmsg.cmd);
        rmsg.parse_err = BAD_CMD;
        return rmsg;        // error, no command code in message;   
    }  // switch
}

void setup() {
  logger.begin(LOG_BITRATE,SERIAL_8N1);
  // set UARTs
  MasterUART.begin(BITRATE,SERIAL_8N1, 21, 22);  // re-routing RxD to  GPIO21 and TxD to GPIO22
  SlaveUART.begin(BITRATE,SERIAL_8N1);
  // allocate data buffers and init message encoding/decoding engines (485_non_blocking library)
  MasterMsgChannel.begin ();      
  SlaveMsgChannel.begin ();  
  boardID = 1;      // TODO - get board ID
  logger.println("Loopback example for Esp32+485");
  logger.printf("MAX_MSG_LENGHT = %d", MAX_MSG_LENGHT  );
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
    logger.print ("\nSlave message received: \n");
    int len = SlaveMsgChannel.getLength ();
    memcpy (inBuf, SlaveMsgChannel.getData (), len); 

    // slave process received message
    byte cmd = ((inBuf[0] >> 4) & 0x0F);
    byte dest = inBuf[0] & 0x0F;
    
    logger.printf ("Slave received CMD: %x; DEST: %x; payload len: %d; PAYLOAD: ", cmd, dest, len);
    logger.write (&inBuf[PAYLOAD_OFFSET], len);

    rcvMsg = parse_msg(SlaveMsgChannel);   
    if (rcvMsg.parse_err) {
      if(rcvMsg.parse_err != BAD_DST)       // if the message is not for us this is not real error, just skip the processing
        logger.printf ("Slave parse message error %d\n", rcvMsg.parse_err);
    } else {
      logger.printf ("Slave received CMD: %x; DEST: %x; payload len: %d; PAYLOAD: ", rcvMsg.cmd, rcvMsg.dst, rcvMsg.len);
      logger.write (rcvMsg.payload, rcvMsg.len);
      // Process message and send reply
      // PING = 0x0,             // ping 
      // POLL_ZONES = 0x1,       // poll the extenders for zones status
      // SET_OUTS = 0x2,         // set output relay
      // FREE_TEXT = 0x3         // send unformatted payload up to MAX_PAYLOAD_SIZE
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
          logger.printf("Command received FREE_TEXT\n");
          // return the same payload converted to uppercase
          for (int i=0; i < rcvMsg.len; i++)
            inBuf[i] = toupper(rcvMsg.payload[i]);
          SlaveSendMessage (FREE_TEXT_RES, MASTER_ADDRESS, inBuf, outBuf, rcvMsg.len);
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
