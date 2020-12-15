#include "gpio-def.h"

// PGM assignment

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
#define PGM_CNT          (sizeof(pgmDB)/sizeof(struct PGM))
//
void pgmSetup() {
  for(int i =0; i < PGM_CNT; i++) {                   // for each PGM 
    pinMode (pgmDB[i].gpio, OUTPUT);                  // set GPIO as output
    digitalWrite(pgmDB[i].gpio, pgmDB[i].iValue);     // set initial value
    pgmDB[i].cValue = pgmDB[i].iValue;
  }
}
//
// set PGM
//
void setPgm(byte idx, bool val) {
  for (int i = 0; i < PGM_CNT; i++) {
    //logger.printf("Looking at index  %d out of  %d:\n", idx, PGM_CNT-1);
    if(idx == pgmDB[i].rNum)  
      digitalWrite(pgmDB[i].gpio, val);               // set output value
    } 
}
//
// get PGM
//
bool getPgm(byte idx) {
  for (int i = 0; i < PGM_CNT; i++) {
    //logger.printf("Looking at index  %d out of  %d:\n", idx, PGM_CNT-1);
    if(idx == pgmDB[i].rNum) 
      return pgmDB[i].cValue;               // read output value
    }
}
