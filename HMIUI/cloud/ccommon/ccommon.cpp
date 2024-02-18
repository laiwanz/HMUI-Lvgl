#include "ccommon.h"
#include "HmiBase/HmiBase.h"
#include "platform/platform.h"
#include "macrosplatform/registerAddr.h"

using namespace PLATFORM_NAMESPACE;
ADDR_TYPE_T toAddrType(unsigned int nType) {
	ADDR_TYPE_T addType = ADDR_TYPE_BIT;
	switch (nType) {
	case 0:
		{
			addType = ADDR_TYPE_BIT;
			break;
		}
	case 1:
		{
			addType = ADDR_TYPE_BYTE;
			break;
		}
	case 2:
		{
			addType = ADDR_TYPE_WORD;
			break;
		}
	case 3:
		{
			addType = ADDR_TYPE_DWORD;
			break;
		}
	default:
		break;
	}
	return addType;
}

int addressTransform(const Json::Value& json, const std::vector<std::string> &vec, \
					 int nCommuId, ADDR_TYPE_T addrType, CRegister& reg, std::string& sAlias) {
	if (vec.empty()
		|| 3 != vec.size()) {
		return -1;
	}
	int nStatNo = -1;
	std::string sRid = "";
	std::string sAddr = "";
	if (json.isMember(vec[0].c_str())) {
		nStatNo = json[vec[0].c_str()].isInt() ? json[vec[0].c_str()].asInt() : -1;
	}
	if (json.isMember(vec[1].c_str())) {
		sRid = json[vec[1].c_str()].isString() ? json[vec[1].c_str()].asString() : "";
	}
	if (json.isMember(vec[2].c_str())) {
		sAddr = json[vec[2].c_str()].isString() ? json[vec[2].c_str()].asString() : "";
		replace(sAddr.begin(), sAddr.end(), ',', '.');
	}

	if (nStatNo > 0) {
		sAlias = cbl::format("%d:%s%s", nStatNo, sRid.c_str(), sAddr.c_str());
	}
	else {
		sAlias = cbl::format("%s%s", sRid.c_str(), sAddr.c_str());
	}
	std::string sAddrString;
	if (CCompileAddress::get_instance().compile(sAlias, nCommuId, addrType, sAddrString) < 0
		|| stringToRegister(sAddrString, reg) < 0) {
		return -2;
	}
	return 0;
}

ALARM_MODE_T toAlarmMode(unsigned short nType) {
	ALARM_MODE_T nMode = ALARM_MODE_UPPERLIMIT;
	switch (nType) {
	case 0:
		{
			nMode = ALARM_MODE_EQUALLIMIT;
			break;
		}
	case 1:
		{
			nMode = ALARM_MODE_NOEQUAL;
			break;
		}
	case 2:
		{
			nMode = ALARM_MODE_UPPERLIMIT;
			break;
		}
	case 3:
		{
			nMode = ALARM_MODE_GREATE_OR_EQUAL;
			break;
		}
	case 4:
		{
			nMode = ALARM_MODE_LOWERLIMIT;
			break;
		}
	case 5:
		{
			nMode = ALARM_MODE_LESS_OR_EQUAL;
			break;
		}
	default:
		break;
	}
	return nMode;
}

unsigned short toAlarmType(const ALARM_MODE_T &mode) {
	unsigned short nType = 0;
	switch (mode)
	{
	case ALARM_MODE_EQUALLIMIT: {
			nType = 0;
		}
		break;
	case ALARM_MODE_NOEQUAL:{
			nType = 1;
		}
		break;
	case ALARM_MODE_UPPERLIMIT: {
			nType = 2;
		}
		break;
	case ALARM_MODE_GREATE_OR_EQUAL: {
			nType = 3;
		}
		break;
	case ALARM_MODE_LOWERLIMIT: {
			nType = 4;
		}
		break;
	case ALARM_MODE_LESS_OR_EQUAL: {
			nType = 5;
		}
		break;
	default:
		break;
	}
	return nType;
}

