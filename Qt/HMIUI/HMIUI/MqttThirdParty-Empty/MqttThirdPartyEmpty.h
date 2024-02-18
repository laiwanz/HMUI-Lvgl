#ifndef _MQTTTHIRDPARTY_H_20221101
#define _MQTTTHIRDPARTY_H_20221101

#include <iostream>

class CMqttThirdParty
{
public:

	CMqttThirdParty(const CMqttThirdParty&) = delete;
	CMqttThirdParty& operator=(const CMqttThirdParty&) = delete;
	/* get instance */
	static CMqttThirdParty& get_instance();
	/* start */
	int start();
	/* stop */
	int stop();
private:
	CMqttThirdParty();
	~CMqttThirdParty();
private:
};

#endif