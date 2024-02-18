#include "cloudparse.h"
#include "../ccommon/ccommon.h"
#include "utility/utility.h"
#include "HmiBase/HmiBase.h"
#include "secure/secure.h"
#include "macrosplatform/registerAddr.h"
#include "macrosplatform/path.h"
#include "../cloudoperator/cloudoperator.h"
#include "GlobalModule/GlobalModule.h"
#include "driveradapter/DriverAdapter.h"
#include "../../../../HmiCloudApp/HmiCloudApp/protobuf/VNetConfig.pb.h"
#define REALCYCLE 100
using namespace com_wecon_vnet_V20210731;
using namespace UTILITY_NAMESPACE;
using namespace SECURE_NAMESPACE;
CCloudParse& CCloudParse::get_instance() {
	static CCloudParse CloudParse;
	return CloudParse;
}

CCloudParse::CCloudParse() {
	m_bEnableReal = m_bEnableHistory = m_bEnableAlarm = true;
	m_bEnableKeepGps = false;
	m_nRealPeriod = 2000;
	m_nRealMapPeriod = 5000;
	m_nGpsPeriod = 1000;
	m_bEnableCloud = m_bEnableThirdCloud = true;
	m_eMode = getCloudMode();
	m_sRemotePwd = m_sProjectMd5 = "";
	m_CloudBindConfig.clear();
	m_CloudUserConfig.clear();
	m_luarsps.clear();
}

CCloudParse::~CCloudParse() {
	CEngineMgr::getInstance().removeAllPlcDriver();
}

int CCloudParse::loadProject(bool bInit) {
	int nRtn = 0;
	WMachine wMachine;
	std::string sDeviceId = "", sFilePath = "";
	std::list<LUASCRIPT_T> scripts;
	if (wMachine.getDeviceId(sDeviceId) < 0 ||
		sDeviceId.empty()) {
		printf("getDeviceId error\r\n");
		return -1;
	}
	unsigned int nOffSet = 0, nOffsetPwd = 0;
	switch (m_eMode)
	{
	case eCLOUDMODE_TEMPLATE:
		{
			std::list<COLLECTION_HISTORY_DATA> listHistoryData;
			std::list<COLLECTION_MONITOR_POINT_DATA> listRealData;
			std::list<COLLECTION_ALARM_DATA> listAlarmData;
			if ((nRtn = this->loadProjectSetup()) < 0) {
				printf("%s:loadProjectSetup error:%d\r\n", __FUNCTION__, nRtn);
				return -2;
			}
			m_bEnableThirdCloud = false;
			COLLECTION_MONITOR_GROUP_DATA monitorGroup;
			monitorGroup.vecGroupName.assign({ "group", "group", "group", "group" , "group" , "group" , "group" , "group" });
			monitorGroup.nInterval = 200;
			monitorGroup.nGroupId = 0;
			if (CWPoint::get_instance().addGroup(monitorGroup, false) < 0) {
				return -3;
			}
			sFilePath = getPrjCfgDir() + std::string("template/json.vbox");
			if (UTILITY::CFile::ExistFile(sFilePath)) {
				this->m_luaPrase.config(false);
				CEngineMgr::getInstance().resetProjectPath(getPrjCfgDir() + std::string("template/"));
				std::string sPLCType = getPrjCfgDir() + std::string("template/PlcType/WECON SIMUTOCOL.plc");
				if (cbl::getFileSize(sPLCType) <= 0) {
					if ((nRtn = this->generalPLCType(getPrjDir())) < 0) {
						printf("%s:general plc type error:%d\r\n", __FUNCTION__, nRtn);
						return -4;
					}
					cbl::makeDir(getPrjCfgDir() + std::string("template/PlcType/"));
					if (!CFile::Copy(getPrjDir() + std::string("PlcType/WECON SIMUTOCOL.plc"), sPLCType, false)) {
						printf("%s:copy %s error\r\n", __FUNCTION__, sPLCType.c_str());
						return -5;
					}
				}
				if ((nRtn = CCompileAddress::get_instance().init(getPrjCfgDir() + std::string("template/"))) < 0) {
					printf("%s:CCompileAddress init error:%d\r\n", __FUNCTION__, nRtn);
					return -6;
				}
				if ((nRtn = this->loadTemplate(sFilePath, listHistoryData, listRealData, listAlarmData, scripts)) < 0) {
					printf("%s:loadTemplate error:%d\r\n", __FUNCTION__, nRtn);
					return -7;
				}
			}
			else {
				sFilePath = getPrjDir() + std::string("test.pi");
				this->m_luaPrase.config(true);
				CEngineMgr::getInstance().resetProjectPath(getPrjDir());
				if ((nRtn = this->generalPLCType(getPrjDir())) < 0) {
					printf("%s:general plc type error:%d\r\n", __FUNCTION__, nRtn);
					return -8;
				}
				if ((nRtn = this->generalModbusRegMap(getPrjDir())) < 0) {
					printf("%s:general modbus reg map error:%d\r\n", __FUNCTION__, nRtn);
					return -9;
				}
				if ((nRtn = CCompileAddress::get_instance().init(getPrjDir())) < 0) {
					printf("%s:CCompileAddress init error:%d\r\n", __FUNCTION__, nRtn);
					return -10;
				}
				if ((nRtn = this->loadConfigini(getPrjDir() + std::string("config.ini"))) < 0) {
					printf("%s:load config ini error:%d\r\n", __FUNCTION__, nRtn);
					return -11;
				}
				if ((nRtn = this->loadNormal(sFilePath, listHistoryData, listRealData, listAlarmData)) < 0) {
					printf("%s: load test.pi error:%d\r\n", __FUNCTION__, nRtn);
					return -12;
				}
				if ((nRtn = this->m_luaPrase.loadDb(scripts)) < 0) {
					printf("%s: load lua db error:%d\r\n", __FUNCTION__, nRtn);
					return -13;
				}
			}
			{
				for (auto &iter : listHistoryData) {
					if ((nRtn = CDataRecordCollect::get_instance().addGroup(iter, false)) < 0) {
						printf("%s:add history Group error:%d\r\n", __FUNCTION__, nRtn);
						return -14;
					}
				}
			}
			{
				for (auto &iter : listAlarmData) {
					if ((nRtn = CAlarmCollect::get_instance().add(iter)) < 0) {
						printf("%s: add alarm error:%d\r\n", __FUNCTION__, nRtn);
						return -15;
					}
				}
			}
			for (auto &iter : listRealData) {
				if ((nRtn = CWPoint::get_instance().addPoint(0, iter, false)) < 0) {
					printf("%s: add real error:%d\r\n", __FUNCTION__, nRtn);
					return -16;
				}
			}
			if (0 > (nRtn = this->m_luaPrase.start(scripts, bInit))) {
				printf("%s: lua start error:%d\r\n", __FUNCTION__, nRtn);
				return -17;
			}
			nOffSet = HSW_REMOTESN_R;
			nOffsetPwd = HSW_REMOTEPWD_R;
		}
		break;
	case eCLOUDMODE_CLOUD:
		{
			this->m_luaPrase.config(false);
			CEngineMgr::getInstance().resetProjectPath(getPrjCfgDir() + std::string("cloudprj/"));
			std::string sPLCType = getPrjCfgDir() + std::string("cloudprj/PlcType/WECON SIMUTOCOL.plc");
			if (cbl::getFileSize(sPLCType) <= 0) {
				if ((nRtn = this->generalPLCType(getPrjDir())) < 0) {
					printf("%s:general plc type error:%d\r\n", __FUNCTION__, nRtn);
					return -2;
				}
				cbl::makeDir(getPrjCfgDir() + std::string("cloudprj/PlcType/"));
				if (!CFile::Copy(getPrjDir() + std::string("PlcType/WECON SIMUTOCOL.plc"), sPLCType, false)) {
					printf("%s:copy %s error\r\n", __FUNCTION__, sPLCType.c_str());
					return -3;
				}
			}
			if ((nRtn = CCompileAddress::get_instance().init(getPrjCfgDir() + std::string("cloudprj/"))) < 0) {
				printf("%s:CCompileAddress init error:%d\r\n", __FUNCTION__, nRtn);
				return -4;
			}
			sFilePath = getPrjCfgDir() + std::string("cloudprj/json.vbox");
			if ((nRtn = this->loadProjectSetup()) < 0) {
				printf("%s:loadProjectSetup error:%d\r\n", __FUNCTION__, nRtn);
				return -5;
			}
			if (cbl::getFileSize(sFilePath) <= 0) {
				return 0;
			}
			if ((nRtn = this->loadCloudProject(sFilePath, scripts)) < 0) {
				printf("%s:load cloud project error:%d\r\n", __FUNCTION__, nRtn);
				return -6;
			}
			if (0 > (nRtn = this->m_luaPrase.start(scripts, bInit))) {
				printf("%s: lua start error:%d\r\n", __FUNCTION__, nRtn);
				return -7;
			}
			m_bEnableAlarm = m_bEnableHistory = m_bEnableReal = false;
			nOffSet = HSW_REMOTESN_R;
			nOffsetPwd = HSW_REMOTEPWD_R;
		}
		break;
	case eCLOUDMODE_HMI:
		{
			std::string		sPath;
			std::string sDir = getPrjDir() + std::string("Script/");
			std::list<std::string> listPreloadPath;
			listPreloadPath.push_back(HMIEXLUAPATH);
			if (cbl::isDir(sDir) ||
				isShortCutKeyValid()) {
				this->m_luaPrase.config(false, sDir, listPreloadPath);
				if (0 > (nRtn = this->m_luaPrase.start(scripts, bInit))) {
					printf("%s: lua start error:%d\r\n", __FUNCTION__, nRtn);
					return -2;
				}
			}
#ifdef WIN32
			char	szPath[128] = { 0 };
			CState::GetTempPathCos(MAX_PATH, szPath);
			sPath = cbl::format("%s/HMIEmulation/cloud/", szPath);
#else
			sPath = "/mnt/data/prj/cloud/";
#ifdef WITH_UI
			if (false == wMachine.enableRemote()) { // 不支持远程
				printf("unable remote device\r\n");
				return -3;
			}
#endif
#endif
			if ((nRtn = this->loadHMICloud(sPath + std::string("cloud.xml"))) < 0) {
				printf("%s:load hmi cloud error:%d\r\n", __FUNCTION__, nRtn);
				return -4;
			}
			sFilePath = getPrjDir() + std::string("test.pi");
			nOffSet = HSW_REMOTESN;
			nOffsetPwd = HSW_REMOTEPWD;
		}
		break;
	case eCLOUDMODE_MAX:
	case eCLOUDMODE_NONE:
	default:
		return -2;
	}
	if (nOffSet == 0 ||
		sDeviceId.empty() ||
		m_sRemotePwd.empty() ||
		!writeSpecialAddress(nOffsetPwd, m_sRemotePwd) ||
		!writeSpecialAddress(nOffSet, sDeviceId)) {
		printf("%s:write device id error\r\n", __FUNCTION__);
		return -3;
	}
	if (!sFilePath.empty()) {
		cbl::md5File(sFilePath, m_sProjectMd5);
	}
	printf("load project success\r\n");
	return nRtn;
}

int CCloudParse::loadProjectSetup() {
	std::string sPath = getPrjCfgDir() + std::string("setup.xml");
#ifdef WIN32
	tinyxml2::TinyXMLDocument doc;
#else
	tinyxml2::XMLDocument doc;
#endif
	tinyxml2::XMLElement *peleRoot;
	if (cbl::loadXmlFile(UTF8_TO_ANSI(sPath.c_str()), doc, peleRoot) < 0) {
		return -1;
	}
	XMLNode *node = peleRoot->FirstChild();
	while (node) {
		XMLElement *elem = node->ToElement();
		if (elem) {
			if (0 == strcmp(node->Value(), "BaseSet")) {
				if (elem->Attribute("RemotePsw")) {
					m_sRemotePwd = elem->Attribute("RemotePsw");
				}
				return 0;
			}
		}
		node = node->NextSiblingElement();
	}
	return -2;
}

int CCloudParse::loadTemplateGlobalConfig(const Json::Value &root) {
	if (root.isMember("global_setting_list") &&
		root["global_setting_list"].isArray()) {
		for (unsigned int nI = 0; nI < root["global_setting_list"].size(); nI++) {
			Json::Value json = root["global_setting_list"][nI];
			if (!json.isMember("type")) {
				continue;
			}
			if (!json.isMember("data_type")) {
				continue;
			}
			int nType = json["type"].isInt() ? json["type"].asInt() : -1;
			int nDataType = json["data_type"].isInt() ? json["data_type"].asInt() : -1;
			if (-1 == nType
				|| -1 == nDataType) {
				continue;
			}
			if (!json.isMember("setting_data")) {
				continue;
			}
			switch (nDataType) {
			case REAL_CONFIGS:
				{
					if (REPORT_CONFIG == nType) {
						m_nRealPeriod = json["setting_data"].isInt() ? json["setting_data"].asInt() : 2000;
					}
					else if (SWITCH_CONFIG == nType) {
						m_bEnableReal = json["setting_data"].isInt() ? json["setting_data"].asInt() == 1 : true;
					}
				}
				break;
			case HIS_CONFIGS:
				{
					m_bEnableHistory = json["setting_data"].isInt() ? json["setting_data"].asInt() == 1 : true;
				}
				break;
			case ALARM_CONFIGS:
				{
					m_bEnableAlarm = json["setting_data"].isInt() ? json["setting_data"].asInt() == 1 : true;
				}
				break;
			default:
				break;
			}
		}
	}
	return 0;
}

int CCloudParse::loadTemplate(const std::string &sPath, std::list<COLLECTION_HISTORY_DATA> &listHistoryData, \
							  std::list<COLLECTION_MONITOR_POINT_DATA> &listRealData, \
							  std::list<COLLECTION_ALARM_DATA> &listAlarmData, \
							  std::list<LUASCRIPT_T> &scripts) {
	Json::Value root;
	if (cbl::loadJsonFile(sPath, root) < 0) {
		addProjectParseStat(-1, "state");
		return -1;
	}
	if (!root.isMember("data")) {
		addProjectParseStat(-2, "state");
		return -2;
	}
	addProjectParseStat(1, "state");
	if (loadTemplateGlobalConfig(root["data"]) < 0) {
		addProjectParseStat(-1, "global_state");
		return -3;
	}
	addProjectParseStat(1, "global_state");
	if (!root["data"].isMember("upd_com_list") ||
		parseCommu(root["data"]["upd_com_list"], getPrjCfgDir() + std::string("template/")) < 0) {
		addProjectParseStat(-1, "plc_state");
	}
	else {
		addProjectParseStat(1, "plc_state");
	}
	if (!root["data"].isMember("upd_his_cfg_dir_list") ||
		parseHistory(root["data"]["upd_his_cfg_dir_list"], listHistoryData) < 0) {
		addProjectParseStat(-1, "history_group_state");
	}
	else {
		addProjectParseStat(1, "history_group_state");
	}
	if (!root["data"].isMember("upd_real_his_cfg_list") || 
		parsePoint(root["data"]["upd_real_his_cfg_list"], listHistoryData, listRealData) < 0) {
		addProjectParseStat(-1, "real_history_state");
	}
	else {
		addProjectParseStat(1, "real_history_state");
	}
	if (!root["data"].isMember("upd_alarm_cfg_list") ||
		parseAlarm(root["data"]["upd_alarm_cfg_list"], listAlarmData) < 0) {
		addProjectParseStat(-1, "alarm_state");
	}
	else {
		addProjectParseStat(1, "alarm_state");
	}
	if (this->loadlua(root, scripts) < 0) {
		printf("%s:load lua error\r\n", __FUNCTION__);
		return -4;
	}
	return 0;
}

int CCloudParse::parseRealPoint(int nCommuId, const Json::Value &root, std::list<COLLECTION_MONITOR_POINT_DATA> &listRealData) {
	COLLECTION_MONITOR_POINT_DATA point;
	point.nCommuID = nCommuId;
	if (root.isMember("addr_id")) {
		point.nUid = root["addr_id"].isInt() ? root["addr_id"].asInt() : 0;
	}
	if (root.isMember("data_id")) {
		point.nDataFormat = root["data_id"].isInt() ? root["data_id"].asInt() : 0;
	}
	if (root.isMember("name")) {
		std::string sName = root["name"].isString() ? root["name"].asString() : "";
		point.vecName.assign({ sName, sName, sName, sName, sName, sName, sName, sName });
	}
	if (root.isMember("addr_type")) {
		point.addrType = toAddrType(root["addr_type"].isInt() ? root["addr_type"].asInt() : 0);
		if (ADDR_TYPE_BIT == point.addrType) {
			point.bBitCollect = true;
		}
		else {
			point.bBitCollect = false;
		}
	}
	if (root.isMember("digit_count")) {
		std::string sDigitCount = root["digit_count"].isString() ? root["digit_count"].asString().c_str() : "";
		if (point.nDataFormat == _String) {
			try {
				point.nDataLen = std::stoi(sDigitCount);
			}
			catch (const std::exception& e) {
				printf("%s\r\n", e.what());
			}
		}
		else {
			if (std::string::npos == sDigitCount.find(",")) {
				point.nInteger = 5;
				point.nDecimals = 0;
			}
			else {
				sscanf(sDigitCount.c_str(), "%d,%d", &(point.nInteger), &(point.nDecimals));
			}
		}
	}
	if (root.isMember("dead_set")) {
		Convert::BuffToUnionData(root["dead_set"].isString() ? root["dead_set"].asString() : "0", point.nDataFormat, point.nDecimals, point.u4wDeadZoneValue);
	}
	if (root.isMember("cfg_role")) {
		point.nPerssion = root["cfg_role"].isInt() ? root["cfg_role"].asInt() : 0;
	}
	if (root.isMember("float_point")) {
		point.nHighLowWord = root["float_point"].isInt() ? root["float_point"].asInt() : -1;
	}
	if (root.isMember("string_point")) {
		point.nHighLowByte = root["string_point"].isInt() ? root["string_point"].asInt() : -1;
	}
	if (root.isMember("traffic_status")) {
		point.bSaveTraffic = (bool)root["traffic_status"].isInt() ? root["traffic_status"].asInt() : 0;
	}
	if (root.isMember("keep_report")) {
		point.bMap = root["keep_report"].isInt() ? root["keep_report"].asInt() == 1: false;
	}
	std::vector<std::string> vecField = { "addr_stat_no", "rid", "addr" };
	if (addressTransform(root, vecField, point.nCommuID, point.addrType, point.addrMonitor, point.sAddrAlias) < 0) {
		addProjectParseStat("address transfrom error, state : -1", point.nUid, 3, -1);
		return -1;
	}
	listRealData.push_back(point);
	return 0;
}

