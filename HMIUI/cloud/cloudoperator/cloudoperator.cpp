#include "cloudoperator.h"
#include "utility/utility.h"
#include "secure/secure.h"
#include "platform/platform.h"
#include "../ccommon/ccommon.h"
#include "../parse/cloudparse.h"
#include "GlobalModule/GlobalModule.h"
#include "macrosplatform/registerAddr.h"
#include "../../../../HmiCloudApp/HmiCloudApp/protobuf/VNetConfig.pb.h"
using namespace com_wecon_vnet_V20210731;
using namespace UTILITY_NAMESPACE;
using namespace SECURE_NAMESPACE;
using namespace PLATFORM_NAMESPACE;
typedef enum
{
	FUNC_SIGN = 0x00000001,
	FUNC_COMPRESS = 0x00000002,
	FUNC_LUA = 0x00000004,
	FUNC_FS = 0x00000008,
	FUNC_CLOUD = 0x00000010,
	FUNC_SQUASHFS = 0x00000020,
	FUNC_COMPRESS_DATA = 0x00000040,
	FUNC_HEART = 0x00000080,
	//FUNC_TEMPLATE			= 0x00000100,
	FUNC_TEMPLATE = 0x00000200,
	FUNC_HAW = 0x00000400,
	FUNC_RESET_BTN = 0x00000800,
	FUNC_VPN_UDP = 0x00001000
} e_VboxFunc;

#define MAXLUACOUNT 500

static const std::string getNetType(const int nType) {
	std::string sNetType = "error";
	switch (nType) {
	case 0:
		sNetType = "TCP_Client";
		break;
	case 1:
		sNetType = "UDP_Client";
		break;
	case 2:
		sNetType = "TCP_Client_2N";
		break;
	case 3:
		sNetType = "UDP_Client_2N";
		break;
	case 4:
		sNetType = "TCP_Server";
		break;
	case 5:
		sNetType = "UDP_Server";
		break;
	default:
		break;
	}
	return sNetType;
}

CCloudOperator::CCloudOperator() {
	m_bInstallmentStat = m_bRunning = m_bServerStat = m_bLuaStat = m_bLanguageStat = false;
	m_nAdditionalFunction = 0;
	m_nLanguage = m_nMode = 0;
	m_ullRealCollectRunningTime = 0;
	m_sLuabuffer = "";
}

CCloudOperator::~CCloudOperator() {

}

CCloudOperator& CCloudOperator::get_instance() {
	static CCloudOperator CloudOperator;
	return CloudOperator;
}

