#include "parseProject.h"
#include "utility/utility.h"
#include "recordcenter/recordcenter.h"
#include "secure/secure.h"
#include "HmiBase/HmiBase.h"
#include "ProtoXML/ProtoXML.h"
#include "hmiScreens/hmiScreens.h"
#include "../EventDispatcher/EventDispatch.h"
#include "webserver/webserver.h"
#include "macrosplatform/path.h"
#include "ScriptMgr/ScriptMgr.h"
#include "../PluginDLL/PluginDLL.h"
#include "LuaScript/LuaScript.h"
#include "macrosplatform/registerAddr.h"
#include "../HSWAddrOperate/HSWAddrOperate.h"
#include "../HSWAddrOperate/serialmanager.h"
#include "common.h"
#include "../cloud/cloud.h"
#ifdef _LINUX_
#include "btl/btl.h"
#endif // _LINUX_

#ifdef WIN32
#define ONLINEEMULATOR		1
#define OFFLINEEMULATOR		0
#include "../Emulator/HmiEmulator.h"
#define FORCE_UPDATE_FILE "C:/projectcomplete"
#define FORCE_UPDATE_SHELL_SCRIPT ""
#elif _LINUX_
#define FORCE_UPDATE_FILE "/wecon/run/projectcomplete"
#define FORCE_UPDATE_SHELL_SCRIPT "/mnt/wecon/app/hmiapp/bin/deletefile.sh"
#define SysParaConfig "/mnt/data/prj/SysSetParaConfig.json"
#endif

