#include "common.h"
#include "macrosplatform/path.h"
#include "utility/utility.h"
#include "GlobalModule/GlobalModule.h"
#include "GlobalModule/HMINet/HMINet.h"
#include "macrosplatform/registerAddr.h"
#include "../HSWAddrOperate/HSWAddrOperate.h"
#include "hmiScreens/hmiScreens.h"
#include "ScriptMgr/ScriptMgr.h"
#include "LuaScript/LuaScript.h"
#include "webserver/webserver.h"
#include "../EventDispatcher/EventDispatch.h"
#include "../PluginDLL/PluginDLL.h"
#include "../webserverCallback/webserverCallback.h"
#include "secure/secure.h"
#include "../MqttThirdParty/MqttThirdParty.h"
#include "webserver/LanSafe.h"
#ifdef WIN32
#include "../Emulator/HmiEmulator.h"
#else
#include <sys/reboot.h>
#include "btl/btl.h"
#endif
#include "../cloud/cloud.h"
#include "../HJ212Control/HJ212Control.h"
#ifdef WITH_UI
#include "HmiGui/HmiGui/HmiGui.h"
#endif

using namespace UTILITY_NAMESPACE;
using namespace PLATFORM_NAMESPACE;
using namespace SECURE_NAMESPACE;
#ifdef WIN32
std::string getIMGPath() {
	std::string     sImgPath = "";
	std::string     sRunPath = "";
	wchar_t         szAppDir[MAX_PATH] = { 0 };
	char            cAppDir[MAX_PATH * 2] = { 0 };

	GetModuleFileNameW(NULL, szAppDir, MAX_PATH - 1);
	CUnicode::utf16_to_utf8((const char *) szAppDir, cAppDir, MAX_PATH * 2 - 1);
	sRunPath.append(cAppDir);
	sRunPath.append(EMULATORPATHFILE);
	if (CFile::ExistFile(sRunPath.c_str())) {
		sImgPath = EMULATORPATH;
		sImgPath.append(WECONA8_DIR);
	}
	else {
		wchar_t     szTempPath[MAX_PATH] = { 0 };
		char        cTempPath[MAX_PATH * 2] = { 0 };

		GetTempPathW(MAX_PATH - 1, szTempPath);
		CUnicode::utf16_to_utf8((char *) szTempPath, cTempPath, MAX_PATH * 2 - 1);
		sImgPath.append(cTempPath);
		sImgPath.append(TMPDIR);
		sImgPath.append("/");
	}
	return sImgPath;
}

void removeRecordDir() {
	cbl::removeDir(ALARMFILEDIR);
	cbl::removeDir(DATALOGFILEDIR);
	cbl::removeDir(SAVEREGFILEDIR);
	cbl::removeDir(RECIPEFILEDIR);
	cbl::removeDir(FORMULADIR);
	cbl::removeDir(RECIPETMPFILEDIR);
	cbl::removeDir(LIVETHRENDDBFILEDIR);
	cbl::removeDir(HISTORYXYTHRENDDBFILEDIR);
	cbl::removeDir(USERMGRNEWDIR);
	cbl::removeDir(INSTALLMENTDBFILEDIR);
	cbl::removeDir(DEBUGMSGDBFILEDIR);
	cbl::removeDir(SAVEALARMDATAPATH);
	cbl::removeDir(SAVEDATAGRPDATAPATH);
	cbl::removeDir(EMUUSBDIR);
	cbl::removeDir(EMUSDDIR);
	CFile::Delete(SCREENPROTECT);
	cbl::makeDir(EMUUSBDIR);
	cbl::makeDir(EMUSDDIR);
	CFile::Delete(FORBIDCHANGETIME);
	CFile::Delete(GRADE_PWD_CONF_FILE);
	CFile::Delete(WEATHER_PART_CONF_DIR);
}

void *RunScript_OnLine(void *arg) {
	static unsigned long long ullRunTime = 0;
	static unsigned long long ullAddTime = 0;
	static unsigned long long ullOldTime = 0;
	unsigned long long ullCurTime = 0;
	ullCurTime = CTime::get_instance().getTickCount();
	if (ullOldTime == 0) {
		ullOldTime = ullCurTime;
	}
	if (ullRunTime == 0) {
		ullRunTime = ullCurTime;
	}
	if (ullCurTime - ullRunTime > 1800000 || ullAddTime > 1800000) {
		exit(0);
	}
	ullAddTime = ullAddTime + ullCurTime - ullOldTime;
	ullOldTime = ullCurTime;
	return NULL;
}

void MessageBoxW_T(int nType, int nLanguage) {
	switch (nType) {
	case -2:
		{
			if (nLanguage == LANGUANE_JTCHINESE ||
				nLanguage == LANGUANE_FTCHINESE) {
				MessageBoxW(NULL, L"模拟程序已经打开,若要新的模拟需要关闭当前窗口重试", L"错误", MB_OK | MB_ICONERROR);
			}
			else {
				MessageBoxW(NULL, L"Simulation window exists, please close it before opening a new window", L"Error", MB_OK | MB_ICONERROR);
			}
		}
		break;
	case -3:
		{
			if (nLanguage == LANGUANE_JTCHINESE || nLanguage == LANGUANE_FTCHINESE) {
				MessageBoxW(NULL, L"请重新选择工程文件（工程路径出错或路径过长）", L"", MB_OK | MB_ICONERROR);
			}
			else {
				MessageBoxW(NULL, L"Please reselect the project file (project path error or too long)", L"", MB_OK | MB_ICONERROR);
			}
		}
		break;
	case -4:
		{
			MessageBoxW(NULL, L"GetTempPath() FAILED!", L"", MB_OK | MB_ICONERROR);
		}
		break;
	case -5:
		{
			if (nLanguage == LANGUANE_JTCHINESE ||
				nLanguage == LANGUANE_FTCHINESE) {
				MessageBoxW(NULL, L"工程校验失败", L"", MB_OK | MB_ICONERROR);
			}
			else {
				MessageBoxW(NULL, L"Failed to check project", L"", MB_OK | MB_ICONERROR);
			}
		}
		break;
	case -7:
		{
			MessageBoxW(NULL, L"TagDictInitErr", L"Err", MB_OK | MB_ICONERROR);
		}
		break;
	default:
		break;
	}
}