int CCloudOperator::config(const CLOUDCONFIG_T &cloudConfig) {
#ifdef WIN32
#ifndef _DEBUG
	return 0;
#endif
#endif
	switch (cloudConfig.eMode)
	{
	case eCLOUDMODE_NONE:
	case eCLOUDMODE_TEMPLATE:
	case eCLOUDMODE_CLOUD:
		{
			m_nAdditionalFunction = FUNC_SIGN | FUNC_COMPRESS | FUNC_COMPRESS_DATA | FUNC_LUA
			| FUNC_CLOUD | FUNC_SQUASHFS | FUNC_HEART | FUNC_TEMPLATE | FUNC_HAW | FUNC_RESET_BTN | FUNC_VPN_UDP;
			m_nMode = (unsigned short) cloudConfig.eMode;
		}
		break;
	case eCLOUDMODE_HMI:
		{
			if (cloudConfig.bEnableScada) {
				m_nAdditionalFunction |= (1 << ADDITIONAL_FUNCTION_CLOUD_SCADA);
			}
			m_nAdditionalFunction |= FUNC_VPN_UDP; // VPN穿透UDP连接
			m_nMode = 0;
		}
		break;
	default:
		break;
	}
	m_cloudConfig = cloudConfig;
	CLIENT_CALL_CONFIG_T clientConfig;
	clientConfig.pfnGetDeviceInfo = [ this ] () {
		DEVICE_INFO_T deviceInfo;
		WMachine wMachine;
		if (wMachine.getDeviceId(deviceInfo.sDeviceId) < 0) {
			printf("%s:get device id error\r\n", __FUNCTION__);
			return -1;
		}
		wMachine.GetProduct(deviceInfo.sProductId);
		if (!wMachine.GetHMIType(deviceInfo.sModel)) {
			printf("%s:get hmi type error\r\n", __FUNCTION__);
			return -2;
		}
#ifdef WIN32
		if (m_cloudConfig.eMode != eCLOUDMODE_HMI) {
			deviceInfo.sSoftwareVersion = std::string("version:E02-2.0.1.23061303");
		}
		if (CPlatBoard::getDeviceMode() == VBOX_MODE) {
			deviceInfo.sOriginalProdouct = "0";
		}
		else if (CPlatBoard::getDeviceMode() == HMI_MODE) {
			deviceInfo.sOriginalProdouct = "1";
		}
#else
		CVersion Version;
		if (Version.getAllVer(deviceInfo.sSoftwareVersion) < 0) {
			printf("%s:get version error\r\n", __FUNCTION__);
			return -3;
		}
		if (CPlatBoard::getDeviceRealMode() == VBOX_MODE) {
			deviceInfo.sOriginalProdouct = "0";
		}
		else if (CPlatBoard::getDeviceRealMode() == HMI_MODE) {
			deviceInfo.sOriginalProdouct = "1";
		}
#endif
		deviceInfo.sProjectMd5 = m_cloudConfig.sProjectMd5;
		if (CTime::get_instance().getTz(deviceInfo.sTimezone) < 0) {
			printf("%s:get timezone error\r\n", __FUNCTION__);
			return -4;
		}
		if (std::string::npos != deviceInfo.sTimezone.find("+")) {
			deviceInfo.sTimezone = cbl::replaceall(deviceInfo.sTimezone, "+", "-");
		}
		else if (std::string::npos != deviceInfo.sTimezone.find("-")) {
			deviceInfo.sTimezone = cbl::replaceall(deviceInfo.sTimezone, "-", "+");
		}
		srand((unsigned int)time(NULL));
		deviceInfo.sClientId = cbl::format("%d%d%d%d%d", rand() % 11, rand() % 245, rand() % 55, rand() % 366, rand() % 188);
		deviceInfo.nWorkState = ONLINE;
		if (CPlatCpu::getNetUUid(deviceInfo.sUUID) < 0) {
			printf("%s:get net uuid error\r\n", __FUNCTION__);
			return -5;
		}
		deviceInfo.nAdditionalFunctionFlag = m_nAdditionalFunction;
		deviceInfo.sPassword = m_cloudConfig.sPassword;
		deviceInfo.nLanguage = m_nLanguage;
		deviceInfo.nMode = m_nMode;
		return m_remoteProcedureCallClient.setDeviceInfo(deviceInfo);
	};
	clientConfig.pfnGetBasicInfo = [ this ] (const DEVICE_INFO_RESPONSE_T &deviceInfoRsp) {
		m_nAdditionalFunction = deviceInfoRsp.nAdditionalFunctionFlag;
		if ((m_nAdditionalFunction & (1 << ADDITIONAL_FUNCTION_CLOUD_SCADA)) == 0) {
			return 0;
		}
		UPDATE_CONFIG_T eUpdateConfig = UPDATE_ALL;
		if (deviceInfoRsp.IsProjectChanged) {
			eUpdateConfig = UPDATE_ALL;
		}
		else if (deviceInfoRsp.IsLanguageChanged) {
			eUpdateConfig = UPDATE_LANGUAGE;
		}
		if (this->getbasicInfo(eUpdateConfig) < 0) {
			printf("%s:get basic info error\r\n", __FUNCTION__);
			return -1;
		}
		return 0;
	};
	clientConfig.pfnForcesync = [ this ] () {
		switch (m_cloudConfig.eMode)
		{
		case eCLOUDMODE_HMI:
			{
				UPDATE_CONFIG_T eUpdateConfig = UPDATE_ALL;
				if (CWPoint::get_instance().setReadCondition(MONITOR_POINT_READ_ALL) < 0) {
					printf("%s: set read condition error\r\n", __FUNCTION__);
					return -1;
				}
				if (m_remoteProcedureCallClient.forceSyncrsp() < 0) {
					printf("%s:force sync response error\r\n", __FUNCTION__);
					return -2;
				}
				if (this->getbasicInfo(eUpdateConfig) < 0) {
					printf("%s:get basic info error\r\n", __FUNCTION__);
					return -3;
				}
				return 0;
			}
			break;
		case eCLOUDMODE_TEMPLATE:
			{
#ifdef WIN32
			cbl::removeDir(getPrjCfgDir() + std::string("template"));
			cbl::removeDir(getPrjCfgDir() + std::string("cloudprj"));
#else
			cbl::removeDir(getPrjCfgDir() + std::string("template"), true);
			cbl::removeDir(getPrjCfgDir() + std::string("cloudprj"), true);
#endif
			CFile::Delete(getPrjCfgDir() + std::string("template.vwmt"));
			CFile::Delete(getPrjCfgDir() + std::string("cloudprj.vwmt"));
				if (CCloudParse::get_instance().unload() < 0) {
					printf("%s:unload error\r\n", __FUNCTION__);
					return -1;
				}
				if (CCloudParse::get_instance().save() < 0) {
					printf("%s:save xml error\r\n", __FUNCTION__);
					return -2;
				}
				if (CCloudParse::get_instance().restart() < 0) {
					printf("%s:restart error\r\n", __FUNCTION__);
					return -3;
				}
				return m_remoteProcedureCallClient.forceSyncrsp();
			}
			break;
		case eCLOUDMODE_NONE:
		case eCLOUDMODE_CLOUD:
		case eCLOUDMODE_MAX:
		default:
			break;
		}
		return 0;
	};
	clientConfig.pfnWritepoint = [ this ] (const std::list<MONITOR_POINT_T> &listMonitor) {
		std::list<MONITOR_POINT_T> listReal;
		for (auto &iter : listMonitor) {
			MONITOR_POINT_T	Point;
			if (iter.nType == MONITOR_DATA_TYPE_ADDRESSDATA) {
				if (writeSpecialAddress(iter.nId, iter.sValue)) {
					Point.bSuccess = true;
				}
				Point.nId = iter.nId;
			}
			else {
				Point.nId = CWPoint::get_instance().write(iter.nId, iter.sValue);
				if (Point.nId > 0) {
					Point.bSuccess = true;
				}
			}
			if (Point.bSuccess) {
				Point.nCommuState = DEVICE_COMMU_NORMAL;
				Point.sValue = iter.sValue;
				Point.ullTimestamp = CTime::get_instance().getUtc();
				Point.nType = iter.nType;
			}
			listReal.push_back(Point);
		}
		if (listReal.empty()) {
			return -1;
		}
		return m_remoteProcedureCallClient.setRealData(listReal);
	};
	clientConfig.pfnUpdateCommu = [ this ] (const std::list<COMMU_CONFIG_T> &listCommu) {
		return this->updateCommu(listCommu);
	};
	clientConfig.pfnDeleteCommu = [ this ] (const std::list<COMMU_CONFIG_T> &listCommu) {
		return this->deleteCommu(listCommu);
	};
	clientConfig.pfnUpdateDriver = [ this ] (const std::list<DRIVER_T> &listDriver) {
		return this->updateDrivers(listDriver);
	};
	clientConfig.pfnUpdateRealConfig = [ this ] (const std::list<MONITOR_POINT_CONFIG_T> &listRealConfig) {
		return this->updateRealConfig(listRealConfig);
	};
	clientConfig.pfnUpdateAlarmConfig = [ this ] (const std::list<ALARM_CONFIG_T> &listAlarmConfig) {
		return this->updateAlarmConfig(listAlarmConfig);
	};
	clientConfig.pfnDeleteRealConfig = [ this ] (const std::list<MONITOR_POINT_CONFIG_T> &listRealConfig) {
		return this->deleteRealConfig(listRealConfig);
	};
	clientConfig.pfnUpdateHistoryGroup = [ this ] (const std::list<MONITOR_GROUP_CONFIG_T> &listHistoryConfig) {
		return this->updateHistoryGroupConfig(listHistoryConfig);
	};
	clientConfig.pfnDeleteHistoryGroup = [ this ] (const std::list<MONITOR_GROUP_CONFIG_T> &listHistoryConfig) {
		return this->deleteHistoryGroupConfig(listHistoryConfig);
	};
	clientConfig.pfnUpdateLuaScript = [ this ] (const std::list<LUASCRIPT_T> &listLuascript) {
		return this->updateLuaScriptConfig(listLuascript);
	};
	clientConfig.pfnUpdateMode = [ this ] (unsigned int nMode) {
		if (nMode != m_nMode) {
			if (setCloudMode((eCLOUDMODE_T)nMode) < 0) {
				printf("%s:set cloud mode error\r\n", __FUNCTION__);
				return -1;
			}
			if (nMode == 1) {
#ifdef WIN32
				cbl::removeDir(getPrjCfgDir() + std::string("cloudprj"));
#else
				cbl::removeDir(getPrjCfgDir() + std::string("cloudprj"), true);
#endif
				CFile::Delete(getPrjCfgDir() + std::string("cloudprj.vwmt"));
			}
			else if (nMode == 2) {
#ifdef WIN32
				cbl::removeDir(getPrjCfgDir() + std::string("template"));
#else
				cbl::removeDir(getPrjCfgDir() + std::string("template"), true);
#endif
				CFile::Delete(getPrjCfgDir() + std::string("template.vwmt"));
			}
			if (PLATFORM::VBOX_MODE == PLATFORM::CPlatBoard::getDeviceMode()) {
				CRegister addr(REG_TYPE_HS_, HSW_RESET_R, DATA_TYPE_WORD);
				unsigned short nValue = 1;
				RW_PARAM_T rw;
				CRegisterRW::write(RW_FORMAT_INT, addr, &nValue, 1, rw);
			}
		}
		return 0;
	};
	clientConfig.pfnUpdateLuaSwitch = [ this ] (bool bStat) {
		m_bLuaStat = bStat;
		return 0;
	};
	clientConfig.pfnUpdateGlobalConfig = [ this ] (const std::list<GLOBAL_CONFIG_T> &listGlobalConfig) {
		return CCloudParse::get_instance().updateGlobalConfig(listGlobalConfig);
	};
	clientConfig.pfnUpdateRealSwitch = [ this ] () {
		unsigned long long ullTime = CTime::get_instance().getTickCount();
		if (CWPoint::get_instance().setPointPageStatus(true) < 0) {
			printf("%s:set page error\r\n", __FUNCTION__);
			return -1;
		}
		m_ullRealCollectRunningTime = ullTime;
		return 0;
	};
	clientConfig.pfnUpdateProject = [ this ] (const DOWNLOAD_PROJECT_INFO_T &downloadInfo) {
		return this->saveDownloadProjectXML(downloadInfo);
	};
	clientConfig.pfnDeleteProject = [ this ] () {
		return this->deleteProject();
	};
	clientConfig.pfnUpdateServerStat = [ this ] (bool bServerStat) {
		m_bServerStat = bServerStat;
		if (m_nMode != 0) {
			unsigned short nStat = bServerStat ? 1 : 2;
			CRegisterRW::writeWord(REG_TYPE_HS_, HSW_MQTTSTATE, nStat);
		}
		return 0;
	};
	clientConfig.pfnUpdate4GInfo = [this] (const FOURTHGENERALGINFO_T &fourthInfo) {
		int nLocationType = fourthInfo.nLocationType == 1 ? 1 : 2;
		char szValue[20] = { 0 };
		if (fourthInfo.sSimStat == "READY") {
			CRegisterRW::writeWord(REG_TYPE_HS_, HSW_SIMSTATE, 2);
		}
		else {
			CRegisterRW::writeWord(REG_TYPE_HS_, HSW_SIMSTATE, 1);
		}
		CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_4GNETLOCALTYPE, 0, (void *)&nLocationType, 1);
		CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_4GSIMREG, 0, (void *)&fourthInfo.nReg, 1);
		CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_4GNETLOCALLAC, 0, (void *)&fourthInfo.nLocationAreaCode, 1);
		CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_4GNETLOCALCI, 0, (void *)&fourthInfo.nCellID, 1);
		CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_4GOPERATOR, 0, (void *)&fourthInfo.nOperator, 1);
		CRegisterRW::write(RW_FORMAT_FLOAT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_4GLOCALLONGIT, 0, (void *)&fourthInfo.dLongitude, 4);
		CRegisterRW::write(RW_FORMAT_FLOAT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_4GLOCALLATIT, 0, (void *)&fourthInfo.dLatitude, 4);
		if (fourthInfo.sImei.length() >= 17) {
			strncpy(szValue, fourthInfo.sImei.c_str(), 16);
		}
		else {
			strncpy(szValue, fourthInfo.sImei.c_str(), fourthInfo.sImei.length());
		}
		CRegisterRW::write(RW_FORMAT_STR, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_4GIMEI, 0, (void *)szValue, 17);
		memset(szValue, 0, sizeof(szValue));
		if (fourthInfo.sModel.length() >= 10) {
			strncpy(szValue, fourthInfo.sModel.c_str(), 9);
		}
		else {
			strncpy(szValue, fourthInfo.sModel.c_str(), fourthInfo.sModel.length());
		}
		CRegisterRW::write(RW_FORMAT_STR, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_4GATITYPE, 0, (void *)szValue, 10);
		memset(szValue, 0, sizeof(szValue));
		if (fourthInfo.sIccid.length() >= 20) {
			strncpy(szValue, fourthInfo.sIccid.c_str(), 19);
		}
		else {
			strncpy(szValue, fourthInfo.sIccid.c_str(), fourthInfo.sIccid.length());
		}
		CRegisterRW::write(RW_FORMAT_STR, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_4GICCID, 0, (void *)szValue, 20);
		return 0;
	};
	clientConfig.pfnUpdateWifiSignal = [this](int nSignal) {
		CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_WIFINETWORKSIGNAL_R, 0, &nSignal, 1);
		return 0;
	};
	if (m_remoteProcedureCallClient.config(clientConfig) < 0) {
		printf("%s:remote procedure call client config error\r\n", __FUNCTION__);
		return -1;
	}
	if (m_remoteProcedureCallClient.start() < 0) {
		printf("%s:remote procedure call client start error\r\n", __FUNCTION__);
		return -2;
	}
	m_bRunning = true;
	return 0;
}

