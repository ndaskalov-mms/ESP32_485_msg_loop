#define GPIO35	35
#define GPIO32	32
#define GPIO33	33
#define GPIO25	25
#define GPIO26	26
#define GPIO27	27
#define GPIO14	28
#define GPIO12	29
#define GPIO13	13
#define GPIO15	15
#define GPIO2 	2
#define GPIO4	  4
#define GPIO36	36
#define GPIO39 	39
#define GPIO34	34
#define GPIO0   0
#define GPIO5   5
#define GPIO19  19
#define GPIO23  23

#define VzoneRef_   GPIO36  // ADC1_CH0               // 4053 mux with zone1A_  - use selectZones(SYSTEM_VOLTAGES) to read
#define ADC_AUX_    GPIO39  // ADC1_CH3               // 4053 mux with zone2A_  - use selectZones(SYSTEM_VOLTAGES) to read
#define ADC_BAT_    GPIO34  // ADC1_CH6               // 4053 mux with zone3A_  - use selectZones(SYSTEM_VOLTAGES) to read
#define Zone1A_     GPIO36  // ADC1_CH0               // 4053 mux with VzoneRef_  - default, use selectZones(Azones) to read 
#define Zone2A_     GPIO39  // ADC1_CH3               // 4053 mux with ADC_AUX    - default, use selectZones(Azones) to read  
#define Zone3A_     GPIO34  // ADC1_CH6               // 4053 mux with ADC_BAT    - default, use selectZones(Azones) to read 
#define Zone1B_     GPIO36  // ADC1_CH0               // jumper mux with VzoneRef_
#define Zone2B_     GPIO39  // ADC1_CH3               // jumper mux with ADC_AUX
#define Zone3B_     GPIO34  // ADC1_CH6               // jumper mux with ADC_BAT
#define Zone1_      GPIO35  // ADC1_CH7 
#define Zone2_      GPIO32  // ADC1_CH4 
#define Zone3_      GPIO33  // ADC1_CH5 
#define Zone4_      GPIO25  // ADC2_CH8 
#define Zone5_      GPIO26  // ADC2_CH9 
#define Zone6_      GPIO27  // ADC2_CH7 
#define Zone7_      GPIO14  // ADC2_CH6 
#define Zone8_      GPIO12  // ADC2_CH5 
#define Zone9_      GPIO13  // ADC2_CH4 
#define Zone10_     GPIO15  // ADC2_CH3 
#define Zone11_     GPIO2   // ADC2_CH2 
#define Zone12_     GPIO4   // ADC2_CH0 
//
// PGM assignment
//
#define PGM1_      GPIO5
#define PGM2_      GPIO19
#ifdef  MASTER
#define PGM3_      Zone4_
#define PGM4_      Zone5_
#define PGM5_      Zone6_
#define PGM6_      Zone7_
#define PGM7_      Zone9_
#define PGM8_      Zone12_
// slave power control pin
#define SLAVE_PWR_CTL      GPIO23
#define SLAVE_PWR_ON       LOW
#define SLAVE_PWR_OFF      HIGH
#endif

//
// outputs database to store outputs (relay's) parameters and status
//
struct PGM {
  byte  gpio;               // gpio to be used for control of the relay 
  byte  rNum;               // the number of output (relay)  by which the master will identify it. 
  byte  iValue;             // initial value
  byte  cValue;             // current
};     