using namespace UTILITY_NAMESPACE;
using namespace SECURE_NAMESPACE;
using namespace PLATFORM_NAMESPACE;
using namespace PROTO_NAMESPACE;
using namespace ProjectPro;
static bool hmiSetInfo(void * pData) {
	LPPROJSETUP	pps = GetProjInfo();
	MsgHmiSet	*pHmiSet = NULL;
	int			nRotateAngle = 0;
	int			nWidth = 0;
	int			nHeight = 0;

	if (NULL == pData) {
		return false;
	}
	pHmiSet = (MsgHmiSet *) pData;
	memset(pps->BaseSet.szHMIType, 0, sizeof(pps->BaseSet.szHMIType));
	strncpy(pps->BaseSet.szHMIType, pHmiSet->type().c_str(), pHmiSet->type().length());
	pps->BaseSet.szHMIType[pHmiSet->type().length()] = '\0';
	nRotateAngle = pHmiSet->rotateangle();
	nWidth = pHmiSet->width();
	nHeight = pHmiSet->height();
	switch (nRotateAngle) {
	case 0:
		{
			pps->nTermHeight = nHeight;
			pps->nTermWidth = nWidth;
			pps->nRotateMode = ScreenRotate_0;
		}
		break;
	case 90:
		{
			pps->nTermHeight = nWidth;
			pps->nTermWidth = nHeight;
			pps->nRotateMode = ScreenRotate_90;
		}
		break;
	case 180:
		{
			pps->nTermHeight = nHeight;
			pps->nTermWidth = nWidth;
			pps->nRotateMode = ScreenRotate_180;
		}
		break;
	case 270:
		{
			pps->nTermHeight = nWidth;
			pps->nTermWidth = nHeight;
			pps->nRotateMode = ScreenRotate_270;
		}
		break;
	default:
		{
			pps->nTermHeight = nHeight;
			pps->nTermWidth = nWidth;
			pps->nRotateMode = ScreenRotate_0;
		}
		break;
	}
	{
#define CacheXMLFileSize	1024*256
		HMISCREENCONFIG screenConfig;
		HMISCREENEVENTCONFIG_T	eventConfig;

		screenConfig.sPrjDir = getPrjDir();
		screenConfig.nInitialScreenNo = pHmiSet->startno();
#ifdef WITH_UI
		switch (CPlatFlash::getPhyMemorySize()) {
		case 64:
			{
				screenConfig.lFileSizeCache = 0;
				screenConfig.nMinFreeCache = 60;
				screenConfig.nMinLoadedScreenNum = 2;
			}
			break;
		case 128:
			{
				screenConfig.lFileSizeCache = CacheXMLFileSize * 20;
				screenConfig.nMinFreeCache = 25;   // 128M内存（留25M）
				screenConfig.nMinLoadedScreenNum = 5;
			}
			break;
		case 256:
			{
				screenConfig.lFileSizeCache = CacheXMLFileSize * 40;
				screenConfig.nMinFreeCache = 45;   // 256M内存（留45M）
				screenConfig.nMinLoadedScreenNum = 5;
			}
			break;
		case 512:
			{
				screenConfig.lFileSizeCache = CacheXMLFileSize * 40;
				screenConfig.nMinFreeCache = 75;   // 512M内存（留75M）
				screenConfig.nMinLoadedScreenNum = 5;
			}
			break;
		default:
			{
				screenConfig.lFileSizeCache = CacheXMLFileSize * 40;
				screenConfig.nMinFreeCache = 75;   // 512M内存（留75M）
				screenConfig.nMinLoadedScreenNum = 5;
			}
			break;
		}
#else
		screenConfig.lFileSizeCache = CacheXMLFileSize * 20;
		screenConfig.nMinFreeCache = 25;   // 128M内存（留25M）
		screenConfig.nMinLoadedScreenNum = 5;
#endif
		{
			eventConfig.funcPopSubscreen = [ ] (int nClientId, int nScreenNo, int nX, int nY, bool bAllClient, bool bTitle) {
				hmiproto::hmievent  event;
				event.set_type(HMIEVENT_POPSCR);
				event.set_clientid(nClientId);
				event.set_scrno(nScreenNo);
				event.set_x(nX);
				event.set_y(nY);
				event.set_title(bTitle);
				event.set_alljump(bAllClient);
				return CEventDispatch::getInstance().sendEvent(event);
			};
			eventConfig.funcPopKb = [ ] (int nClientId, int nScreenNo, int nX, int nY, bool bAllClient, bool bTitle) {
				hmiproto::hmievent  event;
				event.set_type(HMIEVENT_POP_KEYBOARD);
				event.set_clientid(nClientId);
				event.set_scrno(nScreenNo);
				event.set_x(nX);
				event.set_y(nY);
				event.set_title(bTitle);
				return CEventDispatch::getInstance().sendEvent(event);
			};
			eventConfig.funcJumpScreen = [ ] (int nClientId, int nScreenNo, bool bAllClient) {
				hmiproto::hmievent  event;
				event.set_type(HMIEVENT_CHANGESCR);
				event.set_clientid(nClientId);
				event.set_scrno(nScreenNo);
				event.set_alljump(bAllClient ? true : false);
				return CEventDispatch::getInstance().sendEvent(event);
			};
			eventConfig.funcCloseSubscreen = [ ] (int nClientId, int nScreenNo, bool bAllClient) {
				hmiproto::hmievent  event;
				event.set_type(HMIEVENT_CLOSECHILDSCR);
				event.set_clientid(nClientId);
				event.set_scrno(nScreenNo);
				event.set_alljump(bAllClient);
				return CEventDispatch::getInstance().sendEvent(event);
			};
			eventConfig.funcCloseKb = [ ] (int nClientId, int nScreenNo, bool bAllClient) {
				hmiproto::hmievent  event;
				event.set_type(HMIEVENT_CLOSE_KEYBOARD);
				event.set_clientid(nClientId);
				event.set_scrno(nScreenNo);
				return CEventDispatch::getInstance().sendEvent(event);
			};
			eventConfig.funcJumpHtml = [ ] (int nClientId) {
				hmiproto::hmievent  msgEvent;
				msgEvent.set_html(CWebServer::get_instance().getHtml(REMOTE_OFF));
				msgEvent.set_type(HMIEVENT_JUMPHTML);
				msgEvent.set_clientid(nClientId);
				return CEventDispatch::getInstance().sendEvent(msgEvent);
			};
			eventConfig.funcDestroyScreen = [ ] (int nScreenNo) {
				hmiproto::hmievent  event;
				event.set_type(HMIEVENT_FREESCRCACHE);
				event.set_scrno(nScreenNo);
				return CEventDispatch::getInstance().sendEvent(event);
			};
			eventConfig.funcScreenshot = [ ] (const SREENSHOT_T &screenshot) {
				hmiproto::hmievent  event;
				event.set_type(HMIEVENT_PRINTPART);
				event.set_scrno(screenshot.nScreenNo);
				event.set_imgtype(screenshot.nImgType);
				event.set_degrees(screenshot.nDegree);
				event.set_x(screenshot.nX);
				event.set_y(screenshot.nY);
				event.set_width(screenshot.nWidth);
				event.set_height(screenshot.nHeight);
				event.set_paperwidth(screenshot.nPaperWidth);
				event.set_paperheight(screenshot.nPaperHeight);
				event.set_imgpath(screenshot.sImgPath);
				return CEventDispatch::getInstance().sendEvent(event);
			};
			eventConfig.funcDisplayIndirectScreen = [ ] (int nClientId, int nCurScreenNo, int nScreenNo, int nX, int nY, int nWidth, int nHeight, const std::string &sPartName) {
				hmiproto::hmievent	event;
				event.set_type(HMIEVENT_INDIRECTSCRINIT);
				event.set_clientid(nClientId);
				event.set_curscrno(nCurScreenNo);
				event.set_scrno(nScreenNo);
				event.set_x(nX);
				event.set_y(nY);
				event.set_width(nWidth);
				event.set_height(nHeight);
				event.set_partname(sPartName);
				return CEventDispatch::getInstance().sendEvent(event);
			};
			eventConfig.funcUpdateScreen = [ ] () {
				hmiproto::hmievent	event;
				event.set_type(HMIEVENT_UPDATESCREEN);
				return CEventDispatch::getInstance().sendEvent(event);
			};
			eventConfig.funcOpenVirtualKeyboard = [ ] (int nScreenNo, int nClientId, const std::string &sPartName, const std::string &sEventbuffer) {
				hmiproto::hmievent	event;
				event.set_type(HMIEVENT_JSON);
				event.set_scrno(nScreenNo);
				event.set_clientid(nClientId);
				event.set_partname(sPartName);
				event.set_eventbuffer(sEventbuffer);
				return CEventDispatch::getInstance().sendEvent(event);
			};
		}
		screenConfig.bHscSecure = IsProjSecure();
		screenConfig.nHeightPixel = pps->nTermHeight;
		screenConfig.nWidthPixel = pps->nTermWidth;
		CHMIScreens::get_instance().config(screenConfig, eventConfig);
	}
	return true;
}

