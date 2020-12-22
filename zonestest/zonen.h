#define TEST 1
#include "gpio-def.h"       // include gpio and zones definitions and persistant storage
//
//#define AltZoneSelect	      0x1			  // selects aternative zones via 4053 mux
#define muxCtlPin           GPIO0     // 4053 mux control GPIO
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
enum timeoutOper {
  SET = 1,
  GET = 2,
};
// 
// set timeout / check if timeout expired
// TODO - organize all timeouts as separate database similar to commands
// 
bool timeout(int oper, int whichOne) {
static unsigned long zonesAinterval = 0;
static unsigned long zonesBinterval = 0;
static unsigned long muxSetInterval = 0;
//
//
  if (oper == SET) {                     // remenber the current time in milliseconds
    switch (whichOne) {
      case ZONES_A_READ_INTERVAL:
        zonesAinterval = millis();
        break;
      case ZONES_B_READ_INTERVAL:
        zonesBinterval = millis();
        break;
      case MUX_SET_INTERVAL:
        muxSetInterval = millis();
        break;
      default:
        break;
      }
    }  
  else {
    switch (whichOne) {
      case ZONES_A_READ_INTERVAL:
        return ((unsigned long)(millis() - zonesAinterval) > (unsigned long)ZONES_A_READ_INTERVAL);
      case ZONES_B_READ_INTERVAL:
        return ((unsigned long)(millis() - zonesBinterval) > (unsigned long)ZONES_B_READ_INTERVAL);
      case MUX_SET_INTERVAL:
        return ((unsigned long)(millis() - muxSetInterval) > (unsigned long)MUX_SET_INTERVAL);
      default:
        break;
      }
    }  
  return false;
}
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