int CCloudParse::parseAlarmCond(const Json::Value& root, COLLECTION_ALARM_DATA& alarm) {
	if (1 == root.size() && 
		ARITHMETIC_TYPE_SINGLE == alarm.conditionType) {
		Json::Value iterCond = root[0];
		if (ALARM_TYPE_BIT == alarm.type) {
			if (iterCond.isMember("value")) {
				std::string sValue = iterCond["value"].isString() ? iterCond["value"].asString() : "";
				alarm.wTriggerValue = sValue == "ON" ? 1 : 0;
			}
		}
		else if (ALARM_TYPE_WORD == alarm.type) {
			if (iterCond.isMember("value")) {
				if (alarm.nDataFormat == _String) {
					alarm.sCondition = iterCond["value"].isString() ? iterCond["value"].asString() : "";
				}
				else {
					Convert::BuffToUnionData(iterCond["value"].isString() ? iterCond["value"].asString() : "", alarm.nDataFormat, alarm.nDecimals, alarm.u4wRangeEnd);
				}
			}
			if (iterCond.isMember("type")) {
				alarm.mode = toAlarmMode(iterCond["type"].isInt() ? iterCond["type"].asInt() : 0);
			}
		}
	}

	if (2 == root.size() && 
		ARITHMETIC_TYPE_SINGLE != alarm.conditionType) {
		Json::Value iterCondS = root[0];
		Json::Value iterCondE = root[1];
		alarm.mode = ALARM_MODE_RANGELIMIT;
		if (iterCondS.isMember("value")) {
			if (alarm.nDataFormat == _String) {
				alarm.sCondition = iterCondS["value"].asString();
			}
			else {
				Convert::BuffToUnionData(iterCondS["value"].isString() ? iterCondS["value"].asString() : "", alarm.nDataFormat, alarm.nDecimals, alarm.u4wRangeStart);
			}
		}
		if (iterCondS.isMember("type")) {
			alarm.modeStart = toAlarmMode(iterCondS["type"].isInt() ? iterCondS["type"].asInt() : 0);
		}
		if (iterCondE.isMember("value")) {
			if (alarm.nDataFormat == _String) {
				alarm.sConditionEnd = iterCondS["value"].asString();
			}
			else {
				Convert::BuffToUnionData(iterCondE["value"].isString() ? iterCondE["value"].asString() : "", alarm.nDataFormat, alarm.nDecimals, alarm.u4wRangeEnd);
			}
		}
		if (iterCondE.isMember("type")) {
			alarm.modeEnd = toAlarmMode(iterCondE["type"].isInt() ? iterCondE["type"].asInt() : 0);
		}
	}
	return 0;
}

int CCloudParse::parseAlarm(const Json::Value &root, std::list<COLLECTION_ALARM_DATA> &listAlarmData) {
	if (!root.isArray()) {
		return -1;
	}
	for (unsigned int nI = 0; nI < root.size(); nI++) {
		int nCommuId = 0;
		Json::Value json = root[nI];
		if (json.isMember("com")) {
			if (json["com"].isString()) {
				nCommuId = strtol(json["com"].asString().c_str(), nullptr, 10);
			}
			else if (json["com"].isInt()) {
				nCommuId = json["com"].isInt() ? json["com"].asInt() : 0;
			}
		}
		if (json.isMember("cfg_list")) {
			Json::Value jsonCfg = json["cfg_list"];
			for (unsigned int nJ = 0; nJ < jsonCfg.size(); nJ++) {
				Json::Value jsonAlarm = jsonCfg[nJ];
				COLLECTION_ALARM_DATA alarmData;
				alarmData.nMonitorCommuID = nCommuId;
				alarmData.vecText.assign(8, "");
				alarmData.bSave = false;
				alarmData.bReportCloud = true;
				if (jsonAlarm.isMember("addr_id")) {
					alarmData.nUid = jsonAlarm["addr_id"].isInt() ? jsonAlarm["addr_id"].asInt() : 0;
				}
				if (jsonAlarm.isMember("data_id")) {
					alarmData.nDataFormat = jsonAlarm["data_id"].isInt() ? jsonAlarm["data_id"].asInt() : 0;
				}
				if (jsonAlarm.isMember("name")) {
					alarmData.sName = jsonAlarm["name"].isString() ? jsonAlarm["name"].asString() : "";
				}
				if (jsonAlarm.isMember("addr_type")) {
					alarmData.monitorAddrType = toAddrType(jsonAlarm["addr_type"].isInt() ? jsonAlarm["addr_type"].asInt() : 0);
					if (ADDR_TYPE_BIT == alarmData.monitorAddrType) {
						alarmData.type = ALARM_TYPE_BIT;
					}
					else {
						alarmData.type = ALARM_TYPE_WORD;
					}
				}
				if (jsonAlarm.isMember("condition_type")) {
					alarmData.conditionType = (ARITHMETIC_TYPE_T)(jsonAlarm["condition_type"].isInt() ? jsonAlarm["condition_type"].asInt() : 0);
				}

				if (jsonAlarm.isMember("digit_count")) {
					std::string sDigitCount = jsonAlarm["digit_count"].isString() ? jsonAlarm["digit_count"].asString().c_str() : "";
					if (alarmData.nDataFormat == _String) {
						try {
							alarmData.nDataLen = std::stoi(sDigitCount);
						}
						catch (const std::exception& e) {
							printf("%s\r\n", e.what());
						}
					}
					else {
						if (std::string::npos == sDigitCount.find(",")) {
							alarmData.nInteger = 5;
							alarmData.nDecimals = 0;
						}
						else {
							sscanf(sDigitCount.c_str(), "%d,%d", &(alarmData.nInteger), &(alarmData.nDecimals));
						}
					}
				}
				if (jsonAlarm.isMember("float_point")) {
					alarmData.nHighLowWord = jsonAlarm["float_point"].isInt() ? jsonAlarm["float_point"].asInt() : -1;
				}
				if (jsonAlarm.isMember("string_point")) {
					alarmData.nHighLowByte = jsonAlarm["string_point"].isInt() ? jsonAlarm["string_point"].asInt() : -1;
					if (2 == alarmData.nHighLowByte) {
						alarmData.nHighLowByte = HLByte_HighByteBefore;
					}
				}
				std::vector<std::string> vecField = { "addr_stat_no", "rid", "addr" };
				if (addressTransform(jsonAlarm, vecField, alarmData.nMonitorCommuID, alarmData.monitorAddrType, alarmData.addrMonitor, alarmData.sAddressAlias) < 0) {
					addProjectParseStat("address transform error, state : -1", alarmData.nUid, 5, -1);
					continue;;
				}
				if (jsonAlarm.isMember("condition_list")) {
					parseAlarmCond(jsonAlarm["condition_list"], alarmData);
				}
				listAlarmData.push_back(alarmData);
			}
		}
	}
	return 0;
}

int CCloudParse::parseHistoryPoint(int nCommuId, const Json::Value &root, std::list<COLLECTION_HISTORY_DATA> &listHistoryData) {
	COLLECTION_HISTORY_NODE point;
	point.nCommuID = nCommuId;
	point.bReportCloud = true;
	if (root.isMember("addr_id")) {
		point.nId = root["addr_id"].isInt() ? root["addr_id"].asInt() : 0;
	}
	if (root.isMember("data_id")) {
		point.nDataFormat = root["data_id"].isInt() ? root["data_id"].asInt() : 0;
	}
	if (root.isMember("name")) {
		std::string sName = root["name"].isString() ? root["name"].asString() : "";
		point.vecFieldName.assign({ sName, sName, sName, sName, sName, sName, sName, sName });
	}
	if (root.isMember("addr_type")) {
		point.fieldAddrType = toAddrType(root["addr_type"].isInt() ? root["addr_type"].asInt() : 0);
	}

	if (root.isMember("digit_count")) {
		std::string sDigitCount = root["digit_count"].isString() ? root["digit_count"].asString().c_str() : "";
		if (point.nDataFormat == _String) {
			try {
				point.nDataLen = std::stoi(sDigitCount);
			}
			catch (const std::exception& e) {
				printf("%s\r\n", e.what());
			}
		}
		else {
			if (std::string::npos == sDigitCount.find(",")) {
				point.nInteger = 5;
				point.nDecimals = 0;
			}
			else {
				sscanf(sDigitCount.c_str(), "%d,%d", &(point.nInteger), &(point.nDecimals));
			}
		}
	}
	if (root.isMember("dir_id")) {
		point.nGroupId = root["dir_id"].isInt() ? root["dir_id"].asInt() : 0;
	}
	if (root.isMember("float_point")) {
		point.nHighLowWord = root["float_point"].isInt() ? root["float_point"].asInt() : -1;
	}
	if (root.isMember("string_point")) {
		point.nHighLowByte = root["string_point"].isInt() ? root["string_point"].asInt() : -1;
		if (2 == point.nHighLowByte) {
			point.nHighLowByte = HLByte_HighByteBefore;
		}
	}
	std::vector<std::string> vecField = { "addr_stat_no", "rid", "addr" };
	if (addressTransform(root, vecField, point.nCommuID, point.fieldAddrType, point.addrField, point.sFieldAddrAlias) < 0) {
		addProjectParseStat("address transform error, state : -1", point.nId, 4, -1);
		return -1;
	}

	std::list<COLLECTION_HISTORY_DATA>::iterator iter;
	if (listHistoryData.end() != (iter = std::find_if(listHistoryData.begin(), listHistoryData.end(),
		[ point ] (COLLECTION_HISTORY_DATA node) {return (point.nGroupId == node.nGroupId); }))) {
		iter->listField.push_back(point);
	}
	return 0;
}

int CCloudParse::parsePoint(const Json::Value &root, std::list<COLLECTION_HISTORY_DATA> &listHistoryData, std::list<COLLECTION_MONITOR_POINT_DATA> &listRealData) {
	if (!root.isArray()) {
		return -1;
	}
	for (unsigned int nI = 0; nI < root.size(); nI++) {
		int nCommuId = 0;
		Json::Value json = root[nI];
		if (json.isMember("com")) {
			if (json["com"].isInt()) {
				nCommuId = json["com"].asInt();
			}
			else if (json["com"].isString()) {
				nCommuId = strtol(json["com"].asString().c_str(), NULL, 10);
			}
		}
		if (json.isMember("cfg_list")) {
			Json::Value jsonCfg = json["cfg_list"];
			for (unsigned int nJ = 0; nJ < jsonCfg.size(); nJ++) {
				int nDataType = 0;
				if (jsonCfg[nJ].isMember("data_type")) {
					nDataType = jsonCfg[nJ]["data_type"].isInt() ? jsonCfg[nJ]["data_type"].asInt() : 0;
				}
				if (0 == nDataType) {
					parseRealPoint(nCommuId, jsonCfg[nJ], listRealData);
				}
				else if (1 == nDataType) {
					parseHistoryPoint(nCommuId, jsonCfg[nJ], listHistoryData);
				}
			}
		}
	}
	return 0;
}

int CCloudParse::parseHistory(const Json::Value &root, std::list<COLLECTION_HISTORY_DATA> &listHistoryData) {
	if (!root.isArray()) {
		return -1;
	}
	for (unsigned int nI = 0; nI < root.size(); nI++) {
		Json::Value json = root[nI];
		unsigned int nGroupId = 0;
		if (json.isMember("dir_id")) {
			nGroupId = json["dir_id"].isInt() ? json["dir_id"].asInt() : 0;
		}
		COLLECTION_HISTORY_DATA history;
		history.nGroupId = nGroupId;
		if (json.isMember("his_cycle")) {
			history.nInterval = json["his_cycle"].isInt() ? json["his_cycle"].asInt() : 0;
		}
		if (json.isMember("trigger_type")) {
			history.nType = json["trigger_type"].isInt() ? json["trigger_type"].asInt() : 0;
		}
		if (json.isMember("trigger_addr_type")) {
			history.trigerAddrType = toAddrType(json["addr_type"].isInt() ? json["addr_type"].asInt() : 0);
		}
		if (json.isMember("trigger_plc_id")) {
			history.nTrigCommuID = json["trigger_plc_id"].isInt() ? json["trigger_plc_id"].asInt() : 0;
		}
		if (json.isMember("trigger_value")) {
			std::string sValue = json["trigger_value"].isString() ? json["trigger_value"].asString() : "0";
			history.bTriggerValue = strtoll(sValue.c_str(), nullptr, 10) == 0 ? false : true;
		}
		if (history.nType >= HISTORYDATA_MODE_TRIGGER_TIMESAMPLE &&
			history.nType <= HISTORYDATA_MODE_TRIGGER_NORESET) {
			std::vector<std::string> vec = { "trigger_addr_stat_no", "trigger_rid", "trigger_addr" };
			if (addressTransform(json, vec, history.nTrigCommuID, history.trigerAddrType, history.addrTrigger, history.sTriggerAddrAlias) < 0) {
				addProjectParseStat("address transform error, state : -1", nGroupId, 2, -1);
				continue;
			}
		}
		listHistoryData.push_back(history);
	}
	return 0;
}