eCLOUDMODE_T getCloudMode() {
	eCLOUDMODE_T eMode = eCLOUDMODE_NONE;
	std::string sbuffer = "";
	std::vector<std::string> vec;
	if (CPlatBoard::getDeviceMode() == VBOX_MODE) {
		std::string sPath = getPrjCfgDir() + std::string("BoxMode.ini");
		if (cbl::getFileSize(sPath) < 0) {
			printf("%s:%s do not exist\r\n", __FUNCTION__, sPath.c_str());
			return eCLOUDMODE_NONE;
		}
		if (cbl::readFileOnce64(sPath, 0, SEEK_SET, cbl::getFileSize(sPath), sbuffer) < 0 ||
			sbuffer.empty()) {
			printf("%s:readFileOnce64 %s error\r\n", __FUNCTION__, sPath.c_str());
			return eCLOUDMODE_NONE;
		}
		if (cbl::split(sbuffer, "=", &vec) < 0 ||
			vec.size() != 2) {
			printf("%s:%s param error\r\n", __FUNCTION__, sPath.c_str());
			return eCLOUDMODE_NONE;
		}
		try {
			eMode = (eCLOUDMODE_T)std::stoi(vec[1]);
		} catch (const std::exception& e) {
			printf("%s\r\n", e.what());
			return eCLOUDMODE_NONE;
		}
	}
	else {
		eMode = eCLOUDMODE_HMI;
	}
	return eMode;
}

int setCloudMode(eCLOUDMODE_T eMode) {
	if (CPlatBoard::getDeviceMode() == HMI_MODE) {
		return -1;
	}
	std::string sPath = getPrjCfgDir() + std::string("BoxMode.ini");
	int nRtn = 0;
#ifdef WIN32
	nRtn = cbl::saveToFile(cbl::format("Mode=%u", eMode), sPath);
#else
	nRtn = cbl::syncSaveToFile(cbl::format("Mode=%u", eMode), sPath);
#endif
	if (nRtn == 0) {
		return 0;
	}
	return -2;
}

bool writeSpecialAddress(unsigned int nAddressOffset, const std::string &sbuffer) {
	bool bRtn = false;
	switch (nAddressOffset)
	{
	case HSW_LOCALYEAR:
	case HSW_LOCALYEAR_R:
	case HSW_LOCALMONTH:
	case HSW_LOCALMONTH_R:
	case HSW_LOCALDAY:
	case HSW_LOCALDAY_R:
	case HSW_LOCALHOUR:
	case HSW_LOCALHOUR_R:
	case HSW_LOCALMINUTE:
	case HSW_LOCALMINUTE_R:
	case HSW_LOCALSECOND:
	case HSW_LOCALSECOND_R:
	case HSW_MAPFENCE:
	case HSW_MAPFENCE_R:
		{
			unsigned int nValue = 0;
			try {
				nValue = std::stoi(sbuffer);
			} catch (const std::exception& e) {
				printf("%s\r\n", e.what());
				return false;
			}
			bRtn = CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, nAddressOffset, 0, &nValue, 1);
		}
		break;
	case HSW_REMOTEPWD:
	case HSW_REMOTEPWD_R:
		{
			char szValue[32] = { 0 };
			if (sbuffer.length() >= 32) {
				strncpy(szValue, sbuffer.c_str(), sizeof(szValue) - 1);
			}
			else {
				strncpy(szValue, sbuffer.c_str(), sbuffer.length());
			}
			bRtn = CRegisterRW::write(RW_FORMAT_STR, REG_TYPE_HS_, DATA_TYPE_WORD, nAddressOffset, 0, szValue, sizeof(szValue));
		}
		break;
	case HSW_REMOTESN_R:
	case HSW_REMOTESN:
		{
			char szValue[64] = { 0 };
			if (sbuffer.length() >= sizeof(szValue)) {
				strncpy(szValue, sbuffer.c_str(), sizeof(szValue) - 1);
			}
			else {
				strncpy(szValue, sbuffer.c_str(), sbuffer.length());
			}
			bRtn = CRegisterRW::write(RW_FORMAT_STR, REG_TYPE_HS_, DATA_TYPE_WORD, nAddressOffset, 0, szValue, sizeof(szValue));
		}
		break;
	default:
		break;
	}
	return bRtn;
}

