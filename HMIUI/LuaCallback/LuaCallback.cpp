#include "LuaCallback.h"
#include "LuaScript/LuaScript.h"
#include "HmiBase/HmiBase.h"
#include "GlobalModule/GlobalModule.h"
#include "../EventDispatcher/EventDispatch.h"
#include "GlobalModule/DataCollectionCtrl/MonitorPointCollect/cmpoint.h"
#include "secure/secure.h"
#include <cbl/cbl.h>
#include "hmiScreens/hmiScreens.h"
#include "macrosplatform/path.h"
#include "platform/platform.h"
#include "../cloud/cloud.h"
#include "macrosplatform/registerAddr.h"
#include "GlobalModule/UserSecure/Authority/Authority.h"
#include "GlobalModule/UserSecure/Authority/Role.h"

using namespace PLATFORM_NAMESPACE;
static bool isAbsolutePath(const std::string &sPath) {
	if (sPath.compare(0, 2, "./") == 0 ||
		std::string::npos != sPath.find("../")) {
		return false;
	}
	return true;
}

static int luaGetRealDrivePath(const std::string &sScrPath, std::string &sDesPath) {

	std::string sPath = sScrPath;

	if (sPath.compare(0, 5, "user:") == 0) {

		sDesPath = CUSTOMFILEDIR + sPath.erase(0, 5);
	}
	else if (sPath.compare(0, 6, "udisk:") == 0) {

		if (CHmiMedia::isUSBExsit()) {

			std::string sUsbPath;
			CHmiMedia::getUdiskPath(sUsbPath);
			sDesPath = sUsbPath + sPath.erase(0, 6);
		}
	}
	else if (sPath.compare(0, 7, "sdcard:") == 0) {

		if (CHmiMedia::isSDExsit()) {

			std::string sSDPath;

			CHmiMedia::getSDPath(sSDPath);
			sDesPath = sSDPath + sPath.erase(0, 7);
		}
	}
	else if (sPath.compare(0, 5, "wpps:") == 0) {

		sDesPath = SYS_LUA_PLANE_PATH + sPath.erase(0, 5);
	}
	else if (sPath.compare(0, 7, "script:") == 0) {

		sDesPath = getPrjDir() + std::string("Script/") + sPath.erase(0, 7);
	}
	else if (sPath.compare(0, 7, "recipe:") == 0) {
		sDesPath = RECIPEFILEDIR + sPath.erase(0, 7);
	}

	if (sDesPath.empty()) {

		return -1;
	}
	return 0;
}

static int getScreenNo(const std::string &sPartName, int &nScreenNo) {

	try {

		nScreenNo = std::stoi(sPartName);
	}
	catch (const std::exception& e) {

		printf("%s\r\n", e.what());
		return -1;
	}
	return 0;
}

static int luaToRole(const LUAROLE_CONF_T & luaRoleConf,ROLE_CONF_T & roleConf) {
	strncpy(roleConf.szName, luaRoleConf.szName, MAX_LUA_UName_LEN);
	strncpy(roleConf.szPwd, luaRoleConf.szPwd, MAX_LUA_PSW_LEN);
	strncpy(roleConf.szNewPwd, luaRoleConf.szNewPwd, MAX_LUA_PSW_LEN);
	strncpy(roleConf.szNewPwd1, luaRoleConf.szNewPwd1, MAX_LUA_PSW_LEN);
	strncpy(roleConf.szDesc, luaRoleConf.szDesc, MAX_LUA_Descp_LEN);
	roleConf.bHide = luaRoleConf.bHide;
	roleConf.nPermission = luaRoleConf.nPermission;
	return 0;
}

static int roleToLua(const ROLE_CONF_T & roleConf, LUAROLE_CONF_T & luaRoleConf) {
	strncpy(luaRoleConf.szName, roleConf.szName, MAX_LUA_UName_LEN);
	strncpy(luaRoleConf.szPwd, roleConf.szPwd, MAX_LUA_PSW_LEN);
	strncpy(luaRoleConf.szNewPwd, roleConf.szNewPwd, MAX_LUA_PSW_LEN);
	strncpy(luaRoleConf.szNewPwd1, roleConf.szNewPwd1, MAX_LUA_PSW_LEN);
	strncpy(luaRoleConf.szDesc, roleConf.szDesc, MAX_LUA_Descp_LEN);
	luaRoleConf.bHide = roleConf.bHide;
	luaRoleConf.nPermission = roleConf.nPermission;
	return 0;
}

