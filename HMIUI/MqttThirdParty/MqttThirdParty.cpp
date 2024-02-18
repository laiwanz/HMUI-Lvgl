#include "MqttThirdParty.h"
#include "utility/utility.h"
#include "secure/secure.h"
#include "ProtoXML/TagsDict/TagsDict.h"
#include "HmiBase/HmiBase.h"
#include <stdlib.h>
#include <vector>

using namespace UTILITY;

static int parseIIOT(const XML_Char **attr, void * pData) {
	if (nullptr == attr) {
		return -1;
	}
	IIOT_MQTT_CONFIG_T *pMqttConfig = static_cast<IIOT_MQTT_CONFIG_T *>(pData);
	for (int nI = 0; attr[nI]; nI += 2) {
		switch (FindDict((char *)attr[nI], 1)) {
		case Enable: {
			pMqttConfig->nEnable = strtoul((char *)attr[nI + 1], nullptr, 10);
		}
		break;
		default: {
			break;
		}
		}
	}
	return 0;

}

/* parse base set
**/
static int parseBaseSet(const XML_Char **attr, IIOT_MQTT_BASESET_T &baseSet) {

	/* check params */
	if (nullptr == attr) {
		return -1;
	}

	for (int nI = 0; attr[nI]; nI += 2) {
		switch (FindDict((char *)attr[nI], 1)) {
			case CloudService: {
				baseSet.thirdPartType = static_cast<THIRD_PARTY_TYPE>( strtoul((char *) attr[nI + 1], nullptr, 10) );
				break;
			}
			case Protocol: {
				baseSet.mqttProtocolVersion = static_cast<MQTT_PROTOCOL_VERSION>( strtoul((char *) attr[nI + 1], nullptr, 10) );
				break;
			}
			case DomainName: {
				baseSet.sUrl.assign(attr[nI + 1]);
				break;
			}
			case EnableDomain: {
				baseSet.bEnableDomain = strtoul((char *) attr[nI + 1], nullptr, 10);
				break;
			}
			case Port: {
					baseSet.nPort = (unsigned short) strtoul((char *) attr[nI + 1], nullptr, 10);
				break;
			}
			case PrimaryConnectionString: {
				baseSet.sPrimaryConnectionString.assign(attr[nI + 1]);
				break;
			}
			case ClientID: {
				baseSet.sClientId.assign(attr[nI + 1]);
				break;
			}
			case UseUserVerify: {
				baseSet.bEnableAuthentication = strtoul((char *)attr[nI + 1], nullptr, 10);
				break;
			}
			case UserName: {
				baseSet.sName.assign(attr[nI + 1]);
				break;
			}
			case Password: {
				baseSet.sPassword.assign(attr[nI + 1]);
				break;
			}
			case TimestampType: {
				baseSet.timestampType = static_cast<TIMESTAMP_TYPE>(strtoul((char *)attr[nI + 1], nullptr, 10));
				break;
			}
			case UseCache: {
				baseSet.bEnableCache = strtoul((char *)attr[nI + 1], nullptr, 10);
				break;
			}
			case CLIENTID_LENGTH: {
				baseSet.nClientIdLength = (unsigned short) strtoul((char *) attr[nI + 1], nullptr, 10);
			}
			break;
			case USER_PASSWORD_LENGTH: {
				baseSet.nNameAndPasswordLength = (unsigned short) strtoul((char *) attr[nI + 1], nullptr, 10);
			}
			break;
			case DeviceID: {
				baseSet.sDeviceid.assign(attr[nI + 1]);
			}
			break;
			case DeviceSecret: {
				baseSet.sDeviceSecret.assign(attr[nI + 1]);
			}
			break;
			case ProductId: {
				baseSet.sProductId.assign(attr[nI + 1]);
			}
			break;
			case Method: {
				baseSet.aliyunSignature = static_cast<ALIYUNSIGNATURE_T>( strtoul((char *) attr[nI + 1], nullptr, 10) );
			}
			break;
			case NETWorkCard: {
				baseSet.networkCard = static_cast<NETWORKCARD_T>( strtoul((char *) attr[nI + 1], nullptr, 10) );
			}
			break;
			case ConnectTime: {
				baseSet.nConnectTimeout = (unsigned int)strtoul((char *)attr[nI + 1], nullptr, 10);
			}
			break;
			default: {
				break;
			}
		}
	}

	return 0;
}

