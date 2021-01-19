// define maximal configuration per board. IF MODIFIED, COMPILE AND CHANGE SIMULTANEOUSLY BOTH MASTER AND SLAVES!!!!
#define MAX_ZONES_CNT	    18		// Limited by ADC channels, 18 on SLAVE, less on MASTER. System voltages are read via mux and includded
#define SLAVE_ZONES_CNT	  18		
#define MASTER_ZONES_CNT  12		
#define MAX_PGM_CNT		    8		  // 8 on MASTER, 2 on SLAVE
#define MASTER_PGM_CNT	  8		
#define SLAVE_PGM_CNT     2

#define ENABLE_CONFIG_CREATE true
#define FORCE_FORMAT_FS      false
//
// define roles
#define MASTER
#define SLAVE
#define LOOPBACK
#define MAX_SLAVES 1
//
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
#ifdef MASTER
HardwareSerial& MasterUART(Serial2);
const char configFileName[] = "/alarmConfig3.cfg";
#endif
#ifdef SLAVE
#ifdef LOOPBACK
HardwareSerial& SlaveUART(Serial1);
#else
HardwareSerial& SlaveUART(Serial2);
#endif
#endif
//
byte boardID;                           // board ID: master is 0, expanders and others up to 0xE; OxF means bradcast
int  waiting_for_reply = 0;             // tracks current state of the protool
byte zoneInfoValid = 0;                 // track if zonesResult array contain valid info as the host can request info before they are read
//
// include all code as it is not possible to compile project from several user .cpp files:-(
//
#include "mqtt.h"
#include "errors.h"                   // errors definitions and handling
#include "zonestest/zonen.h"   
#include "RS485_cbacks.h"             // callbacks required by RS485 lib and UART related staff (ESP32 specific)
#include "helpers.h"                  // include helper functions. INCLUDE ONLY AFTER SERIAL PORT DEFINITIONS!!!!
#include "Alarm_RS485-cpp.h"          // RS485 transport implementation (library)

//
//
// ------------------------- global variables definition -----------------------------
#ifdef MASTER
//byte masterDataValid = 0;              // bad or missing config file - maybe it is first run or storage is garbage
byte remoteDataValid = 0;              // slaves data not fetched yet
byte MzoneResult[MASTER_ZONES_CNT/2 + MASTER_ZONES_CNT%2];                                  //each zone will be in 4bits
//these are channels to send/receive packets over serial if. The comm to serial is via fRead, fWrite,...
RS485 MasterMsgChannel (MasterRead, MasterAvailable, MasterWrite, ErrWrite, RxBUF_SIZE);   //RS485 myChannel (read_func, available_func, write_func, msg_len);
#endif
#ifdef SLAVE
byte SzoneResult[SLAVE_ZONES_CNT/2 + SLAVE_ZONES_CNT%2];          //each zone will be in 4bits
RS485 SlaveMsgChannel  (SlaveRead, SlaveAvailable, SlaveWrite, ErrWrite, RxBUF_SIZE);      //RS485 myChannel (read_func, available_func, write_func, msg_len);
#endif
//
int err, retCode;                         // holds error returns from some functions                         
struct MSG rcvMsg;                        // temp structs for message tr/rcv
byte tmpMsg [MAX_PAYLOAD_SIZE];
//
#include "protocol.h"                     // send/receive and compse messgaes staff
#include "commands.h"                     // master/slave commands implementation
#ifdef MASTER
#include "alarm_logic.h"
#endif
//
//
//  Arduino setup function - call all local setups her
//
void setup() {
  delay(1000);
  logger.begin(LOG_BITRATE,SERIAL_8N1);
  logger.printf("\n\nStarting setup\n\n");
//
#ifdef SLAVE
   SlaveMsgChannel.begin ();                     // allocate data buffers and init message encoding/decoding engines (485_non_blocking library)
#ifdef LOOPBACK
  SlaveUART.begin(BITRATE,SERIAL_8N1, 21, 22);    // re-routing RxD to  GPIO21 and TxD to GPIO22
#else
  SlaveUART.begin(BITRATE,SERIAL_8N1,)
#endif
#endif
  logger.printf("MAX_MSG_LENGHT = %d\n", MAX_MSG_LENGHT  );
  logger.printf("MAX_PAYLOAD_SIZE = %d\n", MAX_PAYLOAD_SIZE );
  logger.printf("Size of test msg: %d\n", FREE_CMD_DATA_LEN);
  zoneHWSetup();                                  // init mux for zones selection
  pgmSetup(SpgmDB, SLAVE_PGM_CNT);              // init PGMs (output and default value)
  //printErrorsDB();
  logger.printf("\n\nSetup finished\n\n");
//
#ifdef MASTER
   MasterUART.begin(BITRATE,SERIAL_8N1);  
   MasterMsgChannel.begin ();                 // allocate data buffers and init message encoding/decoding engines (485_non_blocking library)
   if(!storageSetup()) {                      // mount file system
      while(true) {                           // loop forever
        ReportMQTT(ERROR_TOPIC, "Error initializing storage");
        delay(60000);                         // wait a minute before send again
      }
   }
   // read config file from storage and init all alarm internals and databases for zones, pgms, partitions, keswitches, etc
   initAlarm();              					// set flag for loop() to know if the initialization was successful
   storageClose();								// unmount FS
   zoneHWSetup();                                  // init mux for zones selection
   pgmSetup(MpgmDB, MASTER_PGM_CNT);             // init PGMs (output and default value)
   ErrWrite(ERR_DEBUG, "ALARM ZONES read from config file\n");
   printAlarmZones((byte *) &alarmConfig.zoneConfigs, MASTER_ADDRESS, MAX_SLAVES);
   printAlarmPgms((byte *) &alarmConfig.pgmConfigs, MASTER_ADDRESS, MAX_SLAVES);
#endif
}
//
//
//
void loop ()
{
#ifdef MASTER
#include "master.h"
#endif
#ifdef SLAVE
#include "slave.h"
#endif
}  // end of loop