int CCloudOperator::verifyUrl(const std::string &sToken, const std::string &sSignature) {
	if (!m_bRunning) {
		return -1;
	}
	return m_remoteProcedureCallClient.verifyUrl(m_cloudConfig.sProjectMd5, sToken, sSignature);
}

int CCloudOperator::setHistoryData(const std::list<MONITOR_GROUP_T> &listMonitorGroup) {
	if (!m_bRunning) {
		return -1;
	}
	if (m_nMode == 0 &&
		(m_nAdditionalFunction & (1 << ADDITIONAL_FUNCTION_CLOUD_SCADA)) == 0) {
		return -2;
	}
	return m_remoteProcedureCallClient.setHistoryData(listMonitorGroup);
}

int CCloudOperator::setAlarmData(const std::list<ALARM_DATA_T> &listAlarmData) {
	if (!m_bRunning) {
		return -1;
	}
	if (m_nMode == 0 &&
		(m_nAdditionalFunction & (1 << ADDITIONAL_FUNCTION_CLOUD_SCADA)) == 0) {
		return -2;
	}
	return m_remoteProcedureCallClient.setAlarmData(listAlarmData);
}

int CCloudOperator::setRealData(const std::list<MONITOR_POINT_T> &listRealData) {
	if (!m_bRunning) {
		return -1;
	}
	if (m_nMode == 0 &&
		(m_nAdditionalFunction & (1 << ADDITIONAL_FUNCTION_CLOUD_SCADA)) == 0) {
		return -2;
	}
	CWPoint::get_instance().setReadCondition(MONITOR_POINT_READ_CHANGED_DATA);
	return m_remoteProcedureCallClient.setRealData(listRealData);
}

int CCloudOperator::setHistoryGroupPeriod(unsigned int nId, unsigned int nPeriod) {
	if (!m_bRunning) {
		return -1;
	}
	if (m_nMode == 0 &&
		(m_nAdditionalFunction & (1 << ADDITIONAL_FUNCTION_CLOUD_SCADA)) == 0) {
		return -2;
	}
	return m_remoteProcedureCallClient.setHistoryGroupPeriod(nId, nPeriod);
}

int CCloudOperator::setAlarmConfigCondition(unsigned int nId, const std::string &sCondition) {
	if (!m_bRunning) {
		return -1;
	}
	if (m_nMode == 0 &&
		(m_nAdditionalFunction & (1 << ADDITIONAL_FUNCTION_CLOUD_SCADA)) == 0) {
		return -2;
	}
	return m_remoteProcedureCallClient.setAlarmConfigCondition(nId, sCondition);
}

int CCloudOperator::setLuaInfo(const std::list<LUA_INFOMATION_T> &listLuaInfo) {
	if (!m_bRunning) {
		return -1;
	}
	{
		std::unique_lock<std::mutex> lck(m_mutexLua);
		for (auto &iter : listLuaInfo) {
			if (!iter.sError.empty()) {
				if (m_listLuaInfo.size() >= MAXLUACOUNT) {
					m_listLuaInfo.pop_front();
				}
				m_listLuaInfo.push_back(iter);
			}
			else if (!iter.sbuffer.empty() &&
				m_bLuaStat) {
				if (m_sLuabuffer.length() <= 1024 * 100 - 4) {
					m_sLuabuffer += iter.sbuffer;
				}
			}
		}
	}
	return 0;
}

