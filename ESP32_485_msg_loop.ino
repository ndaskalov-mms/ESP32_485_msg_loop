#define SW_VERSION  100				// UPDATE ON EVERY CHANGE OF ZONEs, PGMs, etc
#define VCS true					// to compile for debug with Visual studio
#ifdef VCS
typedef unsigned char byte;
#define HIGH 1
#define LOW 0
#endif
//
// define maximal configuration per board. IF MODIFIED, COMPILE AND CHANGE SIMULTANEOUSLY BOTH MASTER AND SLAVES!!!!
#define SLAVE_ZONES_CNT	  18		// number of zones (physical ADC channels)  on slave board	
#define MASTER_ZONES_CNT  12		// number of zones (physical ADC channels)  on master board	
// every zone support two contacts connected to one ADC channel, that's why logical zones are twice more than physical
#define SLAVE_ALARM_ZONES_CNT	(SLAVE_ZONES_CNT*2) // each slave zone (ADC channel)  reports two sensors	
#define MASTER_ALARM_ZONES_CNT	(MASTER_ZONES_CNT*2) // each master zone reports two sensors 
#define MAX_ALARM_ZONES_PER_BOARD	    SLAVE_ALARM_ZONES_CNT		// dimension all arrays to hold up to max alarm zones count
//
#define MAX_PGM_CNT		    8		  // 8 on MASTER, 2 on SLAVE
#define MASTER_PGM_CNT	  8		
#define SLAVE_PGM_CNT     2
#define SZONERES_LEN      (SLAVE_ZONES_CNT/2 + SLAVE_ZONES_CNT%2)
#define MAX_KEYSW_CNT	32
#define MAX_MQTT_TOPIC	256
//
#define ENABLE_CONFIG_CREATE false
#define FORCE_FORMAT_FS      false
#define FORCE_NEW_CONFIG     false          // change ENABLE_CONFIG_CREATE to true alse
//
// define roles
#define MASTER
#define SLAVE
#define LOOPBACK
#define MAX_SLAVES 1							 // absolute max is 16
//
// board IDs
enum ADDR {                                         // board adresses, MASTER is ALLWAYS 0
	MASTER_ADDRESS =  0,
	SLAVE_ADDRESS1,
};
//
// debug print levels
#define DEBUG	  true
#define INFO    true
#define WARNING	true
#define ERROR   true

// some other conf defines
#define POLL_INTERVAL  1000UL           // Shall be 200ms 
#define REPLY_TIMEOUT  10UL            // REPLY_TIMEOUT MUST be at least 2x less POLL_INTERVAL to avoid sending a new command while waiting for 
#define NO_TIMEOUT      0

// define log and errors channel
#define SERIAL_LOG 	true
#define MQTT_LOG	  false
#define ZONES_A_READ_INTERVAL 500      // read A zones at 100mS
#define ZONES_B_READ_INTERVAL 1000     // read system voltages (B zones) at 100mS
#define MUX_SET_INTERVAL      10       // time to set the analog lines after mux switch
#define MASTER_ZONES_READ_INTERVAL 500 // read A zones at 100mS
#define ALARM_LOOP_INTERVAL	  1000	   // how often to run the alarm loop	

