#define TEST 1
#include "gpio-def.h"       // include gpio and zones definitions and persistant storage
#include "zone-def.h"
extern void copyZonesStat(); //  to copy results to MzoneDB
//
enum timeoutOper {
  SET = 1,
  GET = 2,
};

enum TIMERS {
  ZONES_A_READ_TIMER = 1,
  ZONES_B_READ_TIMER = 2,
  MUX_SET_TIMER = 3,
  MASTER_ZONES_READ_TIMER = 4,
  ALARM_LOOP_TIMER = 5,
  };


// command records structure for cmdDB
struct TIMER {
  int timerID;
  unsigned long interval;
  unsigned long setAt;
};
//
// timerss database to look-up timer params TODO - move all timers staff to helpers or separate file. 
// 
struct TIMER timerDB[] = {{ZONES_A_READ_TIMER, ZONES_A_READ_INTERVAL,  0}, {ZONES_B_READ_TIMER, ZONES_B_READ_INTERVAL, 0},\
						  {MUX_SET_TIMER, MUX_SET_INTERVAL, 0}, 		   {MASTER_ZONES_READ_TIMER, MASTER_ZONES_READ_INTERVAL, 0},\
							{ALARM_LOOP_TIMER, ALARM_LOOP_INTERVAL, 0}} ;