int CCloudOperator::getbasicInfo(UPDATE_CONFIG_T eUpdateConfig) {
	std::list<COMMU_CONFIG_T> listCommuConfig;
	std::list<MONITOR_GROUP_CONFIG_T> listRealConfig;
	std::list<MONITOR_GROUP_CONFIG_T> listHistoryConfig;
	std::list<ALARM_CONFIG_T> listAlarmConfig;
	if (CWPoint::get_instance().getConfig(listRealConfig, eUpdateConfig, m_nLanguage) < 0) {
		printf("%s:get real config error\r\n", __FUNCTION__);
		return -1;
	}
	if (CDataRecordCollect::get_instance().getReportConfig(listHistoryConfig) < 0) {
		printf("%s:get history config error\r\n", __FUNCTION__);
		return -2;
	}
	for (auto &iter : listHistoryConfig) {
		iter.nUpdate_Config = eUpdateConfig;
		iter.nCurrentLanguage = m_nLanguage;
		for (auto &iterP : iter.MonitorPoint) {
			iterP.nUpdate_Config = eUpdateConfig;
			iterP.nCurrentLanguage = m_nLanguage;
		}
	}
	if (CAlarmCollect::get_instance().getReportConfig(listAlarmConfig) < 0) {
		printf("%s:get alarm config error\r\n", __FUNCTION__);
		return -3;
	}
	for (auto &iter : listAlarmConfig) {
		iter.nUpdate_Config = eUpdateConfig;
		iter.nCurrentLanguage = m_nLanguage;
	}

#define	 ETHCONFIG_STARTADDR		11004		//网络通讯，目标IP配置寄存器的起始地址
#define  ETHCONFIG_ADDRLEN			7			//网络通讯，目标IP配置寄存器的长度

	std::list<CONNECTNODE> listCommu = CCloudParse::get_instance().getCommu();
	int nCOM1 = 0, nCOM2 = 0, nCOM3 = 0;
	int szCOM1[] = { 10061, 10094, 10120 };
	int szCOM2[] = { 10072, 10105 };
	int szCOM3[] = { 10083 };
	int szBaudRate[] = { 1200,2400,4800,9600,19200,38400,57600,115200,187500,
		230400,460800,500000,576000,921600,1000000,1152000,1500000,
		2000000,2500000,3000000,3500000,4000000 };
	if (eUpdateConfig == UPDATE_LANGUAGE) {
		listCommu.clear();
	}
	for (auto &iter : listCommu) {
		COMMU_CONFIG_T commuConfig;
		if (strncmp("COM", iter.Commset.szInitPortName, 3) == 0) {
			unsigned short szwData[16] = { 0 };
			if (strcmp("1", iter.Commset.szInitPortName + 3) == 0) {
				CRegisterRW::read(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, szCOM1[nCOM1++], 0, &szwData, 11);
			}
			else if (strcmp("2", iter.Commset.szInitPortName + 3) == 0) {
				CRegisterRW::read(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, szCOM2[nCOM2++], 0, &szwData, 11);
			}
			else if (strcmp("3", iter.Commset.szInitPortName + 3) == 0) {
				CRegisterRW::read(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, szCOM3[nCOM3++], 0, &szwData, 11);
			}
			commuConfig.nCommuPortType = (DeviceCommuPortType)szwData[0];
			commuConfig.nBaudrate = szBaudRate[szwData[1]];
			commuConfig.nDataLen = szwData[2];
			commuConfig.nStopBit = szwData[3];
			commuConfig.nCheckBit = (CheckBitType)szwData[4];
			commuConfig.nWaitTimeoutTime = szwData[5];
			commuConfig.nReceiveTimeout = szwData[6];
			commuConfig.nRetryCount = szwData[7];
			commuConfig.nRetryTimeoutTime = szwData[8];
			commuConfig.nDeviceStation = szwData[9];
			commuConfig.nPlcStation = szwData[10];
		}
		else if (strcmp("Ethernet", iter.Commset.szInitPortName) == 0) {
			unsigned short			szwData[8] = { 0 };
			char			szIP[16] = { 0 };
			unsigned int	wEthAddrOffset = ETHCONFIG_STARTADDR + iter.nPortId * ETHCONFIG_ADDRLEN;

			CRegisterRW::read(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, wEthAddrOffset, 0, &szwData, ETHCONFIG_ADDRLEN);
			sprintf(szIP, "%d.%d.%d.%d", szwData[0], szwData[1], szwData[2], szwData[3]);
#ifndef WIN32
			commuConfig.nPlcNetworkIp = inet_addr(szIP);
#endif
			commuConfig.nPlcNetworkPort = szwData[4];
			commuConfig.sNetworkType = getNetType(szwData[5]);
			commuConfig.nDeviceStation = iter.PlcSet.nHmiMachineNo;
			commuConfig.nPlcStation = iter.PlcSet.nPlcMachineNo;
			if (iter.Commset.nCommMode == 6) {
				commuConfig.nCommuPortType = DEVICE_COMMU_PORT_485;
			}
			else {
				commuConfig.nCommuPortType = (DeviceCommuPortType)iter.Commset.nCommMode;
			}
			commuConfig.nWaitTimeoutTime = iter.Commset.nWaitTimeout;
			commuConfig.nReceiveTimeout = iter.Commset.nRecvTimeout;
			commuConfig.nRetryCount = iter.Commset.nRetryTimes;
			commuConfig.nRetryTimeoutTime = iter.Commset.nRetryTimeOut;
			commuConfig.nBroadcastAddress = iter.Commset.wBroadcastAddr;
			commuConfig.IsBroadcast = iter.Commset.bEthernet_IsBroadcast;
		}
		else if (strncmp("USB", iter.Commset.szInitPortName, 3) == 0) {
			if (iter.Commset.nCommMode == 6) {
				commuConfig.nCommuPortType = DEVICE_COMMU_PORT_485;
			}
			else {
				commuConfig.nCommuPortType = (DeviceCommuPortType)iter.Commset.nCommMode;
			}
			commuConfig.nDeviceStation = iter.PlcSet.nHmiMachineNo;
			commuConfig.nPlcStation = iter.PlcSet.nPlcMachineNo;
			commuConfig.nWaitTimeoutTime = iter.Commset.nWaitTimeout;
			commuConfig.nReceiveTimeout = iter.Commset.nRecvTimeout;
			commuConfig.nRetryCount = iter.Commset.nRetryTimes;
			commuConfig.nRetryTimeoutTime = iter.Commset.nRetryTimeOut;
		}
		if (strlen(iter.PlcSet.szPlctypeAlias) != 0) {
			commuConfig.sProtocolName = iter.PlcSet.szPlctypeAlias;
		}
		else {
			commuConfig.sProtocolName = iter.PlcSet.szPLCType;
		}
		commuConfig.sCommuPortName = iter.Commset.szInitPortName;
		commuConfig.nCommuDelayTime = iter.Commset.nComIoDelayTime;
		commuConfig.nContinuousLength = iter.Commset.nComSingleReadLen;
		commuConfig.nIntegrationInterval = iter.Commset.nComStepInterval;
		commuConfig.nCommuId = iter.nPortId + 1;
		listCommuConfig.push_back(commuConfig);
	}
	m_bLanguageStat = false;
	return m_remoteProcedureCallClient.setBasicInfo(listCommuConfig, listRealConfig, listHistoryConfig, listAlarmConfig);
}

static int updateCommuPortId(std::list<CONNECTNODE> &listCommu, const CONNECTNODE &commu) {
	for (auto &iter : listCommu) {
		if (0 == strcmp(iter.PlcSet.szPLCType, commu.PlcSet.szPLCType)) {
			iter.nPortId = commu.nPortId;
			return 0;
		}
	}
	return -1;
}

int CCloudOperator::updateCommu(const std::list<COMMU_CONFIG_T> &listCommu) {
	if (listCommu.empty() ||
		this->verifyProject() < 0) {
		return 0;
	}
	if (!cbl::isDir(getPrjDir() + std::string("PlcType"))) {
		cbl::makeDir(getPrjDir() + std::string("PlcType"));
	}
	if (!cbl::isDir(getPrjDir() + std::string("Modbus"))) {
		cbl::makeDir(getPrjDir() + std::string("Modbus"));
	}
	std::list<COMMU_RESPONSE_T> listCommursp;
	std::list<CONNECTNODE> listCommuNode = CCloudParse::get_instance().getCommu();
	for (auto &iter : listCommu) {
		int nRtn = 0;
		CONNECTNODE Commu;
		memset(&Commu, 0, sizeof(CONNECTNODE));
		int nDriverStat = 1, nPlcTypeStat = 1, nModusStat = 1;
		bool bLoad = false, bFind = false;
		std::string sDriverFileMD5 = iter.sFileMd5;
		std::string sPlcTypeFileMD5 = iter.sPlcTypeMd5;
		std::string sModubusFileMD5 = iter.sModbusMd5;
		unsigned int nFramework = iter.nFramework;
		if (toCommuConfig(iter, Commu) < 0) {
			printf("%s: to commu config error\r\n", __FUNCTION__);
			return -1;
		}
		for (auto &iterCommu : listCommuNode) {
			if (iterCommu.nCommuId != Commu.nCommuId) {
				continue;
			}
			bFind = true;
			if (0 == strcmp(Commu.PlcSet.szPLCType, iterCommu.PlcSet.szPLCType) &&
				0 == strcmp(Commu.PlcSet.szLinkProtocol, iterCommu.PlcSet.szLinkProtocol)) {
				int32_t nPort = iterCommu.nPortId;
				iterCommu = Commu;
				iterCommu.nPortId = nPort;
				if (iterCommu.nPortId == -1) {
					bLoad = true;
				}
				else {
					if ((nRtn = CEngineMgr::getInstance().reLoadPlcDriVer(iterCommu)) < 0) {
						nDriverStat = -1;
						printf("%s: reload plc driver error:%d\r\n", __FUNCTION__, nRtn);
					}
				}
			}
			else {
				if ((nRtn = CCloudParse::get_instance().unloadCommu(iterCommu)) < 0) {
					nDriverStat = -2;
					printf("%s:unload commu error:%d\r\n", __FUNCTION__, nRtn);
				}
				if ((nRtn = CWPoint::get_instance().deletePointByCommuID(iterCommu.nCommuId)) < 0) {
					printf("%s: delete real error:%d\r\n", __FUNCTION__, nRtn);
					nDriverStat = -3;
				}
				if ((nRtn = CDataRecordCollect::get_instance().removeFieldByCommuID(iterCommu.nCommuId)) < 0) {
					printf("%s: delete history error:%d\r\n", __FUNCTION__, nRtn);
					nDriverStat = -4;
				}
				if ((nRtn = CAlarmCollect::get_instance().removeByCommuID(iterCommu.nCommuId)) < 0) {
					printf("%s: delete alarm error:%d\r\n", __FUNCTION__, nRtn);
					nDriverStat = -5;
				}
				if (nDriverStat == 1) {
					bLoad = true;
				}
				iterCommu = Commu;
			}
			break;
		}
		if (!bFind) {
			listCommuNode.push_back(Commu);
			CCloudParse::get_instance().saveModbusRegMap(listCommuNode, getPrjDir());
			bLoad = true;
		}
		std::string sMd5 = "";
		std::string sPLCType = Commu.PlcSet.szPLCType;
		sPLCType = cbl::replaceall(sPLCType, "/", "_");
		sPLCType = cbl::replaceall(sPLCType, "\\", "_");
		sMd5.clear();
		if (!sPlcTypeFileMD5.empty() &&
			(getFileMd5(cbl::format("%sPlcType/%s.plc", getPrjDir().c_str(), sPLCType.c_str()), sMd5) < 0 ||
				cbl::ucase(sMd5) != cbl::ucase(sPlcTypeFileMD5))) {
			nPlcTypeStat = 0;
		}
		sMd5.clear();
		if (!sModubusFileMD5.empty() &&
			(getFileMd5(cbl::format("%sModbus/%s.xml", getPrjDir().c_str(), sPLCType.c_str()), sMd5) < 0 ||
				cbl::ucase(sMd5) != cbl::ucase(sModubusFileMD5))) {
			nModusStat = 0;
		}
		if (FRAMEWORK_NEW != nFramework) {
			sMd5.clear();
			if (!sDriverFileMD5.empty() &&
				(getFileMd5(cbl::format("%s%s", getPrjDir().c_str(), Commu.PlcSet.szLinkProtocol), sMd5) < 0 ||
					cbl::ucase(sMd5) != cbl::ucase(sDriverFileMD5))) {
				nDriverStat = 0;
			}
			
		}
		
		if (1 == nDriverStat &&
			1 == nPlcTypeStat &&
			1 == nModusStat &&
			bLoad) {
			if ((nRtn = CCloudParse::get_instance().loadCommu(Commu, getPrjDir())) < 0) {
				nDriverStat = -6;
				printf("%s:load commu error:%d\r\n", __FUNCTION__, nRtn);
			}
			else {
				if (updateCommuPortId(listCommuNode, Commu) < 0) {
					nDriverStat = -7;
					printf("%s: update commu port id error\r\n", __FUNCTION__);
				}
			}
		}
		COMMU_RESPONSE_T commursp;
		commursp.nCommuId = Commu.nCommuId;
		commursp.nStat = 1 == nDriverStat ? 1 : 0;
		commursp.nType = UPDATE_COMSET_DRIVER_STATE;
		listCommursp.push_back(commursp);
		commursp.nStat = 1 == nPlcTypeStat ? 1 : 0;
		commursp.nType = UPDATE_COMSET_PLC_STATE;
		listCommursp.push_back(commursp);
		commursp.nStat = 1 == nModusStat ? 1 : 0;
		commursp.nType = UPDATE_COMSET_MODBUS_STATE;
		listCommursp.push_back(commursp);
	}
	if (!listCommursp.empty()) {
		if (m_remoteProcedureCallClient.updateCommursp(listCommursp) < 0) {
			printf("%s:update commu response error\r\n", __FUNCTION__);
			return -2;
		}
		if (CCloudParse::get_instance().setCommu(listCommuNode) < 0) {
			printf("%s: set commu error\r\n", __FUNCTION__);
			return -3;
		}
		if (CCloudParse::get_instance().save() < 0) {
			printf("%s: save error\r\n", __FUNCTION__);
			return -4;
		}
	}
	return 0;
}

