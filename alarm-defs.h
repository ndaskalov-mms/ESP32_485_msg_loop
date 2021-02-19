//
// zone definitions
enum ZONE_DEFS_t {
    ZONE_DISABLED = 0,
    INSTANT,
    ENTRY_DELAY1,
    ENTRY_DELAY2,
    FOLLOW,
	STAY,
	STAY_DELAY1,
    STAY_DELAY2_ANTI_MASK,
    H24_BUZZER,
    H24_BURGLAR,
    H24_HOLDUP,
    H24_GAS,
    H24_HEAT,
    H24_WATER,
    H24_FREEZE,
    H24_FIRE_DELAYED,
    H24_FIRE_STANDARD,
};

enum  ZONE_OPTS_t {
    SHUTDWN_EN 			= 0x1,
    BYPASS_EN           = 0x2,                                    // default ON
    STAY_ZONE           = 0x4,
    FORCE_EN            = 0x8,                                    // default ON
    ALARM_TYPE          = (0x10 | 0x20),
    INTELIZONE          = 0X40,
    DELAY_TRM		  	= 0x80,
};
    
#define STEADY_ALARM    0x0
#define PULSED_ALARM    0X20
#define SILENT_ALARM    0x10
#define REPORT_ALARM 	0x30

enum ZONE_EXT_OPT_t {
    ZONE_TAMPER_OPT = (0x2 | 0x1),  		// zone tamper opions bits - see ZONE_TAMPER_OPT_XXX
    ZONE_FOLLOW_PANEL_ONTAMPER  = 0x8,		// true - follow gloabl tamper options, false - follow zone tamper options
    ZONE_ANTIMASK_OPT = (0x20 | 0x10), 		// zone antimask options bits - see ZONE_ANTI_MASK_SUPERVISION_XXX
    ZONE_FOLLOW_GLOBAL_ON_ANTIMASK  = 0x80,	// true - follow gloabl antimask options, false - follow zone antimask options
};
#define ZONE_ANTIMASK_OPT_SHIFT_CNT 4
#define ZONE_ANTIMASK_OPT_BITS 0xF0
#define ZONE_TAMPER_OPT_BITS 0x0F
//
enum RF_SUPERVISION_OPT_t {
	RF_SUPERVISION_DISABLED =  0,
	RF_SUPERVISION_TROUBLE_ONLY = 0x1,
	RF_SUPERVISION_ALARM_WHEN_ARMED = 0x2,
	RF_SUPERVISION_ALARM  = 3,
};
//
enum ZONE_TAMPER_OPT_t {
	ZONE_TAMPER_OPT_DISABLED  =  0
	ZONE_TAMPER_OPT_TROUBLE_ONLY = 1,
	ZONE_TAMPER_OPT_ALARM_WHEN_ARMED  = 2, 
	ZONE_TAMPER_OPT_ALARM  = 3,
}
//    
enum PARTITIONS {
    PARTITION1 = 0,
    PARTITION2,
    PARTITION3,
    PARTITION4,
    PARTITION5,
    PARTITION6,
    PARTITION7,
    PARTITION8,
    MAX_PARTITION = PARTITION8,
};
//
enum  ARM_METHODS_t {
    DISARM = 0,
	REGULAR_ARM = 0x1,
	FORCE_ARM 	= 0x2,
    INSTANT_ARM = 0x4,                                    
    STAY_ARM    = 0x8,
};
//
enum  ARM_RESTRICTIONS_t {
    RESTRICT_ON_SUPERVISOR_LOSS = 0x1,
    RESTRICT_ON_TAMPER          = 0x2,                                    // default ON
    RESTRICT_ON_AC_FAILURE      = 0x4,
    RESTRICT_ON_BATTERY_FAILURE = 0x8,                                    // default ON
    RESTRICT_ON_BELL         	= 0x10, 
	RESTRICT_ON_SLAVE 			= 0x20,
    RESTRICT_ON_ANTIMASK        = 0X40,
};
//
// Keysw
//
enum  KEYSW_OPTS_t {
    ENABLED 				= 0x1,                            
    MAINTAINED           	= 0x2, 				 // MOMENTARY = 0; MAINTAINED = 1  
    GEN_UTL_KEY_ON_OPEN_AND_CLOSE = 0x4,         // GEN_UTL_KEY_ON_OPEN_AND_CLOSE = 1; ON_OPEN_ONLY = 0;
};
//
enum  KEYSW_ACTS_t {
    DISARM_ONLY	= 0x1,                            
    STAY_INSTANT_DISARM_ONLY, 				 
    ARM_ONLY,
	REGULAR_ARM_ONLY,
	STAY_ARM_ONLY,
	FORCE_ARM_ONLY,
	INSTANT_ARM_ONLY,
};
//
enum entryDelay_t {
	NOT_STARTED,
	RUNNING,
	DONE,
};
//
#define NEW_DATA_BIT        0x1					// actual NEW_DATA_BIT will 0x1 shifted left by  board ID 
//
//
// alarm zones records structure to hold all alarm zones related info
//
struct ALARM_ZONE {
  byte  valid;					// data valid	
  byte  bypassed;			    // true if zone is bypassed
  byte  zoneStat;               // status of the zone switch. (open, close, short, line break
  byte  zoneType;				// zone type - enable, entry delay, follow, instant, stay, etc
  byte  zonePartition;          // assigned to partition X
  byte  zoneOptions;            // auto shutdown, bypass, stay, force, alarm type, intellyzone, dealyed transission
  byte  zoneExtOpt;             // zone tamper, tamper supervision, antimask, antimask supervision
  //byte	lastZoneStat;			// last zone status reported
  unsigned long reportedAt;		// the time when the zone status was reported
  char  zoneName[16];           // user friendly name
};        
//
// alarm pgms records structure to hold all alarm pgms related info
//
struct ALARM_PGM {
  byte  valid; 					// data valid	
  byte  iValue;             	// initial value
  byte  cValue;             	// current
  char  pgmName[16];           	// user friendly name
};
//
// keyswitch related staff
//
struct ALARM_KEYSW {
  byte  partition;				// Keyswitch can be assigned to one partition only. If == NO_PARTITION (0) the keyswitch is not defined/valid
  byte  type;					// disabled, momentary, maintained,  generate utility key on open/close, .... see enum  KEYSW_OPTS_t 
  byte  action;					// keyswitch action definition - see enum  KEYSW_ACTS_t 
  byte	boardID;				// the board of which zone will e used as keyswitch belong. Master ID is 0	
  byte  zoneID;                 // the number of zone that will e used as keyswitch
  char  keyswName[16];          // user friendly name
};  
//
struct ALARM_GLOBAL_OPTS_t {
	byte maxSlaveBrds;				// how many slaves are installed. Run-time this value is copied to maxSlaves
	int  armRestrictions;			// all DBs are sized to MAX_SLAVES compile time, means maxSlaveBrds =< maxSlave !!!!
	byte troubleLatch;				// if trouble, latch it or not
	byte tamperBypassEn;			// true - if zone is bypassed ignore tamper; false - follow global or local tamper settings
	byte tamperOpts;				// global tamper optons, same as local - see #define ZONE_TAMPER_OPT_XXXXXX
	byte antiMaskOpt;				// global antimask optons, same as local - see #define ZONE_ANTI_MASK_SUPERVISION__XXXXXX
	byte rfSupervisionOpt;			// wireless sensors supervision see RF_SUPERVISION_XXXX
	unsigned long entryDelay1Start;	//to store the time when entry delay 1 zone opens
	unsigned long entryDelay2Start; //to store the time when entry delay 2 zone opens
};
//
// alarm partition 
//
struct ALARM_PARTITION_t {
// configuration data
	byte armStatus;					//  bitmask, DISARM = 0, REGULAR_ARM, FORCE_ARM, INSTANT_ARM, STAY_ARM 
	byte forceOnRegularArm;			// allways use force arm (bypass open zones) when regular arming
	byte forceOnStayArm;			// allways use force arm (bypass open zones) when stay arming
	byte followZone2entryDelay2;	// if and entry delay zone is bypassed and follow zone is opens, the alarm will be postponed by EntryDelay2 
	byte notBypassedEntyDelayZones; // used to triger follow zones to use ENTRY_DELAY2 if no more notBypassedEntyDelayZones
	byte alarmOutputEn;				// enable to triger bell or siren once alarm condition is detected in partition
	byte alarmCutOffTime;			// cut alarm output after 1-255 seconds
	byte noCutOffOnFire;			// disable cut-off for fire alarms
	byte alarmRecycleTime;			// re-enable after this time if alarm condition not fixed
	byte entryDelay1Interval;		// entry delay 1 delay in seconds 1-255
	byte entryDelay2Interval;		// entry delay 2 delay in seconds 1-255
	byte exitDelay;					// exit delay in seconds 1-255
	byte partitionName[16];			// user readable name
	byte follows[MAX_PARTITION];	// array with positional info, if position x != 0 than this partition follows parititon x
// run-time statistics  TODO - spit config and rt data in separate objects
	unsigned long armTime;			// arm time
	unsigned long entryDelay1;		// entry delay 1 delay start time in mS
	unsigned long entryDelay2;		// entry delay 2 delay start time in mS
	byte entryDelayFSM;				// current state of entry delay FSM. States are: NOT_STARTED, RUNNING, DONE
	byte openZonesCnt;				// count of open zones
	byte bypassedZonesCnt;			// count of bypassed zones
	byte tamperZonesCnt;			// count of zones with tamper and anti-mask 
	byte ignorredTamperZonesCnt;	// count of ignorred zones with tamper and anti-mask 
};
//
// zoneDB - database with all zones (master&slaves) info. Info from slaves are fetched via pul command over RS485
// TODO - use prep to get largest zone count
// All alarm zones zones organized as 2D array - [board][zones]. Contains data for all boards and zones in each board, incl. master
//
typedef struct ALARM_ZONE alarmZoneArr_t[MAX_SLAVES+1][MAX_ALARM_ZONES_PER_BOARD]; // every two zones here report for two contacts connected to one ADC channel
alarmZoneArr_t zonesDB;
//
//
// MASTER PGMs organized as 2D array. All pgms zones organized as 2D array - [board][pgms].
//
typedef struct ALARM_PGM alarmPgmArr_t[MAX_SLAVES+1][MAX_PGM_CNT];		        // typically master has more pgms than slave, so we use the largest denominator
alarmPgmArr_t pgmsDB;
//
// alarm keysw records structure to hold all alarm pgms related info
typedef struct ALARM_KEYSW alarmKeyswArr_t[MAX_KEYSW_CNT];		        
alarmKeyswArr_t keyswDB;
//
// alarm global options storage
struct ALARM_GLOBAL_OPTS_t  alarmGlobalOpts;
//
// alarm partition options storage
typedef struct ALARM_PARTITION_t alarmPartArr_t[MAX_PARTITION];
alarmPartArr_t partitionDB;
//
// Struct to store the all alarm configuration
//
struct CONFIG_t {
  byte  version;
  byte  zoneConfig[sizeof(zonesDB)];
  byte  pgmConfig[sizeof(pgmsDB)];
  byte  keyswConfig[sizeof(keyswDB)];
  byte  alarmOptionsConfig[sizeof(alarmGlobalOpts)];
  byte  alarmPartConfig[sizeof(partitionDB)];
  byte  csum;
} alarmConfig, tmpConfig;