void openConsole() {
	if (cbl::isDir(getPrjDir() + std::string("Script/")) ||
		isShortCutKeyValid()) {
		if (AllocConsole() == false) {
			PRINTAPP("AllocConsole ret false(GetLastError = %d)\n", GetLastError());
		}
		else {
			freopen("CON", "w", stdout);
			freopen("CON", "w", stderr);
		}
	}
}
#endif

int getProjectPath(std::string &sPath) {
#ifdef _WIN32
	char	szPath[MAX_PATH] = { 0 };
	CState::GetTempPathCos(MAX_PATH, szPath);
	sPath = std::string(szPath) + TMPDIR + std::string("/");
#else
	sPath = getPrjDir();
#endif
	return 0;
}

void* loadAllIMGS() {
	while (!CGraphic::get_instance().isAllLoad()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	return (void *) CGraphic::get_instance().getImageList();
}

int getLanguageId(const std::string &sPath, const std::string &sKey, unsigned short &nLanguage) {
	long long llFileSize = 0;
	std::string sbuffer;
	if (( llFileSize = cbl::getFileSize(sPath) ) < 0) {
		return -1;
	}
	if (cbl::readFileOnce64(sPath, 0, SEEK_SET, llFileSize, sbuffer) < 0) {
		return -2;
	}
	std::vector<std::string> vec;
	if (cbl::split(sbuffer, "\n", &vec) < 0 ||
		vec.empty()) {
		return -3;
	}
	for (auto &iter : vec) {
		std::size_t npos = iter.find(sKey);
		if (npos != std::string::npos) {
			std::string sValue = iter.substr(npos + sKey.size() + 1);
			try {
				nLanguage = (unsigned short) std::stoi(sValue);
			}
			catch (const std::exception& e) {
				printf("%s\r\n", e.what());
				return -4;
			}
			return 0;
		}
	}
	return -5;
}

void makeRecordDir() {
	cbl::makeDir(ALARMFILEDIR);
	cbl::makeDir(DATALOGFILEDIR);
	cbl::makeDir(DATALOGFILEMEMDIR);
	cbl::makeDir(LIVETHRENDDBFILEDIR);
	cbl::makeDir(HISTORYXYTHRENDDBFILEDIR);
	cbl::makeDir(USERMGRNEWDIR);
	cbl::makeDir(INSTALLMENTDBFILEDIR);
	cbl::makeDir(MESSAGEDBFILEDIR);
	cbl::makeDir(DEBUGMSGDBFILEDIR);
	cbl::makeDir(CUSTOMFILEDIR);

	{
		std::string sPath = getPrjDir() + std::string("Recipe/");
		if (!cbl::isDir(RECIPEFILEDIR)) {
#ifdef WIN32
			cbl::removeDir(RECIPETMPFILEDIR);
#else
			cbl::removeDir(RECIPETMPFILEDIR, true);
#endif
			CDir::CopyFolder(sPath, RECIPEFILEDIR, "*.*", true);
			CDir::CopyFolder(sPath, RECIPETMPFILEDIR, "*.*", true);
		}
	}
}

static int parseDELTOUCH(unsigned short &nSeconds, unsigned short &nLanguage) {
	std::string sPath, sbuffer;
	std::vector<std::string> vec;

	CHmiMedia::getUdiskPath(sPath);
	sPath += std::string("DelTouch.dat");

	if (cbl::readFileOnce64(sPath, 0, SEEK_SET, 32, sbuffer) < 0 ||
		sbuffer.empty() ||
		sbuffer.size() > 32) {
		return -1;
	}
	sbuffer = cbl::removeCRLF(sbuffer);
	if (cbl::split(sbuffer, " ", &vec) < 0 ||
		vec.empty()) {
		return -2;
	}
	try {
		nSeconds = (unsigned short) std::stoi(vec[0]);
	}
	catch (const std::exception& e) {
		printf("%s:%s\r\n", __FUNCTION__, e.what());
	}
	if (nSeconds <= 0) {
		nSeconds = 30;
	}
	else if (nSeconds > 3 * 60) {
		nSeconds = 3 * 60;
	}

	if (vec.size() == 1) {
		getProjectPath(sPath);
		sPath += LANGUAGEFILEPATH;
		if (getLanguageId(sPath, LANGUAGEID, nLanguage) < 0) {
			return -3;
		}
	}
	else if (vec.size() >= 2) {
		try {
			nLanguage = (unsigned short) std::stoi(vec[1]);
		}
		catch (const std::exception& e) {
			printf("%s:%s\r\n", __FUNCTION__, e.what());
		}
	}
	if (nLanguage == 1 || nLanguage == 2) { // 1表示中文，2表示用英文
		nLanguage--;// 外面判断0才是中文，其它都是英文，所以要减1
	}
	return 0;
}

static bool isFirstTimeUdisk() {
	static bool bExistFlag = false;
	static unsigned int nTimes = 0;

	bool bExist = CHmiMedia::isUSBExsit();
	if (bExistFlag != bExist) {
		bExistFlag = bExist;
		if (bExistFlag) {
			nTimes++;
		}
	}
	if (nTimes == 1) {
		return true;
	}
	return false;
}

static int delCalibrationFile() {
#ifdef _LINUX_
	char *unlinkFile = NULL;
	if (( unlinkFile = getenv("TSLIB_CALIBFILE") ) != nullptr) {
		unlink(unlinkFile);
	}
	else {
		unlink("/etc/pointercal");
	}
	if (( unlinkFile = getenv("TSLIB_WECONFILE") ) != nullptr) {
		unlink(unlinkFile);
	}
	else {
		unlink("/etc/ts_point");
	}
#endif
	return 0;
}

static int calibrationHMI() {
	static unsigned long long ullTimestamp = CTime::get_instance().getTickCount(), ullNowTimestamp = 0;
	static bool	bFlag = false;
	static unsigned short nSeconds = 0, nLanguage = 0;
	if (bFlag) {
		return -1;
	}
	if (CTime::get_instance().getTickCount() - ullTimestamp >= 3 * 60 * 1000) {
		return -2;
	}
	if (!isFirstTimeUdisk()) {
		return -3;
	}
	if (nSeconds == 0 && nLanguage == 0) {
		if (parseDELTOUCH(nSeconds, nLanguage) < 0) {
			bFlag = true;
			return -4;
		}
	}
	if (CTime::get_instance().getTickCount() - ullNowTimestamp > 1000) {
		std::string sbuffer = "";
		ullNowTimestamp = CTime::get_instance().getTickCount();
		if (nLanguage == 0) {
			sbuffer = cbl::format("%d秒后进行校准", nSeconds);
		}
		else {
			sbuffer = cbl::format("Calibrate after %d seconds", nSeconds);
		}
		CRegisterRW::writeBit(REG_TYPE_HS_, HSW_CLEARMESSAGE, 0, 1);
		CMessageCenter::get_instance().insert(hmiproto::message_type_clear, "");
		CMessageCenter::get_instance().insert(hmiproto::message_type_insert, sbuffer);
		if (nSeconds == 0) {
			bFlag = true;
			delCalibrationFile();
			CRegister addr(REG_TYPE_HS_, HSW_RESET, DATA_TYPE_WORD);
			if (VBOX_MODE == PLATFORM::CPlatBoard::getDeviceMode()) {
				addr.setAddr(HSW_RESET_R);
			}
			unsigned short nValue = 1;
			RW_PARAM_T rw;
			CRegisterRW::write(RW_FORMAT_INT, addr, &nValue, 1, rw);
			return 0;
		}
		nSeconds = nSeconds - 1;
	}

	return 0;
}

static int getInstallmentLog() {
	static unsigned long long ullTimestamp = CTime::get_instance().getTickCount();
	std::string sPath = "", sLogPath = "";
	if (CTime::get_instance().getTickCount() - ullTimestamp >= 3 * 60 * 1000) {
		return -1;
	}
	if (!isFirstTimeUdisk()) {
		return -2;
	}
	CHmiMedia::getUdiskPath(sPath);
	if (cbl::getFileSize(sPath + std::string("GetInstallmentLog.dat")) < 0) {
		return -3;
	}
	if (cbl::getFileSize(sPath + std::string("INSTALLMENTLOG")) < 0) {
		return -4;
	}
	if (!CFile::Copy(INSTALLMENTLOG, sPath + std::string("InstallmentLog.db"), false)) {
		return -5;
	}
	return 0;
}

void runSystemTimer() {
	if (VBOX_MODE != CPlatBoard::getDeviceMode()) {
		CSecureCtrl::getInstance().monitor();
		calibrationHMI();
		getInstallmentLog();
	}
	HswAddrMonitor();
}

void setInitialScreenId() {
	PROJSETUP*  pps = GetProjInfo();
	bool        bRet = false;
	unsigned short        wScrNo = 0;
	if (NULL == pps || 
		false == pps->BaseSet.bUseSysContrl) {
		return;
	}
	RW_PARAM_T    rw = { RW_MODE_CACHE_DEV };
	bRet = CRegisterRW::read(RW_FORMAT_INT, pps->BaseSet.addrSysContrl, &wScrNo, 1, rw);
	if (true == bRet) {
		CHMIScreens::get_instance().setInitialScreen(wScrNo);
	}
	return;
}

void *runTriggerActionScript(void *arg) {
	SCRIPTMGR_NAMESPACE::ScriptModule::RunTrigger();
	CHMIScreens::get_instance().runLuaPollScript();
	return nullptr;
}

void *runTimerActionScript(void *) {
	SCRIPTMGR_NAMESPACE::ScriptModule::RunTimer();
	return nullptr;
}

void *runBackgroundLuaScript(void *arg) {
	runPollingBgScript();
	return nullptr;
}

static bool isSystemDataPartitionReadonly() {
#define	SYS_READONLY_PATH	"/mnt/data/IsReadOnly.txt"
#ifdef WIN32
	return false;
#else
	static unsigned long long ullTime = CTime::get_instance().getTickCount();
	static unsigned short nTimes = 0;
	int fd = 0;

	if (CTime::get_instance().getTickCount() - ullTime >= 60 * 1000) {
		HMI_UBI_INFO_T  ubi = { 0 };
		int nRtn = CPlatFlash::getUbiInfo(ubi);
		ullTime = CTime::get_instance().getTickCount();
		if (nRtn < 0) {
			printf("%s:get ubi error:%d\r\n", __FUNCTION__, nRtn);
			return false;
		}
		if (ubi.nReservedBlocks > 2) {
			return false;
		}
		if (cbl::locateFile64(SYS_READONLY_PATH, true, true, 0, SEEK_SET, fd) < 0) {
			nTimes++;
		}
		else {
			nTimes = 0;
		}
		if (fd > 0) close(fd);
	}
	if (nTimes >= 3) {
		CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_READONLY, "The data partition is readonly");
		return true;
	}
	return false;
#endif
}

