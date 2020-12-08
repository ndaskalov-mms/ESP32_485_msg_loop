
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
#define Zone7_			PIO14	// ADC2_CH6	
#define Zone8_			GPIO12	// ADC2_CH5	
#define Zone9_			GPIO13	// ADC2_CH4	
#define Zone10_			GPIO15	// ADC2_CH3	
#define Zone11_			GPIO2 	// ADC2_CH2	
#define Zone12_			GPIO4	// ADC2_CH0	

#define CLUSTER_RD	12

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
struct ZONE zoneDB[] = {	{Zone1 , 0, 0, 0}\
							{Zone2 , 0, 0, 0}\
							{Zone3 , 0, 0, 0}\
							{Zone4 , 0, 0, 0}\
							{Zone5 , 0, 0, 0}\
							{Zone6 , 0, 0, 0}\
							{Zone7 , 0, 0, 0}\
							{Zone8 , 0, 0, 0}\
							{Zone9 , 0, 0, 0}\
							{Zone10, 0, 0, 0}\
							{Zone11, 0, 0, 0}\
							{Zone12, 0, 0, 0}}
void convert zones() {
	static int i = 0; 
	int j = 0;
	// read zones analog value 
	for (i; i < (sizeof(zoneDB[])/sizeof(struct ZONE); i++) {
		for j
