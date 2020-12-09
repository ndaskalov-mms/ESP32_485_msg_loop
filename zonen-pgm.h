
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
#define ADC_AUX_     	GPIO39	// ADC1_CH3	
#define ADC_BAT_     	GPIO34	// ADC1_CH6	
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
#define Zone12_			GPIO4	// ADC2_CH0	

#define CLUSTER_SIZE	4

// zone records structure for zoneDB
struct ZONE {
  byte GPIO;
  unsigned long last_value;
  unsigned long avg_value;
  bool bin_value;
};

struct ZONE VzoneRef 	= {VzoneRef_, 0, 0, 0};
struct ZONE ADC_AUX 	= {ADC_AUX_, 0, 0, 0};
struct ZONE ADC_BAT 	= {ADC_BAT_, 0, 0, 0};

//
// zones database to store data
// 
struct ZONE zoneDB[] = {	{Zone1_ , 0, 0, 0},\
							{Zone2_ , 0, 0, 0},\
							{Zone3_ , 0, 0, 0},\
							{Zone4_ , 0, 0, 0},\
							{Zone5_ , 0, 0, 0},\
							{Zone6_ , 0, 0, 0},\
							{Zone7_ , 0, 0, 0},\
							{Zone8_ , 0, 0, 0},\
							{Zone9_ , 0, 0, 0},\
							{Zone10_, 0, 0, 0},\
							{Zone11_, 0, 0, 0},\
							{Zone12_, 0, 0, 0},};
             
void convert_zones() {
	static int j = 0; 
	int i = 0;
  static unsigned long last_read = 0;
  
	// read zones analog value 
  if (ZONES_READ_THROTTLE)  {                  // time to read??
    unsigned long temp = millis();
    if ((unsigned long)(temp - last_read) < (unsigned long)ZONES_READ_INTERVAL) {
      //logger.printf("Cur %u, last %u, interval %u\n", temp, last_read,ZONES_READ_INTERVAL );
      return;   
    } 
  }     
	ErrWrite(ERR_DEBUG, " Reading zones:", j+i);
	for (i = 0; i < CLUSTER_SIZE; i++) {				// read up to cluster size to minimize the time spent in the function
		// read and conver here
		ErrWrite(ERR_DEBUG, " %d ", j+i);
	}
	j += CLUSTER_SIZE;
  if(j == (sizeof(zoneDB)/sizeof(struct ZONE))) // all zones read?
		j = 0;											                // yes, restart 
  last_read = millis();
	
}
