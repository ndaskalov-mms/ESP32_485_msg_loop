#ifdef MASTER
//
// alarm zones records structure to hold all alarm zones related info
//
struct ALARM_ZONE {
  byte gpio;					// first members  are the same as struct ZONE
  byte mux;                     // 1 - activate mux to read, 0 - read direct
  unsigned long accValue;       // oversampled value
  float mvValue;                // converted value in mV
  byte  zoneID;                 // the number of zone by which the master will identify it. Zero based. Each ADC gpio produces one zone, but with two results
  byte  zoneABstat;             // encodded status of the A and B parts of the zone
  byte	boardID;				// the board which zones belong to. Master ID is 0			
  byte  useZone;				// shall be used or not
};                              

//
// holds all alarm logic data related for zones
// onle one copy in master is available
//#if (SLAVE_ZONES_CNT>MASTER_ZONES_CNT)                      // TODO - use prep to get largest zone count
#define RECORD_ZONES_CNT (SLAVE_ZONES_CNT) // master has some zones and all slaves too
//#else
//#define RECORD_ZONES_CNT (MASTER_ZONES_CNT)
//#endif
//
struct ALARM_ZONE zonesDB[MAX_SLAVES+1][RECORD_ZONES_CNT];		// typically master has fewer zones than slave, so we use the largest denominator
#endif
//
// initializes master's zoneDB with data from master's MzoneDB and SzonesDB as defined in compile time
// zoneDB contains MAX_SLAVE+1 arrays of struct ZONE. Each array corresponds to one slave, and one is for master.
// 
void initAlarmZones() {
	for(int i=0;i<=MAX_SLAVES;i++) {
		for(int j=0;j<RECORD_ZONES_CNT;j++)	{
			zonesDB[i][j].useZone = false;
			zonesDB[i][j].gpio = 0;
			zonesDB[i][j].mux = 0;
			zonesDB[i][j].zoneID = j;
			zonesDB[i][j].boardID = i;
			}
		}	
}
// 
void printAlarmZones() {
	logger.printf("GPIO\tMUX\tBoardID\tZoneID\tUSE ZONE\n");
	for(int i=0;i<=MAX_SLAVES;i++) {
		for(int j=0;j<RECORD_ZONES_CNT;j++)	{
			logger.printf("%d %d %d %d %d    ", zonesDB[i][j].gpio, zonesDB[i][j].mux, zonesDB[i][j].boardID, zonesDB[i][j].zoneID, zonesDB[i][j].useZone);
			}
		logger.printf("\n");
		}	
}