int CCloudParse::parseCommu(const Json::Value &root, const std::string &sDir) {
	if (!root.isArray()) {
		return -1;
	}
	std::list<CONNECTNODE> listCommu;
	for (unsigned int nI = 0; nI < root.size(); nI++) {
		Json::Value json = root[nI];
		CONNECTNODE commu;
		memset(&commu, 0, sizeof(CONNECTNODE));
		commu.nPortId = -1;
		if (json.isMember("plc_id")) {
			if (json["plc_id"].isString()) {
				commu.nCommuId = strtol(json["plc_id"].asString().c_str(), NULL, 10);
			}
			else if (json["plc_id"].isInt()) {
				commu.nCommuId = json["plc_id"].asInt();
			}
		}
		else if (json.isMember("com")) {
			if (json["com"].isString()) {
				commu.nCommuId = strtol(json["com"].asString().c_str(), NULL, 10);
			}
			else if (json["com"].isInt()) {
				commu.nCommuId = json["com"].asInt();
			}
		}
		if (json.isMember("box_stat_no")) {
			if (json["box_stat_no"].isString()) {
				commu.PlcSet.nHmiMachineNo = strtol(json["box_stat_no"].asString().c_str(), NULL, 10);
			}
			else if (json["box_stat_no"].isInt()) {
				commu.PlcSet.nHmiMachineNo = json["box_stat_no"].asInt();
			}
		}
		if (json.isMember("plc_stat_no")) {
			if (json["plc_stat_no"].isString()) {
				commu.PlcSet.nPlcMachineNo = strtol(json["plc_stat_no"].asString().c_str(), NULL, 10);
			}
			else if (json["plc_stat_no"].isInt()) {
				commu.PlcSet.nPlcMachineNo = json["plc_stat_no"].asInt();
			}
		}
		if (json.isMember("comtype")) {
			if (json["comtype"].isString()) {
				commu.Commset.nCommMode = strtol(json["comtype"].asString().c_str(), NULL, 10);
			}
			else if (json["comtype"].isInt()) {
				commu.Commset.nCommMode = json["comtype"].asInt();
			}
		}
		if (json.isMember("baudrate")) {
			if (json["baudrate"].isString()) {
				commu.Commset.nBaudRate = strtol(json["baudrate"].asString().c_str(), NULL, 10);
			}
			else if (json["baudrate"].isInt()) {
				commu.Commset.nBaudRate = json["baudrate"].asInt();
			}
		}
		if (json.isMember("stop_bit")) {
			if (json["stop_bit"].isString()) {
				commu.Commset.nStopBits = strtol(json["stop_bit"].asString().c_str(), NULL, 10);
			}
			else if (json["stop_bit"].isInt()) {
				commu.Commset.nStopBits = json["stop_bit"].asInt();
			}
		}
		if (json.isMember("data_length")) {
			if (json["data_length"].isString()) {
				commu.Commset.nDataLength = strtol(json["data_length"].asString().c_str(), NULL, 10);
			}
			else if (json["data_length"].isInt()) {
				commu.Commset.nDataLength = json["data_length"].asInt();
			}
		}
		if (json.isMember("retry_times")) {
			if (json["retry_times"].isString()) {
				commu.Commset.nRetryTimes = strtol(json["retry_times"].asString().c_str(), NULL, 10);
			}
			else if (json["retry_times"].isInt()) {
				commu.Commset.nRetryTimes = json["retry_times"].asInt();
			}
		}
		if (json.isMember("wait_timeout")) {
			if (json["wait_timeout"].isString()) {
				commu.Commset.nWaitTimeout = strtol(json["wait_timeout"].asString().c_str(), NULL, 10);
			}
			else if (json["wait_timeout"].isInt()) {
				commu.Commset.nWaitTimeout = json["wait_timeout"].asInt();
			}
		}
		if (json.isMember("rev_timeout")) {
			if (json["rev_timeout"].isString()) {
				commu.Commset.nRecvTimeout = strtol(json["rev_timeout"].asString().c_str(), NULL, 10);
			}
			else if (json["rev_timeout"].isInt()) {
				commu.Commset.nRecvTimeout = json["rev_timeout"].asInt();
			}
		}
		if (json.isMember("com_stepinterval")) {
			if (json["com_stepinterval"].isString()) {
				commu.Commset.nComStepInterval = strtol(json["com_stepinterval"].asString().c_str(), NULL, 10);
			}
			else if (json["com_stepinterval"].isInt()) {
				commu.Commset.nComStepInterval = json["com_stepinterval"].asInt();
			}
		}
		if (json.isMember("com_iodelaytime")) {
			if (json["com_iodelaytime"].isString()) {
				commu.Commset.nComIoDelayTime = strtol(json["com_iodelaytime"].asString().c_str(), NULL, 10);
			}
			else if (json["com_iodelaytime"].isInt()) {
				commu.Commset.nComIoDelayTime = json["com_iodelaytime"].asInt();
			}
		}
		if (json.isMember("retry_timeout")) {
			if (json["retry_timeout"].isString()) {
				commu.Commset.nRetryTimeOut = strtol(json["retry_timeout"].asString().c_str(), NULL, 10);
			}
			else if (json["retry_timeout"].isInt()) {
				commu.Commset.nRetryTimeOut = json["retry_timeout"].asInt();
			}
		}
		if (json.isMember("com_singlelen")) {
			if (json["com_singlelen"].isString()) {
				commu.Commset.nComSingleReadLen = strtol(json["com_singlelen"].asString().c_str(), NULL, 10);
			}
			else if (json["com_singlelen"].isInt()) {
				commu.Commset.nComSingleReadLen = json["com_singlelen"].asInt();
			}
		}
		if (json.isMember("net_type")) {
			if (json["net_type"].isString()) {
				commu.Commset.nEthernet_IsUDP = strtol(json["net_type"].asString().c_str(), NULL, 10);
			}
			else if (json["net_type"].isInt()) {
				commu.Commset.nEthernet_IsUDP = json["net_type"].asInt();
			}
		}
		if (json.isMember("net_isbroadcast")) {
			if (json["net_isbroadcast"].isString()) {
				commu.Commset.bEthernet_IsBroadcast = strtol(json["net_isbroadcast"].asString().c_str(), NULL, 10);
			}
			else if (json["net_isbroadcast"].isInt()) {
				commu.Commset.bEthernet_IsBroadcast = json["net_isbroadcast"].asInt();
			}
		}
		if (json.isMember("net_broadcastaddr")) {
			if (json["net_broadcastaddr"].isString()) {
				commu.Commset.wBroadcastAddr = strtol(json["net_broadcastaddr"].asString().c_str(), NULL, 10);
			}
			else if (json["net_broadcastaddr"].isInt()) {
				commu.Commset.wBroadcastAddr = json["net_broadcastaddr"].asInt();
			}
		}
		if (json.isMember("dev_type")) {
			std::string sType = json["dev_type"].isString() ? json["dev_type"].asString() : "";
			if (sType.length() <= sizeof(commu.PlcSet.szPLCType) / sizeof(char)) {
				strncpy(commu.PlcSet.szPLCType, sType.c_str(), sType.length());
			}
			else {
				strncpy(commu.PlcSet.szPLCType, sType.c_str(), sizeof(commu.PlcSet.szPLCType) / sizeof(char) - 1);
				commu.PlcSet.szPLCType[sizeof(commu.PlcSet.szPLCType) / sizeof(char) - 1] = '\0';
			}
		}
		else if (json.isMember("type")) {
			std::string sType = json["type"].isString() ? json["type"].asString() : "";
			if (sType.length() <= sizeof(commu.PlcSet.szPLCType) / sizeof(char)) {
				strncpy(commu.PlcSet.szPLCType, sType.c_str(), sType.length());
			}
			else {
				strncpy(commu.PlcSet.szPLCType, sType.c_str(), sizeof(commu.PlcSet.szPLCType) / sizeof(char) - 1);
				commu.PlcSet.szPLCType[sizeof(commu.PlcSet.szPLCType) / sizeof(char) - 1] = '\0';
			}
		}
		if (json.isMember("frameWork")) {
			int nFramework = json["frameWork"].isInt() ? json["frameWork"].asInt() : 0;
			commu.nFramework = nFramework;
		}
		if (json.isMember("driver")) {
			std::string sDriver = json["driver"].isString() ? json["driver"].asString() : "";
			if (FRAMEWORK_NEW != commu.nFramework)
			{
				sDriver = cbl::lcase(json["driver"].isString() ? json["driver"].asString() : "");
			}
			
			if (sDriver.length() <= sizeof(commu.PlcSet.szLinkProtocol) / sizeof(char)) {
				strncpy(commu.PlcSet.szLinkProtocol, sDriver.c_str(), sDriver.length());
			}
			else {
				strncpy(commu.PlcSet.szLinkProtocol, sDriver.c_str(), sizeof(commu.PlcSet.szLinkProtocol) / sizeof(char) - 1);
				commu.PlcSet.szLinkProtocol[sizeof(commu.PlcSet.szLinkProtocol) / sizeof(char) - 1] = '\0';
			}
		}
		if (json.isMember("port")) {
			std::string sPort = json["port"].isString() ? json["port"].asString() : "";
			if (sPort.length() <= sizeof(commu.Commset.szPortName) / sizeof(char)) {
				strncpy(commu.Commset.szPortName, sPort.c_str(), sPort.length());
			}
			else {
				strncpy(commu.Commset.szPortName, sPort.c_str(), sizeof(commu.Commset.szPortName) / sizeof(char) - 1);
				commu.Commset.szPortName[sizeof(commu.Commset.szPortName) / sizeof(char) - 1] = '\0';
			}
			strncpy(commu.Commset.szInitPortName, commu.Commset.szPortName, sizeof(commu.Commset.szPortName));
		}
		if (json.isMember("check_bit")) {
			std::string sCheckBit = json["check_bit"].isString() ? json["check_bit"].asString() : "";
			if (sCheckBit.length() <= sizeof(commu.Commset.szCheckBit) / sizeof(char)) {
				strncpy(commu.Commset.szCheckBit, sCheckBit.c_str(), sCheckBit.length());
			}
			else {
				strncpy(commu.Commset.szCheckBit, sCheckBit.c_str(), sizeof(commu.Commset.szCheckBit) / sizeof(char) - 1);
				commu.Commset.szCheckBit[sizeof(commu.Commset.szCheckBit) / sizeof(char) - 1] = '\0';
			}
		}
		if (json.isMember("net_port")) {
			int nNetPort = json["net_port"].isInt() ? json["net_port"].asInt() : 0;
			std::string sNetPort = std::to_string(nNetPort);
			commu.Commset.iDstPort = nNetPort;
			if (sNetPort.length() <= sizeof(commu.Commset.szDstNetPort) / sizeof(char)) {
				strncpy(commu.Commset.szDstNetPort, sNetPort.c_str(), sNetPort.length());
			}
			else {
				strncpy(commu.Commset.szDstNetPort, sNetPort.c_str(), sizeof(commu.Commset.szDstNetPort) / sizeof(char) - 1);
				commu.Commset.szDstNetPort[sizeof(commu.Commset.szDstNetPort) / sizeof(char) - 1] = '\0';
			}
		}
		if (json.isMember("net_ipaddr")) {
			std::string sNetIp = json["net_ipaddr"].isString() ? json["net_ipaddr"].asString() : "";
			if (sNetIp.length() <= sizeof(commu.Commset.szDstIPV4) / sizeof(char)) {
				strncpy(commu.Commset.szDstIPV4, sNetIp.c_str(), sNetIp.length());
			}
			else {
				strncpy(commu.Commset.szDstIPV4, sNetIp.c_str(), sizeof(commu.Commset.szDstIPV4) / sizeof(char) - 1);
				commu.Commset.szDstIPV4[sizeof(commu.Commset.szDstIPV4) / sizeof(char) - 1] = '\0';
			}
			commu.Commset.DstIPAddr.sin_addr.s_addr = inet_addr(sNetIp.c_str());
		}
		if (!strcmp(commu.Commset.szPortName, "Ethernet")) {
			commu.Commset.nCommMode = Comm_Ethernet;
		}
		if (!strncmp(commu.Commset.szPortName, "CAN", strlen("CAN"))) { //支持双CAN
			commu.Commset.nCommMode = Comm_CAN;
			if (commu.Commset.szPortName[3] == '\0') {
				commu.Commset.szPortName[3] = '0';  //CAN 为CAN0
				commu.Commset.szInitPortName[3] = '0';
			}
		}
		if (!strncmp(commu.Commset.szPortName, "USB", strlen("USB"))) { //支持USB
			commu.Commset.nCommMode = Comm_USB;
		}
		listCommu.push_back(commu);
	}
	{
		bool bFlag = false;
		for (auto &iter : listCommu) {
			if ( FRAMEWORK_NEW != iter.nFramework )
			{
				continue;
			}
			std::string sSrcMd5 = "", sDstMd5 = "";
			std::string sName = cbl::lcase(iter.PlcSet.szLinkProtocol);
			if (cbl::isDir(sDir + std::string("E02Drv/driver/"))) {
				if (cbl::getFileSize(sDir + std::string("E02Drv/driver/") + sName) < 0) {
					addProjectParseStat(cbl::format("%s do not exist", iter.PlcSet.szPLCType), iter.nCommuId, 1, -1);
					bFlag = true;
					continue;
				}
				if (getFileMd5(sDir + sName, sDstMd5) < 0 ||
					getFileMd5(sDir + std::string("E02Drv/driver/") + sName, sSrcMd5) < 0 ||
					sSrcMd5 != sDstMd5) {
					CFile::Copy(sDir + std::string("E02Drv/driver/") + sName, sDir + sName, false);
				}
			}
			else if (cbl::isDir(sDir + std::string("driver/"))) {
				if (cbl::getFileSize(sDir + std::string("driver/") + sName) < 0) {
					addProjectParseStat(cbl::format("%s do not exist", iter.PlcSet.szPLCType), iter.nCommuId, 1, -1);
					bFlag = true;
					continue;
				}
				if (getFileMd5(sDir + sName, sDstMd5) < 0 ||
					getFileMd5(sDir + std::string("driver/") + sName, sSrcMd5) < 0 ||
					sSrcMd5 != sDstMd5) {
					CFile::Copy(sDir + std::string("driver/") + sName, sDir + sName, false);
				}
			}
			else {
				addProjectParseStat("the driver is old!", iter.nCommuId, 1, -1);
				bFlag = true;
			}
		}
		if (bFlag) {
			return -2;
		}
	}
	if (!listCommu.empty()) {
		this->saveModbusRegMap(listCommu, sDir);
	}
	{
		std::unique_lock<std::mutex> lock(m_mutexCommu);
		for (auto &iter : listCommu) {
			int nRtn = 0;
			if (0 > (nRtn = this->loadCommu(iter, sDir))) {
				if (nRtn != -1) {
					addProjectParseStat(cbl::format("add connect node error, state : %d", nRtn), iter.nCommuId, 1, nRtn);
				}
				else {
					addProjectParseStat(cbl::format("load PLC driver error, state : %d", nRtn), iter.nCommuId, 1, nRtn);
				}
				return -3;
			}
			else {
				m_listCommu.push_back(iter);
			}
		}
	}
	return 0;
}

void CCloudParse::addProjectParseStat(int nStat, const std::string &sbuffer) {
	PROJECT_PRASE_STAT_T projectPraseStat;
	projectPraseStat.nStat = nStat;
	projectPraseStat.sbuffer = sbuffer;
	projectPraseStat.nMode = 1;
	m_listProjectPrase.push_back(projectPraseStat);
}

void CCloudParse::addProjectParseStat(const std::string& sError, unsigned long nId, unsigned int nType, unsigned int nStat) {
	PROJECT_PRASE_STAT_T projectPraseStat;
	projectPraseStat.nStat = nStat;
	projectPraseStat.sError = sError;
	projectPraseStat.nMode = 2;
	projectPraseStat.nId = nId;
	projectPraseStat.nType = nType;
	m_listProjectPrase.push_back(projectPraseStat);
}

int CCloudParse::loadNormal(const std::string &sPath, std::list<COLLECTION_HISTORY_DATA> &listHistoryData, \
							std::list<COLLECTION_MONITOR_POINT_DATA> &listRealData, \
							std::list<COLLECTION_ALARM_DATA> &listAlarmData) {
	if (cbl::getFileSize(sPath) <= 0) {
		return 0;
	}
#ifdef WIN32
	tinyxml2::TinyXMLDocument doc;
#else
	tinyxml2::XMLDocument doc;
#endif
	tinyxml2::XMLElement *peleRoot;
	if (cbl::loadXmlFile(UTF8_TO_ANSI(sPath.c_str()), doc, peleRoot) < 0) {
		return -1;
	}
	tinyxml2::XMLElement *pElement = peleRoot->FirstChildElement();
	while (nullptr != pElement) {
		/* parse olcset */
		if (0 == strcmp("PlcSet", pElement->Name())) {
			this->parseCommu(pElement);
		}
		/* parse real data */
		if (0 == strcmp("RelData", pElement->Name())) {
			this->parseReal(pElement, listRealData);
		}
		/* parse alarm data */
		if (0 == strcmp("AlarmData", pElement->Name())) {
			this->parseAlarm(pElement, listAlarmData);
		}
		/* parse alarm data */
		if (0 == strcmp("HisData", pElement->Name())) {
			this->parseHistory(pElement, listHistoryData);
		}
		pElement = pElement->NextSiblingElement();
	}
	return 0;
}

int CCloudParse::loadConfigini(const std::string &sPath) {
	int nRtn = 0;
	std::string sbuffer = "";
	std::vector<std::string> vecCRLF, vec;
	if (cbl::getFileSize(sPath) < 0) {
		m_bEnableReal = m_bEnableHistory = m_bEnableAlarm = m_bEnableKeepGps = true;
		m_nRealPeriod = 2000;
		m_nRealMapPeriod = 5000;
		m_nGpsPeriod = 1000;
		return 0;
	}
	if (cbl::readFileOnce64(sPath, 0, SEEK_SET, cbl::getFileSize(sPath), sbuffer) < 0 ||
		sbuffer.empty()) {
		return -2;
	}
	if (split(sbuffer, "\n", &vecCRLF) < 0 ||
		vecCRLF.empty()) {
		return -3;
	}
	for (auto &iter : vecCRLF) {
		int nValue = 0;
		if (split(iter, "=", &vec) < 0 ||
			vec.size() != 2) {
			continue;
		}
		if (!vec[1].empty()) {
			try {
				nValue = std::stoi(vec[1]);
			} catch (const std::exception& e) {
				printf("%s\r\n", e.what());
				continue;
			}
		}
		if (vec[0] == "reldata") {
			m_bEnableReal = vec[1].empty() ? true : nValue == 1;
		}
		else if (vec[0] == "hisdata") {
			m_bEnableHistory = vec[1].empty() ? true : nValue == 1;
		}
		else if (vec[0] == "almdata") {
			m_bEnableAlarm = vec[1].empty() ? true : nValue == 1;
		}
		else if (vec[0] == "coltime") {
			m_nRealPeriod = vec[1].empty() || (nValue < 1000) ? 2000 : nValue;
		}
		else if (vec[0] == "colmaptime") {
			m_nRealMapPeriod = vec[1].empty() || (nValue < 2000) ? 5000 : nValue;
		}
		else if (vec[0] == "EnKeepReportGps") {
			m_bEnableKeepGps = vec[1].empty() ? false : nValue == 1;
		}
		else if (vec[0] == "ReportGpsTime") {
			m_nGpsPeriod = vec[1].empty() ? 1000 : nValue;
		}
	}
	sbuffer = cbl::format("[switch]\n\
reldata=%d\n\
hisdata=%d\n\
almdata=%d\n\
coltime=%d\n\
colmaptime=%d\n\
EnKeepReportGps=%d\n\
ReportGpsTime=%d\n", m_bEnableReal, m_bEnableHistory, m_bEnableAlarm, m_nRealPeriod, m_nRealMapPeriod, m_bEnableKeepGps, m_nGpsPeriod);
#ifdef WIN32
	nRtn = cbl::saveToFile(sbuffer, sPath);
#else
	nRtn = cbl::syncSaveToFile(sbuffer, sPath);
#endif
	if (nRtn < 0) {
		return -4;
	}
	return 0;
}

int CCloudParse::parsePLCNode(tinyxml2::XMLElement *pElement, PLCSET &plcSet) {
	/* check params */
	if (nullptr == pElement) {
		return -1;
	}
	if (nullptr != pElement->Attribute("Type")) {
		if (strlen(pElement->Attribute("Type")) <= sizeof(plcSet.szPLCType) / sizeof(char)) {
			strncpy(plcSet.szPLCType, pElement->Attribute("Type"), strlen(pElement->Attribute("Type")));
		}
		else {
			strncpy(plcSet.szPLCType, pElement->Attribute("Type"), sizeof(plcSet.szPLCType) / sizeof(char) - 1);
			plcSet.szPLCType[sizeof(plcSet.szPLCType) / sizeof(char) - 1] = '\0';
		}
	}
	if (nullptr != pElement->Attribute("Driver")) {
		std::string sDriver = pElement->Attribute("Driver");
		if (sDriver.length() <= sizeof(plcSet.szLinkProtocol) / sizeof(char)) {
			strncpy(plcSet.szLinkProtocol, sDriver.c_str(), sDriver.length());
		}
		else {
			strncpy(plcSet.szLinkProtocol, sDriver.c_str(), sizeof(plcSet.szLinkProtocol) / sizeof(char) - 1);
			plcSet.szLinkProtocol[sizeof(plcSet.szLinkProtocol) / sizeof(char) - 1] = '\0';
		}
	}
	if (nullptr != pElement->Attribute("HmiStatNo")) {
		plcSet.nHmiMachineNo = (int)strtol(pElement->Attribute("HmiStatNo"), nullptr, 10);
	}
	if (nullptr != pElement->Attribute("PlcStatNo")) {
		plcSet.nPlcMachineNo = (int)strtol(pElement->Attribute("PlcStatNo"), nullptr, 10);
	}
	return 0;
}

