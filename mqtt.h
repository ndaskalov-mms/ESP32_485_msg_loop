
#define LOG_TOPIC 		""		
#define DEBUG_TOPIC 	""
#define INFO_TOPIC		""
#define WARNING_TOPIC	""
#define ERROR_TOPIC		""

//
// ReportUpstream(cmd, mqtt_msg)  - called that UNRECOVERY  error occured while sending command
//                  - this might triger some actions as send notification over MQTT, e-mail, watchdog reset, etc
// params:      cmd - command with issues 
//              err_code - what went wrong
//

void ReportUpstream(char * topic, char * mqtt_msg) {

     logger.printf("MQTT topic %s: CMD: %d MSG %s", topic, waiting_for_reply, mqtt_msg);
}
