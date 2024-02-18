#include "task.h"
#include "cbl/cbl.h"
#include "platform/platform.h"
#include "utility/utility.h"
#include "parseProject.h"
#include "common.h"
#include "../EventDispatcher/EventDispatch.h"
#include "../EventDispatcher/HandleEvent.h"
#include "../HSWAddrOperate/HSWAddrOperate.h"
#include "GlobalModule/GlobalModule.h"
#include "hmiScreens/hmiScreens.h"
#include "ScriptMgr/ScriptMgr.h"
#include "macrosplatform/path.h"
#include "../EventDispatcher/DispEventHandle.h"
#include "macrosplatform/registerAddr.h"
#include "CSlaveModule/CSlaveModule.h"
#ifdef WITH_UI
#include "HmiGui/HmiGui/HmiGui.h"
#endif
#include "../cloud/cloud.h"

#ifdef WIN32
#pragma data_seg("onlyonce")
int gcount = 0;
#pragma data_seg()
#pragma comment(linker,"/SECTION:onlyonce,RWS")
#define USELUACONSLEDEBUG 1
#endif //WIN32

using namespace UTILITY_NAMESPACE;
using namespace PLATFORM_NAMESPACE;

static int startLOG() {
	std::string sDeviceId, sType;
	SECURE_NAMESPACE::WMachine machineInfo;
	machineInfo.getDeviceId(sDeviceId);
	machineInfo.GetHMIType(sType);
	_LOG_SETDEV(sDeviceId, sType);
	_LOG_MODE(MODE_FILE);
	_LOG_ERROREX("HMIUI", "HMIUI start", 10, "HMIUI is start");
	return 0;
}

CTask::CTask() {
	m_nLanguage = 0;
	if (CPlatBoard::getDeviceMode() == VBOX_MODE) {
		m_bHMIMode = false;
	}
	else {
		m_bHMIMode = true;
	}
	startLOG();
}

CTask::~CTask() {
	exitProject();
}

int CTask::loadProject() {
	int nRtn = 0;
#ifdef WIN32
	if (gcount > 0) {
		return -1;
	}
#endif
	if (m_bHMIMode) {
		nRtn = loadHMIProject();
	}
	else {
		nRtn = loadBOXProject();
	}
	if (nRtn == 0) {
#ifdef WIN32
		if (m_vecParam.size() >= 6 &&
			USELUACONSLEDEBUG == std::stoi(m_vecParam[5])) {
			openConsole();
		}
		gcount++;
#endif
	}
	else {
		return -2;
	}
	return 0;
}

