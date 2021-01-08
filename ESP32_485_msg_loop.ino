// define maximal configuration per board
#define MAX_ZONES_CNT	18		// HW limitation, 18 on SLAVE, less on MASTER
#define MAX_PGM_CNT		8		// 8 on MASTER, 2 on SLAVE
//
// define roles
#define MASTER
//#define SLAVE
#define LOOPBACK
#define MAX_SLAVES 1
// board IDs
enum ADDR {                                         // board adresses, MASTER is ALLWAYS 0
	MASTER_ADDRESS =  0,
	SLAVE_ADDRESS1,
	SLAVE_ADDRESS2,
	SLAVE_ADDRESS3,
	SLAVE_ADDRESS4,
	SLAVE_ADDRESS5,
	SLAVE_ADDRESS6,
	SLAVE_ADDRESS7,
};
//
// debug print levels
#define DEBUG	  true
#define INFO    true
#define WARNING	true
#define ERROR   true

// some other conf defines
#define POLL_INTERVAL  1000           // Shall be 200ms 
#define REPLY_TIMEOUT  100            // REPLY_TIMEOUT MUST be at least 2x less POLL_INTERVAL to avoid sending a new command while waiting for 
#define NO_TIMEOUT      0

// define log and errors channel
#define SERIAL_LOG 	true
#define MQTT_LOG	  false
#define ZONES_A_READ_INTERVAL 100     // read A zones at 100mS
#define ZONES_B_READ_INTERVAL 500    // read system voltages (B zones) at 100mS
#define MUX_SET_INTERVAL      10      // time to set the analog lines after mux switch

constexpr int BITRATE = 115200;
constexpr int LOG_BITRATE = 115200;
HardwareSerial& logger(Serial);
//
                                        // all zones and pgms related staff
//
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
//
byte boardID;                         // board ID: master is 0, expanders and others up to 0xE; OxF means bradcast
int waiting_for_reply = 0;            // tracks current state of the protocol
//
// include all code as it is not possible to compile project from several user .cpp files:-(
//
#include "mqtt.h"
#include "errors.h"                   // errors definitions and handling
#include "zonestest/zonen.h"   
#include "RS485_cbacks.h"             // callbacks required by RS485 lib and UART related staff (ESP32 specific)
#include "helpers.h"                  // include helper functions. INCLUDE ONLY AFTER SERIAL PORT DEFINITIONS!!!!
#include "Alarm_RS485-cpp.h"          // RS485 transport implementation (library)
#include "alarm_logic.h"
//
//
// ------------------------- global variables definition -----------------------------
#ifdef MASTER
byte MzoneResult[MASTER_ZONES_CNT/2 + MASTER_ZONES_CNT%2];                                  //each zone will be in 4bits
//these are channels to send/receive packets over serial if. The comm to serial is via fRead, fWrite,...
RS485 MasterMsgChannel (MasterRead, MasterAvailable, MasterWrite, ErrWrite, RxBUF_SIZE);   //RS485 myChannel (read_func, available_func, write_func, msg_len);
#endif
#ifdef SLAVE
byte SzoneResult[SLAVE_ZONES_CNT/2 + SLAVE_ZONES_CNT%2];          //each zone will be in 4bits
RS485 SlaveMsgChannel  (SlaveRead, SlaveAvailable, SlaveWrite, ErrWrite, RxBUF_SIZE);      //RS485 myChannel (read_func, available_func, write_func, msg_len);
#endif
int err, retCode;                         // holds error returns from some functions                         
struct MSG rcvMsg;                        // temp structs for message tr/rcv
byte tmpMsg [MAX_PAYLOAD_SIZE];
//
#include "protocol.h"                     // send/receive and compse messgaes staff
#include "commands.h"                     // master/slave commands implementation
//
//
//  Arduino setup function - call all local setups her
//
void setup() {
  logger.begin(LOG_BITRATE,SERIAL_8N1);
  logger.printf("\n\nStaring setup\n\n");
  logger.printf("Max board pgm cnt = %d\n", MAX_PGM_CNT);
  logger.printf("Max board zones cnt = %d\n", MAX_ZONES_CNT);
#ifdef MASTER
   MasterUART.begin(BITRATE,SERIAL_8N1);  
   MasterMsgChannel.begin ();                 // allocate data buffers and init message encoding/decoding engines (485_non_blocking library)
   pgmSetup(MpgmDB, MAX_PGM_CNT);             // init PGMs (output and default value)
   initAlarmZones();
   printAlarmZones();
#endif
#ifdef SLAVE
   SlaveMsgChannel.begin ();                     // allocate data buffers and init message encoding/decoding engines (485_non_blocking library)
   pgmSetup(SpgmDB, MAX_PGM_CNT);              // init PGMs (output and default value)
#ifdef LOOPBACK
  SlaveUART.begin(BITRATE,SERIAL_8N1, 21, 22);    // re-routing RxD to  GPIO21 and TxD to GPIO22
#else
  SlaveUART.begin(BITRATE,SERIAL_8N1,)
#endif
#endif
  logger.printf("Loopback example for Esp32+485\n");
  logger.printf("MAX_MSG_LENGHT = %d\n", MAX_MSG_LENGHT  );
  logger.printf("MAX_PAYLOAD_SIZE = %d\n", MAX_PAYLOAD_SIZE );
  logger.printf("Size of test msg: %d\n", FREE_CMD_DATA_LEN);
  zoneSetup();                                  // init mux for zones selection
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
