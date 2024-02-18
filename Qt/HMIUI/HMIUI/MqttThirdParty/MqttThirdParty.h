#ifndef _MQTTTHIRDPARTY_H_20221101
#define _MQTTTHIRDPARTY_H_20221101

#include "IIOT.MQTT/IIOT.MQTT.h"

#define IIOT_ENCRYPT_MD5_LEN			16
#define IIOT_ENCRYPT_RESERVE_LEN		44
#define IIOT_ENCRYPT_MODE_ENCRYPT    0xFFFFFFFF

class CMqttThirdParty
{
public:
	friend void *runIiotTimer(void *arg);

	CMqttThirdParty(const CMqttThirdParty&) = delete;
	CMqttThirdParty& operator=(const CMqttThirdParty&) = delete;
	/* get instance */
	static CMqttThirdParty& get_instance();
	/* start */
	int start();
	/* stop */
	int stop();

	int unlockFile(const string & sPath, string &sDesData);
private:
	CMqttThirdParty();
	~CMqttThirdParty();
	typedef struct _tagIIOTEncrypt {
		unsigned long dwMode;                             // 模式
		unsigned char szMd5[IIOT_ENCRYPT_MD5_LEN];         // 密码的MD5
		unsigned char szReserve[IIOT_ENCRYPT_RESERVE_LEN]; // 保留
	}IIOTENCRYPT_T;

	int getLockFileHead(const string & sPath, IIOTENCRYPT_T &le);
private:
	CIOTMQTT m_iotMqtt;


};

#endif