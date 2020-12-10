
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
#define GPIO4	4
#define GPIO36	36
#define GPIO39 	39
#define GPIO34	34

#define VzoneRef_		GPIO36	// ADC1_CH0	
#define ADC_AUX_    GPIO39	// ADC1_CH3	
#define ADC_BAT_    GPIO34	// ADC1_CH6	
#define Zone1_			GPIO35	// ADC1_CH7	
#define Zone2_			GPIO32	// ADC1_CH4	
#define Zone3_			GPIO33	// ADC1_CH5	
#define Zone4_			GPIO25	// ADC2_CH8	
#define Zone5_			GPIO26	// ADC2_CH9	
#define Zone6_			GPIO27	// ADC2_CH7	
#define Zone7_			GPIO14	// ADC2_CH6	
#define Zone8_			GPIO12	// ADC2_CH5	
#define Zone9_			GPIO13	// ADC2_CH4	
#define Zone10_			GPIO15	// ADC2_CH3	
#define Zone11_			GPIO2 	// ADC2_CH2	
#define Zone12_			GPIO4	  // ADC2_CH0	

#define Azones			HIGH
#define Bzones			LOW
#define muxCtlPin  		GPIO0			// 4053 mux control GPIO
#define AltZoneSelect	0x1			// selects aternative zones via 4053 mux
#define selectZones(which) DigitalWrite(muxCtlPin, which)


unsigned long zone_result;
// zone records structure for zoneDB
struct ZONE {
  byte gpio;
  unsigned long lastValue;
  unsigned long avgValue;
  bool binValue;
};
// some important voltages 
struct ZONE VzoneRef 	= {VzoneRef_, 0, 0, 0};
struct ZONE ADC_AUX 	= {ADC_AUX_, 0, 0, 0};
struct ZONE ADC_BAT 	= {ADC_BAT_, 0, 0, 0};
//
// zones database to store data
// 
struct ZONE zoneDB[] =		 {	{Zone4_ , 0, 0, 0}, 	{Zone5_ , 0, 0, 0}, 	{Zone6_ , 0, 0, 0},\
								{Zone7_ , 0, 0, 0}, 	{Zone8_ , 0, 0, 0}, 	{Zone9_ , 0, 0, 0},\
								{Zone10_, 0, 0, 0}, 	{Zone11_, 0, 0, 0}, 	{Zone12_, 0, 0, 0}};
// those zones are using 4053 mux to double the capacity. Mux control is via AltZoneSelect GPIO								
struct ZONE muxZoneDB[] = 	{    {Zone1_ , 0, 0, 0},		{Zone2_ , 0, 0, 0}, 	{Zone3_ , 0, 0, 0}};
// those zones are using 4053 mux to double the capacity. Mux control is via AltZoneSelect GPIO								
struct ZONE altMuxZoneDB[] = {	 {Zone1_ , 0, 0, 0},		{Zone2_ , 0, 0, 0}, 	{Zone3_ , 0, 0, 0}};
// those are special zones which might or might not be present depends on the board configuration
struct ZONE specZoneDB[] = 	{	 {VzoneRef_, 0, 0, 0}, 	{ADC_AUX_, 0, 0, 0}, 	{ADC_BAT_, 0, 0, 0}};

struct ZONE allZones[] = 	{ muxZoneDB, altMuxZoneDB, zoneDB, specZoneDB};
 
void zonesSetup() {
	pinMode (muxCtlPin, OUTPUT);			// set mux ctl as output
	selectAzones							// select A zones
	//set adc channels???
	
}



void readZones(struct ZONE DB[]) {							
  for (i = 0, i< OVERSAMPLE_CNT, i++) {								// will read and accumulate values for each zone  OVERSAMPLE_CNT times
	for (i = 0; i < (sizeof(DB)/sizeof(struct ZONE)); i++)
		DB[i].lastValue = 0;										// initialize
	for (i = 0; i < (sizeof(DB)/sizeof(struct ZONE)); i++) {		// read zone and store value
		// read and conver here
		ErrWrite(ERR_DEBUG, " %d ", j+i);
		DB[i].lastValue += AnalogRead(DB[i].gpio);					// accumulate
		}
	ErrWrite(ERR_DEBUG, "\n");
  }
  // convert values to voltages
  	for (i = 0; i < (sizeof(DB)/sizeof(struct ZONE)); i++) {		// convert acumulated values to voltages 
		// read and conver here
		ErrWrite(ERR_DEBUG, " %d ", j+i);
		DB[i].lastValue += AnalogRead(DB[i].gpio);					// accumulate
		}
	ErrWrite(ERR_DEBUG, "\n");
}
	
void convert_zones() {
  int i = 0;
  static unsigned long lastRead = 0;
  unsigned long bitVal = 0;
  unsigned long bitMask = 0;
  
	// read zones analog value 
	if (ZONES_READ_THROTTLE)  {                  		// time to read??
		unsigned long temp = millis();
		if ((unsigned long)(temp - lastRead) < (unsigned long)ZONES_READ_INTERVAL) {
		  //logger.printf("Cur %u, last %u, interval %u\n", temp, last_read,ZONES_READ_INTERVAL );
		  return;   
		} 
	}     
	ErrWrite(ERR_DEBUG, " Reading mux zones:");
	selectZones(Azones);
	readZones(muxZoneDB);								// reads and accumulates OVERSAMPLE_CNT times all zones in xxxxxDB 
	selectZones(Bzones);								// read interleaved with other zones to give time to settle input voltages
	// now read normal zones
	ErrWrite(ERR_DEBUG, "\n Reading normal zones:");
	readZones(zoneDB);
	// now read alt zones
	ErrWrite(ERR_DEBUG, "\n Reading alt zones:");
	readZones(altMuxZoneDB);
	selectZones(Azones);								// toggle GPIO to select main mux channel
	// now read special zones
	ErrWrite(ERR_DEBUG, "\n Reading special zones:");
	readZones(specZoneDB);	
  
  
  
  // convert to binary value here
  // reflect the new values in zones_result variable, which will be returned to master
  bitTmp = 0; bitMask = 0;
  for (i = 0; i < CLUSTER_SIZE; i++) {        // 
    bitTmp = (bitTmp << 1) | (zoneDB[i].binValue?0x1:0x0)
    bitMask = (bitMask << 1) | (0x1;
    ErrWrite(ERR_DEBUG, " %x %x", bitTmp, bitMask);
    }
  
	j += CLUSTER_SIZE;
  if(j == (sizeof(zoneDB)/sizeof(struct ZONE))) // all zones read?
		j = 0;											                // yes, restart 
  lastRead = millis();
	
}
