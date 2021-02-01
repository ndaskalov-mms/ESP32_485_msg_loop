#define muxCtlPin           GPIO0     // 4053 mux control GPIO
#define OVERSAMPLE_CNT      8
#define ZONE_ERROR_SHORT    0x4
#define ZONE_ERROR_OPEN     0x8
#define ZONE_ERROR_MASK     0xC
#define ZONE_ERROR_SHIFT    0x2
#define ZONE_A_OPEN         0x1   
#define ZONE_A_CLOSED       0x0
#define ZONE_A_MASK         0x1
#define ZONE_A_SHIFT        0
#define ZONE_B_OPEN         0x2   
#define ZONE_B_CLOSED       0x0
#define ZONE_B_MASK         0x2
#define ZONE_B_SHIFT        1
#define ZONE_ENC_BITS       4
#define ZONE_ENC_MASK       0xF
#define ZONE_A_VALID		1
#define ZONE_B_VALID		2