static int runBadBlockSystemDataPartition() {
	static unsigned long long ullTimestamp = CTime::get_instance().getTickCount();
	static unsigned short nTimes = 0;
	static bool	bFirstTime = true;
	unsigned short nReservedBlocks = 0;

	if (CTime::get_instance().getTickCount() - ullTimestamp >= 24 * 60 * 60 * 1000 ||
		bFirstTime) {
		ullTimestamp = CTime::get_instance().getTickCount();
		bFirstTime = false;
		HMI_UBI_INFO_T  ubi = { 0 };
		int nRtn = CPlatFlash::getUbiInfo(ubi);
		if (nRtn < 0) {
			printf("%s:get ubi error:%d\r\n", __FUNCTION__, nRtn);
			return -1;
		}
		nReservedBlocks = (unsigned short)ubi.nReservedBlocks;
		CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_CHECKBLOCK, 0, &nReservedBlocks, 1);
		if (nReservedBlocks <= 2) {
			nTimes++;
		}
		else {
			nTimes = 0;
		}
	}
	if (nTimes >= 3) {
		vector<string> vecArgs;
		vecArgs.push_back(to_string(nReservedBlocks));
		CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_FLASH_BAD, cbl::format("Flash is running out of the remaining space which may result in data loss, please send it back to the manufacturers for maintenance and replacement %d", nReservedBlocks), vecArgs);
	}
	return 0;
}

