#ifndef _CCOMMON_H_20230414_
#define _CCOMMON_H_20230414_
#include <iostream>
#include "recordcenter/recordcenter.h"
#include "../include/clouddef.h"
#include "remoteProcedureCallApi/remoteProcedureCallApi.h"
ADDR_TYPE_T toAddrType(unsigned int nType);
int addressTransform(const Json::Value& json, const std::vector<std::string> &vec, \
					 int nCommuId, ADDR_TYPE_T addrType, CRegister& reg, std::string& sAlias);
ALARM_MODE_T toAlarmMode(unsigned short nType);
unsigned short toAlarmType(const ALARM_MODE_T &mode);
eCLOUDMODE_T getCloudMode();
int setCloudMode(eCLOUDMODE_T eMode);
bool writeSpecialAddress(unsigned int nAddressOffset, const std::string &sbuffer);
int toCommuConfig(const COMMU_CONFIG_T& fromConfig, CONNECTNODE& toConfig);
int toRealConfig(const MONITOR_POINT_CONFIG_T& fromConfig, COLLECTION_MONITOR_POINT_DATA& toConfig);
int toHistoryConfig(const MONITOR_POINT_CONFIG_T& fromConfig, COLLECTION_HISTORY_NODE& toConfig);
int toAlarmConfig(const ALARM_CONFIG_T& fromConfig, COLLECTION_ALARM_DATA& toConfig);
int toHistoryGroupConfig(const MONITOR_GROUP_CONFIG_T& fromConfig, COLLECTION_HISTORY_DATA& toConfig);
int getFileMd5(const std::string &sPath, std::string &sbuffer);
int adaptOldProjects();
int verifyDrivers(const std::string &sFilePath);
#endif // _CCOMMON_H_20230414_