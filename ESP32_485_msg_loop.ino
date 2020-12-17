
// define role
#define MASTER
#define SLAVE
#define LOOPBACK

// debug print levels
#define DEBUG	  true
#define INFO    true
#define WARNING	true
#define ERROR   true

// define log and errors channel
#define SERIAL_LOG 	true
#define MQTT_LOG	  false

#define ZONES_READ_THROTTLE true
#define ZONES_READ_INTERVAL 100     // read all zones at 100mS


constexpr int BITRATE = 115200;
constexpr int LOG_BITRATE = 115200;
HardwareSerial& logger(Serial);
#ifdef MASTER
HardwareSerial& MasterUART(Serial2);
#endif
#ifdef SLAVE
#ifdef LOOPBACK
HardwareSerial& SlaveUART(Serial1);
#else
HardwareSerial& SlaveUART(Serial2);
#endif
#endif


enum ADDR {							            // board adresses, MASTER is ALLWAYS 0
MASTER_ADDRESS =  0,
SLAVE_ADDRESS1,
SLAVE_ADDRESS2,
SLAVE_ADDRESS3,
SLAVE_ADDRESS4,
SLAVE_ADDRESS5,
SLAVE_ADDRESS6,
SLAVE_ADDRESS7,
};

#include "mqtt.h"
#include "errors.h"                   // errors definitions and handling
#include "RS485_cbacks.h"             // callbacks required by RS485 lib and UART related staff (ESP32 specific)
#include "helpers.h"                  // include helper functions. INCLUDE ONLY AFTER SERIAL PORT DEFINITIONS!!!!
#include "Alarm_RS485-cpp.h"          // RS485 transport implementation (library)

#define POLL_INTERVAL  1000           // Shall be 200ms 
#define REPLY_TIMEOUT  100            // REPLY_TIMEOUT MUST be at least 2x less POLL_INTERVAL to avoid sending a new command while waiting for 
#define NO_TIMEOUT      0

// ------------------------- global variables definition -----------------------------
byte boardID;                             // board ID: master is 0, expanders and others up to 0xE; OxF means bradcast
int waiting_for_reply = 0;                // tracks current state of the protocol
int err, retCode;                         // holds error returns from some functions                         
struct MSG rcvMsg;                        // temp structs for message tr/rcv
//these are channels to send/receive packets over serial if. The comm to serial is via fRead, fWrite,...
RS485 MasterMsgChannel (MasterRead, MasterAvailable, MasterWrite, ErrWrite, RxBUF_SIZE);   //RS485 myChannel (read_func, available_func, write_func, msg_len);
RS485 SlaveMsgChannel  (SlaveRead, SlaveAvailable, SlaveWrite, ErrWrite, RxBUF_SIZE);   //RS485 myChannel (read_func, available_func, write_func, msg_len);
//
#include "protocol.h"                     // send/receive and compse messgaes staff
#include "zonen.h"                        // all xones and pgms related staff
//
//  Arduino setup function - call all local setups her
//
void setup() {
  logger.begin(LOG_BITRATE,SERIAL_8N1);
  logger.printf("\n\nStaring setup\n\n");
  // set UARTs
#ifdef MASTER
    MasterUART.begin(BITRATE,SERIAL_8N1);  
#endif
#ifdef SLAVE
#ifdef LOOPBACK
  SlaveUART.begin(BITRATE,SERIAL_8N1, 21, 22);    // re-routing RxD to  GPIO21 and TxD to GPIO22
#else
  SlaveUART.begin(BITRATE,SERIAL_8N1,)
#endif
#endif
  // allocate data buffers and init message encoding/decoding engines (485_non_blocking library)
  MasterMsgChannel.begin ();      
  SlaveMsgChannel.begin ();  
  
  logger.printf("Loopback example for Esp32+485\n");
  logger.printf("MAX_MSG_LENGHT = %d\n", MAX_MSG_LENGHT  );
  logger.printf("MAX_PAYLOAD_SIZE = %d\n", MAX_PAYLOAD_SIZE );
  //printErrorsDB();
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
