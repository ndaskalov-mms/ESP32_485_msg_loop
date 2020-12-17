
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