int CCloudParse::parseCommSetNode(tinyxml2::XMLElement *pElement, COMMSET &commSet) {
	/* check params */
	if (nullptr == pElement) {
		return -1;
	}
	if (nullptr != pElement->Attribute("Port")) {
		if (strlen(pElement->Attribute("Port")) <= sizeof(commSet.szPortName) / sizeof(char)) {
			strncpy(commSet.szPortName, pElement->Attribute("Port"), strlen(pElement->Attribute("Port")));
		}
		else {
			strncpy(commSet.szPortName, pElement->Attribute("Port"), sizeof(commSet.szPortName) / sizeof(char) - 1);
			commSet.szPortName[sizeof(commSet.szPortName) / sizeof(char) - 1] = '\0';
		}
		if (strlen(pElement->Attribute("Port")) <= sizeof(commSet.szInitPortName) / sizeof(char)) {
			strncpy(commSet.szInitPortName, pElement->Attribute("Port"), strlen(pElement->Attribute("Port")));
		}
		else {
			strncpy(commSet.szInitPortName, pElement->Attribute("Port"), sizeof(commSet.szInitPortName) / sizeof(char) - 1);
			commSet.szInitPortName[sizeof(commSet.szInitPortName) / sizeof(char) - 1] = '\0';
		}
	}
	if (nullptr != pElement->Attribute("ComSel")) {
		if (strlen(pElement->Attribute("ComSel")) <= sizeof(commSet.szComSelect) / sizeof(char)) {
			strncpy(commSet.szComSelect, pElement->Attribute("ComSel"), strlen(pElement->Attribute("ComSel")));
		}
		else {
			strncpy(commSet.szComSelect, pElement->Attribute("ComSel"), sizeof(commSet.szComSelect) / sizeof(char) - 1);
			commSet.szComSelect[sizeof(commSet.szComSelect) / sizeof(char) - 1] = '\0';
		}
	}
	if (nullptr != pElement->Attribute("CheckBit")) {
		if (strlen(pElement->Attribute("CheckBit")) <= sizeof(commSet.szCheckBit) / sizeof(char)) {
			strncpy(commSet.szCheckBit, pElement->Attribute("CheckBit"), strlen(pElement->Attribute("CheckBit")));
		}
		else {
			strncpy(commSet.szCheckBit, pElement->Attribute("CheckBit"), sizeof(commSet.szCheckBit) / sizeof(char) - 1);
			commSet.szCheckBit[sizeof(commSet.szCheckBit) / sizeof(char) - 1] = '\0';
		}
	}
	if (nullptr != pElement->Attribute("ComType")) {
		commSet.nCommMode = (int)strtol(pElement->Attribute("ComType"), nullptr, 10);
	}
	if (nullptr != pElement->Attribute("BaudRate")) {
		commSet.nBaudRate = (int)strtol(pElement->Attribute("BaudRate"), nullptr, 10);
	}
	if (nullptr != pElement->Attribute("DataLength")) {
		commSet.nDataLength = (int)strtol(pElement->Attribute("DataLength"), nullptr, 10);
	}
	if (nullptr != pElement->Attribute("StopBit")) {
		commSet.nStopBits = (int)strtol(pElement->Attribute("StopBit"), nullptr, 10);
	}
	if (nullptr != pElement->Attribute("RetryTimes")) {
		commSet.nRetryTimes = (int)strtol(pElement->Attribute("RetryTimes"), nullptr, 10);
	}
	if (nullptr != pElement->Attribute("WaitTimeout")) {
		commSet.nWaitTimeout = (int)strtol(pElement->Attribute("WaitTimeout"), nullptr, 10);
	}
	if (nullptr != pElement->Attribute("RevTimeout")) {
		commSet.nRecvTimeout = (int)strtol(pElement->Attribute("RevTimeout"), nullptr, 10);
	}
	if (nullptr != pElement->Attribute("RetryTimeOut")) {
		commSet.nRetryTimeOut = (int)strtol(pElement->Attribute("RetryTimeOut"), nullptr, 10);
	}
	if (nullptr != pElement->Attribute("ComIoDelayTime")) {
		commSet.nComIoDelayTime = (int)strtol(pElement->Attribute("ComIoDelayTime"), nullptr, 10);
	}
	if (nullptr != pElement->Attribute("ComSingleLen")) {
		commSet.nComSingleReadLen = (int)strtol(pElement->Attribute("ComSingleLen"), nullptr, 10);
	}
	if (nullptr != pElement->Attribute("ComStepInterval")) {
		commSet.nComStepInterval = (int)strtol(pElement->Attribute("ComStepInterval"), nullptr, 10);
	}
	if (nullptr != pElement->Attribute("FlowMode")) {
		commSet.nFlowControl = (int)strtol(pElement->Attribute("FlowMode"), nullptr, 10);
	}
	if (nullptr != pElement->Attribute("ConnectTime")) {
		commSet.nConnectTime = (int)strtol(pElement->Attribute("ConnectTime"), nullptr, 10);
	}
	if (nullptr != pElement->Attribute("NETBroadcastAddr")) {
		commSet.wBroadcastAddr = (int)strtol(pElement->Attribute("NETBroadcastAddr"), nullptr, 10);
	}
	if (nullptr != pElement->Attribute("NETIsBroadcast")) {
		commSet.bEthernet_IsBroadcast = (int)strtol(pElement->Attribute("NETIsBroadcast"), nullptr, 10);
	}
	if (nullptr != pElement->Attribute("NETIsUDP")) {
		commSet.nEthernet_IsUDP = (int)strtol(pElement->Attribute("NETIsUDP"), nullptr, 10);
	}
	if (nullptr != pElement->Attribute("NETIPaddr")) {
		char szTemp[32] = { 0 };
		strncpy(szTemp, pElement->Attribute("NETIPaddr"), 32);
		commSet.DstIPAddr.sin_addr.s_addr = inet_addr(szTemp);
	}
	if (nullptr != pElement->Attribute("NETIPaddr")) {
		strcpy(commSet.szDstIPV4, pElement->Attribute("NETIPaddr"));
	}
	if (nullptr != pElement->Attribute("NETPort")) {
		commSet.iDstPort = (int)strtol(pElement->Attribute("NETPort"), nullptr, 10);
	}
	if (nullptr != pElement->Attribute("NETPort")) {
		strcpy(commSet.szDstNetPort, pElement->Attribute("NETPort"));
	}
	if (!strcmp(commSet.szPortName, "Ethernet")) {
		commSet.nCommMode = Comm_Ethernet;
	}
	if (!strncmp(commSet.szPortName, "CAN", strlen("CAN"))) //支持双CAN
	{
		commSet.nCommMode = Comm_CAN;
		if (commSet.szPortName[3] == '\0') {
			commSet.szPortName[3] = '0';  //CAN 为CAN0
			commSet.szInitPortName[3] = '0';
		}
	}
	if (!strncmp(commSet.szPortName, "USB", strlen("USB"))) //支持USB
	{
		commSet.nCommMode = Comm_USB;
	}
	return 0;
}

int CCloudParse::parseCommu(tinyxml2::XMLElement *pElement) {
	if (nullptr == pElement) {
		return -1;
	}
	std::list<CONNECTNODE> listCommu;
	tinyxml2::XMLElement *pConnectElement = pElement->FirstChildElement();
	while (nullptr != pConnectElement) {
		if (0 != strcmp("Connection", pConnectElement->Name())) {
			pConnectElement = pConnectElement->NextSiblingElement();
			continue;
		}
		/* parse connect node */
		CONNECTNODE connectNodeInfo;
		memset(&connectNodeInfo, 0, sizeof(CONNECTNODE));
		connectNodeInfo.nPortId = -1;
		if (nullptr != pConnectElement->Attribute("ComID")) {
			connectNodeInfo.nCommuId = (int)strtol(pConnectElement->Attribute("ComID"), nullptr, 10);
		}
		tinyxml2::XMLElement *pConnectChildElement = pConnectElement->FirstChildElement();
		while (nullptr != pConnectChildElement) {
			if (0 == strcmp("PLC", pConnectChildElement->Name()) &&
				parsePLCNode(pConnectChildElement, connectNodeInfo.PlcSet) < 0) {
				return -2;
			}
			else if (0 == strcmp("CommSet", pConnectChildElement->Name()) && 
					 parseCommSetNode(pConnectChildElement, connectNodeInfo.Commset) < 0) {
				return -3;
			}
			else if (0 == strcmp("FrameWorkSet", pConnectChildElement->Name())) {
				connectNodeInfo.nFramework = (int)strtol(pConnectChildElement->Attribute("FrameWork"), nullptr, 10);
			}
			pConnectChildElement = pConnectChildElement->NextSiblingElement();
		}
		listCommu.push_back(connectNodeInfo);
		pConnectElement = pConnectElement->NextSiblingElement();
	}
	if (!listCommu.empty()) {
		this->saveModbusRegMap(listCommu, getPrjDir());
	}
	{
		std::unique_lock<std::mutex> lock(m_mutexCommu);
		for (auto &iter : listCommu) {
			if (0 > this->loadCommu(iter, getPrjDir())) {
				return -4;
			}
			m_listCommu.push_back(iter);
		}
	}
	return 0;
}

int CCloudParse::parseReal(tinyxml2::XMLElement *pElement, std::list<COLLECTION_MONITOR_POINT_DATA> &listRealData) {
	/* check params */
	if (nullptr == pElement) {
		return -1;
	}
	/* parse monitor point */
	tinyxml2::XMLElement *pRealNode = pElement->FirstChildElement();
	while (nullptr != pRealNode) {
		if (0 != strcmp("RelNode", pRealNode->Name())) {
			pRealNode = pRealNode->NextSiblingElement();
			continue;
		}
		COLLECTION_MONITOR_POINT_DATA pointData;
		if (nullptr != pRealNode->Attribute("Name")) {
			string sName = pRealNode->Attribute("Name");
			pointData.vecName.assign({ sName, sName, sName, sName, sName, sName, sName, sName });
		}
		if (nullptr != pRealNode->Attribute("ID")) {
			pointData.nUid = (unsigned int)strtol(pRealNode->Attribute("ID"), nullptr, 10);
		}
		if (nullptr != pRealNode->Attribute("ComID")) {
			pointData.nCommuID = (unsigned int)strtol(pRealNode->Attribute("ComID"), nullptr, 10);
		}
		if (nullptr != pRealNode->Attribute("DispType")) {
			pointData.nDataFormat = (unsigned int)strtol(pRealNode->Attribute("DispType"), nullptr, 10);
		}
		if (nullptr != pRealNode->Attribute("FloatPoint")) {
			pointData.nHighLowWord = (int)strtol(pRealNode->Attribute("FloatPoint"), nullptr, 10);
		}
		if (nullptr != pRealNode->Attribute("StringPoint")) {
			pointData.nHighLowByte = (int)strtol(pRealNode->Attribute("StringPoint"), nullptr, 10);
		}
		if (nullptr != pRealNode->Attribute("RoleType")) {
			pointData.nPerssion = (unsigned int)strtol(pRealNode->Attribute("RoleType"), nullptr, 10) - 1;
		}
		if (nullptr != pRealNode->Attribute("AddrType")) {
			pointData.addrType = (ADDR_TYPE_T)pRealNode->Attribute("AddrType")[0];
			if (0 == strcmp("X", pRealNode->Attribute("AddrType"))) {
				pointData.bBitCollect = true;
			}
			else {
				pointData.bBitCollect = false;
			}
		}
		if (pRealNode->Attribute("Addr")) {
			pointData.sAddrAlias.assign(pRealNode->Attribute("Addr"));
			std::string sAddrString;
			if (0 == CCompileAddress::get_instance().compile(pRealNode->Attribute("Addr"), pointData.nCommuID, pointData.addrType, sAddrString)) {
				stringToRegister(sAddrString, pointData.addrMonitor);
			}
		}
		if (nullptr != pRealNode->Attribute("TrafficStatus")) {
			if (0 == strcmp("0", pRealNode->Attribute("TrafficStatus"))) {
				pointData.bSaveTraffic = false;
			}
			else if (0 == strcmp("1", pRealNode->Attribute("TrafficStatus"))) {
				pointData.bSaveTraffic = true;
			}
		}
		if (nullptr != pRealNode->Attribute("Keep_Report")) {
			if (0 == strcmp("0", pRealNode->Attribute("Keep_Report"))) {
				pointData.bMap = false;
			}
			else if (0 == strcmp("1", pRealNode->Attribute("Keep_Report"))) {
				pointData.bMap = true;
			}
		}
		if (nullptr != pRealNode->Attribute("DigitCount")) {
			if (nullptr == strchr(pRealNode->Attribute("DigitCount"), ' ')) {
				pointData.nInteger = 5;
				pointData.nDecimals = 0;
			}
			else {
				sscanf(pRealNode->Attribute("DigitCount"), "%d %d", &(pointData.nInteger), &(pointData.nDecimals));
			}
		}
		if (nullptr != pRealNode->Attribute("DeadSet")) {
			Convert::BuffToUnionData(pRealNode->Attribute("DeadSet"), pointData.nDataFormat, pointData.nDecimals, pointData.u4wDeadZoneValue);
		}
		if (_String == pointData.nDataFormat) {
			pointData.nDataLen = pointData.nInteger;
		}
		listRealData.push_back(pointData);
		pRealNode = pRealNode->NextSiblingElement();
	}
	return 0;
}

int CCloudParse::parseAlarm(tinyxml2::XMLElement *pElement, std::list<COLLECTION_ALARM_DATA> &listAlarmData) {
	/* check params */
	if (nullptr == pElement) {
		return -1;
	}
	/* parse monitor point */
	tinyxml2::XMLElement *pAlarmNode = pElement->FirstChildElement("AlmNode");
	while (nullptr != pAlarmNode) {
		COLLECTION_ALARM_DATA alarmData;
		alarmData.bSave = false;
		alarmData.bReportCloud = true;
		if (nullptr != pAlarmNode->Attribute("ID")) {
			alarmData.nUid = (unsigned int)strtol(pAlarmNode->Attribute("ID"), nullptr, 10);
		}
		if (nullptr != pAlarmNode->Attribute("Name")) {
			alarmData.sName.assign(pAlarmNode->Attribute("Name"));
		}
		if (nullptr != pAlarmNode->Attribute("ComID")) {
			alarmData.nMonitorCommuID = (unsigned int)strtol(pAlarmNode->Attribute("ComID"), nullptr, 10);
		}
		if (nullptr != pAlarmNode->Attribute("DispType")) {
			alarmData.nDataFormat = (unsigned int)strtol(pAlarmNode->Attribute("DispType"), nullptr, 10);
		}
		if (nullptr != pAlarmNode->Attribute("AddrType")) {
			alarmData.monitorAddrType = (ADDR_TYPE_T)pAlarmNode->Attribute("AddrType")[0];
			if (ADDR_TYPE_BIT == alarmData.monitorAddrType) {
				alarmData.type = ALARM_TYPE_BIT;
			}
			else {
				alarmData.type = ALARM_TYPE_WORD;
			}
		}
		if (nullptr != pAlarmNode->Attribute("DigitCount")) {
			if (nullptr == strchr(pAlarmNode->Attribute("DigitCount"), ' ')) {
				alarmData.nInteger = 5;
				alarmData.nDecimals = 0;
			}
			else {
				sscanf(pAlarmNode->Attribute("DigitCount"), "%d %d", &(alarmData.nInteger), &(alarmData.nDecimals));
			}
		}
		if (nullptr != pAlarmNode->Attribute("CondType")) {
			alarmData.conditionType = (ARITHMETIC_TYPE_T)strtol(pAlarmNode->Attribute("CondType"), nullptr, 10);
		}
		if (nullptr != pAlarmNode->Attribute("FloatPoint")) {
			alarmData.nHighLowWord = (int)strtol(pAlarmNode->Attribute("FloatPoint"), nullptr, 10);
		}
		if (nullptr != pAlarmNode->Attribute("StringPoint")) {
			alarmData.nHighLowByte = (int)strtol(pAlarmNode->Attribute("StringPoint"), nullptr, 10);
		}
		if (nullptr != pAlarmNode->Attribute("Addr")) {
			string sAddrString;
			alarmData.sAddressAlias.assign(pAlarmNode->Attribute("Addr"));
			if (0 == CCompileAddress::get_instance().compile(pAlarmNode->Attribute("Addr"), alarmData.nMonitorCommuID, alarmData.monitorAddrType, sAddrString)) {
				stringToRegister(sAddrString, alarmData.addrMonitor);
			}
		}
		if (_String == alarmData.nDataFormat) {
			alarmData.nDataLen = alarmData.nInteger;
		}
		alarmData.vecText.assign(8, "");
		this->parseAlarmCond(pAlarmNode, alarmData);
		listAlarmData.push_back(alarmData);
		pAlarmNode = pAlarmNode->NextSiblingElement("AlmNode");
	}
	return 0;
}

int CCloudParse::parseAlarmCond(tinyxml2::XMLElement *pElement, COLLECTION_ALARM_DATA& alarm) {
	/* check params */
	if (nullptr == pElement) {
		return -1;
	}
	tinyxml2::XMLElement *pCondition = pElement->FirstChildElement("Condition");
	if (nullptr == pCondition) {
		return -2;
	}
	if (ARITHMETIC_TYPE_SINGLE == alarm.conditionType) {
		if (ALARM_TYPE_BIT == alarm.type) {
			if (pCondition->Attribute("AlValue")) {
				alarm.wTriggerValue = (unsigned short)strtol(pCondition->Attribute("AlValue"), nullptr, 10);
			}
		}
		else if (ALARM_TYPE_WORD == alarm.type) {
			if (pCondition->Attribute("AlValue")) {
				if (alarm.nDataFormat == _String) {
					alarm.sCondition = pCondition->Attribute("AlValue");
				}
				else {
					Convert::BuffToUnionData(pCondition->Attribute("AlValue"), alarm.nDataFormat, alarm.nDecimals, alarm.u4wRangeEnd);
				}
			}
			if (pCondition->Attribute("TrigType")) {
				alarm.mode = toAlarmMode((unsigned short)strtol(pCondition->Attribute("TrigType"), nullptr, 10));
			}
		}
	}
	else {
		alarm.mode = ALARM_MODE_RANGELIMIT;
		if (pCondition->Attribute("AlValue")) {
			if (alarm.nDataFormat == _String) {
				alarm.sCondition = pCondition->Attribute("AlValue");
			}
			else {
				Convert::BuffToUnionData(pCondition->Attribute("AlValue"), alarm.nDataFormat, alarm.nDecimals, alarm.u4wRangeStart);
			}
		}
		if (pCondition->Attribute("TrigType")) {
			alarm.modeStart = toAlarmMode((unsigned short)strtol(pCondition->Attribute("TrigType"), nullptr, 10));
		}
		tinyxml2::XMLElement *pConditionEnd = pCondition->NextSiblingElement("Condition");
		if (nullptr == pConditionEnd) {
			return -3;
		}
		if (pConditionEnd->Attribute("AlValue")) {
			if (alarm.nDataFormat == _String) {
				alarm.sConditionEnd = pConditionEnd->Attribute("AlValue");
			}
			else {
				Convert::BuffToUnionData(pConditionEnd->Attribute("AlValue"), alarm.nDataFormat, alarm.nDecimals, alarm.u4wRangeEnd);
			}
		}
		if (pConditionEnd->Attribute("TrigType")) {
			alarm.modeEnd = toAlarmMode((unsigned short)strtol(pConditionEnd->Attribute("TrigType"), nullptr, 10));
		}
	}
	return 0;
}