#ifdef SLAVE
namespace slave
{
//
// zones database to store data
//                              gpio, mux,  accValue,  mvValue, zNum, zoneABstat
struct ZONE zoneDB[] =        {{Zone1_ , 0,   0, 0, 0*2, 0}, {Zone2_ , 0, 0, 0, 1*2, 0}, {Zone3_ , 0, 0, 0, 2*2, 0},\
                               {Zone4_ , 0,   0, 0, 3*2, 0}, {Zone5_ , 0, 0, 0, 4*2, 0}, {Zone6_ , 0, 0, 0, 5*2, 0},\
                               {Zone7_ , 0,   0, 0, 6*2, 0}, {Zone8_ , 0, 0, 0, 7*2, 0}, {Zone9_ , 0, 0, 0, 8*2, 0},\
                               {Zone10_, 0,   0, 0, 9*2, 0}, {Zone11_, 0, 0, 0,10*2, 0}, {Zone12_, 0, 0, 0,11*2, 0},\
                               {Zone1A_, 0,   0, 0,12*2, 0}, {Zone2A_, 0, 0, 0,13*2, 0}, {Zone3A_, 0, 0, 0,14*2, 0}};           
// these are muxed zones -->   {Zone1B_, 1,   0, 0,15*2, 0}, {Zone2B_, 1, 0, 0,16*2, 0}, {Zone3B_, 1, 0, 0,17*2, 0}};//accessible with altZoneSelect
//
const int zonesCnt =       (sizeof(zoneDB)/sizeof(struct ZONE));
byte zoneResult[(sizeof(zoneDB)/2 + sizeof(zoneDB)%2)];
//
// outputs database to store data
//                              gpio, rNum,  value
struct PGM pgmDB[] =            {{PGM1_, 1, HIGH, 0}, {PGM2_, 2, HIGH, 0}
#ifdef MASTER
//                                gpio,   rNum, initial value, current value
                               , {PGM3_, 3, HIGH, 0}, {PGM4_, 4, HIGH, 0}
                               , {PGM5_, 5, HIGH, 0}, {PGM6_, 6, HIGH, 0}
                               , {PGM7_, 7, HIGH, 0}, {PGM8_, 8, HIGH, 0}
#endif                              
                                }; // <- this semicolon is to finalize array initializaton
//
const int pgmCnt = (sizeof(pgmDB)/sizeof(struct PGM));
}  // end of namespase
#endif
#ifdef MASTER
namespace master
{
//
// zones database to store data
//                              gpio, mux,  accValue,  mvValue, zNum, zoneABstat
struct ZONE zoneDB[] =        {{Zone1_ , 0,   0, 0, 0*2, 0}, {Zone2_ , 0, 0, 0, 1*2, 0}, {Zone3_ , 0, 0, 0, 2*2, 0},\
                               {Zone4_ , 0,   0, 0, 3*2, 0}, {Zone5_ , 0, 0, 0, 4*2, 0}, {Zone6_ , 0, 0, 0, 5*2, 0},\
                               {Zone7_ , 0,   0, 0, 6*2, 0}, {Zone8_ , 0, 0, 0, 7*2, 0}, {Zone9_ , 0, 0, 0, 8*2, 0},\
                               {Zone10_, 0,   0, 0, 9*2, 0}, {Zone11_, 0, 0, 0,10*2, 0}, {Zone12_, 0, 0, 0,11*2, 0},\
                               {Zone1A_, 0,   0, 0,12*2, 0}, {Zone2A_, 0, 0, 0,13*2, 0}, {Zone3A_, 0, 0, 0,14*2, 0}};           
// these are muxed zones -->   {Zone1B_, 1,   0, 0,15*2, 0}, {Zone2B_, 1, 0, 0,16*2, 0}, {Zone3B_, 1, 0, 0,17*2, 0}};//accessible with altZoneSelect
//
int zonesCnt =       (sizeof(zoneDB)/sizeof(struct ZONE));
byte zoneResult[(sizeof(zoneDB)/2 + sizeof(zoneDB)%2)];
//
// outputs database to store data
//                              gpio, rNum,  value
struct PGM pgmDB[] =            {{PGM1_, 1, HIGH, 0}, {PGM2_, 2, HIGH, 0}
#ifdef MASTER
//                                gpio,   rNum, initial value, current value
                               , {PGM3_, 3, HIGH, 0}, {PGM4_, 4, HIGH, 0}
                               , {PGM5_, 5, HIGH, 0}, {PGM6_, 6, HIGH, 0}
                               , {PGM7_, 7, HIGH, 0}, {PGM8_, 8, HIGH, 0}
#endif                              
                                }; // <- this semicolon is to finalize array initializaton
//
int pgmCnt = (sizeof(pgmDB)/sizeof(struct PGM));
}  // end of namespase
#endif

//                                     
// some important voltages 
struct ZONE VzoneRef   = {VzoneRef_, 1, 0, 0, 0, 0};
struct ZONE ADC_AUX   = {ADC_AUX_,  1, 0, 0, 0, 0};
struct ZONE ADC_BAT   = {ADC_BAT_,  1, 0, 0, 0, 0};
