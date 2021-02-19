
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
#define ZONES_BYPASS_STATUS_TOPIC  "/paradox/states/zones/%s/bypass"
#define ZONES_BYPASS_CONTROL_TOPIC  "/paradox/states/zones/%s/bypass"
#define BYPASS_PAYLOAD "true"
#define UNBYPASS_PAYLOAD "false"
#define INVALID_ZONE_PAYLOAD "invalidZone"
#define BYPASS_DISABLE_PAYLOAD "bypassDisabled"
#define ZONES_TAMPER_STATUS_TOPIC  "/paradox/states/zones/%s/tamper"
#define TAMPER_PAYLOAD "true"
//
//  PublishMQTT(char * topic, char * mqtt_msg)  - called that UNRECOVERY  error occured while sending command
//                  - this might triger some actions as send notification over MQTT, e-mail, watchdog reset, etc
// params:      char * topic    - topic where to report 
//              char * mqtt_msg - what went wrong
//

void  PublishMQTT(char * topic, char * mqtt_msg) {
		 //sprintf(tempMQTTbuf, 
     lprintf("MQTT topic: %s MSG: %s\n", topic, mqtt_msg);
}
//
//  PublishMQTT(char * topic, char * subtopic, char * payload)  - called that UNRECOVERY  error occured while sending command
//                  - this might triger some actions as send notification over MQTT, e-mail, watchdog reset, etc
//  params:     char * topic    - topic where to report 
//				char * SUBtopic - SUBtopic where to report. will be concatenated to topic - tipic\subtopic
//              char * mqtt_msg - what went wrong
//

void  PublishMQTT(char * topic, char * subtopic, char * payload) {
	   //sprintf(tempMQTTbuf, 
     lprintf("MQTT topic: %s/%s MSG: %s\n", topic, subtopic, payload);
}