void *runDataRecordCollectionTask(void *arg) {

	CDataRecordCollect::get_instance().collect();
	CAlarmCollect::get_instance().collect();
	CFormulaControl::get_instance().formulaAddrMonitor();
	CWPoint::get_instance().monitor();
	CCMPoint::get_instance().monitor();
	if (VBOX_MODE != CPlatBoard::getDeviceMode()) {
		if (isSystemDataPartitionReadonly() &&
			CRateTimer::timerExits((int) ( &runDataRecordCollectionTask ))) {
			CRateTimer::removeTimer((int) ( &runDataRecordCollectionTask ));
		}
		runBadBlockSystemDataPartition();
		CHisXYTrendCollect::get_instance().collect();
		CLiveTrendCollect::get_instance().collect();
		CFileRecipeCtrl::get_instance().monitor();
		PrintModule_Monitor();
		UploadTool_Mount(GetProjInfo() != nullptr ? GetProjInfo()->BaseSet.nStorePattern : 0);
		CCustomDLL::RecordFunc();
	}
	return nullptr;
}

static void *runAddrmapTask(void *arg) {
	CAddressmaps::get_instance().run();
	return nullptr;
}

int runAddrmap() {
	std::string sPath;
	getProjectPath(sPath);
	sPath = sPath + std::string("SourAddrToDestAddrInfo.xml");
	if (CAddressmaps::get_instance().start(sPath) < 0) {
		return -1;
	}
	CRateTimer::addTimer(runAddrmapTask, NULL, (int) ( &runAddrmapTask ), 300);
	return 0;
}

void *runDataRecordStorageTask(void *arg) {
	if (VBOX_MODE != CPlatBoard::getDeviceMode()) {
		CFileCopy::get_instance().run();
		CRecordCopy::get_instance().monitor();
		CCustomDLL::SaveFunc();
		CHisXYTrendCollect::get_instance().save();
		CLiveTrendCollect::get_instance().save();
		COperationLogRecord::get_instance().save();
	}
	CAlarmCollect::get_instance().save();
	CDataRecordCollect::get_instance().save();
	return nullptr;
}

static void runPluginFunc() {
	if (!CPluginDLL::IsLoad()) {
		return;
	}
	PluginShortMessage&     shortMsg = CPluginDLL::GetMsgFunc();
	PluginMailMessage&      mailMsg = CPluginDLL::GetMailFunc();
	PluginAutoClick&        autoClick = CPluginDLL::GetClickFunc();
	PLUGINAUDIOMODULE_T&	audioMsg = CPluginDLL::GetAudioModuleFunc();
	PLUGINIPCAMERAMODULE_T&	ipCameraModule = CPluginDLL::GetIPCameraModuleFunc();
	if (true == shortMsg.bRun &&
		NULL != shortMsg.Collect) {
		shortMsg.Collect();//20161129 LYQ 短信模块
	}
	if (true == mailMsg.bRun &&
		NULL != mailMsg.Collect) {
		mailMsg.Collect();//20180822 liuhw 邮件模块
	}
	if (audioMsg.bRun &&
		nullptr != audioMsg.collect) {
		audioMsg.collect();//20220414 linzhen 音频模块
	}
	if (ipCameraModule.bRun &&
		nullptr != ipCameraModule.collect) {
		ipCameraModule.collect();	// 20220421 chengh 摄像头搜索模块
	}
	if (true == autoClick.bLoad &&
		NULL != autoClick.Play) {
		unsigned short wScreenID = CRegisterRW::readWord(REG_TYPE_HS_, HSW_SCRJMPID);
		autoClick.Play(wScreenID);
	}
	return;
}

void *runRoutineTask(void *arg) {
	unsigned short	nLanguage = CRegisterRW::readWord(REG_TYPE_HS_, HSW_MSGTIPLANGE);
	if (nLanguage == 0) {
		std::string sPath;
		unsigned short nLanguageId = 0;
		getProjectPath(sPath);
		sPath += LANGUAGEFILEPATH;
		getLanguageId(sPath, LANGUAGEID, nLanguageId);
		if (nLanguageId == 0) {
			nLanguage = 2;
		}
		else {
			nLanguage = 1;
		}
	}
	if (nLanguage > 2) {
		nLanguage = 2;
	}
	CWebServer::get_instance().setLanguage(nLanguage);
	CMessageCenter::get_instance().setConsoleLanguage(nLanguage);
	if (CMessageCenter::get_instance().getFlagAttr()) {
		hmiproto::hmievent			event;
		event.set_type(HMIEVENT_DISPLAYMSG);
		CEventDispatch::getInstance().postEvent(event);
	}
	CHMIScreens::get_instance().setMessageTipLanguage(nLanguage);
	CHJ212Control::get_instance().setLanguage(nLanguage);
	Traffic::GetInstance().Run();
	runPluginFunc();
	Installment_Monitor();
	return nullptr;
}