int CCloudOperator::deleteCommu(const std::list<COMMU_CONFIG_T> &listCommu) {
	if (listCommu.empty() ||
		this->verifyProject() < 0) {
		return 0;
	}
	bool bFind = false;
	std::list<CONNECTNODE> listCommuNode = CCloudParse::get_instance().getCommu();
	for (auto &iter : listCommu) {
		for (std::list<CONNECTNODE>::iterator iterCommu = listCommuNode.begin(); iterCommu != listCommuNode.end();) {
			if ((int)iter.nCommuId != iterCommu->nCommuId) {
				iterCommu++;
				continue;
			}
			if (CCloudParse::get_instance().unloadCommu(*iterCommu) < 0) {
				return -1;
			}
			if (CWPoint::get_instance().deletePointByCommuID(iterCommu->nCommuId) < 0) {
				printf("%s: delete real error\r\n", __FUNCTION__);
				return -2;
			}
			if (CDataRecordCollect::get_instance().removeFieldByCommuID(iterCommu->nCommuId) < 0) {
				printf("%s: delete history error\r\n", __FUNCTION__);
				return -3;
			}
			if (CAlarmCollect::get_instance().removeByCommuID(iterCommu->nCommuId) < 0) {
				printf("%s: delete alarm error\r\n", __FUNCTION__);
				return -4;
			}
			iterCommu = listCommuNode.erase(iterCommu);
			bFind = true;
			break;
		}
	}
	if (bFind) {
		CCloudParse::get_instance().saveModbusRegMap(listCommuNode, getPrjDir());
		if (CCloudParse::get_instance().setCommu(listCommuNode) < 0) {
			printf("%s: set commu error\r\n", __FUNCTION__);
			return -5;
		}
		if (CCloudParse::get_instance().save() < 0) {
			printf("%s: save error\r\n", __FUNCTION__);
			return -6;
		}
	}
	return 0;
}

