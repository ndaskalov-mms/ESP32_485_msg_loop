
//
// initializes master's zoneDB with data from master's MzoneDB and SzonesDB as defined in compile time
// zoneDB contains MAX_SLAVE+1 arrays of struct ZONE. Each array corresponds to one slave, and one is for master.
// 
void initAlarmZones() {
	// init master zones storage first
	for(int j=0;j<MASTER_ZONES_CNT;j++)	{
		zonesDB[MASTER_ADDRESS][j].useZone = true;
		zonesDB[MASTER_ADDRESS][j].gpio = 	 MzoneDB[j].gpio;
		zonesDB[MASTER_ADDRESS][j].mux = 	   MzoneDB[j].mux;
		zonesDB[MASTER_ADDRESS][j].zoneID =  MzoneDB[j].zoneID;
		zonesDB[MASTER_ADDRESS][j].boardID = MASTER_ADDRESS;
		}
	// than for all slave boards
	for(int i=1;i<=MAX_SLAVES;i++) {
		for(int j=0;j<SLAVE_ZONES_CNT;j++)	{
			zonesDB[i][j].useZone = true;
			zonesDB[i][j].gpio 	  = SzoneDB[j].gpio; 	  
			zonesDB[i][j].mux     = SzoneDB[j].mux;     
			zonesDB[i][j].zoneID  = SzoneDB[j].zoneID;  
			zonesDB[i][j].boardID = i;
			}
		}	
}

// 
void printAlarmZones() {
	logger.printf("BoardID\tZoneID\tGPIO\tMUX\tUSE ZONE\n");
	for(int i=0;i<=MAX_SLAVES;i++) {
		for(int j=0;j<RECORD_ZONES_CNT;j++)	{
			logger.printf("%d %d %d %d %d  ",zonesDB[i][j].boardID, zonesDB[i][j].zoneID, zonesDB[i][j].gpio, zonesDB[i][j].mux,  zonesDB[i][j].useZone);
			}
		logger.printf("\n");
		}	
}
