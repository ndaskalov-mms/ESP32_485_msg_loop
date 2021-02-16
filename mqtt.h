
#define LOG_TOPIC 		""		
#define DEBUG_TOPIC 	""
#define INFO_TOPIC		""
#define WARNING_TOPIC	""
#define ERROR_TOPIC		""
#define CRITICAL_TOPIC    ""
#define ARM_TOPIC    ""
#define PARTITIONS_STATES_TOPIC "/paradox/states/partitions/"
#define PARTITIONS_CONTROL_TOPIC "/paradox/control/partitions/"
#define ZONES_STATES_TOPIC "/paradox/states/zones/"
#define ZONES_CONTROL_TOPIC "/paradox/control/zones/"
//
//  ReportMQTT(char * topic, char * mqtt_msg)  - called that UNRECOVERY  error occured while sending command
//                  - this might triger some actions as send notification over MQTT, e-mail, watchdog reset, etc
// params:      char * topic    - topic where to report 
//              char * mqtt_msg - what went wrong
//

void  ReportMQTT(char * topic, char * mqtt_msg) {

     logger.printf("MQTT topic: %s MSG: %s\n", topic, mqtt_msg);
}