/* set storage notify
**/
int setRecordStorageNotify() {
	LPPROJSETUP pps = GetProjInfo();
	if (nullptr == pps) {
		return -1;
	}
	if (STORAGE_SD == pps->BaseSet.nStorePattern) {
		CHmiMedia::addNotifyCallBack(nullptr, bind(&CAlarmCollect::updateStoragePath, &CAlarmCollect::get_instance(), placeholders::_1));
		CHmiMedia::addNotifyCallBack(nullptr, bind(&CDataRecordCollect::updateStoragePath, &CDataRecordCollect::get_instance(), placeholders::_1));
		CHmiMedia::addNotifyCallBack(nullptr, bind(&CHisXYTrendCollect::updateStoragePath, &CHisXYTrendCollect::get_instance(), placeholders::_1));
		CHmiMedia::addNotifyCallBack(nullptr, bind(&COperationLogRecord::updateStoragePath, &COperationLogRecord::get_instance(), placeholders::_1));
		if (pps->BaseSet.bIsUseOptLogFunc) {
			CHmiMedia::addNotifyCallBack(nullptr, bind(&COperationLogRecord::updateStoragePath, &COperationLogRecord::get_instance(), placeholders::_1));
		}
	}
	else if (STORAGE_UDISK == pps->BaseSet.nStorePattern) {
		CHmiMedia::addNotifyCallBack(bind(&CAlarmCollect::updateStoragePath, &CAlarmCollect::get_instance(), placeholders::_1), nullptr);
		CHmiMedia::addNotifyCallBack(bind(&CDataRecordCollect::updateStoragePath, &CDataRecordCollect::get_instance(), placeholders::_1), nullptr);
		CHmiMedia::addNotifyCallBack(bind(&CHisXYTrendCollect::updateStoragePath, &CHisXYTrendCollect::get_instance(), placeholders::_1), nullptr);
		CHmiMedia::addNotifyCallBack(bind(&COperationLogRecord::updateStoragePath, &COperationLogRecord::get_instance(), placeholders::_1), nullptr);
	}
	return 0;
}

static void showCursor(bool bVal) {
	hmiproto::hmievent  event;
	event.set_type(HMIEVENT_SHOWCURSOR);
	event.set_cursor(bVal);
#ifndef WIN32
	CEventDispatch::getInstance().postEvent(event);
#endif
	return;
}

static bool InitPluginShortMessage() {
	std::string sPath;
	if (!CPluginDLL::IsLoad()) {
		return false;
	}
	getProjectPath(sPath);
	UTILITY_NAMESPACE::CDLL& dll = CPluginDLL::GetDLL();
	PluginShortMessage&     shortMsg = CPluginDLL::GetMsgFunc();
	if (false == CFile::ExistFile(sPath + std::string("SmInfo.xml"))) {
		return false;
	}
	shortMsg.Init = (SM_Init) dll.GetDllLibraryFun("ShortMessage_Init");
	if (shortMsg.Init &&
		shortMsg.Init(sPath + std::string("SmInfo.xml"))) {
		shortMsg.DeInit = (SM_DeInit) dll.GetDllLibraryFun("ShortMessage_DeInit");
		shortMsg.Collect = (SM_Collect) dll.GetDllLibraryFun("ShortMessage_Collect");
		shortMsg.bRun = true;
		return true;
	}
	return false;
}

static bool InitPluginMailMessage() {
	std::string sPath;
	if (!CPluginDLL::IsLoad()) {
		return false;
	}
	getProjectPath(sPath);
	UTILITY_NAMESPACE::CDLL& dll = CPluginDLL::GetDLL();
	PluginMailMessage&      mailMsg = CPluginDLL::GetMailFunc();
	if (false == CFile::ExistFile(sPath + std::string("NewMmSet.xml"))) {
		return false;
	}

	mailMsg.Init = (MM_Init) dll.GetDllLibraryFun("MailMessage_Init");
	if (mailMsg.Init &&
		mailMsg.Init(sPath + std::string("NewMmSet.xml"))) {
		mailMsg.DeInit = (MM_DeInit) dll.GetDllLibraryFun("MailMessage_DeInit");
		mailMsg.Collect = (MM_Collect) dll.GetDllLibraryFun("MailMessage_Collect");
		mailMsg.bRun = true;
		return true;
	}
	return true;
}

int InitPluginAudioModule() {
	std::string sPath;
	if (!CPluginDLL::IsLoad()) {
		return -1;
	}
	getProjectPath(sPath);
	UTILITY_NAMESPACE::CDLL& dll = CPluginDLL::GetDLL();
	PLUGINAUDIOMODULE_T&     audioModule = CPluginDLL::GetAudioModuleFunc();
	if (!CFile::ExistFile(sPath + std::string("AudioSetInfo.xml"))) {
		return -2;
	}
	audioModule.init = (AM_INIT) dll.GetDllLibraryFun("AudioPlay_Init");
	if (nullptr == audioModule.init ||
		!audioModule.init(sPath + std::string("AudioSetInfo.xml"))) {
		return -3;
	}
	audioModule.destroy = ( bool(*)( ) )dll.GetDllLibraryFun("AudioPlay_Destroy");
	audioModule.collect = ( void(*)( ) )dll.GetDllLibraryFun("AudioPlay_Collect");
	audioModule.bRun = true;
	return 0;
}

static int InitPluginIPCameraModule() {
	if (!CPluginDLL::IsLoad()) {
		return -1;
	}
	UTILITY_NAMESPACE::CDLL&	dll = CPluginDLL::GetDLL();
	PLUGINIPCAMERAMODULE_T&		ipCameraModule = CPluginDLL::GetIPCameraModuleFunc();
	ipCameraModule.init = ( bool(*)( ) )dll.GetDllLibraryFun("IPCamera_init");
	if (nullptr == ipCameraModule.init ||
		!ipCameraModule.init()) {
		return -2;
	}
	ipCameraModule.destroy = ( bool(*)( ) )dll.GetDllLibraryFun("IPCamera_destroy");
	ipCameraModule.collect = ( void(*)( ) )dll.GetDllLibraryFun("IPCamera_collect");
	ipCameraModule.bRun = true;
	return 0;
}