int CCloudParse::parseHistory(tinyxml2::XMLElement *pElement, std::list<COLLECTION_HISTORY_DATA> &listHistoryData) {
	/* check params */
	if (nullptr == pElement) {
		return -1;
	}
	tinyxml2::XMLElement *pGroupNode = pElement->FirstChildElement();
	while (nullptr != pGroupNode) {
		if (0 == strcmp("Group", pGroupNode->Name())) {
			tinyxml2::XMLElement *pGroup = pGroupNode->FirstChildElement();
			while (nullptr != pGroup) {
				if (0 != strcmp("HisGroup", pGroup->Name())) {
					continue;
				}
				COLLECTION_HISTORY_DATA historyData;
				if (nullptr != pGroup->Attribute("ID")) {
					historyData.nGroupId = (unsigned int)strtol(pGroup->Attribute("ID"), nullptr, 10);
				}
				if (nullptr != pGroup->Attribute("Cycle")) {
					historyData.nInterval = (unsigned int)strtol(pGroup->Attribute("Cycle"), nullptr, 10) / 1000;
				}
				if (nullptr != pGroup->Attribute("TriggerType")) {
					historyData.nType = (unsigned int)strtol(pGroup->Attribute("TriggerType"), nullptr, 10);
				}
				if (nullptr != pGroup->Attribute("TriggerComID")) {
					historyData.nTrigCommuID = (unsigned int)strtol(pGroup->Attribute("TriggerComID"), nullptr, 10);
				}
				if (nullptr != pGroup->Attribute("TriggerAddrType")) {
					historyData.trigerAddrType = (ADDR_TYPE_T)pGroup->Attribute("TriggerAddrType")[0];
				}
				if (nullptr != pGroup->Attribute("TriggerAddr")) {
					historyData.sTriggerAddrAlias.assign(pGroup->Attribute("TriggerAddr"));
					std::string sAddrString;
					if (0 == CCompileAddress::get_instance().compile(pGroup->Attribute("TriggerAddr"), historyData.nTrigCommuID, historyData.trigerAddrType, sAddrString)) {
						stringToRegister(sAddrString, historyData.addrTrigger);
					}
				}
				historyData.bFieldReportCloud = true;
				if (nullptr != pGroup->Attribute("TriggerValue")) {
					if (0 == strcmp("0", pGroup->Attribute("TriggerValue"))) {
						historyData.bTriggerValue = false;
					}
					else if (0 == strcmp("1", pGroup->Attribute("TriggerValue"))) {
						historyData.bTriggerValue = true;
					}
				}
				listHistoryData.push_back(historyData);
				pGroup = pGroup->NextSiblingElement();
			}
		}
		else if (0 == strcmp("HisNode", pGroupNode->Name())) {
			COLLECTION_HISTORY_NODE historyNode;
			if (nullptr != pGroupNode->Attribute("Name")) {
				std::string sName = pGroupNode->Attribute("Name");
				historyNode.vecFieldName.assign({ sName, sName, sName, sName, sName, sName, sName, sName });
			}
			if (nullptr != pGroupNode->Attribute("ID")) {
				historyNode.nId = (unsigned int)strtol(pGroupNode->Attribute("ID"), nullptr, 10);
			}
			if (nullptr != pGroupNode->Attribute("ComID")) {
				historyNode.nCommuID = (unsigned int)strtol(pGroupNode->Attribute("ComID"), nullptr, 10);
			}
			if (nullptr != pGroupNode->Attribute("AddrType")) {
				historyNode.fieldAddrType = (ADDR_TYPE_T)pGroupNode->Attribute("AddrType")[0];
			}
			if (nullptr != pGroupNode->Attribute("DispType")) {
				historyNode.nDataFormat = (unsigned int)strtol(pGroupNode->Attribute("DispType"), nullptr, 10);
			}
			if (nullptr != pGroupNode->Attribute("FloatPoint")) {
				historyNode.nHighLowWord = (int)strtol(pGroupNode->Attribute("FloatPoint"), nullptr, 10);
			}
			if (nullptr != pGroupNode->Attribute("StringPoint")) {
				historyNode.nHighLowByte = (int)strtol(pGroupNode->Attribute("StringPoint"), nullptr, 10);
			}
			if (nullptr != pGroupNode->Attribute("Addr")) {
				historyNode.sFieldAddrAlias.assign(pGroupNode->Attribute("Addr"));
				std::string sAddrString;
				if (0 == CCompileAddress::get_instance().compile(pGroupNode->Attribute("Addr"), historyNode.nCommuID, historyNode.fieldAddrType, sAddrString)) {
					stringToRegister(sAddrString, historyNode.addrField);
				}
			}
			historyNode.bReportCloud = true;
			if (nullptr != pGroupNode->Attribute("DigitCount")) {
				if (nullptr == strchr(pGroupNode->Attribute("DigitCount"), ' ')) {
					historyNode.nInteger = 5;
					historyNode.nDecimals = 0;
				}
				else {
					sscanf(pGroupNode->Attribute("DigitCount"), "%d %d", &(historyNode.nInteger), &(historyNode.nDecimals));
				}
			}
			if (_String == historyNode.nDataFormat) {
				historyNode.nDataLen = historyNode.nInteger;
			}
			if (nullptr != pGroupNode->Attribute("GroupID")) {
				historyNode.nGroupId = (unsigned int)strtoul(pGroupNode->Attribute("GroupID"), nullptr, 10);
				for (auto &iter : listHistoryData) {
					if (iter.nGroupId == historyNode.nGroupId) {
						iter.listField.push_back(historyNode);
					}
				}
			}
		}
		pGroupNode = pGroupNode->NextSiblingElement();
	}
	return 0;
}

int CCloudParse::loadCloudProject(const std::string &sPath, std::list<LUASCRIPT_T> &scripts) {
	Json::Value root;
	std::list<COLLECTION_MONITOR_GROUP_DATA> listMGroup;
	std::list<COLLECTION_MONITOR_POINT_DATA> listMPoint;
	std::vector<std::string> vecCustomTag;
	if (cbl::loadJsonFile(sPath, root) < 0) {
		addProjectParseStat(-1, "state");
		return -1;
	}
	if (!root.isMember("data")) {
		addProjectParseStat(-2, "state");
		return -2;
	}
	addProjectParseStat(1, "state");
	if (!root["data"].isMember("comConfigList") ||
		this->parseCommu(root["data"]["comConfigList"], getPrjCfgDir() + std::string("cloudprj/")) < 0) {
		addProjectParseStat(-1, "plc_state");
	}
	else {
		addProjectParseStat(1, "plc_state");
	}
	if (root["data"].isMember("collectGroupList")){
		this->parseMGroup(root["data"]["collectGroupList"], listMGroup);
	}
	if (root["data"].isMember("collectConfigList")) {
		this->parseMPoint(root["data"]["collectConfigList"], listMPoint, vecCustomTag);
	}
	for (auto &iter :listMGroup) {
		if (CCMPoint::get_instance().addGroup(iter, false) < 0) {
			printf("%s:CCMPoint add group error\r\n", __FUNCTION__);
			return -3;
		}
	}
	for (auto &iter : listMPoint) {
		if (CCMPoint::get_instance().addPoint(iter.nGroupID, iter, false) < 0) {
			printf("%s:CCMPoint add point error\r\n", __FUNCTION__);
			return -4;
		}
	}
	if (!vecCustomTag.empty()) {
		if (CCMPoint::get_instance().setCustomTag(vecCustomTag) < 0) {
			printf("%s:CCMPoint set customtag error\r\n", __FUNCTION__);
			return -5;
		}
	}
	if (this->loadlua(root, scripts) < 0) {
		printf("%s:load lua error\r\n", __FUNCTION__);
		return -6;
	}
	return 0;
}

int CCloudParse::parseMGroup(const Json::Value &root, std::list<COLLECTION_MONITOR_GROUP_DATA> &listMGroup) {
	if (!root.isArray()) {
		return -1;
	}
	for (unsigned int nI = 0; nI < root.size(); nI++) {
		Json::Value json = root[nI];
		COLLECTION_MONITOR_GROUP_DATA mgroup;
		if (json.isMember("group_id")) {
			mgroup.nGroupId = json["group_id"].isInt() ? json["group_id"].asInt() : 0;
		}
		if (json.isMember("his_cycle")) {
			mgroup.nInterval = json["his_cycle"].isInt() ? json["his_cycle"].asInt() * 1000 : 0;
		}
		if (json.isMember("trigger_type")) {
			mgroup.nTriggerMode = json["trigger_type"].isInt() ? json["trigger_type"].asInt() : 0;
		}
		if (json.isMember("condition_type")) {
			std::vector<std::string> vecParms;
			std::string sConditionType = json["trigger_type"].isString() ? json["trigger_type"].asString() : "";
			cbl::split(sConditionType, ",", &vecParms);
			switch (vecParms.size()) {
			case 1:
				{
					mgroup.nTriggerCondF = strtoul(vecParms[0].c_str(), nullptr, 10);
					break;
				}
			case 2:
				{
					mgroup.nTriggerCondF = strtoul(vecParms[0].c_str(), nullptr, 10);
					mgroup.nArithmetic = strtoul(vecParms[1].c_str(), nullptr, 10);
					break;
				}
			case 3:
				{
					mgroup.nTriggerCondF = strtoul(vecParms[0].c_str(), nullptr, 10);
					mgroup.nArithmetic = strtoul(vecParms[1].c_str(), nullptr, 10);
					mgroup.nTriggerCondS = strtoul(vecParms[2].c_str(), nullptr, 10);
					break;
				}
			default:
				break;
			}
		}
		if (json.isMember("plc_id")) {
			mgroup.nCommuID = json["plc_id"].isInt() ? json["plc_id"].asInt() : 0;
		}
		if (json.isMember("data_id")) {
			mgroup.nDataFormat = json["data_id"].isInt() ? json["data_id"].asInt() : 0;
		}
		if (json.isMember("digit_count")) {
			std::string sDigitCount = json["digit_count"].isString() ? json["digit_count"].asString().c_str() : "";
			if (nullptr == strchr(sDigitCount.c_str(), ',')) {
				mgroup.nInteger = 5;
				mgroup.nDecimals = 0;
			}
			else {
				sscanf(sDigitCount.c_str(), "%d,%d", &(mgroup.nInteger), &(mgroup.nDecimals));
			}
		}
		if (json.isMember("addr_type")) {
			mgroup.triggerAddrType = toAddrType(json["addr_type"].isInt() ? json["addr_type"].asInt() : 0);
		}
		if (json.isMember("condition_value")) {
			Convert::BuffToUnionData(json["condition_value"].isString() ? json["condition_value"].asString() : "", mgroup.nDataFormat, mgroup.nDecimals, mgroup.u4wTriValueF);
		}
		if (json.isMember("value")) {
			Convert::BuffToUnionData(json["value"].isString() ? json["value"].asString() : "", mgroup.nDataFormat, mgroup.nDecimals, mgroup.u4wTriValueS);
		}
		if (json.isMember("group_name")) {
			std::string sGroupName = json["group_name"].isString() ? json["group_name"].asString() : "";
			mgroup.vecGroupName.assign({ sGroupName, sGroupName, sGroupName, sGroupName , sGroupName , sGroupName , sGroupName , sGroupName });
		}
		if (json.isMember("float_point")) {
			mgroup.nHighLowWord = json["float_point"].isInt() ? json["float_point"].asInt() : -1;
		}
		if (json.isMember("string_point")) {
			mgroup.nHighLowByte = json["string_point"].isInt() ? json["string_point"].asInt() : -1;
		}
		if (MONITOR_POINT_MODE_WORD == mgroup.nTriggerMode
			|| MONITOR_POINT_MODE_TRIGGER_TIMESAMPLE == mgroup.nTriggerMode
			|| MONITOR_POINT_MODE_TRIGGER_AUTOMATICRESET == mgroup.nTriggerMode
			|| MONITOR_POINT_MODE_TRIGGER_NORESET == mgroup.nTriggerMode) {
			std::vector<std::string> vecField = { "addr_stat_no", "rid", "addr" };
			if (addressTransform(json, vecField, mgroup.nCommuID, mgroup.triggerAddrType, mgroup.addrTrigger, mgroup.sTriggerAlias) < 0) {
				addProjectParseStat("address transform error, state : -1", mgroup.nGroupId, 2, -1);
				continue;
			}
		}
		listMGroup.push_back(mgroup);
	}
	return 0;
}

int CCloudParse::parseMPoint(const Json::Value &root, std::list<COLLECTION_MONITOR_POINT_DATA> &listMPoint,
							 std::vector<std::string> &vecCustomTag) {
	if (!root.isArray()) {
		return -1;
	}
	for (unsigned int nI = 0; nI < root.size(); nI++) {
		Json::Value json = root[nI];
		COLLECTION_MONITOR_POINT_DATA mpoint;
		if (json.isMember("plc_id")) {
			mpoint.nCommuID = json["plc_id"].isInt() ? json["plc_id"].asInt() : 0;
		}
		if (json.isMember("collect_id")) {
			mpoint.nUid = json["collect_id"].isInt() ? json["collect_id"].asInt() : 0;
		}
		if (json.isMember("data_id")) {
			mpoint.nDataFormat = json["data_id"].isInt() ? json["data_id"].asInt() : 0;
			if (_String == mpoint.nDataFormat) {
				mpoint.nDataLen = mpoint.nInteger;
			}
		}
		if (json.isMember("digit_count")) {
			std::string sDigitCount = json["digit_count"].isString() ? json["digit_count"].asString().c_str() : "";
			if (mpoint.nDataFormat == _String) {
				try {
					mpoint.nDataLen = std::stoi(sDigitCount);
				}
				catch (const std::exception& e) {
					printf("%s\r\n", e.what());
				}
			}
			else {
				if (std::string::npos == sDigitCount.find(",")) {
					mpoint.nInteger = 5;
					mpoint.nDecimals = 0;
				}
				else {
					sscanf(sDigitCount.c_str(), "%d,%d", &(mpoint.nInteger), &(mpoint.nDecimals));
				}
			}
		}
		if (json.isMember("collect_name")) {
			std::string sPointName = json["collect_name"].isString() ? json["collect_name"].asString() : "";
			mpoint.vecName.assign({ sPointName, sPointName, sPointName, sPointName , sPointName , sPointName , sPointName , sPointName });
		}
		if (json.isMember("addr_type")) {
			mpoint.addrType = toAddrType(json["addr_type"].isInt() ? json["addr_type"].asInt() : 0);
			if (ADDR_TYPE_BIT == mpoint.addrType) {
				mpoint.bBitCollect = true;
			}
			else {
				mpoint.bBitCollect = false;
			}
		}
		if (json.isMember("cfg_role")) {
			mpoint.nPerssion = json["cfg_role"].isInt() ? json["cfg_role"].asInt() : 0;
		}
		if (json.isMember("group_id")) {
			mpoint.nGroupID = json["group_id"].isInt() ? json["group_id"].asInt() : 0;
		}
		if (json.isMember("addr_info")) {
			std::string sAddrString;
			mpoint.sAddrAlias = json["addr_info"].isString() ? json["addr_info"].asString() : "";
			if (CCompileAddress::get_instance().compile(mpoint.sAddrAlias, mpoint.nCommuID, mpoint.addrType, sAddrString) < 0 || 
				stringToRegister(sAddrString, mpoint.addrMonitor) < 0) {
				addProjectParseStat("compile error state : -1", mpoint.nUid, 3, -1);
				continue;;
			}
		}
		if (json.isMember("dead_set")) {
			Convert::BuffToUnionData(json["dead_set"].isString() ? json["dead_set"].asString() : "0", mpoint.nDataFormat, mpoint.nDecimals, mpoint.u4wDeadZoneValue);
		}
		if (json.isMember("float_point")) {
			mpoint.nHighLowWord = json["float_point"].isInt() ? json["float_point"].asInt() : -1;
		}
		if (json.isMember("string_point")) {
			mpoint.nHighLowByte = json["string_point"].isInt() ? json["string_point"].asInt() : -1;
			if (2 == mpoint.nHighLowByte) {
				mpoint.nHighLowByte = HLByte_HighByteBefore;
			}
		}
		if (json.isMember("parent")) {
			mpoint.bProportion = json["parent"].isInt() ? json["parent"].asInt() == 1 : false;
		}
		if (json.isMember("parent_value")) {
			std::string sParentValue = json["parent_value"].isString() ? json["parent_value"].asString() : "";
			if (std::string::npos != sParentValue.find(",")) {
				sscanf(sParentValue.c_str(), "%lf,%lf", &(mpoint.dMutiple), &(mpoint.dOffset));
			}
		}
		if (json.isMember("customConfigList")) {
			for (unsigned int nJ = 0; nJ < json["customConfigList"].size(); nJ++) {
				if (nI == 0) {
					if (json["customConfigList"][nJ].isMember("fieldName")) {
						vecCustomTag.push_back(json["customConfigList"][nJ]["fieldName"].isString() ? json["customConfigList"][nJ]["fieldName"].asString() : "");
					}
				}
				if (json["customConfigList"][nJ].isMember("fieldContent")) {
					mpoint.vecCustomContent.push_back(json["customConfigList"][nJ]["fieldContent"].isString() ? json["customConfigList"][nJ]["fieldContent"].asString() : "");
				}
			}
		}
		if (mpoint.bBitCollect) {
			mpoint.nType = MONITOR_POINT_TYPE_SWITCH;
		}
		else {
			if (mpoint.nDataFormat == _String) {
				mpoint.nType = MONITOR_POINT_TYPE_STRING;
			}
			else {
				mpoint.nType = MONITOR_POINT_TYPE_NUMBER;
			}
		}
		listMPoint.push_back(mpoint);
	}
	return 0;
}