static bool doRead_BaseSet(void * pData, BASESET *pbs) {
	MsgBaseSet *pBaseSet = NULL;
	int			nBgOffTime = 0;

	if (NULL == pData ||
		NULL == pbs) {
		return false;
	}
	pBaseSet = (MsgBaseSet *) pData;
	pbs->nStorePattern = pBaseSet->storepattern();
	nBgOffTime = pBaseSet->background();
	if (-1 == nBgOffTime) {
		nBgOffTime = 0;
	}
	pbs->nBgOffTime = nBgOffTime;
	if (pBaseSet->has_scridwordaddr()) {
		ProMrg::MsgAddrToHmiAddr(pBaseSet->scridwordaddr(), pbs->addrSysContrl);
	}
	pbs->bUseSysContrl = pBaseSet->isscridvar();
	int nFloatHiLowReverse = pBaseSet->floathilowreverse();
	if (nFloatHiLowReverse == -1 ||
		nFloatHiLowReverse == 0) {
		pbs->bFloatHiLowReverse = false;
	}
	else {
		pbs->bFloatHiLowReverse = true;
	}
	pbs->bHiLowReverAllData = pBaseSet->ishighword();
	pbs->bIsUseOptLogFunc = pBaseSet->isuseoptlogfunc();
	CHMIScreens::get_instance().enableClickType(pBaseSet->isdownvalid());
	pbs->bIsShowCursor = pBaseSet->isshowcursor();
	pbs->nOsLanguage = pBaseSet->headlang();
	pbs->bForbidChangeTime = pBaseSet->isforbidchangetime();
	CScreenProtect& scrProtect = CSecureCtrl::getInstance().getScrProtect();
	scrProtect.parsePrjConf((void*) pBaseSet);
	CScrSaver& scrSaver = CSecureCtrl::getInstance().getScrSaver();
	scrSaver.parsePrjConf((void*) pBaseSet);
	if (pbs->bForbidChangeTime == true) {
		if (CFile::ExistFile(FORBIDCHANGETIME) == false) {
			FILE *fp = NULL;
			fp = CFile::Open(FORBIDCHANGETIME, "w+");
			if (fp != NULL) {
				fclose(fp);
				fp = NULL;
			}
		}
	}
	return true;
}

static bool doRead_AssisSet(void * pData, ASSISSET *pas) {
	MsgAssisSet *pAssisSet = NULL;
	if (NULL == pData ||
		NULL == pas) {
		return false;
	}
	pAssisSet = (MsgAssisSet *) pData;
	pas->byLangType = (BYTE) pAssisSet->languagetype();
	return true;
}

static void makeSystemConfig(const BGENTERTIME &bgEnterTime)
{

	Json::Value root;
#ifdef _WIN32
	LPPROJSETUP	 pps = GetProjInfo();
	std::string sFilePath = pps->sPrjDir + "SysSetParaConfig.json";
#else
	std::string sFilePath = SysParaConfig;
#endif

	if (0 == cbl::loadJsonFile(sFilePath, root) && root.isMember("syssetpara")){
		return;
	}

	Json::Value syssetpara;
	syssetpara["IsEnterTime"] = bgEnterTime.bIsEnterTime;
	syssetpara["IsPowerEnterTime"] = bgEnterTime.bIsPowerEnterTime;
	syssetpara["nEnterTime"] = bgEnterTime.nEnterTime;
	syssetpara["nPowerEnterTime"] = bgEnterTime.nPowerEnterTime;
	syssetpara["szPwd"] = bgEnterTime.szPwd;
	root["syssetpara"] = syssetpara;
	std::string sJsonString;
	int nRet = cbl::saveJsonString(root, sJsonString);
	if (nRet < 0){
		printf("makeSystemConfig saveJsonString Error nRet = [%d]\n", nRet);
		return;
	}
#ifdef _LINUX_
	nRet = cbl::syncSaveToFile(sJsonString, sFilePath);
#else
	nRet = cbl::saveToFile(sJsonString, sFilePath);
#endif
	if (nRet < 0) {
		printf("saveToFile or syncSaveToFile Error nRet = [%d]\n", nRet);
	}
}

static bool doRead_BgEnterTime(void * pData, BGENTERTIME* pBgEnterTime) {
	MsgExtend	*pExtend = NULL;
	int			nNum = 0;
	if (NULL == pData ||
		NULL == pBgEnterTime) {
		return false;
	}
	pExtend = (MsgExtend *) pData;
	strncpy(pBgEnterTime->szPwd, pExtend->backsetpsw().c_str(), pExtend->backsetpsw().length());
	nNum = pExtend->gotobacksettype();
	if (1 == nNum) {
		pBgEnterTime->bIsPowerEnterTime = true;
		pBgEnterTime->bIsEnterTime = false;
	}
	else {
		pBgEnterTime->bIsEnterTime = true;
		pBgEnterTime->bIsPowerEnterTime = false;
	}
	if (pBgEnterTime->bIsPowerEnterTime) {
		pBgEnterTime->nPowerEnterTime = pExtend->pushtime();
	}
	else {
		pBgEnterTime->nEnterTime = pExtend->pushtime();
	}
	pBgEnterTime->bIsKeyBoard = pExtend->iskeyboard();
	pBgEnterTime->bUsbPopScreen = pExtend->isusbpopscreen();
	pBgEnterTime->bShortCutKey = pExtend->isshortcutkey();
	makeSystemConfig(*pBgEnterTime);
	return true;
}

static bool doRead_LANPassword(void * pData, LANPWORD *pwd) {
	MsgLANPassword *pLANPassword = NULL;
	if (NULL == pData ||
		NULL == pwd) {
		return false;
	}
	pLANPassword = (MsgLANPassword *) pData;
	pwd->byEnable = (BYTE) pLANPassword->enable();
	memset(pwd->szAppleId, 0, sizeof(pwd->szAppleId));
	if (pLANPassword->appleid().length() <= sizeof(pwd->szAppleId) / sizeof(char)) {
		strncpy(pwd->szAppleId, pLANPassword->appleid().c_str(), pLANPassword->appleid().length());
	}
	if (pLANPassword->password().length() <= sizeof(pwd->szPassword) / sizeof(char)) {
		strncpy(pwd->szPassword, pLANPassword->password().c_str(), pLANPassword->password().length());
	}
	return true;
}