/* get storage path
**/
static int getStoragePath(string& sStorageParh) {
	LPPROJSETUP pps = GetProjInfo();
	if (nullptr == pps) {
		sStorageParh.assign(FLASHROOTDIR);
	}
	string sSavePath = "";
	switch (pps->BaseSet.nStorePattern) {
	case STORAGE_FLASH: {
			sStorageParh.assign(FLASHROOTDIR);
			break;
		}
	case STORAGE_SD: {
			string sSDParh;
			CHmiMedia::getSDPath(sSDParh);
			sStorageParh.assign(sSDParh);
			break;
		}
	case STORAGE_UDISK: {
			string sUdiskParh;
			CHmiMedia::getUdiskPath(sUdiskParh);
			sStorageParh.assign(sUdiskParh);
			break;
		}
	default:
		break;
	}
	return 0;
}

/* record init
**/
int recordInit() {
	LPPROJSETUP pps = GetProjInfo();
	if (nullptr == pps) {
		return -1;
	}
	/* set path */
	string sStoragePath;
	getStoragePath(sStoragePath);
	CDataRecordCollect::get_instance().setSavePath(pps->BaseSet.nStorePattern, sStoragePath);
	if (VBOX_MODE != CPlatBoard::getDeviceMode()) {
		/* set record error notify */
		setMessageNotify([ ] (int nType, const std::string& sBuffer, int nClientId, int nLevel) {
			return CMessageCenter::get_instance().insert(( hmiproto::message_type )nType, sBuffer, nClientId, ( hmiproto::message_level )nLevel);;
		});
		CHisXYTrendCollect::get_instance().setSavePath(sStoragePath);
		/* parse */
		setRecipeParseFunc(recipeDataParse);
		loadAttribute(getPrjDir() + std::string("ReportRecord.xml"));
		/* init */
		CDataRecordCollect::get_instance().setHeadText();
		CAlarmCollect::get_instance().setHeadText();
		CLiveTrendCollect::get_instance().init();
		CHisXYTrendCollect::get_instance().init();
		if (pps->BaseSet.bIsUseOptLogFunc) {
			COperationLogRecord::get_instance().init(sStoragePath);
		}
	}
	CAlarmCollect::get_instance().init(pps->BaseSet.nStorePattern, sStoragePath);
	CDataRecordCollect::get_instance().init();
	/* start */
	CAlarmCollect::get_instance().start();
	CDataRecordCollect::get_instance().start();
	return 0;
}

static void setSysFreekbytes() {
#ifndef WIN32//模拟的时候不需要运行
#define SYSCMD_64M_FREE_KBYTES			"echo 1024 > /proc/sys/vm/min_free_kbytes"
	//flash系列最小剩余内存设置3000/5000 
#define SYSCMD_FLASHMIN_FREE_KBYTES		"echo 5120 > /proc/sys/vm/min_free_kbytes"
	//EMMC系列最小剩余内存设置8000/9000
#define SYSCMD_EMMCMIN_FREE_KBYTES		"echo 10240 > /proc/sys/vm/min_free_kbytes"
	int		nPhyMemorySize = CPlatFlash::getPhyMemorySize();
	switch (nPhyMemorySize) {
	case 64:
		{
			CPlatShell::runCmd(SYSCMD_64M_FREE_KBYTES, NULL);
		}
		break;
	case 128:
		{
			CPlatShell::runCmd(SYSCMD_FLASHMIN_FREE_KBYTES, NULL);    //设置预留5M
		}
		break;
	case 512:
		{
			CPlatShell::runCmd(SYSCMD_EMMCMIN_FREE_KBYTES, NULL);    //设置预留10M
		}
		break;
	default:
		{
			CPlatShell::runCmd(SYSCMD_FLASHMIN_FREE_KBYTES, NULL);    //设置预留5M
		}
		break;
	}
#endif
}

int initLanSafe() {
	LPPROJSETUP pps = GetProjInfo();
	LANPWORD lp = pps->LANPassword;
	LANSAFEINFO stLanSafeInfo;
	stLanSafeInfo.bEnable = lp.byEnable;
	stLanSafeInfo.sUsrName = lp.szAppleId;
	stLanSafeInfo.sPassword = lp.szPassword;
	stLanSafeInfo.bIsHMI = true;
	CLanSafe::getInstance().init(stLanSafeInfo);
	return 0;
}

void *runCloudTimer(void *arg) {
	CCloudOperator::get_instance().onTimer();
	return nullptr;
}

static int startCloudParse() {
	if (CCloudParse::get_instance().start() < 0) {
		printf("%s:cloud start error\r\n", __FUNCTION__);
		return -1;
	}
	if (CWPoint::get_instance().setReportFunc(bind(&CCloudOperator::setRealData, &CCloudOperator::get_instance(), placeholders::_1)) < 0) {
		printf("%s:set real report func error\r\n", __FUNCTION__);
		return -2;
	}
	if (setHistoryDataNotify(bind(&CCloudOperator::setHistoryGroupPeriod, &CCloudOperator::get_instance(), placeholders::_1, placeholders::_2), \
		[ ](MONITOR_GROUP_T &monitorGroup) {
		std::list<MONITOR_GROUP_T> listMonitorGroup;
		listMonitorGroup.push_back(monitorGroup);
		return CCloudOperator::get_instance().setHistoryData(listMonitorGroup);
	}) < 0) {
		printf("%s:set history func error\r\n", __FUNCTION__);
		return -3;
	}
	if (setAlarmNotify(bind(&CCloudOperator::setAlarmConfigCondition, &CCloudOperator::get_instance(), placeholders::_1, placeholders::_2)
		, bind(&CCloudOperator::setAlarmData, &CCloudOperator::get_instance(), placeholders::_1)) < 0) {
		printf("%s:set alarm func error\r\n", __FUNCTION__);
		return -4;
	}
	if (CHMINet::get_instance().setCallback(bind(&CCloudOperator::updateWifiSiganl, &CCloudOperator::get_instance(), placeholders::_1)) < 0) {
		printf("%s:set wifi signal call back error\r\n", __FUNCTION__);
		return -4;
	}
	return CRateTimer::addTimer(runCloudTimer, NULL, (int)(&runCloudTimer), 500);
}