/* parse address set
**/
static int parseAddressSet(const XML_Char **attr, IIOT_MQTT_ADDR_CONTROL_T& addressSet) {

	/* check params */
	if (nullptr == attr) {
		return -1;
	}

	for (int nI = 0; attr[nI]; nI += 2) {
		switch (FindDict((char *)attr[nI], 1)) {
			case StatusAddress: {
				stringToRegister((char *)attr[nI + 1], addressSet.addrState);
				break;
			}
			case UseCacheAddress: {
				addressSet.bEnableCache = strtoul((char *)attr[nI + 1], nullptr, 10);
				break;
			}
			case CacheAddress: {
				stringToRegister((char *)attr[nI + 1], addressSet.addrCache);
				break;
			}
			case UseControlAddress: {
				addressSet.bEnableControlAddress = strtoul((char *)attr[nI + 1], nullptr, 10);
				break;
			}
			case ControlAddress: {
				stringToRegister((char *)attr[nI + 1], addressSet.addrControl);
				break;
			}
			default: {
				break;
			}
		}
	}

	return 0;
}

/* parse secure set
**/
static int parseSecureSet(const XML_Char **attr, IIOT_MQTT_SECURESET_T& secureSet) {
	/* check params */
	if (nullptr == attr) {
		return -1;
	}
	for (int nI = 0; attr[nI]; nI += 2) {
		switch (FindDict((char *)attr[nI], 1)) {
			case Enable: {
				secureSet.bEnablessl = strtoul((char *)attr[nI + 1], nullptr, 10);
				break;
			}
			case Version: {
				secureSet.tlsProtocolVersion = static_cast<TLS_PROTOCOL_VERSION>(strtoul((char *)attr[nI + 1], nullptr, 10));
				break;
			}
			case InfoMatch: {
				secureSet.bVerify = strtoul((char *)attr[nI + 1], nullptr, 10);
				break;
			}
			case ServerCert: {
				std::string sTrustStore(attr[nI + 1]);
				if (!sTrustStore.empty()) {
					secureSet.sTrustStore = getPrjDir() + std::string("IIOT/") + sTrustStore;
				}
				break;
			}
			case ClientCert: {
				std::string sKeyStore(attr[nI + 1]);
				if (!sKeyStore.empty()) {
					secureSet.sKeyStore = getPrjDir() + std::string("IIOT/") + sKeyStore;
				}
				break;
			}
			case SecretKey: {
				std::string sPrivateKey(attr[nI + 1]);
				if (!sPrivateKey.empty()) {
					secureSet.sPrivateKey = getPrjDir() + std::string("IIOT/") + sPrivateKey;
				}
				break;
			}
			default: {
				break;
			}
		}
	}
	return 0;
}