static bool doRead_ProjInfo(void * pData) {
	LPPROJSETUP pps = GetProjInfo();
	MsgSysSet *pSysSet = (MsgSysSet *) pData;
	if (NULL == pData) {
		return false;
	}
	if (pSysSet->has_baseset()) {
		doRead_BaseSet((void *) &pSysSet->baseset(), &pps->BaseSet);
	}
	if (pSysSet->has_assisset()) {
		doRead_AssisSet((void *) &pSysSet->assisset(), &pps->AssisSet);
	}
	if (pSysSet->has_installmetset()) {
		Installment_DoRead((void *) &pSysSet->installmetset());
	}
	if (pSysSet->has_extend()) {
		doRead_BgEnterTime((void *) &pSysSet->extend(), &pps->BGEnterTime);
	}
	if (pSysSet->has_pwdprotect()) {
		CScreenLock& scrLock = CSecureCtrl::getInstance().getScrLock();
		scrLock.parsePrjConf((void*) &pSysSet->pwdprotect());
		CPartGrade& partGrade = CSecureCtrl::getInstance().getPartGrade();
		partGrade.parsePrjConf((void*) &pSysSet->pwdprotect());
		CAuthority& authority = CSecureCtrl::getInstance().getAuthority();
		authority.setUploadPermission((void*) &pSysSet->pwdprotect());
	}
	if (pSysSet->has_lanpassword()) {
		doRead_LANPassword((void *) &pSysSet->lanpassword(), &pps->LANPassword);
	}
	return true;
}

static bool doRead_PLC(void * pData, PLCSET *ppls) {
	MsgPLC *pMsgPlc = NULL;

	if (NULL == pData ||
		NULL == ppls) {
		return false;
	}

	pMsgPlc = (MsgPLC *) pData;
	memset(ppls->szPLCType, 0, sizeof(ppls->szPLCType));
	if (pMsgPlc->type().length() <= sizeof(ppls->szPLCType) / sizeof(char)) {
		strncpy(ppls->szPLCType, pMsgPlc->type().c_str(), pMsgPlc->type().length());
	}
	else {
		strncpy(ppls->szPLCType, pMsgPlc->type().c_str(), sizeof(ppls->szPLCType) / sizeof(char) - 1);
		ppls->szPLCType[sizeof(ppls->szPLCType) / sizeof(char) - 1] = '\0';
	}
	ppls->nHmiMachineNo = pMsgPlc->hmistatno();
	ppls->nPlcMachineNo = pMsgPlc->plcstatno();
	memset(ppls->szLinkProtocol, 0, sizeof(ppls->szLinkProtocol));
	if (pMsgPlc->driver().length() <= sizeof(ppls->szLinkProtocol) / sizeof(char)) {
		strncpy(ppls->szLinkProtocol, pMsgPlc->driver().c_str(), pMsgPlc->driver().length());
	}
	else {
		strncpy(ppls->szLinkProtocol, pMsgPlc->driver().c_str(), sizeof(ppls->szLinkProtocol) / sizeof(char) - 1);
		ppls->szLinkProtocol[sizeof(ppls->szLinkProtocol) / sizeof(char) - 1] = '\0';
	}

	memset(ppls->szPlctypeAlias, 0, sizeof(ppls->szPlctypeAlias));
	if (pMsgPlc->type_alias().length() <= sizeof(ppls->szPlctypeAlias) / sizeof(char)) {
		strncpy(ppls->szPlctypeAlias, pMsgPlc->type_alias().c_str(), pMsgPlc->type_alias().length());
	}
	else {
		strncpy(ppls->szPlctypeAlias, pMsgPlc->type_alias().c_str(), sizeof(ppls->szPlctypeAlias) / sizeof(char) - 1);
		ppls->szPlctypeAlias[sizeof(ppls->szPlctypeAlias) / sizeof(char) - 1] = '\0';
	}
	return true;
}