int toCommuConfig(const COMMU_CONFIG_T& fromConfig, CONNECTNODE& toConfig) {
	toConfig.nPortId = -1;
	toConfig.nCommuId = fromConfig.nCommuId;
	toConfig.PlcSet.nHmiMachineNo = fromConfig.nDeviceStation;
	toConfig.PlcSet.nPlcMachineNo = fromConfig.nPlcStation;
	if (fromConfig.sProtocolName.length() <= sizeof(toConfig.PlcSet.szPLCType) / sizeof(char)) {
		strncpy(toConfig.PlcSet.szPLCType, fromConfig.sProtocolName.c_str(), fromConfig.sProtocolName.length());
	}
	else {
		strncpy(toConfig.PlcSet.szPLCType, fromConfig.sProtocolName.c_str(), sizeof(toConfig.PlcSet.szPLCType) / sizeof(char) - 1);
		toConfig.PlcSet.szPLCType[sizeof(toConfig.PlcSet.szPLCType) / sizeof(char) - 1] = '\0';
	}
	std::string sDriverName = fromConfig.sDriverFile;
	if (FRAMEWORK_NEW != fromConfig.nFramework) {
		sDriverName = cbl::lcase(fromConfig.sDriverFile);
	}
	if (sDriverName.length() <= sizeof(toConfig.PlcSet.szLinkProtocol) / sizeof(char)) {
		strncpy(toConfig.PlcSet.szLinkProtocol, sDriverName.c_str(), sDriverName.length());
	}
	else {
		strncpy(toConfig.PlcSet.szLinkProtocol, sDriverName.c_str(), sizeof(toConfig.PlcSet.szLinkProtocol) / sizeof(char) - 1);
		toConfig.PlcSet.szLinkProtocol[sizeof(toConfig.PlcSet.szLinkProtocol) / sizeof(char) - 1] = '\0';
	}
	if (fromConfig.sCommuPortName.length() <= sizeof(toConfig.Commset.szPortName) / sizeof(char)) {
		strncpy(toConfig.Commset.szPortName, fromConfig.sCommuPortName.c_str(), fromConfig.sCommuPortName.length());
	}
	else {
		strncpy(toConfig.Commset.szPortName, fromConfig.sCommuPortName.c_str(), sizeof(toConfig.Commset.szPortName) / sizeof(char) - 1);
		toConfig.Commset.szPortName[sizeof(toConfig.Commset.szPortName) / sizeof(char) - 1] = '\0';
	}
	if (fromConfig.sCommuPortName.length() <= sizeof(toConfig.Commset.szInitPortName) / sizeof(char)) {
		strncpy(toConfig.Commset.szInitPortName, fromConfig.sCommuPortName.c_str(), fromConfig.sCommuPortName.length());
	}
	else {
		strncpy(toConfig.Commset.szInitPortName, fromConfig.sCommuPortName.c_str(), sizeof(toConfig.Commset.szInitPortName) / sizeof(char) - 1);
		toConfig.Commset.szInitPortName[sizeof(toConfig.Commset.szInitPortName) / sizeof(char) - 1] = '\0';
	}
	if (fromConfig.sCommuPortName.length() <= sizeof(toConfig.Commset.szComSelect) / sizeof(char)) {
		strncpy(toConfig.Commset.szComSelect, fromConfig.sCommuPortName.c_str(), fromConfig.sCommuPortName.length());
	}
	else {
		strncpy(toConfig.Commset.szComSelect, fromConfig.sCommuPortName.c_str(), sizeof(toConfig.Commset.szComSelect) / sizeof(char) - 1);
		toConfig.Commset.szComSelect[sizeof(toConfig.Commset.szComSelect) / sizeof(char) - 1] = '\0';
	}
	switch (fromConfig.nCheckBit) {
	case 0: {
		strncpy(toConfig.Commset.szCheckBit, "NONE", sizeof("NONE"));
		break;
	}
	case 1: {
		strncpy(toConfig.Commset.szCheckBit, "ODD", sizeof("ODD"));
		break;
	}
	case 2: {
		strncpy(toConfig.Commset.szCheckBit, "EVEN", sizeof("EVEN"));
		break;
	}
	case 3: {
		strncpy(toConfig.Commset.szCheckBit, "SPACE", sizeof("SPACE"));
		break;
	}
	case 4: {
		strncpy(toConfig.Commset.szCheckBit, "MARK", sizeof("MARK"));
		break;
	}
	default: {
		break;
	}
	}
	toConfig.Commset.nCommMode = fromConfig.nCommuPortType;
	toConfig.Commset.nBaudRate = fromConfig.nBaudrate;
	toConfig.Commset.nDataLength = fromConfig.nDataLen;
	toConfig.Commset.nStopBits = fromConfig.nStopBit;
	toConfig.Commset.nRetryTimes = fromConfig.nRetryCount;
	toConfig.Commset.nWaitTimeout = fromConfig.nWaitTimeoutTime;
	toConfig.Commset.nRecvTimeout = fromConfig.nReceiveTimeout;
	toConfig.Commset.nRetryTimeOut = fromConfig.nRetryTimeoutTime;
	toConfig.Commset.nComIoDelayTime = fromConfig.nCommuDelayTime;
	toConfig.Commset.nComSingleReadLen = fromConfig.nContinuousLength;
	toConfig.Commset.nComStepInterval = fromConfig.nIntegrationInterval;
	toConfig.Commset.wBroadcastAddr = fromConfig.nBroadcastAddress;
	toConfig.Commset.bEthernet_IsBroadcast = fromConfig.IsBroadcast;
	toConfig.Commset.nEthernet_IsUDP = (int32_t)strtol(fromConfig.sNetworkType.c_str(), nullptr, 10);
	toConfig.Commset.DstIPAddr.sin_addr.s_addr = fromConfig.nPlcNetworkIp;
	struct in_addr inTemp;
	inTemp.s_addr = fromConfig.nPlcNetworkIp;
	strcpy(toConfig.Commset.szDstIPV4, inet_ntoa(inTemp));
	toConfig.Commset.iDstPort = fromConfig.nPlcNetworkPort;
	std::string sPort = std::to_string(fromConfig.nPlcNetworkPort);
	if (sPort.length() <= sizeof(toConfig.Commset.szDstNetPort) / sizeof(char)) {
		strncpy(toConfig.Commset.szDstNetPort, sPort.c_str(), sPort.length());
	}
	else {
		strncpy(toConfig.Commset.szDstNetPort, sPort.c_str(), sizeof(toConfig.Commset.szDstNetPort) / sizeof(char) - 1);
		toConfig.Commset.szDstNetPort[sizeof(toConfig.Commset.szDstNetPort) / sizeof(char) - 1] = '\0';
	}
	if (!strcmp(toConfig.Commset.szPortName, "Ethernet")) {
		toConfig.Commset.nCommMode = Comm_Ethernet;
	}
	if (!strncmp(toConfig.Commset.szPortName, "CAN", strlen("CAN"))) //支持双CAN
	{
		toConfig.Commset.nCommMode = Comm_CAN;
		if (toConfig.Commset.szPortName[3] == '\0') {
			toConfig.Commset.szPortName[3] = '0';  //CAN 为CAN0
			toConfig.Commset.szInitPortName[3] = '0';
		}
	}
	if (!strncmp(toConfig.Commset.szPortName, "USB", strlen("USB"))) //支持USB
	{
		toConfig.Commset.nCommMode = Comm_USB;
	}
	toConfig.nFramework = fromConfig.nFramework;
	return 0;
}