void *runOtherTask(void *arg) {
	if (VBOX_MODE != CPlatBoard::getDeviceMode()) {
		LPPROJSETUP pps = GetProjInfo();
		if (NULL == pps) {
			return NULL;
		}
		if (pps->BaseSet.bIsShowCursor) {
			showCursor(pps->BaseSet.bIsShowCursor);
		}
		CHMIScreens::get_instance().setOptLogStatus(pps->BaseSet.bIsUseOptLogFunc);
		CSecureCtrl::getInstance().init();
		PlatKeyBoardDev::Init(pps->BGEnterTime.bIsKeyBoard, pps->BaseSet.bIsShowCursor, pps->BGEnterTime.bShortCutKey);
		CHmiMedia::init();
		setRecordStorageNotify();
		Traffic::GetInstance().Init();
		CCustomDLL::Load();
		CCustomDLL::LuaLibFunc(getLuaState(LuaScriptType_Main));
		InitPluginShortMessage();
		InitPluginMailMessage();
		InitPluginAudioModule();	// 音频模块初始化
		InitPluginIPCameraModule();	// 视频搜索模块初始化
		CClearLcd::init();
		CFileRecipeCtrl::get_instance().init();
		startWebServer();
		initLanSafe();
#ifdef _LINUX_
		CFtpServer::getInstance().init();
#endif
		CWeatherInfo::getInstance().start();
	}
	/* record init */
	recordInit();
	setSysFreekbytes();     //设置预留的最小内存,防止内存不足时杀死进程，导致死机   chenfl   2017.07.07
// #ifdef WITH_UI
// 	{
// 		COMMU_MGR_CONFIG_T config;
// 		config.pflistGetCommu = []() {
// 			return CCloudParse::get_instance().getCommu();
// 		};
// 		config.pfnUnload = [](const CONNECTNODE &commectNode) {
// 			int nRtn = 0;
// 			std::list<CONNECTNODE> listCommu = CCloudParse::get_instance().getCommu();
// 			for (auto &iter : listCommu) {
// 				if (iter.nCommuId != commectNode.nCommuId) {
// 					continue;
// 				}
// 				if ((nRtn = CCloudParse::get_instance().unloadCommu(iter, false)) < 0) {
// 					printf("%s:unload commu error:%d\r\n", __FUNCTION__, nRtn);
// 					return -1;
// 				}
// 				if ((nRtn = CCloudParse::get_instance().setCommu(listCommu)) < 0) {
// 					printf("%s:set commu error:%d\r\n", __FUNCTION__, nRtn);
// 					return -2;
// 				}
// 				break;
// 			}
// 			return 0;
// 		};
// 		config.pfnUnloadAll = []() {
// 			int nRtn = 0;
// 			std::list<CONNECTNODE> listCommu = CCloudParse::get_instance().getCommu();
// 			for (auto &iter : listCommu) {
// 				if ((nRtn = CCloudParse::get_instance().unloadCommu(iter, false)) < 0) {
// 					printf("%s:unload commu error:%d\r\n", __FUNCTION__, nRtn);
// 					return -1;
// 				}
// 			}
// 			if ((nRtn = CCloudParse::get_instance().setCommu(listCommu)) < 0) {
// 				printf("%s:set commu error:%d\r\n", __FUNCTION__, nRtn);
// 				return -2;
// 			}
// 			return 0;
// 		};
// 		config.pfnReload = [](const CONNECTNODE &commectNode) {
// 			int nRtn = 0;
// 			std::list<CONNECTNODE> listCommu = CCloudParse::get_instance().getCommu();
// 			for (auto &iter : listCommu) {
// 				if (iter.nCommuId != commectNode.nCommuId) {
// 					continue;
// 				}
// 				if (0 != strcmp(commectNode.PlcSet.szPLCType, iter.PlcSet.szPLCType) ||
// 					0 != strcmp(commectNode.PlcSet.szLinkProtocol, iter.PlcSet.szLinkProtocol)) {
// 					continue;
// 				}
// 				if ((nRtn = CEngineMgr::getInstance().reLoadPlcDriVer(iter)) < 0) {
// 					printf("%s: reload plc driver error:%d\r\n", __FUNCTION__, nRtn);
// 					return -1;
// 				}
// 				if ((nRtn = CCloudParse::get_instance().setCommu(listCommu)) < 0) {
// 					printf("%s:set commu error:%d\r\n", __FUNCTION__, nRtn);
// 					return -2;
// 				}
// 				break;
// 			}
// 			return 0;
// 		};
// 		config.pfnReloadAll = []() {
// 			int nRtn = 0;
// 			std::list<CONNECTNODE> listCommu = CCloudParse::get_instance().getCommu();
// 			if (listCommu.empty()) {
// 				printf("Commu is empty!");
// 				return 0;
// 			}
// 			for (auto &iter : listCommu) {
// 				if ((nRtn = CEngineMgr::getInstance().reLoadPlcDriVer(iter)) < 0) {
// 					printf("%s: reload plc driver error:%d\r\n", __FUNCTION__, nRtn);
// 					return -1;
// 				}
// 			}
// 			if ((nRtn = CCloudParse::get_instance().setCommu(listCommu)) < 0) {
// 				printf("%s:set commu error:%d\r\n", __FUNCTION__, nRtn);
// 				return -2;
// 			}
// 			return 0;
// 		};
// 		config.pfnLoadAll = []() {
// 			int nRtn = 0;
// 			std::list<CONNECTNODE> listCommu = CCloudParse::get_instance().getCommu();
// 			if (listCommu.empty()) {
// 				printf("Commu is empty!");
// 				return 0;
// 			}
// 			for (auto &iter : listCommu) {
// 				if (iter.nPortId >= 0) {
// 					continue;
// 				}
// 				if ((nRtn = CEngineMgr::getInstance().loadPlcDriVer(iter)) < 0) {
// 					printf("%s: reload plc driver error:%d\r\n", __FUNCTION__, nRtn);
// 					return -1;
// 				}
// 			}
// 			if ((nRtn = CCloudParse::get_instance().setCommu(listCommu)) < 0) {
// 				printf("%s:set commu error:%d\r\n", __FUNCTION__, nRtn);
// 				return -2;
// 			}
// 			return 0;
// 		};
// 		HmiGui::setCommuConfig(config);
// 	}
// #endif
	CHJ212Control::get_instance().start();
	startCloudParse();
	CMqttThirdParty::get_instance().start();
	CHMINet::get_instance().start();
	return nullptr;
}