static bool doRead_CommSet(void * pData, COMMSET *pcs) {
	MsgCommSet	*pCommSet = NULL;
	char		szTemp[32] = { 0 };
	if (NULL == pData ||
		NULL == pcs) {
		return false;
	}
	pCommSet = (MsgCommSet	*) pData;
	memset(pcs->szPortName, 0, sizeof(pcs->szPortName));
	if (pCommSet->port().length() <= sizeof(pcs->szPortName) / sizeof(char)) {
		strncpy(pcs->szPortName, pCommSet->port().c_str(), pCommSet->port().length());
	}
	else {
		strncpy(pcs->szPortName, pCommSet->port().c_str(), sizeof(pcs->szPortName) / sizeof(char) - 1);
		pcs->szPortName[sizeof(pcs->szPortName) / sizeof(char) - 1] = '\0';
	}
	memset(pcs->szInitPortName, 0, sizeof(pcs->szInitPortName));
	if (pCommSet->port().length() <= sizeof(pcs->szInitPortName) / sizeof(char)) {
		strncpy(pcs->szInitPortName, pCommSet->port().c_str(), pCommSet->port().length());
	}
	else {
		strncpy(pcs->szInitPortName, pCommSet->port().c_str(), sizeof(pcs->szInitPortName) / sizeof(char) - 1);
		pcs->szInitPortName[sizeof(pcs->szInitPortName) / sizeof(char) - 1] = '\0';
	}
	pcs->nCommMode = pCommSet->comtype();
	memset(pcs->szComSelect, 0, sizeof(pcs->szComSelect));
	if (pCommSet->comsel().length() <= sizeof(pcs->szComSelect) / sizeof(char)) {
		strncpy(pcs->szComSelect, pCommSet->comsel().c_str(), pCommSet->comsel().length());
	}
	else {
		strncpy(pcs->szComSelect, pCommSet->comsel().c_str(), sizeof(pcs->szComSelect) / sizeof(char) - 1);
		pcs->szComSelect[sizeof(pcs->szComSelect) / sizeof(char) - 1] = '\0';
	}
	pcs->nBaudRate = pCommSet->baudrate();
	pcs->nDataLength = pCommSet->datalength();
	memset(pcs->szCheckBit, 0, sizeof(pcs->szCheckBit));
	if (pCommSet->checkbit().length() <= sizeof(pcs->szCheckBit) / sizeof(char)) {
		strncpy(pcs->szCheckBit, pCommSet->checkbit().c_str(), pCommSet->checkbit().length());
	}
	else {
		strncpy(pcs->szCheckBit, pCommSet->checkbit().c_str(), sizeof(pcs->szCheckBit) / sizeof(char) - 1);
		pcs->szCheckBit[sizeof(pcs->szCheckBit) / sizeof(char) - 1] = '\0';
	}
	memset(pcs->szNetName, 0, sizeof(pcs->szNetName));
	if (pCommSet->netcardname().length() <= sizeof(pcs->szNetName) / sizeof(char)) {
		strncpy(pcs->szNetName, pCommSet->netcardname().c_str(), pCommSet->netcardname().length());
	}
	else {
		strncpy(pcs->szNetName, pCommSet->netcardname().c_str(), sizeof(pcs->szNetName) / sizeof(char) - 1);
		pcs->szNetName[sizeof(pcs->szNetName) / sizeof(char) - 1] = '\0';
	}
	pcs->nStopBits = pCommSet->stopbit();
	pcs->nFlowControl = pCommSet->flowmode();
	pcs->nWaitTimeout = pCommSet->waittimeout();
	pcs->nRecvTimeout = pCommSet->revtimeout();
	pcs->nRetryTimes = pCommSet->retrytimes();
	pcs->nRetryTimeOut = pCommSet->retrytimeout();
	pcs->nComIoDelayTime = pCommSet->comiodelaytime();
	pcs->nComStepInterval = pCommSet->comstepinterval();
	pcs->nConnectTime = pCommSet->connecttime();
	pcs->nComSingleReadLen = pCommSet->singlereadlen();
	strncpy(szTemp, pCommSet->netipaddr().c_str(), 32);
	pcs->DstIPAddr.sin_addr.s_addr = inet_addr(szTemp);
	strcpy(pcs->szDstIPV4, pCommSet->netipaddr().c_str());
	pcs->iDstPort = (u_short) strtol(pCommSet->netport().c_str(), NULL, 10);
	strcpy(pcs->szDstNetPort, pCommSet->netport().c_str());
	pcs->wBroadcastAddr = (unsigned short) pCommSet->netbroadcastaddr();
	pcs->bEthernet_IsBroadcast = pCommSet->netisbroadcast();
	pcs->nEthernet_IsUDP = pCommSet->netisudp();
	if (!strcmp(pcs->szPortName, "Ethernet")) {
		pcs->nCommMode = Comm_Ethernet;
	}
	if (!strncmp(pcs->szPortName, "CAN", strlen("CAN"))) //支持双CAN
	{
		pcs->nCommMode = Comm_CAN;
		if (pcs->szPortName[3] == '\0') {
			pcs->szPortName[3] = '0';  //CAN 为CAN0
			pcs->szInitPortName[3] = '0';
		}
	}
	if (!strncmp(pcs->szPortName, "USB", strlen("USB"))) //支持USB
	{
		pcs->nCommMode = Comm_USB;
	}
	return true;
}

static bool doRead_Connect(void * pData, CONNECTNODE& connectNode) {
	MsgConnection *pMsgConnection = NULL;
	if (NULL == pData) {
		return false;
	}
	pMsgConnection = (MsgConnection *) pData;
	doRead_PLC((void *) &pMsgConnection->plc(), &connectNode.PlcSet);
	doRead_CommSet((void *) &pMsgConnection->commset(), &connectNode.Commset);
	return true;
}

static bool PLCInfoSet(void * pData) {
	MsgPLCSet	*pPLCSet = NULL;
	if (NULL == pData) {
		return false;
	}
	std::list<CONNECTNODE> listCommu;
	pPLCSet = (MsgPLCSet *) pData;
	for (int nI = 0; nI < pPLCSet->connection_size(); nI++) {
		WMachine wMachine;
		if (!wMachine.enableFreeTag() && 
			(pPLCSet->connection(nI).plc().freetagflag() || 
			CSerialManager::get_instance().isFreeLabel(pPLCSet->connection(nI).plc().type()))) // 限制ig单机版使用自由标签
		{
			CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_NOT_FREETAG, cbl::format("This model does not support FreeTag!"));
			continue;
		}
		CONNECTNODE Commu;
		memset(&Commu, 0, sizeof(CONNECTNODE));
		Commu.nPortId = listCommu.size();
		doRead_Connect((void *)&pPLCSet->connection(nI), Commu);
		listCommu.push_back(Commu);
	}
	CCloudParse::get_instance().setCommu(listCommu);
	return true;
}

