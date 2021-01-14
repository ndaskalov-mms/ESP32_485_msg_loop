/*
//
// alarm zones records structure to hold all alarm zones related info
//
struct ALARM_ZONE {
  byte	boardID;				// the board which zones belong to. Master ID is 0	
  byte  zoneID;                 // the number of zone by which the master will identify it. Zero based. Each ADC gpio produces one zone, but with two results  
  byte  gpio;					// first members  are the same as struct ZONE
  byte  mux;                    // 1 - activate mux to read, 0 - read direct
  unsigned long accValue;       // oversampled value
  float mvValue;                // converted value in mV
  byte  zoneABstat;             // encodded status of the A and B parts of the zone
  byte  zoneDefs;				// zone type - enable, entry delay, follow, instant, stay, etc
  byte  zonePartition;          // assigned to partition X
  byte  zoneOptions;            // auto shutdown, nypass, stay, force, alarm type, intellyzone, dealyed transission
  byte  zoneExtOpt;             // zone tamper, tamper supervision, antimask, antimask supervision
};         
*/
//
// zone definitions
enum ZONE_DEFS_t {
    ZONE_DISABLED = 0,
    INSTANT,
    ENTRY_DELAY1,
    ENTRY_DELAY2,
    FOLLOW,
    H24_BUZZER,
    H24_BURGLAR,
    H24_HOLDUP,
    H24_GAS,
    H24_HEAT,
    H24_WATER,
    H24_FREEZE,
    DELAYED_24H_FIRE,
    STANDARD_24H_FIRE,
    STAY_DELAY1,
    STAY_DELAY2_ANTI_MASK,
};

enum  ZONE_OPTS_t {
    AUTO_ZONE_SHUTDN_EN = 0x1,
    BYPASS_EN           = 0x2,                                    // default ON
    STAY_ZONE           = 0x4,
    FORCE_EN            = 0x8,                                    // default ON
    ALARM_TYPE          = (0x10 | 0x20),
    INTELIZONE          = 0X40,
    DELAY_TRANSMISSION  = 0x80,
};
    
#define STEADY_ALARM    0x0
#define PULSED_ALARM    0X20
#define SILENT_ALARM    0x10
#define REPORTONLY_ALARM 0x30

enum ZONE_EXT_OPT_t {
    ZONE_TAMPER_OPT  = 0x1,
    ZONE_TAMPER_SUPERVISION = (0x2 | 0x4),
    ZONE_ANTI_MASK_TROUBLE  = 0x8,
    ZONE_ANTI_MASK_SUPERVISION = (0x10 | 0x20),
};

#define ZONE_TAMPER_OPT_DISABLED    0
#define ZONE_TAMPER_OPT_TROUBLE_ONLY  0x4
#define ZONE_TAMPER_OPT_ALARM_WHEN_ARMED  0x2
#define ZONE_TAMPER_OPT_ALARM  (0x4 | 0x2)

#define ZONE_ANTI_MASK_SUPERVISION_DISABLED    0x0
#define ZONE_ANTI_MASK_SUPERVISION_TROUBLE_ONLY   0X20
#define ZONE_ANTI_MASK_SUPERVISION_ALARM_WHEN_ARMED    0x10
#define ZONE_ANTI_MASK_SUPERVISION_ALARM   0x30

    
enum PARTITIONS {
    NO_PARTITION = 0,
    PARTITION1,
    PARTITION2,
    PARTITION3,
    PARTITION4,
    PARTITION5,
    PARTITION6,
    PARTITION7,
    PARTITION8,
    MAX_PARTITION,
};