/* parse system topic
**/
static bool parseSystemTopic(const XML_Char *szNodeName, const XML_Char **attr, void * pData) {

	/* check params */
	if (nullptr == szNodeName
		|| nullptr == attr
		|| nullptr == pData) {
		return false;
	}

	CExpatXml::ExpatStack *pStack = (CExpatXml::ExpatStack *)pData;
	if (CExpatXml::StackEmpty(*pStack)) {
		return false;
	}

	std::list<IIOT_MQTT_SYSTOPIC_T> *pSysTopic = static_cast<std::list<IIOT_MQTT_SYSTOPIC_T> *>(pStack->pTop->pvData);
	if (nullptr == pSysTopic) {
		return false;
	}

	switch (FindDict((char *)szNodeName, 1)) {
		case SysTopic: {

			IIOT_MQTT_SYSTOPIC_T sysTopic;
			for (int nI = 0; attr[nI]; nI += 2) {
				switch (FindDict((char *)attr[nI], 1)) {
					case Enable: {
						sysTopic.bEnable = strtoul((char *) attr[nI + 1], nullptr, 10) == 1 ? true : false;
						break;
					}
					case TopicType: {
						sysTopic.sysTopicType = static_cast<SYSTEMTOPIC_TYPE>(strtoul((char *)attr[nI + 1], nullptr, 10));
						break;
					}
					case Topic: {
						sysTopic.sTopic.assign(attr[nI + 1]);
						break;
					}
					case Retain: {
						sysTopic.bRetain = strtoul((char *)attr[nI + 1], nullptr, 10) == 1 ? true : false;
						break;
					}
					case Qos: {
						sysTopic.nQos = (unsigned short) strtoul((char *)attr[nI + 1], nullptr, 10);
						break;
					}
					case ContentType: {
						sysTopic.contentType = static_cast<CONTENT_TYPE>(strtoul((char *)attr[nI + 1], nullptr, 10));
						break;
					}
					case CustomJson: {
						sysTopic.sCustomJson.assign(attr[nI + 1]);
						break;
					}
					case AddrMap: {
						sysTopic.sAddrmapJson.assign(attr[nI + 1]);
						break;
					}
					default: {
						break;
					}
				}
			}
			if (sysTopic.bEnable) {
				pSysTopic->push_back(sysTopic);
			}
			break;
		}
		default: {
			break;
		}
	}

	CExpatXml::StackPushEmptyNode(*pStack);

	return true;
}

