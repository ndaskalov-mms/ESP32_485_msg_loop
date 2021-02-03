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
//
#define ZONE_ANTI_MASK_SUPERVISION_DISABLED    0x0
#define ZONE_ANTI_MASK_SUPERVISION_TROUBLE_ONLY   0X20
#define ZONE_ANTI_MASK_SUPERVISION_ALARM_WHEN_ARMED    0x10
#define ZONE_ANTI_MASK_SUPERVISION_ALARM   0x30
//
//    
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
    RESTRICT_ON_TIMER           = 0x2,                                    // default ON
    RESTRICT_ON_AC_FAILURE      = 0x4,
    RESTRICT_ON_BATTERY_FAILURE = 0x8,                                    // default ON
    RESTRICT_ON_BELL         	= 0x10, 
	RESTRICT_ON_SLAVE 			= 0x20,
    RESTRICT_ON_ANTI_MASK       = 0X40,
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
	
