
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

#include "Alarm_RS485-cpp.h"              // RS485 transport implementation (library)

// ------------------------- global variables definition -----------------------------
byte boardID;                             // board ID: master is 0, expanders and others up to 0xE; OxF means bradcast
unsigned long last_transmission = 0;      // last transmission time
int waiting_for_reply = 0;
//unsigned long master_err = 0;
//unsigned long slave_err = 0;
int err;
byte test_msg [][MAX_PAYLOAD_SIZE] = {{"5Hello world;6Hello world;7Hello world;8Hello world;9Hello"},\
                                     {"123456789012345678901234567890123456789012345678901234567890"},\
                                     {"~!@#$%^&*()_+~!@#$%^&*()_+~!@#$%^&*()_+~!@#$%^&*()_+~!@#$%"},
                                     {"zxcvbnm,./';lkjhgfdsaqwertyuiop[]\\][poiuytrewqasdfghjkl;'"}};
                                
                                     
struct MSG rcvMsg, tmpMsg;                // temp structs for message tr/rcv
//these are channels to send/receive packets over serial if. The comm to serial is via fRead, fWrite,...
RS485 MasterMsgChannel (MasterRead, MasterAvailable, MasterWrite, ErrWrite, RxBUF_SIZE);   //RS485 myChannel (read_func, available_func, write_func, msg_len);
RS485 SlaveMsgChannel  (SlaveRead, SlaveAvailable, SlaveWrite, ErrWrite, RxBUF_SIZE);   //RS485 myChannel (read_func, available_func, write_func, msg_len);

#include "protocol.h"

void setup() {
  logger.begin(LOG_BITRATE,SERIAL_8N1);
  // set UARTs
  MasterUART.begin(BITRATE,SERIAL_8N1, 21, 22);  // re-routing RxD to  GPIO21 and TxD to GPIO22
  SlaveUART.begin(BITRATE,SERIAL_8N1);
  // allocate data buffers and init message encoding/decoding engines (485_non_blocking library)
  MasterMsgChannel.begin ();      
  SlaveMsgChannel.begin ();  
  logger.printf("Loopback example for Esp32+485\n");
  logger.printf("MAX_MSG_LENGHT = %d\n", MAX_MSG_LENGHT  );
  logger.printf("MAX_PAYLOAD_SIZE = %d\n", MAX_PAYLOAD_SIZE );
}

void loop ()
{
#ifdef MASTER
  #include "master.h"
#endif
#ifdef SLAVE
  #include "slave.h"
#endif
}  // end of loop
