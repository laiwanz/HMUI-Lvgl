#ifndef _CLOUDOPERATOR_20230412_
#define _CLOUDOPERATOR_20230412_
#include <iostream>
#include "remoteProcedureCallApi/remoteProcedureCallApi.h"
#include "../include/clouddef.h"
typedef struct tagCLOUDCONFIG_T {
	std::string sProjectMd5;
	std::string sPassword;
	eCLOUDMODE_T eMode;
	bool bEnableScada;
	tagCLOUDCONFIG_T() {
		sProjectMd5 = sPassword = "";
		eMode = eCLOUDMODE_NONE;
		bEnableScada = true;
	}
	tagCLOUDCONFIG_T& operator=(const tagCLOUDCONFIG_T& cloudConfig) {
		this->sProjectMd5 = cloudConfig.sProjectMd5;
		this->eMode = cloudConfig.eMode;
		this->bEnableScada = cloudConfig.bEnableScada;
		this->sPassword = cloudConfig.sPassword;
		return *this;
	}
} CLOUDCONFIG_T;

class CCloudOperator
{
public:
	~CCloudOperator();
	CCloudOperator(const CCloudOperator&) = delete;
	CCloudOperator& operator=(const CCloudOperator&) = delete;
	static CCloudOperator& get_instance();
	int config(const CLOUDCONFIG_T &cloudConfig);
	int onTimer();
	int verifyUrl(const std::string &sToken, const std::string &sSignature);
	int setHistoryData(const std::list<MONITOR_GROUP_T> &listMonitorGroup);
	int setAlarmData(const std::list<ALARM_DATA_T> &listAlarmData);
	int setRealData(const std::list<MONITOR_POINT_T> &listRealData);
	int setHistoryGroupPeriod(unsigned int nId, unsigned int nPeriod);
	int setAlarmConfigCondition(unsigned int nId, const std::string &sCondition);
	int setLuaInfo(const std::list<LUA_INFOMATION_T> &listLuaInfo);
	int setLanguage(unsigned short nLanguage);
	int updateWifiSiganl(int nSignal);
private:
	CCloudOperator();
	int getbasicInfo(UPDATE_CONFIG_T eUpdateConfig);
	int updateCommu(const std::list<COMMU_CONFIG_T> &listCommu);
	int deleteCommu(const std::list<COMMU_CONFIG_T> &listCommu);
	int updateDrivers(const std::list<DRIVER_T> &listDriver);
	int updateRealConfig(const std::list<MONITOR_POINT_CONFIG_T> &listRealConfig);
	int updateAlarmConfig(const std::list<ALARM_CONFIG_T> &listAlarmConfig);
	int deleteRealConfig(const std::list<MONITOR_POINT_CONFIG_T>& listRealConfig);
	int deleteHistoryGroupConfig(const std::list<MONITOR_GROUP_CONFIG_T> &listHistoryConfig);
	int updateHistoryGroupConfig(const std::list<MONITOR_GROUP_CONFIG_T> &listHistoryConfig);
	int updateLuaScriptConfig(const std::list<LUASCRIPT_T> &listLuascript);
	int saveDownloadProjectXML(const DOWNLOAD_PROJECT_INFO_T &downloadInfo);
	int downloadProject();
	int loadDownloadProject();
	int updateInstallmentStat();
	int deleteProject();
	int verifyProject();
private:
	CRemoteProcedureCallClient m_remoteProcedureCallClient;
	bool m_bRunning;
	CLOUDCONFIG_T m_cloudConfig;
	unsigned short m_nMode;
	unsigned short m_nLanguage;
	bool m_bLanguageStat;
	unsigned int m_nAdditionalFunction;
	bool m_bLuaStat;
	unsigned long long m_ullRealCollectRunningTime;
	bool m_bServerStat;
	bool m_bInstallmentStat;
	std::list<LUA_INFOMATION_T> m_listLuaInfo;
	std::string m_sLuabuffer;
	std::mutex	m_mutexLua;
};
#endif // _CLOUDOPERATOR_20230412_