/* parse topic node
**/
static bool parseTopicNode(const XML_Char *szNodeName, const XML_Char **attr, void * pData) {

	/* check params */
	if (nullptr == szNodeName
		|| nullptr == attr
		|| nullptr == pData) {
		return false;
	}

	CExpatXml::ExpatStack *pStack = (CExpatXml::ExpatStack *)pData;
	if (CExpatXml::StackEmpty(*pStack)) {
		return false;
	}

	std::list<TOPIC_T> *pTopicNode = static_cast<std::list<TOPIC_T> *>(pStack->pTop->pvData);
	if (nullptr == pTopicNode) {
		return false;
	}

	switch (FindDict((char *)szNodeName, 1)) {
		case BaseSet: {

			TOPIC_T topicNode;
			for (int nI = 0; attr[nI]; nI += 2) {
				switch (FindDict((char *)attr[nI], 1)) {
					case TopicType:{
						topicNode.topicType = static_cast<TOPICTYPE>(strtoul((char *)attr[nI + 1], nullptr, 10));
						break;
					}
					case Topic: {
						topicNode.sTopic.assign(attr[nI + 1]);
						break;
					}
					case GroupID: {
						topicNode.sGroupId.assign(attr[nI + 1]);
						break;
					}
					case EdgeNodeID: {
						topicNode.sEdgeNodeId.assign(attr[nI + 1]);
						break;
					}
					case TopicAddress: {
						stringToRegister((char *)attr[nI + 1], topicNode.addrTopic);
						break;
					}
					case TriggerAddress: {
						stringToRegister((char *)attr[nI + 1], topicNode.addrTrigger);
						break;
					}
					case SendMode: {
						topicNode.nMode = static_cast<SEND_MODE>(strtoul((char *)attr[nI + 1], nullptr, 10));
						break;
					}
					case EdgeTrigger: {
						topicNode.nTrigger = static_cast<EDGE_TRIGGER>(strtoul((char *)attr[nI + 1], nullptr, 10));
						break;
					}
					case CompressType: {
						topicNode.compressType = static_cast<COMPRESS_TYPE>(strtoul((char *)attr[nI + 1], nullptr, 10));
						break;
					}
					case ContentType: {
						topicNode.contentType = static_cast<CONTENT_TYPE>(strtoul((char *)attr[nI + 1], nullptr, 10));
						break;
					}
					case TopicLen: {
						topicNode.nTopicLen = (unsigned short) strtoul((char *)attr[nI + 1], nullptr, 10);
						break;
					}
					case UseTimestamp: {
						topicNode.bEnableTimestamp = strtoul((char *)attr[nI + 1], nullptr, 10) == 1 ? true : false;
						break;
					}
					case DefaultJsonHead: {
						topicNode.bDefaultJsonHead = strtoul((char *)attr[nI + 1], nullptr, 10) == 1 ? true : false;
						break;
					}
					case Qos: {
						topicNode.nQos = (unsigned short) strtoul((char *)attr[nI + 1], nullptr, 10);
						break;
					}
					case Retain: {
						topicNode.bRetain = strtoul((char *)attr[nI + 1], nullptr, 10) == 1 ? true : false;
						break;
					}
					case AutoReset: {
						topicNode.bFollow = strtoul((char *)attr[nI + 1], nullptr, 10) == 1 ? true : false;
						break;
					}
					case ChangeMode: {
						topicNode.bChangesBased = strtoul((char *)attr[nI + 1], nullptr, 10) == 1 ? true : false;
						break;
					}
					case PeriodMode: {
						topicNode.bTimeBased = strtoul((char *)attr[nI + 1], nullptr, 10) == 1 ? true : false;
						break;
					}
					case MinInterval: {
						topicNode.nMinTimeBetweenMessages = strtoul((char *)attr[nI + 1], nullptr, 10);
						break;
					}
					case Period: {
						topicNode.nTimeInterval = strtoul((char *)attr[nI + 1], nullptr, 10);
						break;
					}
					default: {
						break;
					}
				}
			}
			pTopicNode->push_back(topicNode);
			CExpatXml::StackPushEmptyNode(*pStack);
			break;
		}
		case AddressSet: {

			TOPIC_T& topicNode = pTopicNode->back();
			for (int nI = 0; attr[nI]; nI += 2) {
				switch (FindDict((char *)attr[nI], 1)) {
					case CustomJson: {
						topicNode.sCustomJson.assign(attr[nI + 1]);
						break;
					}
					case AddrMap: {
						topicNode.sAddrmapJson.assign(attr[nI + 1]);
						break;
					}
					break;
					default: {
						break;
					}
				}
			}
			CExpatXml::StackPushEmptyNode(*pStack);
			break;
		}
		case SecureSet: {

			TOPIC_T& topicNode = pTopicNode->back();
			for (int nI = 0; attr[nI]; nI += 2) {
				switch (FindDict((char *)attr[nI], 1)) {
					case Enable: {
						topicNode.bEnableSecure = strtoul((char *)attr[nI + 1], nullptr, 10);
						break;
					}
					case SecureAddress: {
						stringToRegister((char *)attr[nI + 1], topicNode.addrSecure);
						break;
					}
					case Status: {
						topicNode.bSecureBit = strtoul((char *)attr[nI + 1], nullptr, 10);
						break;
					}
					default: {
						break;
					}
				}
			}
			CExpatXml::StackPushEmptyNode(*pStack);
			break;
		}
		default: {
			break;
		}
	}

	return true;
}

/* parse server config
**/
static bool parseServerConfig(const XML_Char *szNodeName, const XML_Char **attr, void * pData) {

	/* check params */
	if (nullptr == szNodeName
		|| nullptr == attr
		|| nullptr == pData) {
		return false;
	}

	CExpatXml::ExpatStack *pStack = (CExpatXml::ExpatStack *)pData;
	if (CExpatXml::StackEmpty(*pStack)) {
		return false;
	}

	IIOT_MQTT_SERVERSET *pServerSet = static_cast<IIOT_MQTT_SERVERSET *>(pStack->pTop->pvData);
	if (nullptr == pServerSet) {
		return false;
	}

	switch (FindDict((char *)szNodeName, 1)) {
		case BaseSet: {
			parseBaseSet(attr, pServerSet->baseSet);
			CExpatXml::StackPushEmptyNode(*pStack);
			break;
		}
		case AddressSet: {
			parseAddressSet(attr, pServerSet->addressSet);
			CExpatXml::StackPushEmptyNode(*pStack);
			break;
		}
		case SecureSet: {
			parseSecureSet(attr, pServerSet->secureSet);
			CExpatXml::StackPushEmptyNode(*pStack);
			break;
		}
		case SysTopicSet: {
			CExpatXml::StackNode   node = { 0 };
			node.pXmlParse = parseSystemTopic;
			node.pvData = (void *)(&pServerSet->listSysTopics);
			CExpatXml::StackPush(*pStack, node);
			break;
		}
		default: {
			break;
		}
	}

	return true;
}

