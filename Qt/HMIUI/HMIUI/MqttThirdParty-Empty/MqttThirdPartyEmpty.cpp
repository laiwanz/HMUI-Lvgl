#include <stdlib.h>
#include <vector>
#include "MqttThirdPartyEmpty.h"

/* construct
**/
CMqttThirdParty::CMqttThirdParty() {

}

/* destruct
**/
CMqttThirdParty::~CMqttThirdParty() {

}

/* get instance
**/
CMqttThirdParty& CMqttThirdParty::get_instance() {

	static CMqttThirdParty mqttThirdParty;
	return mqttThirdParty;
}

void *runIiotTimer(void *arg) {


	return nullptr;
}

/* start 
**/
int CMqttThirdParty::start() {

	return 0;
}

/* stop 
**/
int CMqttThirdParty::stop() {

	return 0;
}