int getFileMd5(const std::string &sPath, std::string &sbuffer) {
	if (cbl::getFileSize(sPath) <= 0) {
		return -1;
	}
	if (cbl::md5File(sPath, sbuffer) < 0) {
		return -2;
	}
	return 0;
}

int toRealConfig(const MONITOR_POINT_CONFIG_T& fromConfig, COLLECTION_MONITOR_POINT_DATA& toConfig) {
	toConfig.nUid = fromConfig.nID;
	toConfig.nCommuID = fromConfig.nCommuId;
	toConfig.nDataFormat = fromConfig.nFormat;
	toConfig.nInteger = fromConfig.nIntegerDigits;
	toConfig.nDecimals = fromConfig.nDecimalDigits;
	toConfig.nDataLen = fromConfig.nLen;
	toConfig.bMap = fromConfig.bKeep;
	toConfig.nHighLowByte = fromConfig.nHighLowByte;
	toConfig.nHighLowWord = fromConfig.nHighLowWord;
	toConfig.nPerssion = fromConfig.nPermission;
	toConfig.bSaveTraffic = fromConfig.bTraffic;
	toConfig.sAddrAlias.assign(fromConfig.sAddress);
	toConfig.vecName.assign({ fromConfig.sName[0], fromConfig.sName[1], fromConfig.sName[2], fromConfig.sName[3], fromConfig.sName[4],
		fromConfig.sName[5], fromConfig.sName[6], fromConfig.sName[7] });
	toConfig.addrType = toAddrType(fromConfig.nAddressType);
	Convert::BuffToUnionData(fromConfig.sDeadValue, toConfig.nDataFormat, toConfig.nDecimals, toConfig.u4wDeadZoneValue);
	if (ADDR_TYPE_BIT == toConfig.addrType) {
		toConfig.bBitCollect = true;
	}
	else {
		toConfig.bBitCollect = false;
	}
	std::string sAddrString;
	if (CCompileAddress::get_instance().compile(toConfig.sAddrAlias, toConfig.nCommuID, toConfig.addrType, sAddrString) < 0
		|| stringToRegister(sAddrString, toConfig.addrMonitor) < 0) {
		return -1;
	}
	return 0;
}