int CCloudParse::loadHMICloud(const std::string &sPath) {
	if (cbl::getFileSize(sPath) <= 0) {
		return -1;
	}
#ifdef WIN32
	tinyxml2::TinyXMLDocument doc;
#else
	tinyxml2::XMLDocument doc;
#endif
	tinyxml2::XMLElement *peleRoot;
	if (cbl::loadXmlFile(UTF8_TO_ANSI(sPath.c_str()), doc, peleRoot) < 0) {
		return -2;
	}
	tinyxml2::XMLElement *pElement = peleRoot->FirstChildElement();
	while (nullptr != pElement) {
		/* parse baseset */
		if (0 == strcmp("BaseSet", pElement->Name())) {
			if (this->loadBaseSet(pElement) < 0) {
				printf("%s:load base set error\r\n", __FUNCTION__);
				return -3;
			}
		}
		/* parse real data */
		if (0 == strcmp("MqttSet", pElement->Name())) {
			this->loadThirdMqttConfig(pElement);
		}
		/* parse alarm data */
		if (0 == strcmp("DataGrpSet", pElement->Name())) {
			this->loadDataGrpSet(pElement);
		}
		pElement = pElement->NextSiblingElement();
	}
	return 0;
}

int CCloudParse::loadBaseSet(tinyxml2::XMLElement *pElement) {
	if (nullptr == pElement) {
		return -1;
	}
	unsigned int nMode = 0;
	bool bUse = false;
	if (nullptr != pElement) {
		if (nullptr != pElement->Attribute("RemotePswd")) {
			m_sRemotePwd = pElement->Attribute("RemotePswd");
		}
		if (nullptr != pElement->Attribute("UploadTo")) {
			nMode = pElement->UnsignedAttribute("UploadTo");
		}
		if (nullptr != pElement->Attribute("Use")) {
			bUse = pElement->IntAttribute("Use") == 1 ? true : false;
		}
		if (nullptr != pElement->Attribute("UpdateTime")) {
			m_nRealPeriod = pElement->IntAttribute("UpdateTime") * 1000;
		}
	}
	if (!bUse) {
		m_bEnableCloud = false;
		m_bEnableThirdCloud = false;
		printf("%s:unable cloud\r\n", __FUNCTION__);
		return -2;
	}
	if (nMode == 1) {
		m_bEnableCloud = false;
	}
	if (nMode == 0) {
		m_bEnableThirdCloud = false;
	}
	if ((1 == nMode ||
		2 == nMode)) {
		setLuaDataNotify([] (std::list<ALARM_DATA_T> &listAlarm) {
			return luapush_alarmData(listAlarm);
		}, [] (MONITOR_GROUP_T &monitorGroup) {
			return luapush_historyData(monitorGroup);
		});
	}
	return 0;
}

int CCloudParse::loadThirdMqttConfig(tinyxml2::XMLElement *pElement) {
	if (nullptr != pElement) {
		if (nullptr != pElement->Attribute("URL")) {
			m_thirdMqttConfig.sURL = pElement->Attribute("URL");
		}
		if (nullptr != pElement->Attribute("PORT")) {
			m_thirdMqttConfig.nPort = pElement->UnsignedAttribute("PORT");
		}
		if (nullptr != pElement->Attribute("ClientID")) {
			m_thirdMqttConfig.sClientId = pElement->Attribute("ClientID");
		}
		if (nullptr != pElement->Attribute("ConnTimeout")) {
			m_thirdMqttConfig.nConnTimeout = pElement->UnsignedAttribute("ConnTimeout");
		}
		if (nullptr != pElement->Attribute("KeepAlive")) {
			m_thirdMqttConfig.nKeepAlive = pElement->UnsignedAttribute("KeepAlive");
		}
		if (nullptr != pElement->Attribute("Cleansession")) {
			m_thirdMqttConfig.nCleanSession = pElement->UnsignedAttribute("Cleansession");
		}
		if (nullptr != pElement->Attribute("Username")) {
			m_thirdMqttConfig.sUsername = pElement->Attribute("Username");
		}
		if (nullptr != pElement->Attribute("Password")) {
			m_thirdMqttConfig.sPassword = pElement->Attribute("Password");
		}
		if (nullptr != pElement->Attribute("Qos")) {
			m_thirdMqttConfig.nQoS = pElement->UnsignedAttribute("Qos");
		}
		if (nullptr != pElement->Attribute("Retained")) {
			m_thirdMqttConfig.nRetained = pElement->UnsignedAttribute("Retained");
		}
		if (nullptr != pElement->Attribute("LastWill")) {
			m_thirdMqttConfig.sLastWill = pElement->Attribute("LastWill");
		}
		if (nullptr != pElement->Attribute("Testament")) {
			m_thirdMqttConfig.sTestament = pElement->Attribute("Testament");
		}
		if (nullptr != pElement->Attribute("IsCrt")) {
			m_thirdMqttConfig.bIsCrt = pElement->UnsignedAttribute("IsCrt") == 1 ? true : false;
		}
		if (nullptr != pElement->Attribute("CrtName")) {
			m_thirdMqttConfig.sCrtName = pElement->Attribute("CrtName");
		}
		pElement = pElement->NextSiblingElement();
	}
	return 0;
}

int CCloudParse::loadDataGrpSet(tinyxml2::XMLElement *pElement) {
	if (nullptr == pElement) {
		return -1;
	}
	if (nullptr != pElement) {
		if (nullptr != pElement->Attribute("CustomName")) {
			std::string sCustomTag = pElement->Attribute("CustomName");
			std::string sSeparator = cbl::format("%c", 0x2);
			std::vector<std::string> vecCustomTag;
			if (split(sCustomTag, sSeparator, &vecCustomTag) > 0 &&
				!vecCustomTag.empty()) {
				CCMPoint::get_instance().setCustomTag(vecCustomTag);
			}
		}
	}
	tinyxml2::XMLElement *pChildElem = pElement->FirstChildElement();
	while (nullptr != pChildElem) {
		if (0 == strcmp("Group", pChildElem->Name())) {
			this->parseGroup(pChildElem, 0);
		}
		else if (0 == strcmp("CloudMerchant", pChildElem->Name())) {
			this->parseGroup(pChildElem, 1);
		}
		pChildElem = pChildElem->NextSiblingElement();
	}
	return 0;
}

int CCloudParse::parseGroup(tinyxml2::XMLElement *pElement, int nType) {
	std::string sTriggerValue;
	COLLECTION_MONITOR_GROUP_DATA groupSet;
	if (nullptr != pElement) {
		if (nullptr != pElement->Attribute("ID")) {
			groupSet.nGroupId = pElement->UnsignedAttribute("ID");
		}
		if (nullptr != pElement->Attribute("Trigger")) {
			groupSet.nTriggerMode = pElement->UnsignedAttribute("Trigger");
		}
		if (nullptr != pElement->Attribute("Timer")) {
			groupSet.nInterval = pElement->IntAttribute("Timer") * 1000;
		}
		if (nullptr != pElement->Attribute("Cond")) {
			groupSet.nTriggerCondF = pElement->UnsignedAttribute("Cond");
		}
		if (nullptr != pElement->Attribute("TriggerValue")) {
			sTriggerValue = pElement->Attribute("TriggerValue");
		}
		if (nullptr != pElement->Attribute("GroupName")) {
			std::vector<std::string> vec;
			std::string str = cbl::format("%c", 0x2);
			split(pElement->Attribute("GroupName"), str, &vec);
			for (unsigned int nI = 0; nI < MAX_MultiLangNum; nI++) {
				if (nI < vec.size()) {
					groupSet.vecGroupName.push_back(vec[nI]);
				}
				else {
					groupSet.vecGroupName.push_back("");
				}
			}
		}
		if (nullptr != pElement->Attribute("DataFormat")) {
			groupSet.nDataFormat = pElement->UnsignedAttribute("DataFormat");
		}
		if (nullptr != pElement->Attribute("TriggAddr")) {
			stringToRegister(pElement->Attribute("TriggAddr"), groupSet.addrTrigger);
		}
	}
	Convert::BuffToUnionData(sTriggerValue, groupSet.nDataFormat, 0, groupSet.u4wTriValueF);
	if (1 == nType && (!(MONITOR_POINT_MODE_TIMESAMPLE == groupSet.nTriggerMode
		|| MONITOR_POINT_MODE_TRIGGER_TIMESAMPLE == groupSet.nTriggerMode))) {
		groupSet.nInterval = -1;
	}
	/* add group */
	if (0 == nType) {
		CWPoint::get_instance().addGroup(groupSet);
	}
	else if (1 == nType) {
		CCMPoint::get_instance().addGroup(groupSet);
	}
	tinyxml2::XMLElement *pChildElem = pElement->FirstChildElement();
	while (nullptr != pChildElem) {
		if (0 == strcmp("Data", pChildElem->Name())) {
			this->parsePoint(pChildElem, nType, groupSet.nGroupId);
		}
		pChildElem = pChildElem->NextSiblingElement();
	}
	return 0;
}

int CCloudParse::parsePoint(tinyxml2::XMLElement *pElement, int nType, unsigned int nGroupId) {
	if (nullptr == pElement) {
		return -1;
	}
	COLLECTION_MONITOR_POINT_DATA pointSet;
	if (nullptr != pElement->Attribute("ID")) {
		pointSet.nUid = pElement->UnsignedAttribute("ID") + nGroupId * 1000;
	}
	if (nullptr != pElement->Attribute("DataFormat")) {
		pointSet.nDataFormat = pElement->UnsignedAttribute("DataFormat");
	}
	if (nullptr != pElement->Attribute("DataLimit")) {
		std::string sLimit = pElement->Attribute("DataLimit");
		if (std::string::npos != sLimit.find(",")) {
			std::vector<std::string> vec;
			split(sLimit, ",", &vec);
			if (vec.size() >= 2) {
				pointSet.sMinRange = vec[0];
				pointSet.sMaxRange = vec[1];
			}
		}
	}
	if (nullptr != pElement->Attribute("ByteLen")) {
		pointSet.nDataLen = pElement->UnsignedAttribute("ByteLen");
	}
	if (nullptr != pElement->Attribute("DigitCount")) {
		std::string sDigitCount = pElement->Attribute("DigitCount");
		if (std::string::npos != sDigitCount.find(".")) {
			sscanf(sDigitCount.c_str(), "%d.%d", &(pointSet.nInteger), &(pointSet.nDecimals));
		}
	}
	if (nullptr != pElement->Attribute("IsBitCollect")) {
		pointSet.bBitCollect = pElement->UnsignedAttribute("IsBitCollect") == 1 ? true : false;
	}
	if (nullptr != pElement->Attribute("RW")) {
		pointSet.nPerssion = pElement->UnsignedAttribute("RW");
	}
	if (nullptr != pElement->Attribute("Name")) {
		std::string sName = pElement->Attribute("Name");
		std::vector<std::string> vec;
		string str = format("%c", 0x2);
		split(sName, str, &vec);
		for (unsigned int nI = 0; nI < MAX_MultiLangNum; nI++) {
			if (nI < vec.size()) {
				pointSet.vecName.push_back(vec[nI]);
			}
			else {
				pointSet.vecName.push_back("");
			}
		}
	}
	if (nullptr != pElement->Attribute("CustomValue")) {
		std::string sSeparator = format("%c", 0x2);
		std::string sbuffer = pElement->Attribute("CustomValue");
		split(sbuffer, sSeparator, &pointSet.vecCustomContent);
	}
	if (nullptr != pElement->Attribute("WordAddr")) {
		std::string sbuffer = pElement->Attribute("WordAddr");
		stringToRegister(sbuffer, pointSet.addrMonitor);
	}
	if (nullptr != pElement->Attribute("WordAddrAlias")) {
		pointSet.sAddrAlias = pElement->Attribute("WordAddrAlias");
	}
	if (nullptr != pElement->Attribute("SaveTraffic")) {
		pointSet.bSaveTraffic = pElement->UnsignedAttribute("SaveTraffic") == 1 ? true : false;
	}
	/* set commu id */
	if (isLocalRegister(pointSet.addrMonitor)) {
		pointSet.nCommuID = 0;
	}
	else {
		pointSet.nCommuID = pointSet.addrMonitor.getConnectNum();
	}
	/* set type */
	if (pointSet.bBitCollect) {
		pointSet.nType = MONITOR_POINT_TYPE_SWITCH;
	}
	else {
		if (pointSet.nDataFormat == _String) {
			pointSet.nType = MONITOR_POINT_TYPE_STRING;
		}
		else {
			pointSet.nType = MONITOR_POINT_TYPE_NUMBER;
		}
	}
	/* add point */
	if (0 == nType) {
		CWPoint::get_instance().addPoint(nGroupId, pointSet);
	}
	else if (1 == nType) {
		CCMPoint::get_instance().addPoint(nGroupId, pointSet);
	}
	return 0;
}

#ifdef WIN32
int CCloudParse::saveCommu(tinyxml2::TinyXMLDocument &doc, tinyxml2::XMLElement *pElement)
#else
int CCloudParse::saveCommu(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *pElement)
#endif 
{
	if (nullptr == pElement) {
		return -1;
	}
	std::unique_lock<std::mutex> lock(m_mutexCommu);
	for (auto &iter : m_listCommu) {
		tinyxml2::XMLElement *pConnect = doc.NewElement("Connection");
		if (nullptr == pConnect) {
			return -2;
		}
		pConnect->SetAttribute("ComID", iter.nCommuId);
		tinyxml2::XMLElement *pPlc = doc.NewElement("PLC");
		if (nullptr == pPlc) {
			return -3;
		}
		pPlc->SetAttribute("Type", iter.PlcSet.szPLCType);
		pPlc->SetAttribute("Driver", iter.PlcSet.szLinkProtocol);
		pPlc->SetAttribute("HmiStatNo", iter.PlcSet.nHmiMachineNo);
		pPlc->SetAttribute("PlcStatNo", iter.PlcSet.nPlcMachineNo);
		pConnect->InsertEndChild(pPlc);
		tinyxml2::XMLElement *pCommSet = doc.NewElement("CommSet");
		if (nullptr == pCommSet) {
			return -4;
		}
		pCommSet->SetAttribute("Port", iter.Commset.szComSelect);
		pCommSet->SetAttribute("ComType", iter.Commset.nCommMode);
		pCommSet->SetAttribute("BaudRate", iter.Commset.nBaudRate);
		pCommSet->SetAttribute("StopBit", iter.Commset.nStopBits);
		pCommSet->SetAttribute("DataLength", iter.Commset.nDataLength);
		pCommSet->SetAttribute("CheckBit", iter.Commset.szCheckBit);
		pCommSet->SetAttribute("RetryTimes", iter.Commset.nRetryTimes);
		pCommSet->SetAttribute("WaitTimeout", iter.Commset.nWaitTimeout);
		pCommSet->SetAttribute("RevTimeout", iter.Commset.nRecvTimeout);
		pCommSet->SetAttribute("RetryTimeOut", iter.Commset.nRetryTimeOut);
		pCommSet->SetAttribute("ComIoDelayTime", iter.Commset.nComIoDelayTime);
		pCommSet->SetAttribute("ComSingleLen", iter.Commset.nComSingleReadLen);
		pCommSet->SetAttribute("ComStepInterval", iter.Commset.nComStepInterval);
		pCommSet->SetAttribute("NETIPaddr", iter.Commset.szDstIPV4);
		pCommSet->SetAttribute("NETPort", iter.Commset.iDstPort);
		pCommSet->SetAttribute("NETBroadcastAddr", iter.Commset.wBroadcastAddr);
		pCommSet->SetAttribute("NETIsBroadcast", iter.Commset.bEthernet_IsBroadcast);
		pCommSet->SetAttribute("NETIsUDP", iter.Commset.nEthernet_IsUDP);
		pConnect->InsertEndChild(pCommSet);
		tinyxml2::XMLElement *pFrameWork = doc.NewElement("FrameWorkSet");
		if (nullptr == pFrameWork) {
			return -5;
		}
		pFrameWork->SetAttribute("FrameWork", iter.nFramework);
		pConnect->InsertEndChild(pFrameWork);

		pElement->InsertEndChild(pConnect);
	}
	return 0;
}

#ifdef WIN32
int CCloudParse::saveReal(tinyxml2::TinyXMLDocument &doc, tinyxml2::XMLElement *pElement)
#else
int CCloudParse::saveReal(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *pElement)
#endif
{
	if (nullptr == pElement) {
		return -1;
	}
	std::list<COLLECTION_MONITOR_POINT_DATA> listRealData;
	if (CWPoint::get_instance().getPointConfigList(listRealData) < 0) {
		return -2;
	}
	for (auto &iter : listRealData) {
		tinyxml2::XMLElement *pReal = doc.NewElement("RelNode");
		if (nullptr == pReal) {
			return -3;
		}
		std::string sDeadZoneValue;
		Convert::Union2String(iter.nDataFormat, iter.nDecimals, iter.u4wDeadZoneValue, sDeadZoneValue);
		pReal->SetAttribute("DeadSet", sDeadZoneValue.c_str());
		pReal->SetAttribute("ID", iter.nUid);
		pReal->SetAttribute("ComID", iter.nCommuID);
		pReal->SetAttribute("AddrType", cbl::format("%c", iter.addrType).c_str());
		pReal->SetAttribute("DispType", iter.nDataFormat);
		pReal->SetAttribute("DigitCount", cbl::format("%d %d", iter.nInteger, iter.nDecimals).c_str());
		pReal->SetAttribute("FloatPoint", iter.nHighLowWord);
		pReal->SetAttribute("StringPoint", iter.nHighLowByte);
		pReal->SetAttribute("TrafficStatus", (int)iter.bSaveTraffic);
		pReal->SetAttribute("Addr", iter.sAddrAlias.c_str());
		pReal->SetAttribute("RoleType", iter.nPerssion + 1);
		pReal->SetAttribute("Keep_Report", (int)iter.bMap);
		if (iter.vecName.empty()) {
			pReal->SetAttribute("Name", "");
		}
		else {
			pReal->SetAttribute("Name", iter.vecName[0].c_str());
		}
		pElement->InsertEndChild(pReal);
	}
	return 0;
}