bool loadHmp(const std::string &sPrjPath) {
	MsgProjectInfo	Project;
	if (!UTILITY_NAMESPACE::CFile::ExistFile(sPrjPath)) {
		return false;
	}

	if (!IsProjSecure()) {
		XmlAndProto::XmlToClassData(XmlType_Project, sPrjPath, &Project);
	}
	else {
		std::string sbuffer;
		std::string sCpuId;
		if (CPlatCpu::getCpuId(sCpuId) != 0) {
			printf("%s:getCpuId error\r\n", __FUNCTION__);
			return false;
		}
		WSecure pSecure(sCpuId);
		pSecure.decryptNormalFile(sPrjPath.c_str(), sbuffer);
		XmlAndProto::SerializeToClassData(XmlType_Project, sbuffer, &Project);
	}

	if (Project.has_hmiset()) {
		hmiSetInfo((void *) &Project.hmiset());
	}
	if (Project.has_sysset()) {
		doRead_ProjInfo((void *) &Project.sysset());
	}
	if (Project.has_screenset()) {
		CHMIScreens::get_instance().parse((void *) &Project.screenset());
	}
	if (Project.has_info()) {
		SCRIPTMGR_NAMESPACE::ScriptModule::Load(-1, (void *) &Project.info());
	}
	if (Project.has_plcset()) {
		PLCInfoSet((void *) &Project.plcset());
	}
	return true;
}