//
int findTimer(byte timer) {
  //ErrWrite(ERR_DEBUG, "Looking for record for timer ID   %d \n", timer);
    for (int i = 0; i < sizeof(timerDB)/sizeof(struct TIMER); i++) {
    //lprintf("Looking at index  %d out of  %d:\n", i, sizeof(cmdDB)/sizeof(struct COMMAND)-1);
    if(timerDB[i].timerID == timer) {
      //ErrWrite(ERR_DEBUG,"Found timer at  index %d\n", i);
      return  i;
    }
  }
  ErrWrite(ERR_DEBUG, "Timer not found!!!!!!!\n");
  return ERR_DB_INDEX_NOT_FND;
}
// 
// set timeout / check if timeout expired
// TODO - organize all timeouts as separate database similar to commands
// 
bool timeoutOps(int oper, int whichOne) {
int index;
// find timer index first
  if((index = findTimer(whichOne))<0)	    // timer not found
  	return false;						              // TODO - report error
  if (oper == SET)                        // record the current time in milliseconds
	  timerDB[index].setAt = millis();
  else 
    return ((unsigned long)(millis() - timerDB[index].setAt) > (unsigned long)timerDB[index].interval);
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
struct THRESHOLD thresholds[] = {{0,    450,    ZONE_ERROR_SHORT},                   // below 450mV; LINE SHORT, zoneA error, zoneB error LINE SHORT
                                 {451,  1300,   ZONE_A_CLOSED     | ZONE_B_CLOSED},  // from 451 to 1300mV;  zoneA closed, zoneB closed
                                 {1301, 2000,   ZONE_A_OPEN       | ZONE_B_CLOSED},  // from 1301 to 2000mV; zoneA closed, zoneB open
                                 {2001, 2400,   ZONE_A_CLOSED     | ZONE_B_OPEN},    // from 2001 to 2400mV; zoneA open,   zoneB closed
                                 {2401, 3000,   ZONE_A_OPEN       | ZONE_B_OPEN},    // from 2401 to 3000mV; zoneA open,   zoneB open
                                 {3001, 3500,   ZONE_ERROR_OPEN}};                   // above 3001mV; zoneA error LINE OPEN , zoneB error LINE OPEN 

#define THRESHOLDS_CNT (sizeof(thresholds)/(sizeof(struct THRESHOLD)))

unsigned long zoneTest[64];
//
void selectZones(int which) {         // TODO To avoid issue with the button, make pin input when selectin A zones and rely on pull-up. 
  digitalWrite(muxCtlPin, which);
  //lprintf("%ld: Switching mux to %s channel\n", millis(), (which?"Azones":"Bzones"));
}
//
//  fill zone data for test 
//  parms: struct ZONE DB[]  - (pointer ???) to array of ZONE  containing the zones to be read and converted
//
void fillZonesTestData(unsigned long zoneTest[], int zones_cnt) { 
    int i; 
    int increment;
    increment = 4096/zones_cnt;
    //lprintf ("Zone filled data: ");
    for (i = 0; i <  zones_cnt; i++) {               // iterate
       zoneTest[i] = i*increment; //+increment/3;
       lprintf ("%d ", zoneTest[i]);
    }
    lprintf ("\n");
    //zoneTest[0] = 4000;
}
//
// add to arduino setup func
//
void zoneHWSetup() {
	pinMode (muxCtlPin, OUTPUT);			// set mux ctl as output
	selectZones	(Azones);					// select A zones
	//set adc channels???
   if(TEST)                                 // prepare test data
    fillZonesTestData(zoneTest, SLAVE_ZONES_CNT);

}
//
//convert oversampled ADC value to mV
//
float convert2mV (unsigned long adcVal) {
    return float (((adcVal*3200)/4096)/OVERSAMPLE_CNT);
}
//
//  print input zones data
//  parms: struct ZONE DB[]  - (pointer) to array of ZONE  containing the zones to be printed
//
void printZones(struct ZONE DB[], int zones_cnt) { 
    int i; 
    for (i = 0; i <  zones_cnt; i++) {                        // iterate
       lprintf ("Zone data: Zone ID: %d: GPIO: %2d: \tAvg ADC Value: %lu\tAvg mV val: %4.3f mV;\tZoneABstat: %x\n", DB[i].zoneID, DB[i].gpio, DB[i].accValue, DB[i].mvValue, DB[i].zoneABstat );
    }
}
//
//  print PGM data
//  parms: struct PGM DB[]  - (pointer ???) to array of PGM  containing the pgm to be printed
//
void printPGMs(struct PGM DB[], int pgm_cnt) { 
    int i; 
    for (i = 0; i <  pgm_cnt; i++) {                        // iterate
       lprintf ("PGM data: PGM ID: %d: GPIO: %2d: \tinit val: %d\tcur val: %d\n", DB[i].rNum, DB[i].gpio, DB[i].iValue, DB[i].cValue);
    }
}
//
//  Convert zone analog value (mV or ADC code)  to binary code using look-up in thresholds array. Store in zoneABcode field
//
void zoneVal2Code(struct ZONE DB[], int zoneCnt) {
  int i, thrIndex;
  for (i = 0; i < zoneCnt; i++){                              // for all zones in the array
    for (thrIndex = 0;((thrIndex < THRESHOLDS_CNT)&&(DB[i].mvValue >= thresholds[thrIndex].tMin)); thrIndex++) {// look-up the input voltage  in the input voltage ranges
      ;                                                       // keep searching while the input voltage is lower than min input voltage for the range
      //lprintf("zone val: %.2f ; Index: %d Threshold %.2f\n", DB[i].mvValue, thrIndex, (float)(thresholds[thrIndex].tMin));
      }
    //   
    if (thrIndex == THRESHOLDS_CNT)                           // check for out of range, shall never happen
      if(DB[i].mvValue > thresholds[thrIndex-1].tMax)         // correct the index to point to the exact range
        ErrWrite(ERR_DEBUG, "ADC value out of range!!!!!!!\n");
    thrIndex--;                                               // correct the index to point to the exact range                                           
    DB[i].zoneABstat = thresholds[thrIndex].zoneABstat;       // get zones A&B status  code   
    //lprintf("Index %d value %d\n",thrIndex,DB[i].zoneABstat );
    } 
}
//
void printZonesPayload(byte buffer[], int cnt) {
int i;
    lprintf("Payload: ");
    for (i = 0; i < cnt; i++)      
      lprintf(" %2x", buffer[i]);
    lprintf("\n");
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
       //lprintf(" Zeroing zone %d\t GPIO: %d\n", i , DB[i].gpio);
       }
  for (j = 0; j< OVERSAMPLE_CNT; j++) {                 // will read and accumulate values for each zone  OVERSAMPLE_CNT times
     for (i = 0;  i < zones_cnt; i++) {                 // read zone and store value
        //lprintf("Reading zone index %d; mux %d; gpio %d; value %d;\n",i, mux, DB[i].gpio, DB[i].accValue ); 
        if(mux != DB[i].mux)                            // skip the zones not selected by mux     
          continue;                                     // but the one under consideration is not muxed
        // read and convert here
        if(TEST)                                        // skip reading in test mode as the data are filled already by fillZones()
          val = zoneTest[i];                            // accumulate
        else
          val = analogRead(DB[i].gpio);                 // read from ADC
        DB[i].accValue = DB[i].accValue+val;            // accumulate
        //if((j==0))                                        // print once
          //lprintf("Reading zone %d: gpio: %d; mux %d; Acc val = %d; Current val: %d\n",DB[i].zoneID, DB[i].gpio, mux,  DB[i].accValue, val); 
        }                                               // zones loop
     }                                                  // oversample loop
    // convert values to voltages
    for (i = 0; i <  zones_cnt; i++) {    // convert acumulated values to voltages 
      DB[i].mvValue = convert2mV(DB[i].accValue);    // full scale (4096) represent 3.2V, and value is oversampled 8 times
      //lprintf ("Converted zone zone %d: GPIO: %2d \tAvg ADC Value %lu = \t%4.3f mV\n",DB[i].zoneID, DB[i].gpio, DB[i].accValue, DB[i].mvValue );
    }
}
//
// read and convert zone adc values to mV first and after to zone status
// allows to read A/B zones (or Azones/system voltages with different timing, as we don't need to update the system voltages so often
// supports mux set interval before read in order to allow analog inputs to settle
// implements small state machine which states are switched when timer expires
// states are WAITING_FOR_MUX, WAITING_TO_READ_A_ZONES, WAITING_TO_READ_B_ZONES
// before swiching zones we need to set mux and give some time for voltages to settle
//
void convertZones(struct ZONE DB[], int zoneCnt, byte zoneResult[]) {
  int i = 0;
  unsigned long lastRead = 0;
  static int  muxState = Azones;
// this is state machine cannot be called from both master and slave to read zones analog value   
  if(!timeoutOps(GET, MUX_SET_TIMER))						  // do we wait for mux set-up time?	
    return;                                               // can't do anything, wait for analog inputs to settle
  lastRead = millis();                                    // to calculate how much time we spend in here
  if(muxState == Azones) {
    if(timeoutOps(GET, ZONES_A_READ_TIMER)) {                // time to read A zones?
      lprintf("%ld: Reading A zones\n", millis());
	  zoneInfoValid |=	ZONE_A_VALID;					  // mark as valid to avoid sending invalid info	
      readZones(DB, zoneCnt, muxState);                   // do read
      timeoutOps(SET, ZONES_A_READ_TIMER);                   // remember when
      }
    else if(timeoutOps(GET, ZONES_B_READ_TIMER)) {           // time to read B zones?
      timeoutOps(SET, MUX_SET_TIMER);                        // start mux settle time
      lprintf("%ld: Mux B timeout started\n", millis());
      selectZones(muxState = Bzones);                     // switch mux
      return;                                             // nothing to do, wait mux timeout to expire
      }
    else
      return;											  // do nothing, time to read did not came yet	
  }
  else {                                                  // time to read zones B, mux is set and setup time interval expired                   
    lprintf("%ld: Reading B zones\n", millis());
    readZones(DB, zoneCnt, muxState);                     // do read
	zoneInfoValid |=	ZONE_B_VALID;					  // mark as valid to avoid sending invalid info	
    timeoutOps(SET, ZONES_B_READ_TIMER);                     // remember when    
    timeoutOps(SET, MUX_SET_TIMER);                          // start mux settle time switching back to A channel
    lprintf("%ld: Mux A timeout started\n", millis());
    selectZones(muxState = Azones);   					   // switching back to A channel	
    }
  zoneVal2Code(DB, zoneCnt);                               // convert analog values to digital status
  //printZones(DB, zoneCnt);
  if(!zoneResult)											// do we need to prepare the zones info in format for sending
	return;													// no, return now (case for Master which does not need to send zones)
#ifdef SLAVE
  // copy converted zones results to results array which will be send to master
  for (i=0; i < zoneCnt; i++) {                            // combine two zones in one byte, 12, 34, 56, ....
    if(!(i%2))
      zoneResult[i/2] = 0;                                 // clear results array
    zoneResult[i/2] = ((zoneResult[i/2] << ZONE_ENC_BITS) & ~ZONE_ENC_MASK) | DB[i].zoneABstat ;
    }  
  //printZonesPayload(zoneResult, zoneCnt%2?(zoneCnt/2+1):zoneCnt/2);
  //lprintf ("Elapsed time %d millisec\n", (unsigned long)(millis() - lastRead));
#endif
}
//
//  PGM control code here
//
void pgmSetup(struct PGM pDB[], const int pgmCnt) {
  for(int i =0; i < pgmCnt; i++) {                   // for each PGM 
    pinMode (pDB[i].gpio, OUTPUT);                  // set GPIO as output
    digitalWrite(pDB[i].gpio, pDB[i].iValue);     // set initial value
    pDB[i].cValue = pDB[i].iValue;
  }
}
//
// set PGM
//
void setPgm(struct PGM pDB[], byte idx, bool val, const int pgmCnt) {
  for (int i = 0; i < pgmCnt; i++) {
    //lprintf("Looking at index  %d out of  %d:\n", idx, pgmCnt-1);
    if(idx == pDB[i].rNum)  
      digitalWrite(pDB[i].gpio, val);               // set output value
    } 
}
//
// get PGM
//
bool getPgm(struct PGM pDB[], byte idx, const int pgmCnt) {
  for (int i = 0; i < pgmCnt; i++) {
    //lprintf("Looking at index  %d out of  %d:\n", idx, pgmCnt-1);
    if(idx == pDB[i].rNum) 
      return pDB[i].cValue;               // read output value
    }
}
