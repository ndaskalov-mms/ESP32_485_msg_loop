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
#define Azones              HIGH
#define Bzones              LOW
#endif
//
// PGM database record to store PGM outputs (relay's) parameters and status
//
struct PGM {
  byte  gpio;               // gpio to be used for control of the relay 
  byte  rNum;               // the number of output (relay)  by which the master will identify it. 
  byte  iValue;             // initial value
  byte  cValue;             // current
};     
//
// zone records structure to hold all zone related info
//
struct ZONE {
  byte gpio;
  byte mux;                     // 1 - activate mux to read, 0 - read direct
  unsigned long accValue;       // oversampled value
  float mvValue;                // converted value in mV
  byte  zoneID;                 // the number of zone by which the master will identify it. Zero based. Each ADC gpio produces one zone, but with two results
  byte  zoneABstat;             // encodded status of the A and B parts of the zone
};                              

//                                     
// some important voltages - gpio, mux,  accValue,  mvValue, zoneID, zoneABstat
struct ZONE VzoneRef   = {VzoneRef_, 1, 0, 0, 0, 0};
struct ZONE ADC_AUX   = {ADC_AUX_,  1, 0, 0, 0, 0};
struct ZONE ADC_BAT   = {ADC_BAT_,  1, 0, 0, 0, 0};
//
//
// Zones  and pgms database to store data
//   
//
#ifdef SLAVE  
// Zones 1A, 2A, 3A are read with Mux = Azones (0, default); Zones 1B, 2B, 3B  are read with Mux = Bzones (1) AND if selected by jumpers
// othervise SYSTEM VOLTAGES VzoneRef, ADC_AUX, ADC_BAT are read with  Mux = Bzones (1)
// that means that when Bzones selected by GPIO, Zone<X>B zones will read either VzoneRef, ADC_AUX, ADC_BAT or Zone<X>B depends on jumper settings
// ZONES                     gpio,   mux, accValue,  mvValue, zoneID, zoneABstat
struct ZONE SzoneDB[] =    {{Zone1_, Azones, 0, 0, 0, 0}, {Zone2_, Azones, 0, 0, 1, 0}, {Zone3_, Azones, 0, 0, 2, 0}, {Zone4_, Azones, 0, 0, 3, 0},\
                            {Zone5_, Azones, 0, 0, 4, 0}, {Zone6_, Azones, 0, 0, 5, 0}, {Zone7_, Azones, 0, 0, 6, 0}, {Zone8_, Azones, 0, 0, 7, 0},\
                            {Zone9_, Azones, 0, 0, 8, 0}, {Zone10_,Azones, 0, 0, 9, 0}, {Zone11_,Azones, 0, 0,10, 0}, {Zone12_,Azones, 0, 0,11, 0},\
                            {Zone1A_,Azones, 0, 0,12, 0}, {Zone2A_,Azones, 0, 0,13, 0}, {Zone3A_,Azones, 0, 0,14, 0},\            
                            {Zone1B_,Bzones, 0, 0,15, 0}, {Zone2B_,Bzones, 0, 0,16, 0}, {Zone3B_,Bzones, 0, 0,17, 0}}; 
//
#define SLAVE_ZONES_CNT  (sizeof(SzoneDB)/sizeof(struct ZONE))
//
// PGMs                         gpio, rNum,  iValue, cValue
struct PGM SpgmDB[] =          {{PGM1_, 1, HIGH, 0}, {PGM2_, 2, HIGH, 0}};
//
#define SLAVE_PGM_CNT (sizeof(SpgmDB)/sizeof(struct PGM))
#endif  //SLAVE
//
#ifdef MASTER
// Zones 1A, 2A, 3A are read with Mux = Azones (0, default); Zones 1B, 2B, 3B  are read with Mux = Bzones (1) AND if selected by jumpers
// othervise SYSTEM VOLTAGES VzoneRef, ADC_AUX, ADC_BAT are read with  Mux = Bzones (1)
//
//  ZONES                         gpio, mux,  accValue,  mvValue, zoneID, zoneABstat
struct ZONE MzoneDB[] =        {{Zone1_ , Azones,   0, 0, 0, 0}, {Zone2_ , Azones, 0, 0, 1, 0}, {Zone3_ , Azones, 0, 0, 2, 0},\
                                {Zone8_ , Azones,   0, 0, 3, 0}, {Zone10_, Azones, 0, 0, 4, 0}, {Zone11_, Azones, 0, 0, 5, 0},\
                                {Zone1A_, Azones,   0, 0, 6, 0}, {Zone2A_, Azones, 0, 0, 7, 0}, {Zone3A_, Azones, 0, 0, 8, 0},\           
                                {Zone1B_, Bzones,   0, 0, 9, 0}, {Zone2B_, Bzones, 0, 0,10, 0}, {Zone3B_, Bzones, 0, 0, 11, 0}};//accessible with altZoneSelect
//
#define MASTER_ZONES_CNT       (sizeof(MzoneDB)/sizeof(struct ZONE))
//
// PGMs                        gpio, rNum,  iValue, cValue
struct PGM MpgmDB[] =         {{PGM1_, 1, HIGH, 0}, {PGM2_, 2, HIGH, 0},\
                               {PGM3_, 3, HIGH, 0}, {PGM4_, 4, HIGH, 0},\
                               {PGM5_, 5, HIGH, 0}, {PGM6_, 6, HIGH, 0},\
                               {PGM7_, 7, HIGH, 0}, {PGM8_, 8, HIGH, 0}};
//
#define MASTER_PGM_CNT        (sizeof(MpgmDB)/sizeof(struct PGM))
#endif                      // MASTER