int toHistoryConfig(const MONITOR_POINT_CONFIG_T& fromConfig, COLLECTION_HISTORY_NODE& toConfig) {
	toConfig.nId = fromConfig.nID;
	toConfig.nGroupId = fromConfig.nGroupId;
	toConfig.nCommuID = fromConfig.nCommuId;
	toConfig.nDataFormat = fromConfig.nFormat;
	toConfig.nInteger = fromConfig.nIntegerDigits;
	toConfig.nDecimals = fromConfig.nDecimalDigits;
	toConfig.nDataLen = fromConfig.nLen;
	toConfig.bReportCloud = true;
	toConfig.nHighLowByte = fromConfig.nHighLowByte;
	toConfig.nHighLowWord = fromConfig.nHighLowWord;
	toConfig.sFieldAddrAlias.assign(fromConfig.sAddress);
	toConfig.vecFieldName.assign({ fromConfig.sName[0], fromConfig.sName[1], fromConfig.sName[2], fromConfig.sName[3], fromConfig.sName[4],
		fromConfig.sName[5], fromConfig.sName[6], fromConfig.sName[7] });
	toConfig.fieldAddrType = toAddrType(fromConfig.nAddressType);
	std::string sAddrString;
	if (!toConfig.sFieldAddrAlias.empty()
		&& (CCompileAddress::get_instance().compile(toConfig.sFieldAddrAlias, toConfig.nCommuID, toConfig.fieldAddrType, sAddrString) < 0
			|| stringToRegister(sAddrString, toConfig.addrField) < 0)) {
		return -1;
	}
	return 0;
}