constexpr int BITRATE = 115200;
constexpr int LOG_BITRATE = 115200;
HardwareSerial& logger(Serial);
#define lprintf logger.printf
//
#ifdef MASTER
HardwareSerial& MasterUART(Serial2);
const char configFileName[] = "/alarmConfig3.cfg";
int newZonesDataAvailable = 0;		// bitmap to know which board data was changed, master is 1, slave 1 is 2, etc
int maxSlaves = 0;
#endif
#ifdef SLAVE
#ifdef LOOPBACK
HardwareSerial& SlaveUART(Serial1);
#else
HardwareSerial& SlaveUART(Serial2);
#endif
#endif
//
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
byte slaveAdr;                          // board ID: master is 0, expanders and others up to 0xE; OxF means bradcast
byte SzoneResult[SZONERES_LEN];          //each zone will be in 4bits
RS485 SlaveMsgChannel  (SlaveRead, SlaveAvailable, SlaveWrite, ErrWrite, RxBUF_SIZE);      //RS485 myChannel (read_func, available_func, write_func, msg_len);
#endif
//
int err, retCode;                         // holds error returns from some functions                         
struct MSG rcvMsg;                        // temp structs for message tr/rcv
byte tmpMsg [MAX_PAYLOAD_SIZE];
byte tempMQTTbuf [MAX_MQTT_TOPIC];
//
// cooperative multitasking staff
//
#define _TASK_SLEEP_ON_IDLE_RUN
#include <TaskScheduler.h>
Scheduler taskScheduler;
// Callback methods prototypes
#ifdef MASTER
void master();
Task t1(3, TASK_FOREVER, &master, &taskScheduler, true);
#endif
#ifdef SLAVE
void slave();
Task t2(3, TASK_FOREVER, &slave, &taskScheduler, true);
#endif
//
#include "protocol.h"                     // send/receive and compse messgaes staff
#ifdef MASTER
#include "alarm_logic.h"
#endif
#include "commands.h"                     // master/slave commands implementation

//
//  Arduino setup function - call all local setups her
//
void setup() {
  delay(500);
  logger.begin(LOG_BITRATE,SERIAL_8N1);
  lprintf("\n\nStarting setup\n\n");
//
#ifdef SLAVE
   slaveAdr = readOwnAdr();		       // Slave destination ---------   TODO - only for loopback testing
   SlaveMsgChannel.begin ();                     // allocate data buffers and init message encoding/decoding engines (485_non_blocking library)
#ifdef LOOPBACK
  SlaveUART.begin(BITRATE,SERIAL_8N1, 21, 22);    // re-routing RxD to  GPIO21 and TxD to GPIO22
#else
  SlaveUART.begin(BITRATE,SERIAL_8N1,)
#endif
#endif
  lprintf("MAX_MSG_LENGHT = %d\n", MAX_MSG_LENGHT  );
  lprintf("MAX_PAYLOAD_SIZE = %d\n", MAX_PAYLOAD_SIZE );
  lprintf("Size of test msg: %d\n", FREE_CMD_DATA_LEN);
  zoneHWSetup();                                  // init mux for zones selection
  pgmSetup(SpgmDB, SLAVE_PGM_CNT);              // init PGMs (output and default value)
  //printErrorsDB();
  lprintf("\n\nSetup finished\n\n");
//
#ifdef MASTER
   MasterUART.begin(BITRATE,SERIAL_8N1);  
   MasterMsgChannel.begin ();                 // allocate data buffers and init message encoding/decoding engines (485_non_blocking library)
   if(!storageSetup()) {                      // mount file system
      while(true) {                           // loop forever
        PublishMQTT(ERROR_TOPIC, "Error initializing storage");
        delay(60000);                         // wait a minute before send again
      }
   }
   // read config file from storage and init all alarm internals and databases for zones, pgms, partitions, keswitches, etc
   initAlarm();              					// read setting from storage and set all variables
   storageClose();								// unmount FS
   maxSlaves = alarmGlobalOpts.maxSlaveBrds;	// 
   zoneHWSetup();                                  // init mux for zones selection
   pgmSetup(MpgmDB, MASTER_PGM_CNT);             // init PGMs (output and default value)
   ErrWrite(ERR_DEBUG, "ALARM ZONES read from config file\n");
   printAlarmZones((byte *) &alarmConfig.zoneConfig, MASTER_ADDRESS, maxSlaves);
   //printAlarmPgms((byte *) &alarmConfig.pgmConfig, MASTER_ADDRESS, maxSlaves);
#endif

  taskScheduler.startNow();  // set point-in-time for scheduling start
}
//
//
#ifdef MASTER
#include "master.h"
#endif
#ifdef SLAVE
#include "slave.h"
#endif
//
void loop ()
{
/*
#ifdef MASTER
#include "master.h"
#endif
#ifdef SLAVE
#include "slave.h"
#endif
*/
taskScheduler.execute();
}  // end of loop
