#define TEST 1


#define Azones			        HIGH
#define Bzones              LOW
#define SYSTEM_VOLTAGES     LOW
#define AltZoneSelect	      0x1			  // selects aternative zones via 4053 mux
#define muxCtlPin           GPIO0     // 4053 mux control GPIO
#define selectZones(which)  digitalWrite(muxCtlPin, which)
#define OVERSAMPLE_CNT      8
#define ZONE_ERROR_SHORT    0x4
#define ZONE_ERROR_OPEN     0x8
#define ZONE_A_OPEN         0x1   
#define ZONE_A_CLOSED       0x0
#define ZONE_B_OPEN         0x2   
#define ZONE_B_CLOSED       0x0
#define ZONE_ENC_BITS       4
#define ZONE_ENC_MASK       0xF
//
// struct to hold zone voltages ranges and corresponding binary code
//
struct THRESHOLD {
  unsigned tMin;                     // level in mV, TODO change to ADC code
  unsigned tMax;
  byte     zoneABstat;
};
//
// array to hold all possible zone voltages
// zone codes ERROR_FLAG (b10)  | ZONE OPEN or ZONE CLOSED
struct THRESHOLD thresholds[] = {{0,    450,    ZONE_ERROR_SHORT  | ZONE_ERROR_SHORT},// below 450mV; LINE SHORT, zoneA error, zoneB error LINE SHORT
                                 {451,  1300,   ZONE_A_CLOSED     | ZONE_B_CLOSED},   // from 451 to 1300mV;  zoneA closed, zoneB closed
                                 {1301, 2000,   ZONE_A_OPEN       | ZONE_B_CLOSED},     // from 1301 to 2000mV; zoneA closed, zoneB open
                                 {2001, 2400,   ZONE_A_CLOSED     | ZONE_B_OPEN},   // from 2001 to 2400mV; zoneA open,   zoneB closed
                                 {2401, 3000,   ZONE_A_OPEN       | ZONE_B_OPEN},     // from 2401 to 3000mV; zoneA open,   zoneB open
                                 {3001, 3500,   ZONE_ERROR_OPEN   | ZONE_ERROR_OPEN}};// above 3001mV; zoneA error LINE OPEN , zoneB error LINE OPEN 
//
// zone records structure to hold all zone related info
struct ZONE {
  byte gpio;
  byte mux;                     // 1 - activate mux to read, 0 - read direct
  unsigned long accValue;				// oversampled value
  float mvValue;								// converted value in mV
  byte  zNum;                   // the number of zone by which the master will identify it. As each zone supports two channels, zNum must be multiple of 2
  byte  zoneABstat;             // encodded status of the A and B parts of the zone
};                              

#include "gpio-def.h"       // include gpio and zones definitions and persistant storage

unsigned long zoneTest[64];