/* parse IIOT node
**/
static bool parseTopicConfig(const XML_Char *szNodeName, const XML_Char **attr, void * pData) {

	/* check params */
	if (nullptr == szNodeName
		|| nullptr == attr
		|| nullptr == pData) {
		return false;
	}

	CExpatXml::ExpatStack *pStack = (CExpatXml::ExpatStack *)pData;
	if (CExpatXml::StackEmpty(*pStack)) {
		return false;
	}

	switch (FindDict((char *)szNodeName, 1)) {
		case Topic: {
			CExpatXml::StackNode   node = { 0 };
			node.pXmlParse = parseTopicNode;
			node.pvData = pStack->pTop->pvData;
			CExpatXml::StackPush(*pStack, node);
			break;
		}
		default: {
			break;
		}
	}

	return true;
}

/* parse IIOT node
**/
static bool parseIIOTNode(const XML_Char *szNodeName, const XML_Char **attr, void * pData) {

	/* check params */
	if (nullptr == szNodeName
		|| nullptr == attr
		|| nullptr == pData) {
		return false;
	}

	CExpatXml::ExpatStack *pStack = (CExpatXml::ExpatStack *)pData;
	if (CExpatXml::StackEmpty(*pStack)) {
		return false;
	}

	IIOT_MQTT_CONFIG_T *pMqttConfig = static_cast<IIOT_MQTT_CONFIG_T *>(pStack->pTop->pvData);
	if (nullptr == pMqttConfig) {
		return false;
	}
	switch (FindDict((char *)szNodeName, 1)) {
		case ServerConfig: {
			CExpatXml::StackNode   node = { 0 };
			node.pXmlParse = parseServerConfig;
			node.pvData = (void *)(&pMqttConfig->serverSet);
			CExpatXml::StackPush(*pStack, node);
			break;
		}
		case TopicConfig: {
			CExpatXml::StackNode   node = { 0 };
			node.pXmlParse = parseTopicConfig;
			node.pvData = (void *)(&pMqttConfig->listTopic);
			CExpatXml::StackPush(*pStack, node);
			break;
		}
		default: {
			break;
		}
	}

	return true;
}

/* parse  attribute
**/
static bool parseAttribute(const XML_Char *szNodeName, const XML_Char **attr, void * pData) {

	/* check params */
	if (nullptr == szNodeName
		|| nullptr == attr
		|| nullptr == pData) {
		return false;
	}

	CExpatXml::ExpatStack	*pStack = (CExpatXml::ExpatStack *)pData;
	if (CExpatXml::StackEmpty(*pStack)) {
		return false;
	}

	switch (FindDict((char *)szNodeName, 1)) {
		case IIOT: {
			CExpatXml::StackNode   node = { 0 };
			parseIIOT(attr, pStack->pTop->pvData);
			node.pXmlParse = parseIIOTNode;
			node.pvData = pStack->pTop->pvData;
			CExpatXml::StackPush(*pStack, node);
			break;
		}
		default: {
			break;
		}
	}

	return true;
}

/* load xml 
**/
static int loadXMLConfig(const std::string &sFileData, IIOT_MQTT_CONFIG_T& mqttConfig) {

	if (sFileData.empty())
	{
		return -1;
	}

	CExpatXml::ExpatStack	stack = { 0 };
	CExpatXml::StackNode	node = { 0 };
	CExpatXml::StackInit(stack);

	node.pXmlParse = parseAttribute;
	node.pvData = (void *)(&mqttConfig);
	CExpatXml::StackPush(stack, node);

	if (false == CExpatXml::ParseString(sFileData, &stack, CExpatXml::ExpatStart, CExpatXml::ExpatEnd, NULL))
	{
		return -2;
	}

	return 0;
}