int toAlarmConfig(const ALARM_CONFIG_T& fromConfig, COLLECTION_ALARM_DATA& toConfig) {
	if (fromConfig.listAlarmCondition.empty()) {
		return -1;
	}
	toConfig.bSave = false;
	toConfig.bReportCloud = true;
	toConfig.nUid = fromConfig.nId;
	toConfig.nMonitorCommuID = fromConfig.nCommuId;
	toConfig.nDataFormat = fromConfig.nDataFormat;
	toConfig.nInteger = fromConfig.nIntegerDigits;
	toConfig.nDecimals = fromConfig.nDecimalDigits;
	toConfig.sName.assign(fromConfig.sName);
	toConfig.conditionType = (ARITHMETIC_TYPE_T)fromConfig.nConditionType;
	toConfig.nHighLowByte = fromConfig.nHighLowByte;
	toConfig.nHighLowWord = fromConfig.nHighLowWord;
	toConfig.sAddressAlias.assign(fromConfig.sAddress);
	toConfig.vecText.assign(8, "");

	if (_String == toConfig.nDataFormat) {
		toConfig.nDataLen = toConfig.nInteger;
	}
	toConfig.monitorAddrType = toAddrType(fromConfig.nAddressType);
	if (ADDR_TYPE_BIT == toConfig.monitorAddrType) {
		toConfig.type = ALARM_TYPE_BIT;
	}
	else {
		toConfig.type = ALARM_TYPE_WORD;
	}
	std::string sAddrString;
	if (CCompileAddress::get_instance().compile(toConfig.sAddressAlias, toConfig.nMonitorCommuID, toConfig.monitorAddrType, sAddrString) < 0
		|| stringToRegister(sAddrString, toConfig.addrMonitor) < 0) {
		return -2;
	}
	if (ARITHMETIC_TYPE_SINGLE == toConfig.conditionType) {
		auto iter = fromConfig.listAlarmCondition.begin();
		if (ALARM_TYPE_BIT == toConfig.type) {
			if ("ON" == iter->sConditionValue) {
				toConfig.wTriggerValue = 1;
			}
			else if ("OFF" == iter->sConditionValue) {
				toConfig.wTriggerValue = 0;
			}
		}
		else if (ALARM_TYPE_WORD == toConfig.type) {
			if (toConfig.nDataFormat == _String) {
				toConfig.sCondition = iter->sConditionValue;
			}
			else {
				Convert::BuffToUnionData(iter->sConditionValue.c_str(), toConfig.nDataFormat, toConfig.nDecimals, toConfig.u4wRangeEnd);
			}
			toConfig.mode = toAlarmMode((unsigned short)strtol(iter->sCondition.c_str(), nullptr, 10));
		}
	}
	else {
		int nIndex = 0;
		toConfig.mode = ALARM_MODE_RANGELIMIT;
		for (auto &iter : fromConfig.listAlarmCondition) {
			if (toConfig.nDataFormat == _String) {
				if (0 == nIndex) {
					toConfig.sCondition = iter.sConditionValue;
					toConfig.modeStart = toAlarmMode((unsigned short)strtol(iter.sCondition.c_str(), nullptr, 10));
				}
				else {
					toConfig.sConditionEnd = iter.sConditionValue;
					toConfig.modeEnd = toAlarmMode((unsigned short)strtol(iter.sCondition.c_str(), nullptr, 10));
					break;
				}
			}
			else {
				if (0 == nIndex) {
					Convert::BuffToUnionData(iter.sConditionValue.c_str(), toConfig.nDataFormat, toConfig.nDecimals, toConfig.u4wRangeStart);
					toConfig.modeStart = toAlarmMode((unsigned short)strtol(iter.sCondition.c_str(), nullptr, 10));
				}
				else {
					Convert::BuffToUnionData(iter.sConditionValue.c_str(), toConfig.nDataFormat, toConfig.nDecimals, toConfig.u4wRangeEnd);
					toConfig.modeEnd = toAlarmMode((unsigned short)strtol(iter.sCondition.c_str(), nullptr, 10));
					break;
				}
			}
			nIndex++;
		}
	}
	return 0;
}

int toHistoryGroupConfig(const MONITOR_GROUP_CONFIG_T& fromConfig, COLLECTION_HISTORY_DATA& toConfig) {
	toConfig.bFieldReportCloud = true;
	toConfig.bTriggerValue = (bool)strtoul(fromConfig.sTriggerValue.c_str(), nullptr, 10);
	toConfig.sTriggerAddrAlias.assign(fromConfig.sAddress);
	toConfig.nType = fromConfig.nTriggerType;
	toConfig.nGroupId = fromConfig.nID;
	toConfig.nInterval = fromConfig.nPeriod;
	toConfig.nTrigCommuID = fromConfig.nCommuId;
	toConfig.vecGroupName.assign({ fromConfig.sName[0], fromConfig.sName[1], fromConfig.sName[2], fromConfig.sName[3], fromConfig.sName[4],
		fromConfig.sName[5], fromConfig.sName[6], fromConfig.sName[7] });
	toConfig.trigerAddrType = toAddrType(fromConfig.nAddressType);

	std::string sAddrString;
	if (!toConfig.sTriggerAddrAlias.empty()
		&& (CCompileAddress::get_instance().compile(toConfig.sTriggerAddrAlias, toConfig.nTrigCommuID, toConfig.trigerAddrType, sAddrString) < 0
			|| stringToRegister(sAddrString, toConfig.addrTrigger) < 0)) {
		return -1;
	}
	return 0;
}

#define DRIVER_DIR "/mnt/data/driver/"