//
//  fill zone data for test 
//  parms: struct ZONE DB[]  - (pointer ???) to array of ZONE  containing the zones to be read and converted
//
void fillZones(unsigned long zoneTest[], int zones_cnt) { 
    int i; 
    int increment;
    increment = 4096/zones_cnt;
    for (i = 0; i <  zones_cnt; i++) {               // iterate
       zoneTest[i] = i*increment+increment/3;
       logger.printf ("Zone filled data %d\n", zoneTest[i]);
    }
    //zoneTest[0] = 4000;
}
//
// add to arduino setup func
//
void zoneSetup() {
	pinMode (muxCtlPin, OUTPUT);			// set mux ctl as output
	selectZones	(Azones);					// select A zones
	//set adc channels???
  logger.printf("Nr of zones defined = %d\n");        //, ZONES_CNT);
}
//
//convert oversampled ADC value to mV
//
float convert2mV (unsigned long adcVal) {
    return float (((adcVal*3200)/4096)/OVERSAMPLE_CNT);
}
//
//  print single zone data
//  parms: struct ZONE DB[]  - (pointer ???) to array of ZONE  containing the zones to be read and converted
//
void printZones(struct ZONE DB[], int zones_cnt) { 
    int i; 
    for (i = 0; i <  zones_cnt; i++) {                        // iterate
       logger.printf ("Zone data: Zone Nr: %d: GPIO: %2d: \tAvg ADC Value: %lu\tAvg mV val: %4.3f mV;\tZoneABstat: %x\n", DB[i].zNum/2, DB[i].gpio, DB[i].accValue, DB[i].mvValue, DB[i].zoneABstat );
    }
}
//
//  Convert zone analog value (mV or ADC code)  to binary code using look-up in thresholds array. Store in zoneABcode field
//
void zoneVal2Code(struct ZONE DB[], int zoneCnt) {
  int i, thrIndex;
  for (i = 0; i < zoneCnt; i++){                              // for all zones in the array
    for (thrIndex = 0; DB[i].mvValue > thresholds[thrIndex].tMin; thrIndex++) // look-up the input voltage  in the input voltage ranges
      ;                                                       // keep searching while the input voltage is lower than min input voltage for the range
    thrIndex--;                                               // correct the index to point to the exact range
    DB[i].zoneABstat = thresholds[thrIndex].zoneABstat;       // get zones A&B status  code   
    } 
}
//
//  Read and convert all zone inputs
//  parms: struct ZONE DB[]  - (pointer ???) to array of ZONE  containing the zones to be read and converted
//  NOTE: if mux is set, only muxed zones will be read and convert, if mux == false, only non-muxed zones will be read
//
void readZones(struct ZONE DB[], int zones_cnt, bool mux) {              
  int i, j; 
  unsigned long val;
  for (i = 0;  i < zones_cnt; i++) 
     if(!mux == !DB[i].mux)   {                         
       DB[i].accValue = 0;                              // init only records that will be read 
       //logger.printf(" Zeroing zone %d\t GPIO: %d\n", i , DB[i].gpio);
     }
  for (j = 0; j< OVERSAMPLE_CNT; j++) {                 // will read and accumulate values for each zone  OVERSAMPLE_CNT times
     for (i = 0;  i < zones_cnt; i++) {                 // read zone and store value
        //logger.printf(" Reading zone index %d gpio %d value %d\n",i, DB[i].gpio, DB[i].accValue ); 
        if(!mux != !DB[i].mux)                          // logical exclusive, read only the zones selected by mux     
          continue;                                     // but the one under consideration is not muxed
        // read and convert here
        if(TEST)                                        // skip reading in test mode as the data are filled already by fillZones()
          val = zoneTest[i];                            // accumulate
        else
          val = analogRead(DB[i].gpio);                 // read from ADC
        DB[i].accValue = DB[i].accValue+val;            // accumulate
        //logger.printf("Reading zone %d: gpio: %d Acc val = %d; Current val: %d\n",DB[i].zNum/2, DB[i].gpio,  DB[i].accValue, val); 
        }                                               // zones loop
     }                                                  // oversample loop
    // convert values to voltages
    for (i = 0; i <  zones_cnt; i++) {    // convert acumulated values to voltages 
      DB[i].mvValue = convert2mV(DB[i].accValue);    // full scale (4096) represent 3.2V, and value is oversampled 8 times
      logger.printf ("Converted zone zone %d: GPIO: %2d \tAvg ADC Value %lu = \t%4.3f mV\n",DB[i].zNum/2, DB[i].gpio, DB[i].accValue, DB[i].mvValue );
    }
}
//
// read and convert zone adc values to mV first and after to zone status
//
void convertZones(struct ZONE DB[], int zoneCnt, byte zoneResult[]) {
  int i = 0;
  static unsigned long lastRead = 0;
  int thrIndex; 
  if(TEST)                                                  // prepare test data
   fillZones(zoneTest, zoneCnt);
	// read zones analog value 
	if (ZONES_READ_THROTTLE)  {                  		          // time to read??
		unsigned long temp = millis();
		if ((unsigned long)(temp - lastRead) < (unsigned long)ZONES_READ_INTERVAL) {
		  //logger.printf("Cur %u, last %u, interval %u\n", temp, lastRead,ZONES_READ_INTERVAL );
		  return;   
		}
	}     
  lastRead = millis();                                        // record read time
	selectZones(Azones);
	logger.printf("Switching mux to A channel\n");
	readZones(DB, zoneCnt, false);                              // reads  and accumulates OVERSAMPLE_CNT times all zones in zone DB with mux chan A
	//selectZones(Bzones);                                      // toggle GPIO to select mux channel B                       								                     
	//logger.printf("Switching mux to B channel\n");
	//readZones(DB, zoneCnt, true);                             // now read zones with mux switched to mux channel B
	//selectZones(Azones);								                      // toggle GPIO to select A mux channel again
  logger.printf ("Elapsed time %d millisec\n", (unsigned long)(millis() - lastRead));
  // convert all zones ADC values to encodded binary value here
  for (i = 0; i < zoneCnt; i++) {                             // for all zones in the array
    for (thrIndex = 0; DB[i].mvValue > thresholds[thrIndex].tMin; thrIndex++) // look-up the input voltage  in the input voltage ranges
      ;                                                       // keep searching while the input voltage is lower than min input voltage for the range
    thrIndex--;                                               // correct the index to point to the exact range
    DB[i].zoneABstat = thresholds[thrIndex].zoneABstat;   // get zones A&B status  code   
    }                                                         // loop over zones
  printZones(DB, zoneCnt);
  // time to copy results to results array
  for (i=0; i < zoneCnt; i++) {                             // combine two zones in one byte, 12, 34, 56, ....
    if(!(i%2))
      zoneResult[i/2] = 0;                                    // clear results array
    zoneResult[i/2] = ((zoneResult[i/2] << ZONE_ENC_BITS) & ~ZONE_ENC_MASK) | DB[i].zoneABstat ;
    if(i%2 || i==zoneCnt-1)
      logger.printf("Payload: %d content: %2x\n", i/2, zoneResult[i/2]);
    }  
  lastRead = millis();
}
//
//  PGM control code here
//
void pgmSetup(struct PGM pgmDB[], const int pgmCnt) {
  for(int i =0; i < pgmCnt; i++) {                   // for each PGM 
    pinMode (pgmDB[i].gpio, OUTPUT);                  // set GPIO as output
    digitalWrite(pgmDB[i].gpio, pgmDB[i].iValue);     // set initial value
    pgmDB[i].cValue = pgmDB[i].iValue;
  }
}
//
// set PGM
//
void setPgm(struct PGM pgmDB[], byte idx, bool val, const int pgmCnt) {
  for (int i = 0; i < pgmCnt; i++) {
    //logger.printf("Looking at index  %d out of  %d:\n", idx, pgmCnt-1);
    if(idx == pgmDB[i].rNum)  
      digitalWrite(pgmDB[i].gpio, val);               // set output value
    } 
}
//
// get PGM
//
bool getPgm(struct PGM pgmDB[], byte idx, const int pgmCnt) {
  for (int i = 0; i < pgmCnt; i++) {
    //logger.printf("Looking at index  %d out of  %d:\n", idx, pgmCnt-1);
    if(idx == pgmDB[i].rNum) 
      return pgmDB[i].cValue;               // read output value
    }
}