int CCloudOperator::updateDrivers(const std::list<DRIVER_T>& listDriver) {
	if (listDriver.empty() ||
		this->verifyProject() < 0) {
		return 0;
	}
	std::list<DRIVER_RESPONSE_T> listDriverrsp;
	std::list<CONNECTNODE> listCommu = CCloudParse::get_instance().getCommu();
	int nRtn = 0;
	bool bFlag = false;

	// unload old driver
	for (auto &iter : listDriver) {
		for (auto &iterCommu : listCommu) {
				if(iter.nCommu != (unsigned int)iterCommu.nCommuId) {
				continue;
			}
			if ((nRtn = CCloudParse::get_instance().unloadCommu(iterCommu)) < 0) {
				printf("%s:unload commu error:%d\r\n", __FUNCTION__, nRtn);
			}
			break;
		}
	}

	for (auto &iter : listDriver) {
		DRIVER_RESPONSE_T driverrsp;
		std::string sFileName = "", sFilePath = getPrjDir();
		driverrsp.nCommuId = iter.nCommu;
		driverrsp.sDriverName = iter.sDriverName;
		if (std::string::npos != iter.sFileName.find(".so")) {
			sFileName = cbl::lcase(iter.sFileName);
		}
		else {
			if (std::string::npos != iter.sFileName.find(".plc")) {
				sFileName = cbl::format("%s.plc", iter.sDriverName.c_str());
				sFilePath += "PlcType/";
			}
			else if (iter.sFileName != "ABMicro850FreeTags.xml" && 
				std::string::npos != iter.sFileName.find(".xml")) {
				sFileName = cbl::format("%s.xml", iter.sDriverName.c_str());
				sFilePath += "Modbus/";
			}
			else {
				sFileName = iter.sFileName;
			}
			sFileName = cbl::replaceall(sFileName, "/", "_");
			sFileName = cbl::replaceall(sFileName, "\\", "_");
		}
		if (sFileName.empty()) {
			continue;
		}
		sFilePath += sFileName;
		cbl::CBase64 base64;
		std::string sbuffer = "";
		std::string sMd5 = "";
		// file exist and file md5 is right
		if (cbl::getFileSize(sFilePath) > 0) {
			if ((nRtn = cbl::md5File(sFilePath, sMd5)) == 0 &&
				cbl::ucase(sMd5) == cbl::ucase(iter.sFileMd5)) {
				listDriverrsp.push_back(driverrsp);
				printf("%s:%s has same so\r\n", __FUNCTION__, sFilePath.c_str());
				continue;
			}
			else {
				CFile::Delete(sFilePath);
				printf("%s:delete %s old file!\r\n", __FUNCTION__, sFilePath.c_str());
			}
		}
		if ((nRtn = base64.decode(iter.sFileBuffer, sbuffer)) < 0) {
			printf("%s:base64 decode error:%d\r\n", __FUNCTION__, nRtn);
			driverrsp.nStat = -1;
			listDriverrsp.push_back(driverrsp);
			continue;
		}
		if ((nRtn = cbl::md5String(sbuffer, sMd5)) < 0) {
			printf("%s:md5 error:%d\r\n", __FUNCTION__, nRtn);
			driverrsp.nStat = -2;
			listDriverrsp.push_back(driverrsp);
			continue;
		}
		driverrsp.sDriverMd5 = sMd5;
		if (cbl::ucase(sMd5) != cbl::ucase(iter.sFileMd5)) {
			driverrsp.nStat = -3;
			listDriverrsp.push_back(driverrsp);
			continue;
		}
#ifdef WIN32
		if ((nRtn = cbl::saveToFile(sbuffer, sFilePath)) < 0) {
#else
		if ((nRtn = cbl::syncSaveToFile(sbuffer, sFilePath)) < 0) {
#endif
			printf("%s:save file error\r\n", __FUNCTION__);
			driverrsp.nStat = -4;
			listDriverrsp.push_back(driverrsp);
			continue;
		}
		listDriverrsp.push_back(driverrsp);
	}
	for (auto &iter : listDriverrsp) {
		if (iter.nStat < 0) {
			continue;
		}
		for (auto &iterCommu : listCommu) {
			if(iter.nCommuId != (unsigned int)iterCommu.nCommuId) {
				continue;
			}
			if (iterCommu.nPortId >= 0) {
				if ((nRtn = CCloudParse::get_instance().unloadCommu(iterCommu)) < 0) {
					printf("%s:unload commu error:%d\r\n", __FUNCTION__, nRtn);
					iter.nStat = -5;
				}
			}
			if (iter.nStat == 0) {
				CCloudParse::get_instance().saveModbusRegMap(listCommu, getPrjDir());
				if ((nRtn = CCloudParse::get_instance().loadCommu(iterCommu, getPrjDir())) < 0) {
					printf("%s:load commu error:%d\r\n", __FUNCTION__, nRtn);
					iter.nStat = -6;
				}
				else {
					iter.nStat = 1;
					bFlag = true;
				}
			}
			break;
		}
	}
	if (!listDriverrsp.empty()) {
		if (m_remoteProcedureCallClient.updateDriverrsp(listDriverrsp) < 0) {
			printf("%s: update driver response error\r\n", __FUNCTION__);
			return -1;
		}
		if (bFlag) {
			if (CCloudParse::get_instance().setCommu(listCommu) < 0) {
				printf("%s: set commu error\r\n", __FUNCTION__);
				return -2;
			}
			if (CCloudParse::get_instance().save() < 0) {
				printf("%s: save error\r\n", __FUNCTION__);
				return -3;
			}
		}
	}
	return 0;
}

int CCloudOperator::updateRealConfig(const std::list<MONITOR_POINT_CONFIG_T> &listRealConfig) {
	if (listRealConfig.empty() ||
		this->verifyProject() < 0) {
		return 0;
	}
	std::list<REAL_CONFIG_RESPONSE_T> listRealrsp;
	bool bFlag = false;
	for (auto &iter : listRealConfig) {
		REAL_CONFIG_RESPONSE_T realrsp;
		realrsp.nId = iter.nID;
		realrsp.nType = iter.nType;
		if (MONITOR_DATA_TYPE_REALDATA == iter.nType) {
			COLLECTION_MONITOR_POINT_DATA realPoint;
			if (toRealConfig(iter, realPoint) < 0 || 
				CWPoint::get_instance().addPoint(iter.nGroupId, realPoint, false) < 0) {
				listRealrsp.push_back(realrsp);
				continue;
			}
			realrsp.nStat = 1;
			listRealrsp.push_back(realrsp);
			bFlag = true;
		}
		else if (MONITOR_DATA_TYPE_HISTORYDATA == iter.nType) {
			COLLECTION_HISTORY_NODE historyPoint;
			if (toHistoryConfig(iter, historyPoint) < 0 ||
				CDataRecordCollect::get_instance().addField(iter.nGroupId, historyPoint) < 0) {
				listRealrsp.push_back(realrsp);
				continue;
			}
			realrsp.nStat = 1;
			listRealrsp.push_back(realrsp);
			bFlag = true;
		}
	}
	if (bFlag) {
		if (CCloudParse::get_instance().save() < 0) {
			printf("%s:save error\r\n", __FUNCTION__);
			return -1;
		}
	}
	if (!listRealrsp.empty()) {
		if (m_remoteProcedureCallClient.updateRealrsp(listRealrsp) < 0) {
			printf("%s:update real response error\r\n", __FUNCTION__);
			return -2;
		}
	}
	return 0;
}

int CCloudOperator::updateAlarmConfig(const std::list<ALARM_CONFIG_T> &listAlarmConfig) {
	if (listAlarmConfig.empty() ||
		this->verifyProject() < 0) {
		return 0;
	}
	bool bFlag = false;
	std::list<REAL_CONFIG_RESPONSE_T> listRealrsp;
	for (auto &iter : listAlarmConfig) {
		COLLECTION_ALARM_DATA alarmConfig;
		REAL_CONFIG_RESPONSE_T realrsp;
		realrsp.nId = iter.nId;
		realrsp.nType = iter.nType;
		if (toAlarmConfig(iter, alarmConfig) < 0 ||
			CAlarmCollect::get_instance().add(alarmConfig, false) < 0) {
			listRealrsp.push_back(realrsp);
			continue;
		}
		bFlag = true;
		realrsp.nStat = 1;
		listRealrsp.push_back(realrsp);
	}
	if (bFlag) {
		if (CCloudParse::get_instance().save() < 0) {
			printf("%s:save error\r\n", __FUNCTION__);
			return -1;
		}
	}
	if (!listRealrsp.empty()) {
		if (m_remoteProcedureCallClient.updateAlarmrsp(listRealrsp) < 0) {
			printf("%s:update alarm response error\r\n", __FUNCTION__);
			return -2;
		}
	}
	return 0;
}

int CCloudOperator::deleteRealConfig(const std::list<MONITOR_POINT_CONFIG_T>& listRealConfig) {
	if (listRealConfig.empty() ||
		this->verifyProject() < 0) {
		return 0;
	}
	bool bFlag = false;
	std::list<REAL_CONFIG_RESPONSE_T> listRealrsp;
	for (auto &iter : listRealConfig) {
		REAL_CONFIG_RESPONSE_T realrsp;
		realrsp.nId = iter.nID;
		realrsp.nType = iter.nType;
		if (MONITOR_DATA_TYPE_REALDATA == iter.nType) {
			if (CWPoint::get_instance().existPoint(iter.nID) &&
				0 == CWPoint::get_instance().deletePoint(iter.nID)) {
				bFlag = true;
			}
			realrsp.nStat = 1;
			listRealrsp.push_back(realrsp);
		}
		else if (MONITOR_DATA_TYPE_HISTORYDATA == iter.nType) {
			if (CDataRecordCollect::get_instance().existField(iter.nID) && 
				0 == CDataRecordCollect::get_instance().removeField(iter.nID)) {
				bFlag = true;
			}
			realrsp.nStat = 1;
			listRealrsp.push_back(realrsp);
		}
		else if (MONITOR_DATA_TYPE_ALARMDATA == iter.nType) {
			if (CAlarmCollect::get_instance().exist(iter.nID) &&
				0 == CAlarmCollect::get_instance().remove(iter.nID)) {
				bFlag = true;
			}
			realrsp.nStat = 1;
			listRealrsp.push_back(realrsp);
		}
	}
	if (bFlag) {
		if (CCloudParse::get_instance().save() < 0) {
			printf("%s:save error\r\n", __FUNCTION__);
			return -1;
		}
	}
	if (!listRealrsp.empty()) {
		if (m_remoteProcedureCallClient.deleteRealrsp(listRealrsp) < 0) {
			printf("%s:update alarm response error\r\n", __FUNCTION__);
			return -2;
		}
	}
	return 0;
}

int CCloudOperator::deleteHistoryGroupConfig(const std::list<MONITOR_GROUP_CONFIG_T> &listHistoryConfig) {
	if (listHistoryConfig.empty() ||
		this->verifyProject() < 0) {
		return 0;
	}
	bool bFlag = false;
	std::list<REAL_CONFIG_RESPONSE_T> listRealrsp;
	for (auto &iter : listHistoryConfig) {
		REAL_CONFIG_RESPONSE_T realrsp;
		realrsp.nId = iter.nID;
		if (!CDataRecordCollect::get_instance().existGroup(iter.nID) ||
			CDataRecordCollect::get_instance().removeGroup(iter.nID) == 0) {
			realrsp.nStat = 1;
			bFlag = true;
		}
		listRealrsp.push_back(realrsp);
	}
	if (bFlag) {
		if (CCloudParse::get_instance().save() < 0) {
			printf("%s:save error\r\n", __FUNCTION__);
			return -1;
		}
	}
	if (!listRealrsp.empty()) {
		if (m_remoteProcedureCallClient.deleteHistoryGrouprsp(listRealrsp) < 0) {
			printf("%s:delete history group response error\r\n", __FUNCTION__);
			return -2;
		}
	}
	return 0;
}

int CCloudOperator::updateHistoryGroupConfig(const std::list<MONITOR_GROUP_CONFIG_T> &listHistoryConfig) {
	if (listHistoryConfig.empty() ||
		this->verifyProject() < 0) {
		return 0;
	}
	bool bFlag = false;
	std::list<REAL_CONFIG_RESPONSE_T> listRealrsp;
	for (auto &iter : listHistoryConfig) {
		REAL_CONFIG_RESPONSE_T realrsp;
		realrsp.nId = iter.nID;
		COLLECTION_HISTORY_DATA historyConfig;
		if (toHistoryGroupConfig(iter, historyConfig) == 0 &&
			CDataRecordCollect::get_instance().addGroup(historyConfig, false) == 0) {
			realrsp.nStat = 1;
			bFlag = true;
		}
		listRealrsp.push_back(realrsp);
	}
	if (bFlag) {
		if (CCloudParse::get_instance().save() < 0) {
			printf("%s:save error\r\n", __FUNCTION__);
			return -1;
		}
	}
	if (!listRealrsp.empty()) {
		if (m_remoteProcedureCallClient.updateHistoryGrouprsp(listRealrsp) < 0) {
			printf("%s:delete history group response error\r\n", __FUNCTION__);
			return -2;
		}
	}
	return 0;
}

int CCloudOperator::updateLuaScriptConfig(const std::list<LUASCRIPT_T> &listLuascript) {
	if (listLuascript.empty() ||
		this->verifyProject() < 0) {
		return 0;
	}
	std::list<LUA_INFOMATION_T> listLuaInfo;
	for (auto iter : listLuascript) {
		LUA_INFOMATION_T luaInfo;
		int nRtn = 0;
		luaInfo.nId = iter.nId;
		if (!iter.sContent.empty()) {
			std::string sCompilebuffer;
			std::string sError;
			if (CCompileAddress::get_instance().compileLua(iter.sContent, sCompilebuffer, sError) < 0) {
				luaInfo.nStat = LUARESULT_COMFAILED;
				luaInfo.sError = sError;
				listLuaInfo.push_back(luaInfo);
				continue;
			}
			iter.sContent = sCompilebuffer;
		}
		switch (iter.nOptStat) {
		case LUAOPT_ADD:
		case LUAOPT_UPDATE:
		case LUAOPT_START:
			{
				nRtn = CCloudParse::get_instance().m_luaPrase.insert(iter);
			}
			break;
		case LUAOPT_DELETE:
		case LUAOPT_STOP:
			{
				nRtn = CCloudParse::get_instance().m_luaPrase.del(iter.nId);
			}
			break;
		case LUAOPT_NONE:
		case LUAOPT_OTHER:
		default:
			break;
		}
		if (0 == nRtn) {
			luaInfo.nStat = LUARESULT_NORMAL;
			listLuaInfo.push_back(luaInfo);
		}
	}
	if (!listLuaInfo.empty()) {
		if (m_remoteProcedureCallClient.setLuaStat(listLuaInfo) < 0) {
			printf("%s:set lua stat response error\r\n", __FUNCTION__);
			return -2;
		}
	}
	return 0;
}

int CCloudOperator::saveDownloadProjectXML(const DOWNLOAD_PROJECT_INFO_T &downloadInfo) {
#ifdef WIN32
	tinyxml2::TinyXMLDocument doc;
#else
	tinyxml2::XMLDocument doc;
#endif
	std::string sPath = getPrjCfgDir() + std::string("httpDownLoad.xml");
	if (UTILITY::CFile::ExistFile(sPath)) {
		UTILITY::CFile::Delete(sPath);
	}
	if (0 != doc.Parse("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n<down>\r\n</down>\r\n")) {
		return -1;
	}
	tinyxml2::XMLElement *pDown = doc.RootElement();
	if (nullptr == pDown) {
		return -2;
	}
	tinyxml2::XMLElement *pDownLoad = doc.NewElement("downLoad");
	if (nullptr == pDownLoad) {
		return -3;
	}
	pDownLoad->SetAttribute("port", downloadInfo.nPort);
	pDownLoad->SetAttribute("FileName", downloadInfo.sFileName.c_str());
	pDownLoad->SetAttribute("addr", downloadInfo.sDomainName.c_str());
	pDownLoad->SetAttribute("FilePath", downloadInfo.sUrlPath.c_str());
	pDownLoad->SetAttribute("FileMD5", downloadInfo.sProjectMd5.c_str());
	pDown->InsertEndChild(pDownLoad);
	int nRtn = -1;
	XMLPrinter printer;
	doc.Print(&printer);
#ifdef WIN32
	nRtn = cbl::saveToFile(printer.CStr(), sPath);
#else
	nRtn = cbl::syncSaveToFile(printer.CStr(), sPath);
#endif
	if (nRtn < 0) {
		return -4;
	}
	return 0;
}

int CCloudOperator::downloadProject() {
	int nRtn = 0;
	std::string sPath = getPrjCfgDir() + std::string("httpDownLoad.xml");
	DOWNLOAD_PROJECT_INFO_T downloadInfo;
	if (!UTILITY::CFile::ExistFile(sPath)) {
		return -1;
	}
#ifdef WIN32
	tinyxml2::TinyXMLDocument doc;
#else
	tinyxml2::XMLDocument doc;
#endif
	doc.LoadFile(sPath.c_str());
	tinyxml2::XMLElement *pElement = doc.RootElement();
	if (nullptr == pElement) {
		return -2;
	}
	tinyxml2::XMLElement *pDownload = pElement->FirstChildElement("downLoad");
	if (nullptr == pDownload) {
		return -3;
	}
	if (pDownload->Attribute("port")) {
		downloadInfo.nPort = (unsigned int)strtoul(pDownload->Attribute("port"), nullptr, 10);
	}
	if (pDownload->Attribute("FileName")) {
		downloadInfo.sFileName = pDownload->Attribute("FileName");
	}
	if (pDownload->Attribute("addr")) {
		downloadInfo.sDomainName = pDownload->Attribute("addr");
	}
	if (pDownload->Attribute("FilePath")) {
		downloadInfo.sUrlPath = pDownload->Attribute("FilePath");
	}
	if (pDownload->Attribute("FileMD5")) {
		downloadInfo.sProjectMd5 = pDownload->Attribute("FileMD5");
	}
	if (downloadInfo.sProjectMd5.empty() ||
		downloadInfo.sUrlPath.empty() ||
		downloadInfo.sFileName.empty() ||
		downloadInfo.sDomainName.empty()) {
		printf("%s: download param is empty\r\n", __FUNCTION__);
		return -4;
	}
	bool bDownload = true;
	std::string sFilePath = getPrjCfgDir() + downloadInfo.sFileName;
	std::string sMd5 = "";
	if (UTILITY::CFile::ExistFile(sFilePath)) {
		if (cbl::md5File(sFilePath, sMd5) < 0) {
			printf("%s: md5File error\r\n", __FUNCTION__);
			return -5;
		}
		if (cbl::ucase(sMd5) == cbl::ucase(downloadInfo.sProjectMd5)) {
			UTILITY::CFile::Delete(sPath);
			bDownload = false;
		}
		else {
			UTILITY::CFile::Delete(sFilePath);
		}
	}
	if (bDownload) {
		cbl::CHttpDownload http;
		std::vector<std::string> vecExtraName;
		vecExtraName.push_back("114.114.114.114");	// 电信做的国内免费公众DNS服务
		vecExtraName.push_back("8.8.8.8");			// Google提供的免费DNS服务器
		vecExtraName.push_back("223.5.5.5");		// 阿里推出公共DNS
		vecExtraName.push_back("8.8.4.4");			// Google提供的免费DNS服务器,备
		vecExtraName.push_back("223.6.6.6");		// 阿里推出公共DNS
		vecExtraName.push_back("208.67.222.222");	// 美国opendns
		vecExtraName.push_back("180.76.76.76");		// 百度公共DNS
		vecExtraName.push_back("208.67.222.123");	// 美国opendns
		std::string sUrl = cbl::format("http://%s:%d/%s", downloadInfo.sDomainName.c_str(), downloadInfo.nPort, downloadInfo.sUrlPath.c_str());
		if ((nRtn = http.setExtraNameServer(vecExtraName)) < 0) {
			printf("%s:setExtraNameServer error:%d\r\n", __FUNCTION__, nRtn);
			return -6;
		}

		if ((nRtn = http.get(sUrl, sFilePath)) < 0) {
			printf("%s:http download file:%s error\r\n", __FUNCTION__, sUrl.c_str());
			return -7;
		}
		if (cbl::md5File(sFilePath, sMd5) < 0) {
			printf("%s: md5File error\r\n", __FUNCTION__);
			return -8;
		}
		if (cbl::ucase(sMd5) == cbl::ucase(downloadInfo.sProjectMd5)) {
			UTILITY::CFile::Delete(sPath);
		}
	}
	if (UTILITY::CFile::ExistFile(sPath)) {
		return -9;
	}
	std::string sUnzipPath = getPrjCfgDir() + cbl::lcase(cbl::prefixName(downloadInfo.sFileName));
	if (cbl::isDir(sUnzipPath)) {
#ifdef WIN32
		cbl::removeDir(sUnzipPath);
#else
		cbl::removeDir(sUnzipPath, true);
#endif
	}
	CUnzipUtil uzip;
	if (uzip.openZip(sFilePath, "123") < 0) {
		printf("%s: open zip error\r\n", __FUNCTION__);
		return -10;
	}
	uzip.extractAll(sUnzipPath, true);
	uzip.closeZip();
#ifdef WIN32
	cbl::listDir64(sUnzipPath, 1, [ sUnzipPath ](const string& sFilePath, const struct __stat64& statFile) {
		if (_S_IFREG & statFile.st_mode) {
#else 
	cbl::listDir64(sUnzipPath, 1, [sUnzipPath](const string& sFilePath, const struct stat64& statFile) {
		if (S_ISREG(statFile.st_mode)) {
#endif
			std::string sSuffix = cbl::suffixName(sFilePath);
			if (sSuffix == ".so") {
				std::string sDstFile = cbl::dirName(sFilePath) + '/' + cbl::lcase(cbl::baseName(sFilePath));
				UTILITY::CFile::Rename(sFilePath, sDstFile);
			}
		}
		return 0;
	});
	if (cbl::isDir(sUnzipPath + std::string("/E02Drv/driver/"))) {
		UTILITY::CDir::CopyFolder(sUnzipPath + std::string("/E02Drv/driver/"), sUnzipPath + std::string("/"), "*.so", false);
	}
	else if (cbl::isDir(sUnzipPath + std::string("/driver/"))) {
		UTILITY::CDir::CopyFolder(sUnzipPath + std::string("/driver/"), sUnzipPath + std::string("/"), "*.so", false);
	}
	return 0;
}

int CCloudOperator::loadDownloadProject() {
	if (CCloudParse::get_instance().unload() < 0) {
		printf("%s:load download project error\r\n", __FUNCTION__);
		return -1;
	}
	if (CCloudParse::get_instance().restart() < 0) {
		printf("%s:project restart error\r\n", __FUNCTION__);
		return -2;
	}
	return 0;
}

int CCloudOperator::onTimer() {
	if (!m_bRunning) {
		return -1;
	}
	if (this->downloadProject() == 0) {
		if (this->loadDownloadProject() < 0) {
			printf("%s:load download project error\r\n", __FUNCTION__);
			return -2;
		}
	}
	if (!m_bServerStat) {
		return 0;
	}
	{
		unsigned long long ullTime = CTime::get_instance().getTickCount();
		if (m_ullRealCollectRunningTime == 0) {
			m_ullRealCollectRunningTime = ullTime;
		}
		if (CWPoint::get_instance().getRunStatus() &&
			ullTime - m_ullRealCollectRunningTime >= 30000) {
			if (CWPoint::get_instance().setReadCondition(MONITOR_POINT_READ_ALL) < 0) {
				printf("%s:set real read all error\r\n", __FUNCTION__);
				return -3;
			}
			if (CWPoint::get_instance().setPointPageStatus(false) < 0) {
				printf("%s:set page error\r\n", __FUNCTION__);
				return -4;
			}
			m_ullRealCollectRunningTime = ullTime;
		}
	}
	if (m_nMode == 0) {
		if (m_bLanguageStat) {
			UPDATE_CONFIG_T eUpdateConfig = UPDATE_LANGUAGE;
			if (this->getbasicInfo(eUpdateConfig) < 0) {
				printf("%s:get basic info error\r\n", __FUNCTION__);
			}
		}
		{
			if (this->updateInstallmentStat() < 0) {
				printf("%s:update installment stat error\r\n", __FUNCTION__);
				return -5;
			}
		}
	}
	else {
		if (!CCloudParse::get_instance().m_listProjectPrase.empty() &&
			0 == m_remoteProcedureCallClient.setProjectParseStat(CCloudParse::get_instance().m_listProjectPrase)) {
			CCloudParse::get_instance().m_listProjectPrase.clear();
		}
		if (!CCloudParse::get_instance().m_luarsps.empty() &&
			0 == m_remoteProcedureCallClient.setLuaStat(CCloudParse::get_instance().m_luarsps)) {
			CCloudParse::get_instance().m_luarsps.clear();
		}
		{
			unsigned long long ullTime = CTime::get_instance().getTickCount();
			static unsigned long long ullPeriod = 0;
			if (ullPeriod == 0) {
				ullPeriod = ullTime;
			}
			std::unique_lock<std::mutex> lck(m_mutexLua);
			if (!m_listLuaInfo.empty() &&
				0 == m_remoteProcedureCallClient.setLuaInfo(m_listLuaInfo)) {
				m_listLuaInfo.clear();
			}
			if (ullTime - ullPeriod > 3000 && !m_sLuabuffer.empty()) {
				std::list<LUA_INFOMATION_T> listLuaInfo;
				LUA_INFOMATION_T luaInfo;
				luaInfo.sbuffer = m_sLuabuffer;
				m_sLuabuffer.clear();
				listLuaInfo.push_back(luaInfo);
				m_remoteProcedureCallClient.setLuaInfo(listLuaInfo);
				ullPeriod = ullTime;
			}
		}
	}
	return 0;
}

int CCloudOperator::setLanguage(unsigned short nLanguage) {
	if (m_nLanguage == nLanguage) {
		return 0;
	}
	m_nLanguage = nLanguage;
	m_bLanguageStat = true;
	return 0;
}

int CCloudOperator::updateWifiSiganl(int nSignal) {
	if (!m_bRunning) {
		return -1;
	}
	return m_remoteProcedureCallClient.updateWifiSignal(nSignal);
}

int CCloudOperator::updateInstallmentStat() {
	bool bFlag = false;
	static bool bFirstTime = true;
	bool bStat = Installment_GetStatus();
	if (bFirstTime ||
		bStat != m_bInstallmentStat) {
		m_bInstallmentStat = bStat;
		bFirstTime = false;
		bFlag = true;
	}
	if (bFlag) {
		return m_remoteProcedureCallClient.setInstallmetStat(bStat ? 1 : 0);
	}
	return 0;
}

int CCloudOperator::deleteProject() {
	if (CCloudParse::get_instance().unload() < 0) {
		printf("%s: unload project error\r\n", __FUNCTION__);
		return -1;
	}
#ifdef WIN32
	cbl::removeDir(getPrjCfgDir() + std::string("template"));
#else
	cbl::removeDir(getPrjCfgDir() + std::string("template"), true);
#endif
	CFile::Delete(getPrjCfgDir() + std::string("template.vwmt"));
	if (CCloudParse::get_instance().save() < 0) {
		printf("%s:save xml error\r\n", __FUNCTION__);
		return -2;
	}
	if (CCloudParse::get_instance().restart() < 0) {
		printf("%s:restart error\r\n", __FUNCTION__);
		return -3;
	}
	return m_remoteProcedureCallClient.deleteProjectrsp(true);
}

int CCloudOperator::verifyProject() {
	if ((m_nMode == eCLOUDMODE_TEMPLATE &&
		CFile::ExistFile(getPrjCfgDir() + std::string("template/json.vbox"))) ||
		m_nMode == eCLOUDMODE_CLOUD ||
		m_nMode == eCLOUDMODE_HMI) {
		return -1;
	}
	return 0;
}