int registerLuaFunction() {

	PARTSFUNCSET_T		partsFuncSet;

	partsFuncSet.setState = [ ] (const std::string &sPartName, const std::string &sValue) {

		if (sPartName.empty() ||
			sValue.empty()) {

			return -1;
		}
		
		int nScreenNo = 0;

		if (0 > getScreenNo(sPartName, nScreenNo)) {

			return -2;
		}

		return CHMIScreens::get_instance().setPartState(nScreenNo, sPartName, sValue);
	};

	partsFuncSet.getState = [ ] (const std::string &sPartName, std::string &sValue) {
		
		if (sPartName.empty()) {

			return -1;
		}

		int nScreenNo = 0;

		if (0 > getScreenNo(sPartName, nScreenNo)) {

			return -2;
		}
		return CHMIScreens::get_instance().getPartState(nScreenNo, sPartName, sValue);
	};

	partsFuncSet.setTouchState = [ ] (const std::string &sPartName, bool bEnable) {
		
		if (sPartName.empty()) {

			return -1;
		}

		int nScreenNo = 0;

		if (0 > getScreenNo(sPartName, nScreenNo)) {

			return -2;
		}
		return CHMIScreens::get_instance().setPartTouchState(nScreenNo, sPartName, bEnable);
	};

	partsFuncSet.getTouchState = [ ] (const std::string &sPartName, bool &bEnable) {
		
		if (sPartName.empty()) {

			return -1;
		}

		int nScreenNo = 0;

		if (0 > getScreenNo(sPartName, nScreenNo)) {

			return -2;
		}
		return CHMIScreens::get_instance().getPartTouchState(nScreenNo, sPartName, bEnable);
	};

	partsFuncSet.setHideState = [ ] (const std::string &sPartName, unsigned int nState) {
		
		if (sPartName.empty()) {

			return -1;
		}

		int nScreenNo = 0;

		if (0 > getScreenNo(sPartName, nScreenNo)) {

			return -2;
		}
		return CHMIScreens::get_instance().setPartHideState(nScreenNo, sPartName, nState);
	};

	partsFuncSet.getHideState = [ ] (const std::string &sPartName, unsigned int &nState) {
		
		if (sPartName.empty()) {

			return -1;
		}

		int nScreenNo = 0;

		if (0 > getScreenNo(sPartName, nScreenNo)) {

			return -2;
		}
		return CHMIScreens::get_instance().getPartHideState(nScreenNo, sPartName, nState);
	};

	partsFuncSet.setRect = [ ] (const std::string &sPartName, const LUARECT_T &luaRect) {
		
		RECT	rc;

		if (sPartName.empty()) {

			return -1;
		}

		int nScreenNo = 0;

		if (0 > getScreenNo(sPartName, nScreenNo)) {

			return -2;
		}
		rc.left = luaRect.nLeft;
		rc.right = luaRect.nRight;
		rc.top = luaRect.nTop;
		rc.bottom = luaRect.nBottom;

		return CHMIScreens::get_instance().setPartCurrentRect(nScreenNo, sPartName, rc);
	};

	partsFuncSet.getRect = [ ] (const std::string &sPartName, LUARECT_T &luaRect) {
		
		RECT	rc;

		if (sPartName.empty()) {

			return -1;
		}

		int nScreenNo = 0;

		if (0 > getScreenNo(sPartName, nScreenNo)) {

			return -2;
		}
		CHMIScreens::get_instance().getPartCurrentRect(nScreenNo, sPartName, rc);
		luaRect.nLeft = rc.left;
		luaRect.nRight = rc.right;
		luaRect.nTop = rc.top;
		luaRect.nBottom = rc.bottom;
		return 0;
	};

	partsFuncSet.getOriginalRect = [ ] (const std::string &sPartName, LUARECT_T &luaRect) {
		
		RECT	rc;

		if (sPartName.empty()) {

			return -1;
		}

		int nScreenNo = 0;

		if (0 > getScreenNo(sPartName, nScreenNo)) {

			return -2;
		}
		CHMIScreens::get_instance().getPartOriginalRect(nScreenNo, sPartName, rc);
		luaRect.nLeft = rc.left;
		luaRect.nRight = rc.right;
		luaRect.nTop = rc.top;
		luaRect.nBottom = rc.bottom;
		return 0;
	};
	registerPartsFuncSet(partsFuncSet);
	OTHERFUNCSET_T		otherFuncSet;
	DEVICE_MODE_T nMode = CPlatBoard::getDeviceMode();
	if (HMI_MODE == nMode) {
		otherFuncSet.printToScreen = std::bind(&CMessageCenter::insertDebugInfo, &CMessageCenter::get_instance(), std::placeholders::_1);
		otherFuncSet.printToScreenOpen = std::bind(&CMessageCenter::setDebugFlag, &CMessageCenter::get_instance(), std::placeholders::_1);
	}
	else if (VBOX_MODE == nMode) {
		otherFuncSet.printToScreen = [] (const std::string &sText) {
			if (sText.empty()) {
				return -1;
			}
			std::list<LUA_INFOMATION_T> listLuaInfo;
			LUA_INFOMATION_T luaInfo;
			luaInfo.sbuffer = sText + std::string("<br>");
			listLuaInfo.push_back(luaInfo);
			return CCloudOperator::get_instance().setLuaInfo(listLuaInfo);
		};
	}
	
	otherFuncSet.closeScreen = [ ] (int nScreenNo) {
		hmiproto::hmievent  event;
		event.set_type(HMIEVENT_CLOSECHILDSCR);
		event.set_scrno(nScreenNo);
		return CEventDispatch::getInstance().postEvent(event);
	};

	otherFuncSet.jumpScreen = [ ] (int nScreenNo) {
		hmiproto::hmievent  event;
		event.set_type(HMIEVENT_CHANGESCR);
		event.set_scrno(nScreenNo);
		event.set_alljump(false);
		return CEventDispatch::getInstance().postEvent(event);
	};

	otherFuncSet.popScreen = [ ] (int nScreenNo, int nX, int nY) {
		hmiproto::hmievent  event;
		event.set_type(HMIEVENT_POPSCR);
		event.set_scrno(nScreenNo);
		event.set_x(nX);
		event.set_y(nY);
		event.set_title(false);
		return CEventDispatch::getInstance().postEvent(event);
	};

	otherFuncSet.getDeviceSetting = [ ] (MACHINESETTING &machineSetting) {	
		SECURE_NAMESPACE::WMachine	wMac;
#ifndef WIN32
		UTILITY::CVersion version;
		if (version.getAllVer(machineSetting.sImage) < 0) {
			return -1;
		}
#endif
		wMac.getDeviceId(machineSetting.sDeviceId);
		wMac.GetHMIType(machineSetting.sModel);
		return 0;
	};
	otherFuncSet.clickScreen = [ ] (const std::string &sPartName, CLICKTYPE_T nType) {
		EVENT_T    event;
		if (nType == CLICK_UP) {
			event.nType = EVENT_LUACLICKUP;
		}
		else {
			event.nType = EVENT_LUACLICKDOWN;
		}
		event.nClientId = QTCLIENT_T;
		if (!sPartName.empty()) {
			sscanf(sPartName.c_str(), "%d_", &event.nScreenID);
		}
		event.sPartName = sPartName;
		return CEventDispatch::getInstance().postEvent(event);
	};

	otherFuncSet.getRealPath = [ ] (const std::string &sScrPath, std::string &sDesPath) {
		// 根路径转换
		if (0 > luaGetRealDrivePath(sScrPath, sDesPath)) {
			return -1;
		}
		// 绝对路径判断
		if (!isAbsolutePath(sDesPath)) {
			return -2;
		}
		return 0;
	};

	/* 获取是否有写入权限：0：有权限，<0：没有权限 */
	otherFuncSet.getWritePermission = [ ] (const std::string &sScrPath) {
		std::string sPath = sScrPath;
		int nPerssion = -1;
		bool bForcePermission = false;

		/* 读取寄存器 */
		int nForcePermission = CRegisterRW::readWord(REG_TYPE_HS_, HSW_ForceFlashPermission);
		if (nForcePermission == 1) {
			bForcePermission = true;
		}

		if (sPath.compare(0, 6, "udisk:") == 0) {
			if (CHmiMedia::isUSBExsit()) {
				nPerssion = 0;
			}
		}
		else if (sPath.compare(0, 7, "sdcard:") == 0) {
			if (CHmiMedia::isSDExsit()) {
				nPerssion = 0;
			}
		}
		else if (sPath.compare(0, 5, "user:") == 0) {
			nPerssion = bForcePermission ? 0 : -1;
		}
		else if (sPath.compare(0, 7, "recipe:") == 0) {

			nPerssion = bForcePermission ? 0 : -1;
		}

		return nPerssion;
	};
	otherFuncSet.addDebugMessage = [ ] (const std::string &sMessage) {
		size_t npos = 0;
		std::string sMsg, sSuffix = ".lua";
		if (std::string::npos != ( npos = sMessage.find(sSuffix) )) {
			sMsg = cbl::baseName(sMessage.substr(0, npos + sSuffix.length())) + sMessage.substr(npos + sSuffix.length());
		}
		else {
			sMsg = sMessage;
		}
		return CMessageCenter::get_instance().insert(hmiproto::message_type_insert, sMsg);
	};
	otherFuncSet.addErrorMsg = [] (unsigned long long ullTimestamp, unsigned int nId, \
								   unsigned int nStat, const std::string &sError) {
		std::list<LUA_INFOMATION_T> listLuaInfo;
		LUA_INFOMATION_T luaInfo;
		luaInfo.nId = nId;
		luaInfo.nStat = nStat;
		luaInfo.sError = sError;
		listLuaInfo.push_back(luaInfo);
		return CCloudOperator::get_instance().setLuaInfo(listLuaInfo);
	};
	otherFuncSet.getPLCSet = [ ] (std::list<CONNECTNODE> &listPLCSet) {
		listPLCSet = CCloudParse::get_instance().getCommu();
		return 0;
	};

	otherFuncSet.getMqttSet = [ ] (LUAMQTTCONFIG &mqttSet) {
		mqttSet = CCloudParse::get_instance().getMQTTConfig();
		return 0;
	};

	otherFuncSet.isFlashPath = [ ] (const std::string &sPath) {
		bool	bRet = false;
		if (sPath.compare(0, 5, "user:") == 0) {
			bRet = true;
		}
		else if (sPath.compare(0, 5, "wpps:") == 0) {
			bRet = true;
		}
		else if (sPath.compare(0, 7, "script:") == 0) {
			bRet = true;
		}
		else if (sPath.compare(0, 7, "recipe:") == 0) {
			bRet = true;
		}

		return bRet;
	};

	otherFuncSet.changeFtpPwd = [ ] (const std::string &sPwd, const std::string &sNewPwd, std::string &sErrorMessage) {
		int nRtn = 0;
#ifdef _LINUX_
		nRtn = CFtpServer::getInstance().changePasswd(sPwd, sNewPwd);
#endif
		switch (nRtn) {
		case -2: 
			{
				sErrorMessage = "Prohibit changing password!";
			}
			break;
		case -5:
			{
				sErrorMessage = "Password is wrong!";
			}
			break;
		default:
			{
				sErrorMessage = "Not activated ftpserver!";
			}
			break;
		}
		return nRtn;
	};
	registerOtherFuncSet(otherFuncSet);

	CLOUDFUNCSET_T	cloudFuncSet;

	cloudFuncSet.getHistorySet = std::bind(&CDataRecordCollect::getReportConfig, &CDataRecordCollect::get_instance(), std::placeholders::_1);
	cloudFuncSet.getAlarmSet = std::bind(&CAlarmCollect::getReportConfig, &CAlarmCollect::get_instance(), std::placeholders::_1);
	cloudFuncSet.getGroupSet = std::bind(&CCMPoint::getGroupConfig, &CCMPoint::get_instance(), std::placeholders::_1);
	cloudFuncSet.getGroupData = std::bind(&CCMPoint::getGroupData, &CCMPoint::get_instance(), std::placeholders::_1, std::placeholders::_2);
	cloudFuncSet.getPointData = std::bind(&CCMPoint::getData, &CCMPoint::get_instance(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	cloudFuncSet.setPointData = std::bind(&CCMPoint::setData, &CCMPoint::get_instance(), std::placeholders::_1, std::placeholders::_2);
	cloudFuncSet.savePointToCache = std::bind(&CCMPoint::setOneCache, &CCMPoint::get_instance(), std::placeholders::_1);
	cloudFuncSet.getPointAtCondition = std::bind(&CCMPoint::getAllData, &CCMPoint::get_instance(), std::placeholders::_1);
	cloudFuncSet.getPointFromCache = std::bind(&CCMPoint::getAllCache, &CCMPoint::get_instance(), std::placeholders::_1);
	cloudFuncSet.getCloudBindConfig = [] (std::list <CUSTOMCONFIG> &listConfig) {
		listConfig = CCloudParse::get_instance().getCloudBindConfig();
		return 0;
	};
	cloudFuncSet.getCloudUserConfig = [] (std::list <CUSTOMCONFIG> &listConfig) {
		listConfig = CCloudParse::get_instance().getCloudUserConfig();
		return 0;
	};
	registerCloudFuncSet(cloudFuncSet);
	NORMALFUNCSET_T normalFuncSet;
	normalFuncSet.getNormalAllData = [ ] (std::list<MONITOR_POINT_T> &listPoints) {
		return CWPoint::get_instance().readAll(listPoints);
	}; 
	normalFuncSet.getNormalPointData = [ ] (const std::string &sName, std::string &sValue) {
		return CWPoint::get_instance().read(sName, sValue);
	};
	normalFuncSet.setNormalPointData = [] (const std::string &sName, const std::string &sValue) {
		return CWPoint::get_instance().write(sName, sValue);
	};
	registerNormalFuncSet(normalFuncSet);

	SERCUEFUNCSET_T secureFuncSet;
	secureFuncSet.getAllRoleData = [ ](std::vector<LUAROLE_CONF_T>& vecLuaRole) {
		CAuthority& authority = CSecureCtrl::getInstance().getAuthority();
		std::vector<ROLE_CONF_T> vecRole;

		if (authority.getRoleInfo(vecRole) < 0) {
			return -1;
		}
		for (auto & it : vecRole) {
			LUAROLE_CONF_T luaroleConf = {0};
			roleToLua(it , luaroleConf);
			vecLuaRole.push_back(luaroleConf);
		}
		return 0;
	};
	secureFuncSet.updateRolePwd = [ ](const LUAROLE_CONF_T & luaroleConf) {
		CAuthority& authority = CSecureCtrl::getInstance().getAuthority();
		ROLE_CONF_T roleConf = {0};
		luaToRole(luaroleConf, roleConf);
		return authority.luaChangePwd(roleConf);

	};
	secureFuncSet.updateRolePermission = [](const LUAROLE_CONF_T & luaroleConf) {
		CAuthority& authority = CSecureCtrl::getInstance().getAuthority();
		ROLE_CONF_T roleConf = {0};
		luaToRole(luaroleConf, roleConf);
		return authority.luaChangePermission(roleConf);

	};
	secureFuncSet.addRoleInfo = [ ](const LUAROLE_CONF_T & luaroleConf) {
		CAuthority& authority = CSecureCtrl::getInstance().getAuthority();
		ROLE_CONF_T roleConf = {0};
		luaToRole(luaroleConf, roleConf);
		return authority.luaAddRole(roleConf);
	};
	secureFuncSet.deleteRoleInfo = [ ](const string & sName) {
		CAuthority& authority = CSecureCtrl::getInstance().getAuthority();
		
		return authority.luaRmRole(sName);
	};
	secureFuncSet.roleLoginSet = [] (const LUAROLE_CONF_T & luaroleConf){
		CAuthority& authority = CSecureCtrl::getInstance().getAuthority();
		ROLE_CONF_T roleConf = {0};
		luaToRole(luaroleConf, roleConf);
		return authority.roleLogin(roleConf,QTCLIENT_T);
	};
	secureFuncSet.roleLogoutSet = [] (){
		CAuthority& authority = CSecureCtrl::getInstance().getAuthority();
		return authority.luaRoleLogout(QTCLIENT_T);
	};

	registerSscureFuncSet(secureFuncSet);
	return 0;
}