static int adaptso(const std::string &sDir) {
	std::vector<std::string> vec;
#ifdef WIN32
	cbl::listDir64(sDir, 1, [ &vec ](const string& sFilePath, const struct __stat64& statFile) {
		if (_S_IFREG & statFile.st_mode) {
#else
	cbl::listDir64(sDir, 1, [ &vec ] (const string& sFilePath, const struct stat64& statFile) {
		if (S_ISREG(statFile.st_mode)) {
#endif
			std::string sSuffix = cbl::suffixName(sFilePath);
			if (sSuffix == ".so") {
				for (auto &iter : vec) {
					if (cbl::lcase(cbl::baseName(iter)) != cbl::lcase(cbl::baseName(sFilePath))) {
						continue;
					}
					UTILITY_NAMESPACE::CFile::Copy(std::string(DRIVER_DIR) + iter, sFilePath, false);
					break;
				}
			}
		}
		return 0;
	});
	return 0;
}

int adaptOldProjects() {
	std::vector<std::string> vec;
	if (!cbl::isDir(DRIVER_DIR)) {
		return 0;
	}
	vec.push_back("/mnt/wecon/wecon/template.vwmt");
	vec.push_back("/mnt/wecon/wecon/cloudprj.vwmt");
	vec.push_back("/mnt/wecon/wecon/httpDownLoad.xml");
	vec.push_back("/mnt/wecon/wecon/TestBoard");
	vec.push_back("/mnt/wecon/wecon/update/md5List.config");
	vec.push_back("/mnt/wecon/wecon/update/setup.log");
	vec.push_back("/mnt/wecon/wecon/update/update.log");
	vec.push_back("/mnt/wecon/wecon/run/BoxUpdate");
	vec.push_back("/mnt/wecon/wecon/run/backup");
	vec.push_back("/mnt/wecon/wecon/run/baseset.conf");
	vec.push_back("/mnt/wecon/wecon/run/cert");
	vec.push_back("/mnt/wecon/wecon/run/libvbox.so");
	vec.push_back("/mnt/wecon/wecon/run/server.json");
	vec.push_back("/mnt/wecon/wecon/run/serverlist.xml");
	vec.push_back("/mnt/wecon/wecon/run/server_ASEAN.json");
	vec.push_back("/mnt/wecon/wecon/run/server_CHINA.json");
	vec.push_back("/mnt/wecon/wecon/run/server_EUROPE.json");
	vec.push_back("/mnt/wecon/wecon/run/setup.xml.bak");
	vec.push_back("/mnt/wecon/wecon/run/tz.conf");
	vec.push_back("/mnt/wecon/wecon/run/update.log");
	vec.push_back("/mnt/wecon/wecon/run/vprj/Default.xml");
	vec.push_back("/mnt/wecon/wecon/run/vprj/ModbusRegMap.xml.old");
	vec.push_back("/mnt/wecon/wecon/run/vprj/SaveReg.bin");
	vec.push_back("/mnt/wecon/wecon/run/vprj/cloudDataBuffer.db");
	vec.push_back("/mnt/wecon/wecon/run/vprj/hisDataBuffer.db");
	vec.push_back("/mnt/wecon/wecon/run/vprj/json.vbox");
	vec.push_back("/mnt/wecon/wecon/run/rundir");
	for (auto &iter : vec) {
		if (cbl::isDir(iter)) {
#ifdef WIN32
			cbl::removeDir(iter);
#else
			cbl::removeDir(iter, true);
#endif
		}
		else {
			UTILITY_NAMESPACE::CFile::Delete(iter);
		}
	}
	if (cbl::isDir(getPrjCfgDir() + std::string("CloudPrj"))) {
		UTILITY_NAMESPACE::CDir::CopyFolder(getPrjCfgDir() + std::string("CloudPrj"), getPrjCfgDir() + std::string("cloudprj"), "*", false);
#ifdef WIN32
		cbl::removeDir(getPrjCfgDir() + std::string("CloudPrj"));
#else
		cbl::removeDir(getPrjCfgDir() + std::string("CloudPrj"), true);
#endif
	}
	adaptso(getPrjDir() + std::string("vprj"));
	adaptso(getPrjCfgDir() + std::string("cloudprj"));
	adaptso(getPrjCfgDir() + std::string("template"));
	if (cbl::isDir(getPrjCfgDir() + std::string("cloudprj"))) {
		if (cbl::isDir(getPrjCfgDir() + std::string("cloudprj/E02Drv"))) {
			cbl::makeDir(getPrjCfgDir() + std::string("cloudprj/E02Drv/driver"));
			UTILITY_NAMESPACE::CDir::CopyFolder(getPrjCfgDir() + std::string("cloudprj"), getPrjCfgDir() + std::string("cloudprj/E02Drv/driver"), "*.so", false, nullptr, false);
		}
		else {
			cbl::makeDir(getPrjCfgDir() + std::string("cloudprj/driver"));
			UTILITY_NAMESPACE::CDir::CopyFolder(getPrjCfgDir() + std::string("cloudprj"), getPrjCfgDir() + std::string("cloudprj/driver"), "*.so", false, nullptr, false);
		}
	}
	if (cbl::isDir(getPrjCfgDir() + std::string("template"))) {
		if (cbl::isDir(getPrjCfgDir() + std::string("template/E02Drv"))) {
			cbl::makeDir(getPrjCfgDir() + std::string("template/E02Drv/driver"));
			UTILITY_NAMESPACE::CDir::CopyFolder(getPrjCfgDir() + std::string("template"), getPrjCfgDir() + std::string("template/E02Drv/driver"), "*.so", false, nullptr, false);
		}
		else {
			cbl::makeDir(getPrjCfgDir() + std::string("template/driver"));
			UTILITY_NAMESPACE::CDir::CopyFolder(getPrjCfgDir() + std::string("template"), getPrjCfgDir() + std::string("template/driver"), "*.so", false, nullptr, false);
		}
	}
#ifdef WIN32
	cbl::removeDir(std::string(DRIVER_DIR));
#else
	cbl::removeDir(std::string(DRIVER_DIR), true);
#endif
	return 0;
}

int verifyDrivers(const std::string &sFilePath) {
	if (cbl::getFileSize(sFilePath) < 128) {
		printf("%s:%s is empty!", __FUNCTION__, sFilePath.c_str());
		return -1;
	}
	typedef struct {
		char szHead[6];/*固定info:*/
		char szPlatform[16];/*平台信息*/
		char szFirmware[4];/*固件版本*/
		char szReserve[102];/*保留*/
	} DRIVER_INFO_T;
	std::string sbuffer = "", sPlatform = "";
	int nRtn = 0;
	if ((nRtn = cbl::readFileOnce64(sFilePath, -128, SEEK_END, 128, sbuffer)) < 0) {
		printf("%s:readFileOnce64 error:%d\r\n", __FUNCTION__, nRtn);
		return -2;
	}
	DRIVER_INFO_T driverInfo;
	memcpy(&driverInfo, sbuffer.c_str(), sizeof(DRIVER_INFO_T));
	if (0 != strncmp(driverInfo.szHead, "info:", strlen("info:"))) {
		printf("head error\r\n");
		return -3;
	}
	DEVICE_BOARD_TYPE_T nType = PLATFORM_NAMESPACE::CPlatBoard::getType();
	switch (nType)
	{
	case A8_FLASH_BOARD_v2:
	case A8_FLASH_BOARD_3N_v2:
	case A8_EMMC_BOARD_v2:
		{
			sPlatform = "a8";
		}
		break;
	case A9_EMMC_BOARD_v2:
	case ARM9_FLASH_2G_BOARD:
	case ARM9_FLASH_4G_BOARD:
		{
			sPlatform = "arm9";
		}
		break;
	case A7_FLASH_BOARD_v2:
		{
			sPlatform = "a7";
		}
		break;
	case RK3308B_FLASH64Mmem_BOARD_v2:
	case RK3308B_FLASH_BOARD_v2:
	case rk3308b_eMMC_v2:
	case RK3308B_FLASH64Mmem_IN_v2:
	case RK3308B_FLASH64Mmembox_BOARD_v2:
		{
			sPlatform = "rk3308";
		}
		break;
	case SSD212_FLASH64Mmem_v2:
		{
			sPlatform = "ssd212";
		}
		break;
	case UnKnow_BOARD_v2:
	case WECON_ALL_BOARD_v2:
	default:
		break;
	}
#ifdef _LINUX_
	if (sPlatform != driverInfo.szPlatform) {
		printf("platform error platform = %s, so's version = %s\r\n", sPlatform.c_str(), driverInfo.szPlatform);
		return -4;
	}
#endif
	return 0;
}