unsigned long zoneTest[64];
//
void selectZones(int which) {
  digitalWrite(muxCtlPin, which);
  logger.printf("Switching mux to %s channel\n", (which?"Azones":"Bzones"));
}
//
//  fill zone data for test 
//  parms: struct ZONE DB[]  - (pointer ???) to array of ZONE  containing the zones to be read and converted
//
void fillZones(unsigned long zoneTest[], int zones_cnt) { 
    int i; 
    int increment;
    increment = 4096/zones_cnt;
    logger.printf ("Zone filled data: ");
    for (i = 0; i <  zones_cnt; i++) {               // iterate
       zoneTest[i] = i*increment+increment/3;
       logger.printf ("%d ", zoneTest[i]);
    }
    logger.printf ("\n");
    //zoneTest[0] = 4000;
}
//
// add to arduino setup func
//
void zoneSetup() {
	pinMode (muxCtlPin, OUTPUT);			// set mux ctl as output
	selectZones	(Azones);					// select A zones
	//set adc channels???
   if(TEST)                                                  // prepare test data
#ifdef MASTER
    fillZones(zoneTest, MASTER_ZONES_CNT);
#endif
#ifdef SLAVE
    fillZones(zoneTest, SLAVE_ZONES_CNT);
#endif
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
       logger.printf ("Zone data: Zone ID: %d: GPIO: %2d: \tAvg ADC Value: %lu\tAvg mV val: %4.3f mV;\tZoneABstat: %x\n", DB[i].zoneID, DB[i].gpio, DB[i].accValue, DB[i].mvValue, DB[i].zoneABstat );
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
void readZones(struct ZONE DB[], int zones_cnt, int mux) {              
  int i, j; 
  unsigned long val;
//
  for (i = 0;  i < zones_cnt; i++) 
     if(mux == DB[i].mux)   {                           // init only zones selected by mux
       DB[i].accValue = 0;                              // init only records that will be read 
       //logger.printf(" Zeroing zone %d\t GPIO: %d\n", i , DB[i].gpio);
       }
  for (j = 0; j< OVERSAMPLE_CNT; j++) {                 // will read and accumulate values for each zone  OVERSAMPLE_CNT times
     for (i = 0;  i < zones_cnt; i++) {                 // read zone and store value
        //logger.printf("Reading zone index %d; mux %d; gpio %d; value %d;\n",i, mux, DB[i].gpio, DB[i].accValue ); 
        if(mux != DB[i].mux)                            // skip the zones not selected by mux     
          continue;                                     // but the one under consideration is not muxed
        // read and convert here
        if(TEST)                                        // skip reading in test mode as the data are filled already by fillZones()
          val = zoneTest[i];                            // accumulate
        else
          val = analogRead(DB[i].gpio);                 // read from ADC
        DB[i].accValue = DB[i].accValue+val;            // accumulate
        //if((j==0))                                        // print once
          //logger.printf("Reading zone %d: gpio: %d; mux %d; Acc val = %d; Current val: %d\n",DB[i].zoneID, DB[i].gpio, mux,  DB[i].accValue, val); 
        }                                               // zones loop
     }                                                  // oversample loop
    // convert values to voltages
    for (i = 0; i <  zones_cnt; i++) {    // convert acumulated values to voltages 
      DB[i].mvValue = convert2mV(DB[i].accValue);    // full scale (4096) represent 3.2V, and value is oversampled 8 times
      //logger.printf ("Converted zone zone %d: GPIO: %2d \tAvg ADC Value %lu = \t%4.3f mV\n",DB[i].zoneID, DB[i].gpio, DB[i].accValue, DB[i].mvValue );
    }
}
//
// read and convert zone adc values to mV first and after to zone status
// allows to read A/B zones (or Azones/system voltages with different timing, as we don't need to update the system voltages so often
// supports mux set interval before read in order to allow analog inputs to settle
//
void convertZones(struct ZONE DB[], int zoneCnt, byte zoneResult[]) {
  int i = 0;
  int thrIndex; 
  static int  muxState = Azones;
//
  // read zones analog value   
  if(!timeout(GET, MUX_SET_INTERVAL))
    return;                                                 // can't do anything, wait for analog inputs to settle
  unsigned long lastRead = millis();
  if(muxState == Azones) {
    if(timeout(GET, ZONES_A_READ_INTERVAL)) {               // time to read A zones
      logger.printf("Reading A zones\n");
      readZones(DB, zoneCnt, muxState);                     // do read
      timeout(SET, ZONES_A_READ_INTERVAL);                  // remember when
      }
    else if(timeout(GET, ZONES_B_READ_INTERVAL)) {               // time to read B zones?
      timeout(SET, MUX_SET_INTERVAL);                       // start mux settle time
      logger.printf("Mux B timeout started\n");
      selectZones(muxState = Bzones);                       // switch mux
      }
    else
      return;
  }
  else {                                                    // zones B are selected and mux set interval expired                   
    logger.printf("Reading B zones\n");
    readZones(DB, zoneCnt, muxState);                       // do read
    timeout(SET, ZONES_B_READ_INTERVAL);                    // remember when    
    logger.printf("Mux A timeout started\n");
    selectZones(muxState = Azones);   
    }

  logger.printf ("Elapsed time %d millisec\n", (unsigned long)(millis() - lastRead));
  // convert all zones ADC values to encodded binary value here
  for (i = 0; i < zoneCnt; i++) {                             // for all zones in the array
    for (thrIndex = 0; DB[i].mvValue > thresholds[thrIndex].tMin; thrIndex++) // look-up the input voltage  in the input voltage ranges
      ;                                                       // keep searching while the input voltage is lower than min input voltage for the range
    thrIndex--;                                               // correct the index to point to the exact range
    DB[i].zoneABstat = thresholds[thrIndex].zoneABstat;   // get zones A&B status  code   
    }                                                         // loop over zones
  //printZones(DB, zoneCnt);
  // time to copy results to results array
  for (i=0; i < zoneCnt; i++) {                             // combine two zones in one byte, 12, 34, 56, ....
    if(!(i%2))
      zoneResult[i/2] = 0;                                    // clear results array
    zoneResult[i/2] = ((zoneResult[i/2] << ZONE_ENC_BITS) & ~ZONE_ENC_MASK) | DB[i].zoneABstat ;
    //if(i%2 || i==zoneCnt-1)
      //logger.printf("Payload: %d content: %2x\n", i/2, zoneResult[i/2]);
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
