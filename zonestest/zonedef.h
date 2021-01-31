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