void *runLuaBnsScriptTask(void *arg) {
	runPollingBnsScript();
	return nullptr;
}

static void DeInitPluginFunc() {
	if (!CPluginDLL::IsLoad()) {
		return;
	}
	PluginShortMessage&     shortMsg = CPluginDLL::GetMsgFunc();
	PluginMailMessage&      mailMsg = CPluginDLL::GetMailFunc();
	PLUGINAUDIOMODULE_T&	audioModule = CPluginDLL::GetAudioModuleFunc();
	PLUGINIPCAMERAMODULE_T&	ipcameraSearch = CPluginDLL::GetIPCameraModuleFunc();
	if (true == shortMsg.bRun &&
		NULL != shortMsg.DeInit) {
		shortMsg.DeInit();
	}
	if (true == mailMsg.bRun &&
		NULL != mailMsg.DeInit) {
		mailMsg.DeInit();
	}
	if (audioModule.bRun &&
		nullptr != audioModule.destroy) {
		audioModule.destroy();
	}
	if (ipcameraSearch.bRun &&
		nullptr != ipcameraSearch.destroy) {
		ipcameraSearch.destroy();
	}
	return;
}

void exitProject() {
	CRateTimer::destory();
	if (VBOX_MODE != CPlatBoard::getDeviceMode()) {
		CClearLcd::destory();
		CEventDispatch::getInstance().stop();
		SCRIPTMGR_NAMESPACE::ScriptModule::Unload();//卸载脚本
		CCustomDLL::DestoryFunc();
		CCustomDLL::Unload();
		DeInitPluginFunc();
		Traffic::DestroyInstance();
		Installment_DeInit();
		CHMIScreens::get_instance().destroyAllScreen();
#ifdef _LINUX_
		unsigned short wValue = CRegisterRW::readWord(REG_TYPE_HS_, HSW_RESET);
		if (1 == wValue) {
			CRegisterRW::writeWord(REG_TYPE_HS_, HSW_RESET, 0);
		}
		CFtpServer::getInstance().exit();
		if (wValue == 1 &&
			!CPlatCpu::enableReboot()) {

			std::string			sReturn;
			CLocalSocket::htmRequest(SOCKET_ASSIST_FILE, CLocalSocket::TYPE_ASSIST_LOGO, "", sReturn, 3000);
			CPlatShell::reboot("");
		}
		system("killall vcplayer");
#endif
	}
	if (cbl::isDir(getPrjDir() + std::string("Script/")) ||
		isShortCutKeyValid()) {
#ifdef WIN32
		FreeConsole();
#endif
	}
	CHJ212Control::get_instance().stop();
}

static void waitPartitionMount() {
#ifdef _LINUX_
			std::cout << "waitPartitionMount ------- " << std::endl;

	std::string sFile = "/tmp/mountdata";
	unsigned short nTimes = 0;
	while (1) {
		if (CFile::ExistFile(sFile)) {
			break;
		}
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			if (nTimes++ > 50) {
				reboot(RB_AUTOBOOT);
			}
		}
	}
#endif
}

static void waitCalibration() {
		std::cout << "HMIUI waitCalibration" << std::endl;

#ifdef _LINUX_
	const char *ts_point = "/etc/ts_point";
	const char *pointercal = "/etc/pointercal";
	if (CPlatBoard::getDeviceRealMode() != HMI_MODE ||
		CPlatBoard::getType() == Rk3566_emmc_BOARD_v2) {
		return;
	}
	while (1) {//只有两个校准文件都存在了，HMI才允许运行 Lizh 20191204
		if (CFile::ExistFile(ts_point) &&
			CFile::ExistFile(pointercal)) {
			std::cout << "HMIUI ExistFile" << std::endl;

			break;
		}
		else {
			//等待Monitor启动校准程序并完成校准
			std::cout << "HMIUI wait sleep" << std::endl;

			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}
#endif
}

int preloadProject() {
	std::cout << "HMIUI preloadProject-----------------" << std::endl;

#ifdef _LINUX_
	if (cbl::isDir(TMP_APP)) {
		cbl::removeDir(TMP_APP, true);
	}
	cbl::makeDir(TMP_APP);
#endif
	waitPartitionMount();
	std::string sCupId;
	CPlatCpu::getCpuId(sCupId);
	if (A7_FLASH_BOARD_v2 == CPlatBoard::getType() ||
		( A8_FLASH_BOARD_v2 == CPlatBoard::getType() && sCupId.length() > 0 )) {
		UTILITY::CTime::get_instance().initTz();
	}
	waitCalibration();
	return 0;
}