#ifdef WIN32
int CCloudParse::saveHistory(tinyxml2::TinyXMLDocument &doc, tinyxml2::XMLElement *pElement)
#else
int CCloudParse::saveHistory(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *pElement)
#endif
{
	if (nullptr == pElement) {
		return -1;
	}
	/* check node */
	tinyxml2::XMLElement *pGroup = pElement->FirstChildElement("Group");
	if (nullptr == pGroup) {
		pGroup = doc.NewElement("Group");
		if (nullptr == pGroup) {
			return -2;
		}
		pElement->InsertFirstChild(pGroup);
	}
	std::list<COLLECTION_HISTORY_DATA> listHistoryData;
	if (0 > CDataRecordCollect::get_instance().getGroupConfigList(listHistoryData)) {
		return -3;
	}
	for (auto &iter : listHistoryData) {
		tinyxml2::XMLElement *pHistoryGroup = doc.NewElement("HisGroup");
		if (nullptr == pHistoryGroup) {
			return -4;
		}
		pHistoryGroup->SetAttribute("ID", iter.nGroupId);
		pHistoryGroup->SetAttribute("Cycle", iter.nInterval * 1000);
		pHistoryGroup->SetAttribute("TriggerType", iter.nType);
		pHistoryGroup->SetAttribute("TriggerAddrType", cbl::format("%c", iter.trigerAddrType).c_str());
		pHistoryGroup->SetAttribute("TriggerComID", iter.nTrigCommuID);
		pHistoryGroup->SetAttribute("TriggerAddr", iter.sTriggerAddrAlias.c_str());
		pHistoryGroup->SetAttribute("TriggerValue", (int)iter.bTriggerValue);
		pGroup->InsertEndChild(pHistoryGroup);
		for (auto &iterP : iter.listField) {
			tinyxml2::XMLElement *pHistoryPoint = doc.NewElement("HisNode");
			if (nullptr == pHistoryPoint) {
				return -5;
			}
			pHistoryPoint->SetAttribute("ID", iterP.nId);
			pHistoryPoint->SetAttribute("ComID", iterP.nCommuID);
			pHistoryPoint->SetAttribute("AddrType", cbl::format("%c", iterP.fieldAddrType).c_str());
			pHistoryPoint->SetAttribute("DispType", iterP.nDataFormat);
			pHistoryPoint->SetAttribute("DigitCount", cbl::format("%d %d", iterP.nInteger, iterP.nDecimals).c_str());
			pHistoryPoint->SetAttribute("FloatPoint", iterP.nHighLowWord);
			pHistoryPoint->SetAttribute("StringPoint", iterP.nHighLowByte);
			pHistoryPoint->SetAttribute("Addr", iterP.sFieldAddrAlias.c_str());
			pHistoryPoint->SetAttribute("GroupID", iterP.nGroupId);
			if (iterP.vecFieldName.empty()) {
				pHistoryPoint->SetAttribute("Name", "");
			}
			else {
				pHistoryPoint->SetAttribute("Name", iterP.vecFieldName[0].c_str());
			}
			pElement->InsertEndChild(pHistoryPoint);
		}
	}
	return 0;
}

#ifdef WIN32
int CCloudParse::saveAlarm(tinyxml2::TinyXMLDocument &doc, tinyxml2::XMLElement *pElement)
#else
int CCloudParse::saveAlarm(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *pElement)
#endif
{
	if (nullptr == pElement) {
		return -1;
	}
	std::list<COLLECTION_ALARM_DATA> listAlarmData;
	if (0 > CAlarmCollect::get_instance().getConfigList(listAlarmData)) {
		return -2;
	}
	/* add node */
	for (auto &iter : listAlarmData) {
		tinyxml2::XMLElement *pAlmNode = doc.NewElement("AlmNode");
		if (nullptr == pAlmNode) {
			return -3;
		}
		pAlmNode->SetAttribute("ID", iter.nUid);
		pAlmNode->SetAttribute("Name", iter.sName.c_str());
		pAlmNode->SetAttribute("ComID", iter.nMonitorCommuID);
		pAlmNode->SetAttribute("AddrType", cbl::format("%c", iter.monitorAddrType).c_str());
		pAlmNode->SetAttribute("DispType", iter.nDataFormat);
		pAlmNode->SetAttribute("DigitCount", cbl::format("%d %d", iter.nInteger, iter.nDecimals).c_str());
		pAlmNode->SetAttribute("CondType", iter.conditionType);
		pAlmNode->SetAttribute("FloatPoint", iter.nHighLowWord);
		pAlmNode->SetAttribute("StringPoint", iter.nHighLowByte);
		pAlmNode->SetAttribute("Addr", iter.sAddressAlias.c_str());
		tinyxml2::XMLElement *pCondition = doc.NewElement("Condition");
		if (nullptr == pCondition) {
			return -4;
		}
		if (ARITHMETIC_TYPE_SINGLE == iter.conditionType) {
			if (ALARM_TYPE_BIT == iter.type) {
				pCondition->SetAttribute("AlValue", iter.wTriggerValue);
				if (0 == iter.wTriggerValue) {
					pCondition->SetAttribute("TrigType", 7);
				}
				else {
					pCondition->SetAttribute("TrigType", 6);
				}
			}
			else if (ALARM_TYPE_WORD == iter.type) {
				std::string sValue;
				if (_String == iter.nDataFormat) {
					sValue = iter.sCondition;
				}
				else {
					Convert::Union2String(iter.nDataFormat, iter.nDecimals, iter.u4wRangeEnd, sValue);
				}
				pCondition->SetAttribute("AlValue", sValue.c_str());
				pCondition->SetAttribute("TrigType", toAlarmType(iter.mode));
			}
			pAlmNode->InsertEndChild(pCondition);
		}
		else {
			std::string sValueStart = "", sValueEnd = "";
			if (iter.nDataFormat == _String) {
				sValueStart = iter.sCondition;
				sValueEnd = iter.sConditionEnd;
			}
			else {
				Convert::Union2String(iter.nDataFormat, iter.nDecimals, iter.u4wRangeStart, sValueStart);
				Convert::Union2String(iter.nDataFormat, iter.nDecimals, iter.u4wRangeEnd, sValueEnd);
			}
			pCondition->SetAttribute("AlValue", sValueStart.c_str());
			pCondition->SetAttribute("TrigType", toAlarmType(iter.modeStart));
			pAlmNode->InsertEndChild(pCondition);
			pCondition = doc.NewElement("Condition");
			if (nullptr == pCondition) {
				return -6;
			}
			pCondition->SetAttribute("AlValue", sValueEnd.c_str());
			pCondition->SetAttribute("TrigType", toAlarmType(iter.modeEnd));
			pAlmNode->InsertEndChild(pCondition);
		}
		pElement->InsertEndChild(pAlmNode);
	}
	return 0;
}

int CCloudParse::save() {
	if (eCLOUDMODE_TEMPLATE != m_eMode) {
		return -1;
	}
	std::string sTemplatePrj = getPrjCfgDir() + std::string("template/json.vbox");
	if (UTILITY::CFile::ExistFile(sTemplatePrj)) {
		return 0;
	}
	std::string sPath = getPrjDir() + std::string("test.pi");
#ifdef WIN32
	tinyxml2::TinyXMLDocument doc;
#else
	tinyxml2::XMLDocument doc;
#endif
	std::string sProject = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<PIBox filever=\"1.0\">\n<SysSet></SysSet>\n<PlcSet >\n</PlcSet>\n</PIBox>";
	if (0 != doc.Parse(sProject.c_str())) {
		return -2;
	}
	tinyxml2::XMLElement *pElement = doc.RootElement();
	if (nullptr == pElement) {
		return -3;
	}
	/* check node */
	tinyxml2::XMLElement *pPlcSet = pElement->FirstChildElement("PlcSet");
	if (nullptr == pPlcSet) {
		pPlcSet = doc.NewElement("PlcSet");
		if (nullptr == pPlcSet) {
			return -4;
		}
		pElement->InsertEndChild(pPlcSet);
	}
	if (0 > this->saveCommu(doc, pPlcSet)) {
		printf("%s:save Commu error\r\n", __FUNCTION__);
		return -5;
	}
	/* check node */
	tinyxml2::XMLElement *pReal = pElement->FirstChildElement("RelData");
	if (nullptr == pReal) {
		pReal = doc.NewElement("RelData");
		if (nullptr == pReal) {
			return -6;
		}
		pReal->SetAttribute("Cycle", REALCYCLE);
		pElement->InsertEndChild(pReal);
	}
	if (0 > this->saveReal(doc, pReal)) {
		printf("%s:save real error\r\n", __FUNCTION__);
		return -7;
	}
	tinyxml2::XMLElement *pHistory = pElement->FirstChildElement("HisData");
	if (nullptr == pHistory) {
		pHistory = doc.NewElement("HisData");
		if (nullptr == pHistory) {
			return -8;
		}
		pElement->InsertEndChild(pHistory);
	}
	if (0 > this->saveHistory(doc, pHistory)) {
		printf("%s:save history error\r\n", __FUNCTION__);
		return -9;
	}
	/* check node */
	tinyxml2::XMLElement *pAlarmData = pElement->FirstChildElement("AlarmData");
	if (nullptr == pAlarmData) {
		pAlarmData = doc.NewElement("AlarmData");
		if (nullptr == pAlarmData) {
			return -10;
		}
		pElement->InsertEndChild(pAlarmData);
	}
	if (0 > this->saveAlarm(doc, pAlarmData)) {
		printf("%s:save history error\r\n", __FUNCTION__);
		return -11;
	}
	XMLPrinter printer;
	doc.Print(&printer);
#ifdef WIN32
	int nRtn = cbl::saveToFile(printer.CStr(), sPath);
#else
	int nRtn = cbl::syncSaveToFile(printer.CStr(), sPath);
#endif
	if (nRtn < 0) {
		return -12;
	}
	return 0;
}

int CCloudParse::loadlua(const Json::Value &root, std::list<LUASCRIPT_T> &scripts) {
	if (!root["data"].empty() &&
		!root["data"]["scriptConfigList"].empty()) {
		for (unsigned int nI = 0; nI < root["data"]["scriptConfigList"].size(); nI++) {
			tagLUA_INFOMATION_T rsp;
			rsp.nStat = LUARESULT_NORMAL;
			LUASCRIPT_T script;
			std::string sContent;
			std::string sError;
			if (root["data"]["scriptConfigList"][nI]["scriptId"].isUInt()) {
				script.nId = root["data"]["scriptConfigList"][nI]["scriptId"].asUInt();
			}
			if (root["data"]["scriptConfigList"][nI]["script_type"].isUInt()) {
				script.nType = root["data"]["scriptConfigList"][nI]["script_type"].asUInt();
			}
			if (root["data"]["scriptConfigList"][nI]["cycle"].isUInt()) {
				script.nCycle = root["data"]["scriptConfigList"][nI]["cycle"].asUInt();
			}
			if (root["data"]["scriptConfigList"][nI]["script_name"].isString()) {
				script.sName = root["data"]["scriptConfigList"][nI]["script_name"].asString();
			}
			if (root["data"]["scriptConfigList"][nI]["save_script"].isString()) {
				sContent = root["data"]["scriptConfigList"][nI]["save_script"].asString();
			}
			if (CCompileAddress::get_instance().compileLua(sContent, script.sContent, sError) < 0) {
				printf("%s:compileLua error\r\n", __FUNCTION__);
				rsp.nStat = LUARESULT_COMFAILED;
			}
			if (!script.sName.empty() &&
				!script.sContent.empty()) {
				rsp.nId = script.nId;
				scripts.push_back(script);
				m_luarsps.push_back(rsp);
			}
		}
	}
	if (!root["data"].empty() &&
		!root["data"]["upd_lua_script"].empty()) {
		for (unsigned int nI = 0; nI < root["data"]["upd_lua_script"].size(); nI++) {
			tagLUA_INFOMATION_T rsp;
			rsp.nStat = LUARESULT_NORMAL;
			LUASCRIPT_T script;
			std::string sContent;
			std::string sError;
			if (root["data"]["upd_lua_script"][nI]["strategy_id"].isUInt()) {
				script.nId = root["data"]["upd_lua_script"][nI]["strategy_id"].asUInt();
			}
			if (root["data"]["upd_lua_script"][nI]["strategy_type"].isUInt()) {
				script.nType = root["data"]["upd_lua_script"][nI]["strategy_type"].asUInt();
			}
			if (root["data"]["upd_lua_script"][nI]["cycle"].isUInt()) {
				script.nCycle = root["data"]["upd_lua_script"][nI]["cycle"].asUInt();
			}
			if (root["data"]["upd_lua_script"][nI]["strategy_name"].isString()) {
				script.sName = root["data"]["upd_lua_script"][nI]["strategy_name"].asString();
			}
			if (root["data"]["upd_lua_script"][nI]["save_script"].isString()) {
				sContent = root["data"]["upd_lua_script"][nI]["save_script"].asString();
			}
			if (CCompileAddress::get_instance().compileLua(sContent, script.sContent, sError) < 0) {
				printf("%s:compileLua error\r\n", __FUNCTION__);
				rsp.nStat = LUARESULT_COMFAILED;
			}
			if (!script.sName.empty() &&
				!script.sContent.empty()) {
				rsp.nId = script.nId;
				scripts.push_back(script);
				m_luarsps.push_back(rsp);
			}
		}
	}
	if (!root["data"].empty() &&
		!root["data"]["mqttConfig"].empty()) {
		if (root["data"]["mqttConfig"]["address"].isString()) {
			m_thirdMqttConfig.sURL = root["data"]["mqttConfig"]["address"].asString();
		}
		if (root["data"]["mqttConfig"]["port"].isUInt()) {
			m_thirdMqttConfig.nPort = root["data"]["mqttConfig"]["port"].asUInt();
		}
		if (root["data"]["mqttConfig"]["clientID"].isString()) {
			m_thirdMqttConfig.sClientId = root["data"]["mqttConfig"]["clientID"].asString();
		}
		if (root["data"]["mqttConfig"]["keepAlive"].isUInt()) {
			m_thirdMqttConfig.nKeepAlive = root["data"]["mqttConfig"]["keepAlive"].asUInt();
		}
		if (root["data"]["mqttConfig"]["cleanSession"].isUInt()) {
			m_thirdMqttConfig.nCleanSession = root["data"]["mqttConfig"]["cleanSession"].asUInt();
		}
		if (root["data"]["mqttConfig"]["userName"].isString()) {
			m_thirdMqttConfig.sUsername = root["data"]["mqttConfig"]["userName"].asString();
		}
		if (root["data"]["mqttConfig"]["passWord"].isString()) {
			m_thirdMqttConfig.sPassword = root["data"]["mqttConfig"]["passWord"].asString();
		}
		if (!root["data"]["mqttConfig"]["cart"].empty()) {
			if (root["data"]["mqttConfig"]["cart"]["enable"].isUInt()) {
				m_thirdMqttConfig.bIsCrt = root["data"]["mqttConfig"]["cart"]["enable"].asUInt() == 1 ? true : false;
			}
			if (root["data"]["mqttConfig"]["cart"]["caFileName"].isString()) {
				m_thirdMqttConfig.sCrtName = getPrjCfgDir() + std::string("cloudprj/") + root["data"]["mqttConfig"]["cart"]["caFileName"].asString();
			}
			if (root["data"]["mqttConfig"]["cart"]["clientPem"].isString()) {
				m_thirdMqttConfig.sClientPemName = getPrjCfgDir() + std::string("cloudprj/") + root["data"]["mqttConfig"]["cart"]["clientPem"].asString();
			}
			if (root["data"]["mqttConfig"]["cart"]["clientKey"].isString()) {
				m_thirdMqttConfig.sClientKeyName = getPrjCfgDir() + std::string("cloudprj/") + root["data"]["mqttConfig"]["cart"]["clientKey"].asString();
			}
			if (root["data"]["mqttConfig"]["cart"]["clientKeyPassword"].isString()) {
				m_thirdMqttConfig.sClientKeyPassword = root["data"]["mqttConfig"]["cart"]["clientKeyPassword"].asString();
			}
		}
		if (!root["data"]["mqttConfig"]["lwt"].empty()) {
			if (root["data"]["mqttConfig"]["lwt"]["retain"].isUInt()) {
				m_thirdMqttConfig.nRetained = root["data"]["mqttConfig"]["lwt"]["retain"].asUInt();
			}
			if (root["data"]["mqttConfig"]["lwt"]["qos"].isUInt()) {
				m_thirdMqttConfig.nQoS = root["data"]["mqttConfig"]["lwt"]["qos"].asUInt();
			}
			if (root["data"]["mqttConfig"]["lwt"]["topic"].isString()) {
				m_thirdMqttConfig.sLastWill = root["data"]["mqttConfig"]["lwt"]["topic"].asString();
			}
			if (root["data"]["mqttConfig"]["lwt"]["message"].isString()) {
				m_thirdMqttConfig.sTestament = root["data"]["mqttConfig"]["lwt"]["message"].asString();
			}
		}
	}
	if (!root["data"].empty() &&
		!root["data"]["customConfigList"].empty()) {
		for (unsigned int nI = 0; nI < root["data"]["customConfigList"].size(); nI++) {
			if (root["data"]["customConfigList"][nI]["fieldName"].isString() &&
				root["data"]["customConfigList"][nI]["fieldContent"].isString()) {
				CUSTOMCONFIG configC;
				configC.sKey = root["data"]["customConfigList"][nI]["fieldName"].asString();
				configC.sValue = root["data"]["customConfigList"][nI]["fieldContent"].asString();
				m_CloudUserConfig.push_back(configC);
			}
		}
	}
	if (!root["data"].empty() &&
		!root["data"]["deviceCustomList"].empty()) {
		for (unsigned int nI = 0; nI < root["data"]["deviceCustomList"].size(); nI++) {
			if (root["data"]["deviceCustomList"][nI]["fieldName"].isString() &&
				root["data"]["deviceCustomList"][nI]["fieldContent"].isString()) {
				CUSTOMCONFIG configC;
				configC.sKey = root["data"]["deviceCustomList"][nI]["fieldName"].asString();
				configC.sValue = root["data"]["deviceCustomList"][nI]["fieldContent"].asString();
				m_CloudBindConfig.push_back(configC);
			}
		}
	}
	return 0;
}

const LUAMQTTCONFIG& CCloudParse::getMQTTConfig() {
	return m_thirdMqttConfig;
}

const std::list<CUSTOMCONFIG>& CCloudParse::getCloudBindConfig() {
	return m_CloudBindConfig;
}