/* construct
**/
CMqttThirdParty::CMqttThirdParty() {

}

/* destruct
**/
CMqttThirdParty::~CMqttThirdParty() {

}

int CMqttThirdParty::getLockFileHead(const string & sPath, IIOTENCRYPT_T & le)
{
	std::string sData;

	if (cbl::getFileSize(sPath) < sizeof(IIOTENCRYPT_T)) {

		return -1;
	}

	if (0 > cbl::readFileOnce64(sPath, 0, SEEK_SET, sizeof(IIOTENCRYPT_T), sData) ||
		sData.empty()) {

		return -2;
	}

	memcpy(&le, sData.c_str(), sizeof(IIOTENCRYPT_T));

	return 0;
}

/* get instance
**/
CMqttThirdParty& CMqttThirdParty::get_instance() {

	static CMqttThirdParty mqttThirdParty;
	return mqttThirdParty;
}

void *runIiotTimer(void *arg) {

	if (nullptr == arg) {

		return nullptr;
	}

	CMqttThirdParty *pThirdParty = static_cast<CMqttThirdParty *>(arg);

	pThirdParty->m_iotMqtt.onTimer();

	return nullptr;
}

/* start 
**/
int CMqttThirdParty::start() {
	IIOT_MQTT_CONFIG_T mqttConfig;
	std::string sIIOTPath = getPrjDir() + std::string("IIOT/IIOT_MQTT.xml");
	std::string sDecData;
	if (unlockFile(sIIOTPath, sDecData) < 0) {
		return -1;
	}
	/* parse */
	if (loadXMLConfig(sDecData, mqttConfig) < 0) {
		printf("[CMqttThirdParty][%s-%d] parse xml file error \n", __FUNCTION__, __LINE__);
		return -2;
	}
	if (!mqttConfig.nEnable) {
		return 0;
	}
	/* start */
	if (m_iotMqtt.start(mqttConfig) < 0) {
		printf("[CMqttThirdParty][%s-%d] m_iotMqtt start error \n", __FUNCTION__, __LINE__);
		return -3;
	}
	CRateTimer::addTimer(runIiotTimer, this, (int) ( &runIiotTimer ), 50);
	return 0;
}

/* stop 
**/
int CMqttThirdParty::stop() {

	return m_iotMqtt.stop();
}

/* unlock File
**/
int CMqttThirdParty::unlockFile(const string & sPath, string & sDesData)
{
	/* check params */
	if (sPath.empty()) {
		return -1;
	}

	/* check file */
	if (false == UTILITY_NAMESPACE::CFile::ExistFile(sPath)) {
		return -2;
	}

	IIOTENCRYPT_T le = { 0 };
	std::string sSrcData;

	if ((getLockFileHead(sPath, le) < 0))
	{
		printf("[%s-%d] file:%s get head faild \n", __FUNCTION__, __LINE__, sPath.c_str());
		return -3;
	}

	if (le.dwMode == IIOT_ENCRYPT_MODE_ENCRYPT) {
		if (0 > cbl::readFileOnce64(sPath, sizeof(IIOTENCRYPT_T), SEEK_SET, cbl::getFileSize(sPath) - sizeof(IIOTENCRYPT_T), sSrcData) ||
			sSrcData.empty()) {

			return -4;
		}
		
		string sKey;
		sKey.assign((char *)le.szMd5, IIOT_ENCRYPT_MD5_LEN);
		SECURE_NAMESPACE::WSecure obj(sKey);

		if (0 > obj.decryptCache(sSrcData, sDesData) ||
			sDesData.empty()) {

			return -5;
		}
	}
	else {
		if (0 > cbl::readFileOnce64(sPath, 0, SEEK_SET, cbl::getFileSize(sPath), sDesData) ||
			sDesData.empty()) {

			return -6;
		}
	}

	return 0;
}
