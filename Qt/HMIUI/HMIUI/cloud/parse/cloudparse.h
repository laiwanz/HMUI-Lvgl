#ifndef _CLOUDPARSE_20230412_
#define _CLOUDPARSE_20230412_
#include <iostream>
#include "cbl/cbl.h"
#include "../include/clouddef.h"
#include "remoteProcedureCallApi/remoteProcedureCallApi.h"
#include "DevCacheMgr/DriverConfig.h"
#include "recordcenter/recordcenter.h"
#include "./luaparse.h"
class CCloudOperator;
class CCloudParse
{
public:
	friend class CCloudOperator;
	~CCloudParse();
	CCloudParse(const CCloudParse&) = delete;
	CCloudParse& operator=(const CCloudParse&) = delete;
	static CCloudParse& get_instance();
	int load(bool bInit = true);
	int unload();
	int start();
	int restart();
	int save();
	int setCommu(const std::list<CONNECTNODE> &listCommu);
	std::list<CONNECTNODE> getCommu();
	int updateGlobalConfig(const std::list<GLOBAL_CONFIG_T> &listGlobalConfig);
	const LUAMQTTCONFIG& getMQTTConfig();
	const std::list<CUSTOMCONFIG>& getCloudBindConfig();
	const std::list<CUSTOMCONFIG>& getCloudUserConfig();
	int unloadCommu(CONNECTNODE &commu, bool bRemoveCompileAddr = true);
	int loadCommu(CONNECTNODE &commu, const std::string &sDir);
private:
	CCloudParse();
	int loadProject(bool bInit = true);
	int loadProjectSetup();
	int loadConfigini(const std::string &sPath);
	int loadNormal(const std::string &sPath, std::list<COLLECTION_HISTORY_DATA> &listHistoryData, \
				   std::list<COLLECTION_MONITOR_POINT_DATA> &listRealData, \
				   std::list<COLLECTION_ALARM_DATA> &listAlarmData);
	int loadTemplate(const std::string &sPath, std::list<COLLECTION_HISTORY_DATA> &listHistoryData, \
					 std::list<COLLECTION_MONITOR_POINT_DATA> &listRealData, \
					 std::list<COLLECTION_ALARM_DATA> &listAlarmData, \
					 std::list<LUASCRIPT_T> &scripts);
	int loadCloudProject(const std::string &sPath, std::list<LUASCRIPT_T> &scripts);
	int loadHMICloud(const std::string &sPath);
	int loadTemplateGlobalConfig(const Json::Value &root);
	int parseCommu(const Json::Value &root, const std::string &sDir);
	int parseHistory(const Json::Value &root, std::list<COLLECTION_HISTORY_DATA> &listHistoryData);
	int parsePoint(const Json::Value &root, std::list<COLLECTION_HISTORY_DATA> &listHistoryData, std::list<COLLECTION_MONITOR_POINT_DATA> &listRealData);
	int parseRealPoint(int nCommuId, const Json::Value &root, std::list<COLLECTION_MONITOR_POINT_DATA> &listRealData);
	int parseHistoryPoint(int nCommuId, const Json::Value &root, std::list<COLLECTION_HISTORY_DATA> &listHistoryData);
	int parseAlarm(const Json::Value &root, std::list<COLLECTION_ALARM_DATA> &listAlarmData);
	int parseAlarmCond(const Json::Value& root, COLLECTION_ALARM_DATA& alarm);
	int parseCommu(tinyxml2::XMLElement *pElement);
	int parseReal(tinyxml2::XMLElement *pElement, std::list<COLLECTION_MONITOR_POINT_DATA> &listRealData);
	int parseAlarm(tinyxml2::XMLElement *pElement, std::list<COLLECTION_ALARM_DATA> &listAlarmData);
	int parseAlarmCond(tinyxml2::XMLElement *pElement, COLLECTION_ALARM_DATA& alarm);
	int parseHistory(tinyxml2::XMLElement *pElement, std::list<COLLECTION_HISTORY_DATA> &listHistoryData);
	int parsePLCNode(tinyxml2::XMLElement *pElement, PLCSET &plcSet);
	int parseCommSetNode(tinyxml2::XMLElement *pElement, COMMSET &commSet);
	int parseMGroup(const Json::Value &root, std::list<COLLECTION_MONITOR_GROUP_DATA> &listMGroup);
	int parseMPoint(const Json::Value &root, std::list<COLLECTION_MONITOR_POINT_DATA> &listMPoint,
					std::vector<std::string> &vecCustomTag);
	int loadBaseSet(tinyxml2::XMLElement *pElement);
	int loadThirdMqttConfig(tinyxml2::XMLElement *pElement);
	int loadDataGrpSet(tinyxml2::XMLElement *pElement);
	int parseGroup(tinyxml2::XMLElement *pElement, int nType);
	int parsePoint(tinyxml2::XMLElement *pElement, int nType, unsigned int nGroupId);
#ifdef WIN32
	int saveCommu(tinyxml2::TinyXMLDocument &doc, tinyxml2::XMLElement *pElement);
	int saveReal(tinyxml2::TinyXMLDocument &doc, tinyxml2::XMLElement *pElement);
	int saveHistory(tinyxml2::TinyXMLDocument &doc, tinyxml2::XMLElement *pElement);
	int saveAlarm(tinyxml2::TinyXMLDocument &doc, tinyxml2::XMLElement *pElement);
#else
	int saveCommu(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *pElement);
	int saveReal(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *pElement);
	int saveHistory(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *pElement);
	int saveAlarm(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *pElement);
#endif
	int loadlua(const Json::Value &root, std::list<LUASCRIPT_T> &scripts);
	void addProjectParseStat(int nStat, const std::string &sbuffer);
	void addProjectParseStat(const std::string& sError, unsigned long nId, unsigned int nType, unsigned int nStat);
	int generalPLCType(const std::string &sDir);
	int generalModbusRegMap(const std::string &sDir);
	int saveModbusRegMap(const std::list<CONNECTNODE> &listCommu, const std::string &sDir);
private:
	std::string m_sRemotePwd;
	eCLOUDMODE_T m_eMode;
	bool m_bEnableReal;		/* real data report switch */
	bool m_bEnableHistory;	/* history data report switch */
	bool m_bEnableAlarm;	/* alarm data report switch */
	int m_nRealPeriod;		/* save traffic period */
	int m_nRealMapPeriod;	/* maping point collect period */
	bool m_bEnableKeepGps;	/* gps report switch */
	int m_nGpsPeriod;		/* gps report period */
	bool m_bEnableCloud;
	bool m_bEnableThirdCloud;
	std::string m_sProjectMd5;
	std::list<PROJECT_PRASE_STAT_T> m_listProjectPrase;
	std::mutex m_mutexCommu;
	std::list<CONNECTNODE> m_listCommu;
	LUAMQTTCONFIG m_thirdMqttConfig;
	CLuaPrase m_luaPrase;
	std::list<CUSTOMCONFIG> m_CloudBindConfig;
	std::list<CUSTOMCONFIG> m_CloudUserConfig;
	std::list<tagLUA_INFOMATION_T> m_luarsps;
};
#endif // _CLOUDPARSE_20230412_