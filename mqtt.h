
#define LOG_TOPIC 		""		
#define DEBUG_TOPIC 	""
#define INFO_TOPIC		""
#define WARNING_TOPIC	""
#define ERROR_TOPIC		""

//
//  ReportMQTT(char * topic, char * mqtt_msg)  - called that UNRECOVERY  error occured while sending command
//                  - this might triger some actions as send notification over MQTT, e-mail, watchdog reset, etc
// params:      char * topic    - topic where to report 
//              char * mqtt_msg - what went wrong
//

void  ReportMQTT(char * topic, char * mqtt_msg) {

     logger.printf("MQTT topic %s: CMD: %d MSG %s", topic, waiting_for_reply, mqtt_msg);
}