int CTask::loadHMIProject() {
	int nRtn = 0;
#ifdef WIN32
	std::string sPath = "";
	getProjectPath(sPath);
	sPath += LANGUAGEFILEPATH;
	if (getLanguageId(sPath, LANGUAGEID, m_nLanguage) < 0) {
		m_nLanguage = 0;
	}
	m_nLanguage = m_nLanguage > 0 ? m_nLanguage : 0;
#elif _LINUX_
	if (m_vecParam.size() > 2) {
		getLanguageId(m_vecParam[1] + std::string("/") + LANGUAGEFILEPATH, LANGUAGEID, m_nLanguage);
	}
	m_nLanguage = m_nLanguage > 0 ? m_nLanguage : 0;
#endif
	if ((nRtn = initHMIProject(m_vecParam)) < 0){
#ifdef WIN32
		MessageBoxW_T(nRtn, m_nLanguage);
#endif
		printf("initProject error:%d\r\n", nRtn);
		return -2;
	}
	hswAddrInit();
	runSystemTimer();
	CCloudParse::get_instance().load();
	CFormulaControl::get_instance().config(FORMULADIR);
	CFormulaControl::get_instance().parse(getPrjDir() + std::string("FormulaRecord.xml"));
	if ((nRtn = CLocalSocket::htmInit()) < 0) {
		printf("CLocalSocket::htmInit error:%d\r\n", nRtn);
		return -3;
	}
	SCRIPTMGR_NAMESPACE::ScriptModule::RunGlobalInit();
	setInitialScreenId();
	if (( nRtn = CHMIScreens::get_instance().loadInitialScreen() ) < 0) {
		printf("loadInitialScreen error:%d\r\n", nRtn);
		return -4;
	}
	if (( nRtn = CEventDispatch::getInstance().start() ) < 0) {
		printf("CEventDispatch::getInstance().start error:%d\r\n", nRtn);
		return -5;
	}
	CSecureCtrl::getInstance().getAuthority().makeRoleXmlFile();
	{
#ifdef WITH_UI
		/* 等第一个画面显示出来后再启动定时器，防止占用qt显示资源 */
		while (false == CDispEventHandle::getInstance().dispStates()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
#ifdef _LINUX_
		std::this_thread::sleep_for(std::chrono::milliseconds(500)); //发送完第一个画面初始化的数据后，再等一点时间，让资源给UI画图
#endif
#else
		{ // simulate QT ID
			EVENT_T hmiEvent;
			hmiEvent.nScreenID = -1;
			hmiEvent.nType = EVENT_SCRINIT;
			hmiEvent.nClientId = QTCLIENT_T;
			CEventDispatch::getInstance().postEvent(hmiEvent);
		}
#endif
		CCustomDLL::InitFunc();
		PrintModule_init();
		if (!CSlaveModule::get_instance().isSlaveMode()) {
		
			CRateTimer::addTimer(runTriggerActionScript, NULL, (int)(&runTriggerActionScript), 20);
			CRateTimer::addTimer(runTimerActionScript, NULL, (int)(&runTimerActionScript), 50);
			CUtiThreadpool::get_instance().commit((int)(&runOtherTask), runOtherTask, nullptr);
			CRateTimer::addTimer(runBackgroundLuaScript, NULL, (int)(&runBackgroundLuaScript), 50);
			CRateTimer::addTimer(runDataRecordCollectionTask, NULL, (int)(&runDataRecordCollectionTask), 100);
			runAddrmap();
			CRateTimer::addTimer(runDataRecordStorageTask, NULL, (int)(&runDataRecordStorageTask), 100);
			CRateTimer::addTimer(runRoutineTask, NULL, (int)(&runRoutineTask), 200);
		}
	}
	return 0;
}

int CTask::loadBOXProject() {
	PROJSETUP  *pps = GetProjInfo();
	if (!pps) {
		return -1;
	}
	pps->bOnLine = true;
	int nRtn = 0;
	if ((nRtn = initBOXProject()) < 0) {
		printf("%s:initBOXProject error:%d\r\n", __FUNCTION__, nRtn);
		return -2;
	}
	runSystemTimer();
	if (( nRtn = CLocalSocket::htmInit() ) < 0) {
		printf("CLocalSocket::htmInit error:%d\r\n", nRtn);
		return -3;
	}
	if ((nRtn = CEventDispatch::getInstance().start()) < 0) {
		printf("CEventDispatch::getInstance().start error:%d\r\n", nRtn);
		return -4;
	}
	CUtiThreadpool::get_instance().commit((int) ( &runOtherTask ), runOtherTask, nullptr);
	CRateTimer::addTimer(runBackgroundLuaScript, NULL, (int) ( &runBackgroundLuaScript ), 50);
	CRateTimer::addTimer(runDataRecordCollectionTask, NULL, (int) ( &runDataRecordCollectionTask ), 100);
	CRateTimer::addTimer(runDataRecordCollectionTask, NULL, (int) ( &runDataRecordCollectionTask ), 100);
	CRateTimer::addTimer(runDataRecordStorageTask, NULL, (int) ( &runDataRecordStorageTask ), 100);
	if (eCLOUDMODE_CLOUD == getCloudMode()) {
		CRateTimer::addTimer(runLuaBnsScriptTask, NULL, (int)(&runLuaBnsScriptTask), 100);
	}
	return 0;
}

int CTask::runTask() {
	int nRtn = 0;
	if ((nRtn = loadProject()) < 0) {
		printf("%s:loadProject error:%d\r\n", __FUNCTION__, nRtn);
		return -1;
	}
	return 0;
}

int CTask::execWithUI(int argc, char* argv[]) {
#ifdef WITH_UI
	if (isHMIProjectEmpty()) {
		std::string sbuffer = R"+*(The project is incomplete or the version is too low, please upgrade the project again!\n工程不完整或版本过低，请重新升级工程!)+*";
		runHMIEmptyProject();
		return HmiGui::runMsg(argc, argv, sbuffer);
	}
	std::string sIMGPath = "";
	CRateTimer::init(10);
	if (!CSlaveModule::get_instance().isSlaveMode()) {
		CUtiThreadpool::get_instance().commit(std::mem_fn(&CTask::runTask), this);
	}
	HmiGui::SetSendFunc(std::bind(( int( CEventDispatch:: * )( const EVENT_T& ) )&CEventDispatch::postEvent, &CEventDispatch::getInstance(), placeholders::_1));
#ifdef WIN32
	if (argc < 3) {
		return -1;
	}
	argc = 3;
	sIMGPath = getIMGPath();
#else
	if (argv[1] == nullptr) {
		sIMGPath = PROJECT_FILE_PATH;
	}
	else {
		sIMGPath.append(argv[1]);
		sIMGPath.append("/");
	}
#endif

	if (CSlaveModule::get_instance().isSlaveMode()) {
		sIMGPath = slaveResourcePath;
	}
	return HmiGui::runGui(argc, argv, sIMGPath, loadAllIMGS());
#else
	return 0;
#endif
}

int CTask::execWithoutUI(int argc, char* argv[]) {
#ifndef WITH_UI
	setEventConfig([this]() {
			return this->stop();
	});
#endif
	if (!m_bHMIMode) {
		std::string sProject, sProjectCfg;
		std::string sbuffer;
#ifdef WIN32
		CState::GetModuleFilePath(sProject);
		sProject += TMPDIR + std::string("/");
		sProjectCfg = sProject;
#else
		sProject = "/mnt/wecon/wecon/run/vprj/";
		sProjectCfg = "/mnt/wecon/wecon/run/";
#endif
		setPrjDir(sProject);
		setPrjCfgDir(sProjectCfg);
		eCLOUDMODE_T eMode = getCloudMode();
		if (eMode == eCLOUDMODE_NONE) {
			return 0;
		}
	}
	CRateTimer::init(10);
	CUtiThreadpool::get_instance().commit(std::mem_fn(&CTask::runTask), this);
	std::unique_lock <std::mutex> lck(m_mutex);
	m_cv.wait(lck);
	return 0;
}

int CTask::exec(int argc, char* argv[]) {
	for (int nI = 0; nI < argc; nI++) {
		if (argv[nI] != nullptr) {
			m_vecParam.push_back(argv[nI]);
		}
	}
#ifdef WITH_UI
	return this->execWithUI(argc, argv);
#else
#ifdef WIN32
	if (m_vecParam.size() >= 7 ) {
		if (m_vecParam[6] == "2") {
			if (CPlatBoard::setDeviceMode(VBOX_MODE) == 0) {
				m_bHMIMode = false;
			}
		}
		else {
			if (CPlatBoard::setDeviceMode(HMI_MODE) == 0) {
				m_bHMIMode = true;
			}
		}
	}
#endif
	return this->execWithoutUI(argc, argv);
#endif
}

int CTask::stop() {
	std::unique_lock <std::mutex> lck(m_mutex);
	m_cv.notify_all();
	return 0;
}