const std::list<CUSTOMCONFIG>& CCloudParse::getCloudUserConfig() {
	return m_CloudUserConfig;
}

int CCloudParse::start() {
	CLOUDCONFIG_T cloudConfig;
	WMachine wMachine;
	if (m_bEnableThirdCloud) {
		if (CCMPoint::get_instance().start() < 0) {
			printf("%s:CCMPoint start error\r\n", __FUNCTION__);
			return -1;
		}
		if (setLuaDataNotify(luapush_alarmData, luapush_historyData) < 0) {
			printf("%s:set lua notify error\r\n", __FUNCTION__);
			return -2;
		}
	}
	if (m_bEnableHistory) {
		if (CDataRecordCollect::get_instance().setReportStatus(true) < 0) {
			printf("%s:history set report error\r\n", __FUNCTION__);
			return -3;
		}
	}
	if (m_bEnableReal) {
		if (CWPoint::get_instance().setMapPointPeriod(m_nRealMapPeriod) < 0) {
			printf("%s:real set map period error\r\n", __FUNCTION__);
			return -4;
		}
		if (CWPoint::get_instance().setSaveTrafficPeriod(m_nRealPeriod) < 0) {
			printf("%s:real set save period error\r\n", __FUNCTION__);
			return -5;
		}
		if (CWPoint::get_instance().start() < 0) {
			printf("%s:real start error\r\n", __FUNCTION__);
			return -6;
		}
	}
	if (m_bEnableAlarm) {
		if (CAlarmCollect::get_instance().setReportStatus(true) < 0) {
			printf("%s:alarm start error\r\n", __FUNCTION__);
			return -7;
		}
	}
	if (!m_bEnableCloud) {
		return -8;
	}
	cloudConfig.sPassword = m_sRemotePwd;
	cloudConfig.eMode = m_eMode;
#ifdef WIN32
	cloudConfig.bEnableScada = true;
#else
	if (wMachine.enableCloud() ||
		PLATFORM_NAMESPACE::CPlatBoard::getDeviceRealMode() == PLATFORM_NAMESPACE::VBOX_MODE) {
		cloudConfig.bEnableScada = true;
	}
	else {
		cloudConfig.bEnableScada = false;
	}
#endif
	cloudConfig.sProjectMd5 = m_sProjectMd5;
	return CCloudOperator::get_instance().config(cloudConfig);
}

int CCloudParse::load(bool bInit) {
	if (this->loadProject(bInit) < 0) {
		m_bEnableCloud = m_bEnableThirdCloud = false;
		return -1;
	}
	return 0;
}

int CCloudParse::setCommu(const std::list<CONNECTNODE> &listCommu) {
	std::unique_lock<std::mutex> lock(m_mutexCommu);
	m_listCommu = listCommu;
	return 0;
}

std::list<CONNECTNODE> CCloudParse::getCommu() {
	std::unique_lock<std::mutex> lock(m_mutexCommu);
	return m_listCommu;
}

int CCloudParse::unload() {
	if (m_eMode != eCLOUDMODE_TEMPLATE &&
		m_eMode != eCLOUDMODE_CLOUD) {
		return 0;
	}
	m_listProjectPrase.clear();
	m_thirdMqttConfig.reset();
	m_CloudBindConfig.clear();
	m_CloudUserConfig.clear();
	m_luarsps.clear();
	if (m_luaPrase.clearDb() < 0) {
		printf("%s:clear db error\r\n", __FUNCTION__);
		return -1;
	}
	if (m_eMode == eCLOUDMODE_TEMPLATE) {
		if (CAlarmCollect::get_instance().clear() < 0) {
			printf("%s:alarm clear error\r\n", __FUNCTION__);
			return -2;
		}
		if (CDataRecordCollect::get_instance().clear() < 0) {
			printf("%s:history clear error\r\n", __FUNCTION__);
			return -3;
		}
		if (CWPoint::get_instance().clear() < 0) {
			printf("%s:real clear error\r\n", __FUNCTION__);
			return -4;
		}

		COLLECTION_MONITOR_GROUP_DATA monitorGroup;
		monitorGroup.vecGroupName.assign({ "group", "group", "group", "group" , "group" , "group" , "group" , "group" });
		monitorGroup.nInterval = 200;
		monitorGroup.nGroupId = 0;
		if (CWPoint::get_instance().addGroup(monitorGroup, false) < 0) {
			printf("%s:real add error\r\n", __FUNCTION__);
			return -5;
		}
	}
	else {
		if (CCMPoint::get_instance().clear() < 0) {
			printf("%s:CCMPoint clear error\r\n", __FUNCTION__);
			return -6;
		}
	}
	{
		std::unique_lock<std::mutex> lock(m_mutexCommu);
		for (auto &iter : m_listCommu) {
			this->unloadCommu(iter);
		}
		m_listCommu.clear();
	}
	return 0;
}

int CCloudParse::updateGlobalConfig(const std::list<GLOBAL_CONFIG_T> &listGlobalConfig) {
	if (listGlobalConfig.empty()) {
		return 0;
	}
	for (auto &iter : listGlobalConfig) {
		switch (iter.nConfigType)
		{
		case GLOBAL_CONFIG_UPLOAD_SWITCH: 
			{
				switch (iter.nType) 
				{
				case MONITOR_DATA_TYPE_REALDATA: 
					{
						if (m_bEnableReal == iter.isOpened) {
							break;
						}
						m_bEnableReal = iter.isOpened;
						if (m_bEnableReal) {
							if (CWPoint::get_instance().start() < 0) {
								printf("%s: cwpoint start error\r\n", __FUNCTION__);
								return -1;
							}
						}
						else {
							if (CWPoint::get_instance().stop() < 0) {
								printf("%s: cwpoint stop error\r\n", __FUNCTION__);
								return -2;
							}
						}
					}
					break;
				case MONITOR_DATA_TYPE_HISTORYDATA: 
					{
						if (m_bEnableHistory == iter.isOpened) {
							break;
						}
						m_bEnableHistory = iter.isOpened;
						if (CDataRecordCollect::get_instance().setReportStatus(m_bEnableHistory) < 0) {
							printf("%s:set history report stat error\r\n", __FUNCTION__);
							return -3;
						}
					}
					break;
				case MONITOR_DATA_TYPE_ALARMDATA: 
					{
						if (m_bEnableAlarm == iter.isOpened) {
							break;
						}
						m_bEnableAlarm = iter.isOpened;
						if (CAlarmCollect::get_instance().setReportStatus(m_bEnableAlarm) < 0) {
							printf("%s:set alarm report stat error\r\n", __FUNCTION__);
							return -4;
						}
					}
					break;
				default: 
					break;
				}
			}
			break;
		case GLOBAL_CONFIG_SAVE_TRAFFIC_PERIOD: 
			{
				if (m_nRealPeriod == (int)iter.ullPeriod) {
					break;
				}
				m_nRealPeriod = (int)iter.ullPeriod;
				if (CWPoint::get_instance().setSaveTrafficPeriod(iter.ullPeriod) < 0) {
					printf("%s:set real period error\r\n", __FUNCTION__);
					return -5;
				}
			}
			break;
		case GLOBAL_CONFIG_DATA_MAPPING_PERIOD: 
			{
				if (m_nRealMapPeriod == (int)iter.ullPeriod) {
					break;
				}
				m_nRealMapPeriod = (int)iter.ullPeriod;
				if (CWPoint::get_instance().setMapPointPeriod(iter.ullPeriod) < 0) {
					printf("%s:set real map period error\r\n", __FUNCTION__);
					return -6;
				}
			}
			break;
		default:
			break;
		}
	}
	std::string sbuffer = "[switch]\n";
	sbuffer += cbl::format("reldata=%d\n", m_bEnableReal ? 1 : 0);
	sbuffer += cbl::format("hisdata=%d\n", m_bEnableHistory ? 1 : 0);
	sbuffer += cbl::format("almdata=%d\n", m_bEnableAlarm ? 1 : 0);
	sbuffer += cbl::format("coltime=%d\n", m_nRealPeriod);
	sbuffer += cbl::format("colmaptime=%d\n", m_nRealMapPeriod);
	std::string sPath = getPrjDir() + std::string("config.ini");
#ifdef WIN32
	int nRtn = cbl::saveToFile(sbuffer, sPath);
#else
	int nRtn = cbl::syncSaveToFile(sbuffer, sPath);
#endif
	if (nRtn < 0) {
		return -7;
	}
	return 0;
}

int CCloudParse::restart() {
	if (this->load(false) < 0) {
		printf("%s:load project error\r\n", __FUNCTION__);
		return -1;
	}
	if (m_bEnableThirdCloud) {
		if (CCMPoint::get_instance().start() < 0) {
			printf("%s:CCMPoint start error\r\n", __FUNCTION__);
			return -2;
		}
	}
	if (m_bEnableHistory) {
		if (CDataRecordCollect::get_instance().setReportStatus(true) < 0) {
			printf("%s:history set report error\r\n", __FUNCTION__);
			return -3;
		}
	}
	if (m_bEnableReal) {
		if (CWPoint::get_instance().setMapPointPeriod(m_nRealMapPeriod) < 0) {
			printf("%s:real set map period error\r\n", __FUNCTION__);
			return -4;
		}
		if (CWPoint::get_instance().setSaveTrafficPeriod(m_nRealPeriod) < 0) {
			printf("%s:real set save period error\r\n", __FUNCTION__);
			return -5;
		}
		if (CWPoint::get_instance().start() < 0) {
			printf("%s:real start error\r\n", __FUNCTION__);
			return -6;
		}
		if (CWPoint::get_instance().setPointPageStatus(false) < 0) {
			printf("%s:set real page status error\r\n", __FUNCTION__);
			return -7;
		}
		if (CWPoint::get_instance().setReadCondition(MONITOR_POINT_READ_ALL) < 0) {
			printf("%s:set real read all error\r\n", __FUNCTION__);
			return -8;
		}
	}
	if (m_bEnableAlarm) {
		if (CAlarmCollect::get_instance().setReportStatus(true) < 0) {
			printf("%s:alarm start error\r\n", __FUNCTION__);
			return -9;
		}
	}
	return 0;
}

int CCloudParse::unloadCommu(CONNECTNODE &commu, bool bRemoveCompileAddr) {
	int nRtn = 0;
	if (commu.nPortId >= 0) {
		if ((nRtn = CEngineMgr::getInstance().unLoadPlcDriVer(commu.nPortId + 1)) == 0) {
			printf("%s:unload %s success!\r\n", __FUNCTION__, commu.PlcSet.szLinkProtocol);
			commu.nPortId = -1;
		}
		else {
			printf("%s:unload %s failed:%d!\r\n", __FUNCTION__, commu.PlcSet.szLinkProtocol, nRtn);
			return -1;
		}
	}
	if (bRemoveCompileAddr) {
		if ((nRtn = CCompileAddress::get_instance().remove(commu.nCommuId)) == 0) {
			printf("%s:delete connect node success!\r\n", __FUNCTION__);
		}
		else {
			printf("%s:delete connect node failed:%d!\r\n", __FUNCTION__, nRtn);
			return -2;
		} 
	}
	return 0;
}

int CCloudParse::loadCommu(CONNECTNODE &commu, const std::string &sDir) {
	int nRtn = 0;
	if (FRAMEWORK_NEW != commu.nFramework)
	{
		if ((nRtn = verifyDrivers(sDir + commu.PlcSet.szLinkProtocol)) < 0) {
			printf("%s:verify drivers error:%d\r\n", __FUNCTION__, nRtn);
			return -1;
		}
	}
	
	if (commu.nPortId >= 0 ||
		(nRtn = CEngineMgr::getInstance().loadPlcDriVer(commu)) < 0) {
		printf("%s: load plc driver error:%d\r\n", __FUNCTION__, nRtn);
		return -2;
	}
	printf("%s:load commu %s success\r\n", __FUNCTION__, commu.PlcSet.szLinkProtocol);
	CONNECTION_NODE_T compileConnect;
	compileConnect.nIndex = commu.nPortId + 1;
	compileConnect.nCommuId = commu.nCommuId;
	compileConnect.sDriver.assign(commu.PlcSet.szPLCType);
	std::string sPath = sDir + std::string("PlcType/");
	if (cbl::getFileSize(sPath + compileConnect.sDriver + std::string(".plc")) >= 0) {
		sPath += compileConnect.sDriver + std::string(".plc");
	}
	else {
		std::string  sPLCType = cbl::replaceall(compileConnect.sDriver, "/", "_");
		sPLCType = cbl::replaceall(sPLCType, "\\", "_");
		sPath += sPLCType + std::string(".plc");
	}
	if ((nRtn = CCompileAddress::get_instance().add(compileConnect, sPath)) < 0) {
		CEngineMgr::getInstance().unLoadPlcDriVer(commu.nPortId + 1);
		printf("%s: add %s connect node error:%d\r\n", __FUNCTION__, sPath.c_str(), nRtn);
		return -3;
	}
	return 0;
}

int CCloudParse::generalPLCType(const std::string &sDir) {
	std::string sPath = sDir + std::string("PLCType.plc");
	if (cbl::getFileSize(sPath) <= 0) {
		return 0;
	}
#ifdef WIN32
	tinyxml2::TinyXMLDocument doc;
#else
	tinyxml2::XMLDocument doc;
#endif
	tinyxml2::XMLElement *pElementRoot;
	std::string sbuffer;
	if (cbl::loadXmlFile(UTF8_TO_ANSI(sPath.c_str()), doc, pElementRoot) < 0) {
		return -1;
	}
	for (XMLNode *pNode = pElementRoot->FirstChild(); pNode != nullptr; pNode = pNode->NextSibling()) {
		if (strcmp(pNode->Value(), "plc") != 0) {
			continue;
		}
		std::string sName = "";
		tinyxml2::XMLElement *pElement = pNode->ToElement();
		if (nullptr != pElement->Attribute("plctype")) {
			sName = pElement->Attribute("plctype");
			sName = cbl::replaceall(sName, "/", "_");
			sName = cbl::replaceall(sName, "\\", "_");
		}
		if (!sName.empty()) {
#ifdef WIN32
			tinyxml2::TinyXMLDocument docDst;
#else
			tinyxml2::XMLDocument docDst;
#endif
			XMLNode *pNodeDst = pNode->DeepClone(&docDst);
			if (nullptr == pNodeDst) {
				return -2;
			}
			docDst.LinkEndChild(pNodeDst);
			XMLPrinter printer;
			docDst.Print(&printer);
#ifdef WIN32
			int nRtn = cbl::saveToFile(printer.CStr(), sDir + std::string("PlcType/") + sName + std::string(".plc"));
#else
			int nRtn = cbl::syncSaveToFile(printer.CStr(), sDir + std::string("PlcType/") + sName + std::string(".plc"));
#endif
			if (nRtn < 0) {
				return -3;
			}
		}
	}
	if (!CFile::Delete(sPath)) {
		return -4;
	}
	return 0;
}

int CCloudParse::generalModbusRegMap(const std::string &sDir) {
	std::string sPath = sDir + std::string("ModbusRegMap.xml");
	if (cbl::getFileSize(sPath) <= 0) {
		return 0;
	}
	if (cbl::isDir(sDir + std::string("Modbus/"))) {
		return 0;
	}
#ifdef WIN32
	tinyxml2::TinyXMLDocument doc;
#else
	tinyxml2::XMLDocument doc;
#endif
	tinyxml2::XMLElement *pElementRoot;
	std::string sbuffer;
	if (cbl::loadXmlFile(UTF8_TO_ANSI(sPath.c_str()), doc, pElementRoot) < 0) {
		return -1;
	}
	for (XMLNode *pNode = pElementRoot->FirstChild(); pNode != nullptr; pNode = pNode->NextSibling()) {
		if (strcmp(pNode->Value(), "plc") != 0) {
			continue;
		}
		std::string sName = "";
		tinyxml2::XMLElement *pElement = pNode->ToElement();
		if (nullptr != pElement->Attribute("plctype")) {
			sName = pElement->Attribute("plctype");
			sName = cbl::replaceall(sName, "/", "_");
			sName = cbl::replaceall(sName, "\\", "_");
		}
		if (!sName.empty()) {
#ifdef WIN32
			tinyxml2::TinyXMLDocument docDst;
#else
			tinyxml2::XMLDocument docDst;
#endif
			XMLNode *pNodeDst = pNode->DeepClone(&docDst);
			if (nullptr == pNodeDst) {
				return -2;
			}
			docDst.LinkEndChild(pNodeDst);
			XMLPrinter printer;
			docDst.Print(&printer);
#ifdef WIN32
			int nRtn = cbl::saveToFile(printer.CStr(), sDir + std::string("Modbus/") + sName + std::string(".xml"));
#else
			int nRtn = cbl::syncSaveToFile(printer.CStr(), sDir + std::string("Modbus/") + sName + std::string(".plc"));
#endif
			if (nRtn < 0) {
				return -3;
			}
		}
	}
	return 0;
}

int CCloudParse::saveModbusRegMap(const std::list<CONNECTNODE> &listCommu, const std::string &sDir) {
	std::string sPath = sDir + std::string("ModbusRegMap.xml");
	int nRtn = 0;
	std::list<std::string> listDrivers;
	std::string sbuffer = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<plcdesc filever=\"1.0\">\n";
	for (auto &iter : listCommu) {
		if (strlen(iter.PlcSet.szPLCType) == 0) {
			continue;
		}
		listDrivers.push_back(iter.PlcSet.szPLCType);
	}
	listDrivers.unique();
	for (auto &iter : listDrivers) {
		std::string sXML = "";
		std::string sName = cbl::replaceall(iter, "/", "_");
		sName = cbl::replaceall(sName, "\\", "_");
		sName = sDir + std::string("Modbus/") + sName + std::string(".xml");
		if (cbl::getFileSize(sName) <= 0) {
			continue;
		}
		if ((nRtn = cbl::readFileOnce64(sName, 0, SEEK_SET, cbl::getFileSize(sName), sXML)) < 0) {
			return -1;
		}
		sbuffer += sXML + std::string("\n");
	}
	sbuffer += "</plcdesc>";
#ifdef WIN32
	nRtn = cbl::saveToFile(sbuffer, sPath);
#else
	nRtn = cbl::syncSaveToFile(sbuffer, sPath);
#endif
	return nRtn;
}