void InitGlobalFunc() {
	GM_CALLBACK_T   conf;
	conf.scrIsOpen = std::bind(( bool( CHMIScreens:: * )( int, int ) )&CHMIScreens::isScreenRunning, &CHMIScreens::get_instance(), std::placeholders::_2, std::placeholders::_1);
	conf.scrIsExist = std::bind(&CHMIScreens::isScreenExist, &CHMIScreens::get_instance(), std::placeholders::_1);
	conf.scrSize = std::bind(&CHMIScreens::getScreenWidthAndHeight, &CHMIScreens::get_instance(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	conf.scrPopPos = std::bind(&CHMIScreens::getKeyboardPos, &CHMIScreens::get_instance(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	conf.scrExecKey = std::bind(&CHMIScreens::setKeystrokebuffer, &CHMIScreens::get_instance(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	conf.getPartArea = std::bind(&CHMIScreens::getPartOriginalRect, &CHMIScreens::get_instance(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	conf.addEvent = std::bind(( int( CEventDispatch:: * )( const EVENT_T& ) )&CEventDispatch::postEvent, &CEventDispatch::getInstance(), placeholders::_1);
	conf.addScrEvent = std::bind(( int( CEventDispatch:: * )( const hmiproto::hmievent& ) )&CEventDispatch::postEvent, &CEventDispatch::getInstance(), placeholders::_1);
	conf.scrGetTopScr = std::bind(&CHMIScreens::getTopScreenNo, &CHMIScreens::get_instance());
	GMExtern::setCallback(conf);
	std::string     sDLLPath;
#ifdef WIN32
	CState::GetModuleFilePath(sDLLPath);
	sDLLPath += "Plugin.dll";
#else
	sDLLPath = "/mnt/wecon/app/hmiapp/lib/libPlugin.so";
#endif

	if (CFile::ExistFile(sDLLPath) &&
		CPluginDLL::Load(sDLLPath)) {
		printf("load %s success\r\n", sDLLPath.c_str());
	}
	BaseExtern::SetAddScrEventFunc(std::bind(( int( CEventDispatch::* )( const hmiproto::hmievent& ) )&CEventDispatch::postEvent, &CEventDispatch::getInstance(), std::placeholders::_1));
	BaseExtern::SetGetScrAreaFunc(std::bind(&CHMIScreens::getScreenWidthAndHeight, &CHMIScreens::get_instance(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	return;
}

static void getLocation(int nLocation, std::string &sLocation) {
	if (nLocation >= 0) {
		sLocation = cbl::format("Screen %d", nLocation);
		return;
	}
	switch (nLocation) {
	case REG_SRC_SCRIPT:
		sLocation = "Script";
		break;
	case REG_SRC_ADDRESSMAP:
		sLocation = "Address Map";
		break;
	case REG_SRC_BITALARM:
		sLocation = "Bit Alarm";
		break;
	case REG_SRC_WORDALARM:
		sLocation = "Word Alarm";
		break;
	case REG_SRC_DATAGRP:
		sLocation = "Data Grp";
		break;
	case REG_SRC_SQL:
		sLocation = "SQL";
		break;
	case REG_SRC_HISTORYXYTHREAD:
		sLocation = "History XYThread";
		break;
	case REG_SRC_LIVETHREAD:
		sLocation = "Live Thread";
		break;
	case REG_SRC_RECIPEDATA:
		sLocation = "Recipe Data";
		break;
	case REG_SRC_USERAUTHORITY:
		sLocation = "UserAuthority";
		break;
	case REG_SRC_INSTALLMENT:
		sLocation = "Installment";
		break;
	case REG_SRC_OTHER:
		sLocation = "Other";
		break;
	case REG_SRC_CURVE:
		sLocation = "Curve";
		break;
	case REG_SRC_SMRECV:
		sLocation = "Short Message";
		break;
	case REG_SRC_BITEADDRMAP:
		sLocation = "Bit Address Map";
		break;
	case REG_SRC_WORDADDRMAP:
		sLocation = "Word Address Map";
		break;
	case REG_SRC_MMRECV:
		sLocation = "Mail Message";
		break;
	case REG_SRC_NULL:
		sLocation = "NULL";
		break;
	default:
		break;
	}
}

static void addrErrHandle(const CRegister& addr, unsigned int size, const RW_PARAM_T& rw) {
	if (rw.dwNCTime < MIN_NCTime) {
		return;
	}
	PROJSETUP*		pps = GetProjInfo();
	unsigned short  nValue = CRegisterRW::readWord(REG_TYPE_HS_, HSW_NONCMSG);
	if (nValue == 1 ||
		false == pps->bOnLine) {
		return;
	}
	std::string		sLocation = "";
	std::string     sReg = "";
	getLocation(rw.nSource, sLocation);
	registerToString_Old(addr, sReg);
	CMessageCenter::get_instance().insert(sLocation, sReg, (int) rw.dwNCTime);
}

static void InitHmiAddress() {
	PROJSETUP*  pps = GetProjInfo();
	if (NULL == pps) {
		return;
	}
	{
		REG_RW_CONF_T   conf;
		memset(&conf, 0, sizeof(conf));
		conf.bSwapAllDataHiLow = pps->BaseSet.bHiLowReverAllData;
		conf.bSwapFloatHiLow = pps->BaseSet.bFloatHiLowReverse;
		CRegisterRW::init(conf);
	}
	WMachine    wMachine;
	int         nUserNum = MAX_SeesionID_Low;
#ifdef WITH_UI
	if (true == wMachine.enableRemote()) {  // hmi允许远程
		nUserNum = MAX_SessionID_High;
	}
	else {
		nUserNum = MAX_SeesionID_Low;
	}
#else
	if (HMI_MODE == CPlatBoard::getDeviceMode()) {
		nUserNum = MAX_SessionID_High;
	}
	else if (VBOX_MODE == CPlatBoard::getDeviceMode()) {
		nUserNum = MAX_SeesionID_Low;
	}
#endif
	unsigned long   ulHdwRegSize = 0;
	unsigned long   ulHawRegSize = 0;
	unsigned long   ulHswRegSize = HSW_MAXHSWNUMBER;
	unsigned long   ulHuwRegSize = HUW_MAXHUWLEN;
	unsigned int    nPhyMemorySize = CPlatFlash::getPhyMemorySize();
	switch (nPhyMemorySize) {
	case 64: 
		{  // ie
			ulHdwRegSize = HDW_MAX_SIZE_Low;
			ulHawRegSize = HAW_MAX_SIZE_Low;
		}
		break;
	case 128:
	case 256:
	case 512: 
		{
			ulHdwRegSize = HDW_MAX_SIZE_High;
			ulHawRegSize = HAW_MAX_SIZE_High;
		}
		break;
	default: 
		{
			ulHdwRegSize = HDW_MAX_SIZE_High;
			ulHawRegSize = HAW_MAX_SIZE_High;
		}
		break;
	}

	/* HDW寄存器配置 */
	{
		CACHE_REG_CONF_T    conf;
		conf.type = REG_TYPE_HD_;
		conf.ulCacheSize = ulHdwRegSize;
		conf.funcChangeNotify = [] (const CRegister& addr, uint32_t lSize, const RW_PARAM_T& rw) {
			return CFileRecipeCtrl::get_instance().indexAddressChangeCallBack(addr, lSize, rw);
		};
		CEngineMgr::getInstance().add(conf);
	}

	if (false == cbl::isDir(SAVEREGFILEDIR)) {
		cbl::makeDir(SAVEREGFILEDIR);
	}

	/* HSW寄存器配置 */
	{
		FILE_REG_CONF_T    conf;
		conf.type = REG_TYPE_HS_;
		conf.ulCacheSize = ulHswRegSize;
		conf.ulSaveBegin = HSW_SAVE_START;
		conf.ulSaveSize = ulHswRegSize - HSW_SAVE_START;
		conf.ulSaveInteval = 2000;
		_snprintf(conf.szFilePath, sizeof(conf.szFilePath) - 1, "%s%s", SAVEREGFILEDIR, HMISAVEREGFILE_HSW);
		conf.funcWriteNotify = hswRegWriteCheck;
		conf.funcChangeNotify = hswRegChangeNotify;
		conf.funcLoadOldFile = loadHswRegFile;
		conf.funcRemoveOldFile = reMoveOldFile;
		CEngineMgr::getInstance().add(conf);
	}

	/* HAW寄存器配置 */
	{
		FILE_REG_CONF_T    conf;
		conf.type = REG_TYPE_HA_;
		conf.ulCacheSize = ulHawRegSize;
		conf.ulSaveBegin = 0;
		conf.ulSaveSize = ulHawRegSize;
		conf.ulSaveInteval = 2000;
		_snprintf(conf.szFilePath, sizeof(conf.szFilePath) - 1, "%s%s", SAVEREGFILEDIR, HMISAVEREGFILE_HAW);
		conf.funcChangeNotify = [](const CRegister& addr, uint32_t lSize, const RW_PARAM_T& rw) {
			return CFileRecipeCtrl::get_instance().indexAddressChangeCallBack(addr, lSize, rw);
		};
		conf.funcLoadOldFile = loadHawRegFile;
		conf.funcRemoveOldFile = reMoveOldFile;
		CEngineMgr::getInstance().add(conf);
	}

	/* HUW寄存器配置 */
	{
		CACHE_REG_CONF_T    conf;
		conf.type = REG_TYPE_HU_;
		conf.bUseFlag = true;
		conf.nFlag = -1;
		conf.ulCacheSize = ulHuwRegSize;
		for (int i = 0; i < nUserNum; i++) {
			CEngineMgr::getInstance().add(conf);
		}
	}
	/* PLC寄存器配置 */
	{
		WMachine    wMachine;
		for (int i = 0; i < 3; i++) {
			if (true == wMachine.isInvalidSn()) {
				wMachine.reload();
			}
			else {
				break;
			}
		}
		if (true == wMachine.isInvalidSn()) {
			vector<string> vecArgs;
			CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_GET_MACHINEID_ERR, "Failed to get Machine ID", vecArgs, -1, hmiproto::message_level_toplevel); 
			pps->bOnLine = false;
		}
		CSerialManager::get_instance().start();
		PLC_REG_CONF_T  conf;
		conf.type = REG_TYPE_DEV;
		conf.bOnline = pps->bOnLine;
		conf.listPlcInfoSet = CCloudParse::get_instance().getCommu();
		conf.sPrjPath = getPrjDir();
		conf.funcErrNotify = addrErrHandle;
		conf.funcRegRead = [](uint8_t type, const CRegister& reg, void* pvData, uint32_t size, RW_PARAM_T& rw) {
			return CRegisterRW::read(type, reg, pvData, size, rw);
		};
		conf.funcRegWrite = [](uint8_t type, const CRegister& reg, const void* pvData, uint32_t size, RW_PARAM_T& rw) {
			return CRegisterRW::write(type, reg, pvData, size, rw);
		};
		if (PLATFORM::HMI_MODE == PLATFORM::CPlatBoard::getDeviceMode()) {
			CEngineMgr::getInstance().add(conf);
		}
		else if (PLATFORM::VBOX_MODE == PLATFORM::CPlatBoard::getDeviceMode()) {
			CEngineMgr::getInstance().add(conf, LOAD_MODE_SINGLE);
		}
	}

	/* GPIO配置 */
	{
		//GPIOIN
		GPIO_REG_CONF_T conf;
		conf.type = REG_TYPE_X_;
#ifdef WIN32
		GetTempPathA(MAX_PATH, conf.szPath);
#else
		strcpy(conf.szPath, "/dev/WHAL/gpio/");
#endif // WIN32
		CEngineMgr::getInstance().add(conf);
		//GPIOOUT
		conf.type = REG_TYPE_Y_;
		CEngineMgr::getInstance().add(conf);
	}
	/* RPW寄存器配置 */
	{
		CTraditionalRecipe::get_instance().setRpwAddressConfig(loadRpwRegFile, reMoveOldFile);
		CTraditionalRecipe::get_instance().init();
	}
	CCompileAddress::get_instance().setAddressSize(ulHdwRegSize, ulHawRegSize);
}

int initProject_SAX(const std::string &sPrjPath) {
	if (!ProtoXML::XmlAndProto::XmlTagInit()) {
		return -1;
	}
	std::string		sProjectDir = getPrjDir();
	InitGlobalFunc();
	SCRIPTMGR_NAMESPACE::ScriptModule::Init(NULL);
	Installment_Init();
	CGraphic::get_instance().load(sProjectDir, IsProjSecure());
	unsigned short    nLang = CRegisterRW::readWord(REG_TYPE_HS_, HSW_LANGNUM);
	nLang = nLang < MAX_MultiLangNum ? nLang + 1 : 1;
	CMessageCenter::get_instance().start(sProjectDir + std::string("IMsgText.db"), nLang);
	if (!loadHmp(sPrjPath)) {
		return -2;
	}
	/* parse traditional recipe */
	string sRecipeXml = format("%sRecipe/Rep0000.rcp", sProjectDir.c_str());
	if (CFile::ExistFile(sRecipeXml)) {
		traditionalRecipeParse(sRecipeXml);
	}
	InitHmiAddress();
	return 0;
}

int initHMIProject(const std::vector<std::string> &vecParam) {
	PROJSETUP  *pps = GetProjInfo();
	if (!pps) {
		return -1;
	}
	std::string sPrjPath = "", sPrjDir = "";
#ifdef WIN32
	std::string	sWmtPath = "";
	char		szbuffer[MAX_PATH] = { 0 };
	int			nEmulatorType = 0;

	if (vecParam.size() >= 3) {
#ifdef _DEBUG
		CUnicode::mbs_to_utf8(vecParam[3].c_str(), szbuffer, MAX_PATH);
		sWmtPath = szbuffer;
#else
		sWmtPath = vecParam[3];
#endif
		nEmulatorType = std::stoi(vecParam[4]);
		pps->bOnLine = nEmulatorType;
		if (nEmulatorType == ONLINEEMULATOR) {
			CRateTimer::addTimer(RunScript_OnLine, nullptr, (int) ( &RunScript_OnLine ), 10000);
		}
	}
	if (sWmtPath.empty()) {
		return -3;
	}
	int nRtn = 0;
	if (std::string::npos != sWmtPath.find(".wmt3")) {
		std::string sKey;
		if (vecParam.size() >= 7) {
			sKey = vecParam[6];
		}
		if (( nRtn = UnTgzProject(sWmtPath, sKey, sPrjPath, sPrjDir) ) < 0) {
			return nRtn;
		}
	}
	else {
		if (( nRtn = UnzipProject(sWmtPath, sPrjPath, sPrjDir) ) < 0) {
			return nRtn;
		}
	}
	removeRecordDir();
#else
	if (vecParam.size() < 2) {
		return -6;
	}
	sPrjPath = vecParam[1] + std::string("/test.pi");
	sPrjDir = vecParam[1] + "/";
	pps->bOnLine = true;
#endif
	strncpy(pps->szProPath, sPrjPath.c_str(), FILENAME_MAX - 1);
	setPrjDir(sPrjDir);
	makeRecordDir();
	CUserMgr::getInstances().init();
	CSecureCtrl::getInstance().getAuthority().init();
	if (initProject_SAX(sPrjPath) < 0) {
		printf("HMITerm InitProject Fail!");
		return -7;
	}
	return 0;
}

int initBOXProject() {
	InitHmiAddress();
	adaptOldProjects();
	return CCloudParse::get_instance().load();
}

int runHMIEmptyProject() {
#ifdef _LINUX_
	btl::backlightEnable(false);
	btl::backlightBrightness(100);
#endif // _LINUX_

	CPlatShell::runCmd(FORCE_UPDATE_SHELL_SCRIPT, NULL);
	CLocalSocket::htmInit();
	return 0;
}

bool isHMIProjectEmpty() {
#ifdef _LINUX_
	if (CFile::ExistFile(FORCE_UPDATE_FILE)) {
		return false;
	}
	return true;
#else
	return false;
#endif
}