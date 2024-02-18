#include "HSWAddrOperate.h"
#include "HmiBase/HmiBase.h"
#include "GlobalModule/GlobalModule.h"
#include "../EventDispatcher/EventDispatch.h"
#include "../PluginDLL/PluginDLL.h"
#include "platform/platform.h"
#include <cbl/cbl.h>
#include <secure/secure.h>
#include <utility/utility.h>
#include "RegisterOpt/RegisterOpt.h"
#include "hmiScreens/hmiScreens.h"
#include "macrosplatform/registerAddr.h"
#include "macrosplatform/path.h"
#include "macrosplatform/calculateRunTime.h"

using namespace SECURE_NAMESPACE;
using namespace UTILITY_NAMESPACE;
using namespace PLATFORM_NAMESPACE;

void DateAddrMonitor() {
	unsigned short  byDataBuffer[7] = { 0 };
	DATETIME_T      stCurTime { 0, 0, 0, 0, 0, 0, 0, 0 };

	CTime::get_instance().getLocal(stCurTime);
	byDataBuffer[0] = stCurTime.nYear;
	byDataBuffer[1] = stCurTime.nMonth;
	byDataBuffer[2] = stCurTime.nDay;
	byDataBuffer[3] = stCurTime.nHour;
	byDataBuffer[4] = stCurTime.nMinute;
	byDataBuffer[5] = stCurTime.nSecond;
	byDataBuffer[6] = stCurTime.nDayOfWeek;

	if (HMI_MODE == PLATFORM::CPlatBoard::getDeviceMode()) {
		CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_LOCALYEAR, 0, byDataBuffer, 7);
	}
	else if (VBOX_MODE == PLATFORM::CPlatBoard::getDeviceMode()) {
		CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_LOCALYEAR_R, 0, byDataBuffer, 7);
	}
}

static void FlashMemeryAddrMonitor() {
	static unsigned long long	ullTime = 0;
	unsigned short				wEnable = 0;
	unsigned int				dwFreeSpace = 0;
	unsigned int				dwALLSpace = 0;
	unsigned int				dwTotal = 0;
	unsigned int				dwFree = 0;
	unsigned int				dwUsed = 0;

	wEnable = CRegisterRW::readWord(REG_TYPE_HS_, HSW_SHOWMEMORYENABLE);
	if (wEnable != 1 || CTime::get_instance().getTickCount() - ullTime <= 60 * 1000) // 1分钟更新一次
	{
		return;
	}

	ullTime = CTime::get_instance().getTickCount();
	if (0 == CState::GetDiskFreeSize(USERFILEDIR, dwALLSpace, dwFreeSpace)) {
		CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_USERFREESPACE, 0, &dwFreeSpace, 2);
		CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_USERALLSPACE, 0, &dwALLSpace, 2);
	}

	CState::GetMemoryStatus(dwTotal, dwFree);
	dwUsed = dwTotal - dwFree;
	CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_ALLMEMORYSPACE, 0, &dwTotal, 2);		//860-861    触摸屏内存总大小
	CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_SURPLUSMEMORYSPACE, 0, &dwFree, 2);	//862-863    触摸屏剩余内存大小
	CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_SURPLUSMEMORYUSED, 0, &dwUsed, 2);	//862-863    触摸屏剩余内存大小

	printf("HSW_UserFreeSpace %d HSW_All %d", dwFreeSpace, dwTotal);
	printf("HSW_UserFreeSpace HSW_Free %d", dwFree);
	printf("HSW_UserFreeSpace HSW_Used %d", dwUsed);
}

static void NandFlashReadAddrMonitor() {
	unsigned int        dwWrite = 0;
	unsigned int        dwErase = 0;
	unsigned short      wIsNew = 0;
	int                 nErase = 0;
	int                 nWrite = 0;
	unsigned short		wEnableShow = 0;
	DATETIME_T          stEnd { 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned long long  llTime = 0;
	unsigned short      wSaveType = 0;
	unsigned int        dwSaveInfo = 0;
	static bool			bFirst = true;
	static unsigned long long	ullTime = CTime::get_instance().getTickCount();
	static unsigned int			dwValue = 0;
	static DATETIME_T	stStart { 0, 0, 0, 0, 0, 0, 0, 0 };

	if (CTime::get_instance().getTickCount() - ullTime < 10000) {
		return;
	}

	ullTime = CTime::get_instance().getTickCount();
	wEnableShow = CRegisterRW::readWord(REG_TYPE_HS_, HSW_FLASH_NAND);
	if (wEnableShow != 2)   // 未启用查看flash擦写信息
	{
		return;
	}

	if (bFirst) {
		CTime::get_instance().getLocal(stStart);
		CPlatShell::runCmd("mount -t debugfs nodev /sys/kernel/debug", NULL);
		CPlatShell::runCmd("/etc/init.d/__S02logging start", NULL);//开启syslog
		bFirst = false;
	}

	if (0 != CPlatFlash::getEraseWriteCount(nErase, nWrite))   // 获取信息失败
	{
		return;
	}

	dwWrite = (unsigned int) nWrite;
	dwErase = (unsigned int) nErase;
	wIsNew = 1;     //标记是优化过的版本

	CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_FLASH_NANDWRITE, 0, &dwWrite, 2);
	CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_FLASH_NANDERASE, 0, &dwErase, 2);
	CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_FLASH_ISNEW, 0, &wIsNew, 1);
	if (dwValue != dwErase)//块数量变化了就发送给服务器
	{
		char                szEvent[512] = { 0 };
		dwValue = dwErase;
		CRegisterRW::read(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_FLASH_SAVETYPE, 0, &wSaveType, 1);
		CRegisterRW::read(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_FLASH_SAVEINFO, 0, &dwSaveInfo, 2);

		CTime::get_instance().getLocal(stEnd);
		llTime = CTime::get_instance().getSpan(stStart, stEnd, CTime::Second);//获取起始时间和终止时间的间隔
		sprintf(szEvent, "time,%llu,Block,%u,Page,%u,Type,%d,info,%u,isnew,%d\n", llTime, dwErase, dwWrite, wSaveType, dwSaveInfo, wIsNew);
	}

	return;
}

static void NandFlashShowAddrMonitor() {
	HMI_UBI_INFO_T ubi = { 0 };
	unsigned int nMaxEraseValue = 0;
	unsigned int nMeanEraseValue = 0;
	unsigned short nReservedBlocks = 0;
	unsigned short nTotalBlocks = 0;
	static unsigned long long ullTime = CTime::get_instance().getTickCount();
	static unsigned long long ullStartTime = CTime::get_instance().getTickCount();
	static bool bFirst = true;
	int nRtn = 0;

	if (20 * 1000 >= CTime::get_instance().getTickCount() - ullTime) {// 开机20s后才允许进行刷新
		return;
	}
	ullTime = CTime::get_instance().getTickCount();
	nRtn = CPlatFlash::getUbiInfo(ubi);
	if (nRtn < 0) {
		printf("%s:get ubi error:%d\r\n", __FUNCTION__, nRtn);
		return;
	}
	nMaxEraseValue = (unsigned int) ubi.nMaxEraseValue;
	nMeanEraseValue = (unsigned int) ubi.nMeanEraseValue;
	nReservedBlocks = (unsigned short) ubi.nReservedBlocks;
	nTotalBlocks = (unsigned short) ubi.nTotalBlocks;
	CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_FLASHINFO_ERAMAX, 0, &nMaxEraseValue, 2);
	CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_FLASHINFO_ERAMEAN, 0, &nMeanEraseValue, 2);
	CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_FLASHINFO_RESERVE, 0, &nReservedBlocks, 1);
	CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_FLASHINFO_ALLBLOCK, 0, &nTotalBlocks, 1);
	if (bFirst == false || 
		CTime::get_instance().getTickCount() - ullStartTime > 5 * 60 * 1000) {//大于5分钟后不使用
		bFirst = false;
		return;
	}
	std::string sUSBPath = "", sSDPath = "";
	CHmiMedia::getUdiskPath(sUSBPath);
	CHmiMedia::getSDPath(sSDPath);
	std::string sPath = "";
	if (CHmiMedia::isUSBExsit() && 
		!sUSBPath.empty()) {
		sPath = sUSBPath;
	}
	else if (CHmiMedia::isSDExsit() && 
		sSDPath.empty()) {
		sPath = sSDPath;
	}
	if (sPath.empty()) {
		return;
	}
	if (true == CFile::ExistFile(sPath + std::string("WECOMSHWOFLASHINFO")) ||
		true == CFile::ExistFile(sPath + std::string("WECONSHOWFLASHINFO")) ||
		true == cbl::isDir(sPath + std::string("showflashinfo"))) {
		string sMsg = cbl::format("flashinfo(%d.%d.%d.%d)", nMaxEraseValue, nMeanEraseValue, nReservedBlocks, nTotalBlocks);
		vector<string> vecArgs;
		vecArgs.push_back(to_string(nMaxEraseValue));
		vecArgs.push_back(to_string(nMeanEraseValue));
		vecArgs.push_back(to_string(nReservedBlocks));
		vecArgs.push_back(to_string(nTotalBlocks));
		CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_FLASHINFO, sMsg, vecArgs);
		bFirst = false;
	}
}

static void IntoBackgroundMonitor() {
	unsigned short    wValue = 0;
	wValue = CRegisterRW::readWord(REG_TYPE_HS_, HSW_ENTERBACKGROUND);     //读取进入后台寄存器的数值，1  进入后台   0  不进入
	if (1 == wValue){ // 进到后台画面

#ifdef WIN32
#ifndef _DEBUG
		CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_ENTERBACKGROUND_FAILD);
		CRegisterRW::writeWord(REG_TYPE_HS_, HSW_ENTERBACKGROUND, 0);
		return;
#endif
#endif // DEBUG

		if (PLATFORM::RK3308B_FLASH64Mmem_BOARD_v2 == PLATFORM::CPlatBoard::getType() ||
			PLATFORM::RK3308B_FLASH64Mmem_IN_v2 == PLATFORM::CPlatBoard::getType() ||
			PLATFORM::SSD212_FLASH64Mmem_v2 == PLATFORM::CPlatBoard::getType()) {
			std::string strReturn;
			int nRet = CLocalSocket::htmRequest(SOCKET_ASSIST_FILE, CLocalSocket::TYPE_ASSIST_BACKSTAGE, "", strReturn, 1000);
			if ((nRet < 0) || (*((const int*)strReturn.c_str()) != CLocaludp::udpOK)) {
				PRINTAPP("hmitoRequest EnterBackstage error \n");
				CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_ENTERBACKGROUND_FAILD);
			}
		}
		else{
			GMExtern::showSysSetScreen(0);
		}
	}

	CRegisterRW::writeWord(REG_TYPE_HS_, HSW_ENTERBACKGROUND, 0);
}

static void CpuAddrMonitor() {
	int		nCpuTemperature = 0;
	int		nCpuPercent = 0;

	CPlatCpu::getTemperature(nCpuTemperature);
	CPlatCpu::getUsePercent(nCpuPercent);

	CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_CPU_TEMPERATURE, 0, &nCpuTemperature, 2);
	CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_CPU_PERCENT, 0, &nCpuPercent, 1);
}

static void HswScrAddrMonitor() {
	unsigned short    wValue = CRegisterRW::readWord(REG_TYPE_HS_, HSW_SCRJMPID);
	if (CHMIScreens::get_instance().isSubscreen(wValue)) {
		return;
	}
	std::list<int>	listClientId;
	CUserMgr::getInstances().getAllUserID(listClientId);
	for (auto &iter : listClientId) {
		CUser*  pUser = CUserMgr::getInstances().getUser(iter);
		if (NULL == pUser ||
			wValue == pUser->getCurBaseScrID()) {
			continue;
		}
		SET_TIME_CHANGE_SCREE;
		hmiproto::hmievent      event;
		event.set_type(HMIEVENT_CHANGESCR);
		event.set_clientid(iter);
		event.set_scrno(wValue);
		event.set_alljump(false);
        CEventDispatch::getInstance().postEvent(event);
	}
	return;
}

static void HuwScrAddrMonitor() {
	std::list<int>	listClientId;
	CUserMgr::getInstances().getAllUserID(listClientId);
	for (auto &iter : listClientId) {
		unsigned short    wValue = CRegisterRW::readWord(REG_TYPE_HU_, HUW_SCRJUMPID, iter);
		if (CHMIScreens::get_instance().isSubscreen(wValue)) {
			continue;
		}
		CUser*  pUser = CUserMgr::getInstances().getUser(iter);
		if (NULL == pUser ||
			wValue == pUser->getCurBaseScrID()) {
			continue;
		}
		hmiproto::hmievent      event;
		event.set_type(HMIEVENT_CHANGESCR);
		event.set_clientid(iter);
		event.set_scrno(wValue);
		event.set_alljump(false);
		CEventDispatch::getInstance().postEvent(event);
	}
	return;
}

static void ProScrAddrMonitor() {

	PROJSETUP*      pps = GetProjInfo();
	if (NULL == pps || false == pps->BaseSet.bUseSysContrl) {
		return;
	}

	CUser*  pUser = CUserMgr::getInstances().getUser(QTCLIENT_T);
	if (NULL == pUser) {
		return;
	}

	bool            bRet = false;
	unsigned short  wValue = 0;
	RW_PARAM_T        rw = { RW_MODE_CACHE_DEV };

	bRet = CRegisterRW::read(RW_FORMAT_INT, pps->BaseSet.addrSysContrl, &wValue, 1, rw);
	if (false == bRet) {
		return;
	}

	if (CHMIScreens::get_instance().isSubscreen(wValue)) {
		return;
	}

	if (wValue == pUser->getCurBaseScrID()) {
		return;
	}

	std::list<int>			listClientId;

	CUserMgr::getInstances().getAllUserID(listClientId);
	for (auto &iter : listClientId) {

		CUser*  pTemp = CUserMgr::getInstances().getUser(iter);
		if (NULL == pTemp ||
			wValue == pTemp->getCurBaseScrID()) {
			continue;
		}

		hmiproto::hmievent      event;
		event.set_type(HMIEVENT_CHANGESCR);
		event.set_clientid(iter);
		event.set_scrno(wValue);
		event.set_alljump(false);
        CEventDispatch::getInstance().postEvent(event);
	}

	return;
}

static void ScreenJumpMonitor() {
	ProScrAddrMonitor();

	if (0 == CRegisterRW::readWord(REG_TYPE_HS_, HSW_SCRJUMPTYPE)) {
		HswScrAddrMonitor();
	}
	else {
		HuwScrAddrMonitor();
	}

	return;
}

static void ClickStausMonitor() {
	unsigned short			wClickStatus = 0;
	int						nClientNum = CUserMgr::getInstances().getUserNum();
	static unsigned short	wPreClickStatus = 0;
	static int				nPreClientNum = 0;

	CRegisterRW::read(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_CLIENTOPTLIMITTYPE, 0, &wClickStatus, 1);
	if (wPreClickStatus != wClickStatus ||
		nClientNum != nPreClientNum) {

        hmiproto::hmievent  event;
        event.set_type(HMIEVENT_CLICKSTAUS);

		if (wClickStatus == 1 ||
			wClickStatus == 2) {
            event.set_eventbuffer("ClickOff");
		}
		else {
            event.set_eventbuffer("ClickOn");
		}

        CEventDispatch::getInstance().postEvent(event);
		wPreClickStatus = wClickStatus;
		nPreClientNum = nClientNum;
	}
}

static void updateUserOptTime() {

	static	unsigned long long	    ullLastTime = CTime::get_instance().getTickCount();
	if (CTime::get_instance().getTickCount() - ullLastTime < 300) {//从1000ms修改为300ms  防止出现计数不连续的现象 Lizh 20180915
		return;
	}

	ullLastTime = CTime::get_instance().getTickCount();

	unsigned long long			ullClickTime = 0;
	std::list<int>				listClientId;

	CUserMgr::getInstances().getAllUserID(listClientId);
	for (auto &iter : listClientId) {
		CUser*  pUser = CUserMgr::getInstances().getUser(iter);
		if (NULL == pUser) {
			continue;
		}

		if (ullClickTime < pUser->getOptTime()) {
			ullClickTime = pUser->getOptTime();
		}

		RW_PARAM_T	rw = { RW_MODE_CACHE_DEV, 0, 0, iter, REG_SRC_USERAUTHORITY };
		unsigned int       dwTime = (unsigned int) ( ullLastTime - pUser->getOptTime() ) / 1000;
		CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HU_, DATA_TYPE_WORD, HUW_CLICKTIMEINTERVAL, 0, &dwTime, 2, &rw);
	}

	unsigned int       dwTimeAll = 0;
	dwTimeAll = (unsigned int) ( ullLastTime - ullClickTime ) / 1000;
	CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_CLICKTIMEINTERVAL, 0, &dwTimeAll, 2);

	return;
}

/* monitor fast collect
**/
static int monitorFastCollect() {

	static bool bPreFastCollect = false;

	bool bFastCollect = CRegisterRW::readWord(REG_TYPE_HS_, HSW_FASTCOLLECTDATA);
	if (bFastCollect != bPreFastCollect) {

		CDataRecordCollect::get_instance().setFstCollect(bFastCollect);
		bPreFastCollect = bFastCollect;
	}

	return 0;
}

static int monitorClearScrCache() {
	int  nClearScrCache = CRegisterRW::readWord(REG_TYPE_HS_, HSW_ChangeLanguageCleanCache);;
	static int nLastvalue = 0;
	if (3 == nClearScrCache) {
		CHMIScreens::get_instance().removeScreen(true);
		CHMIScreens::get_instance().setLoadScreenFalg(false);
	}
	if (2 == nClearScrCache){
		CHMIScreens::get_instance().removeScreen(true);
		CRegisterRW::writeWord(REG_TYPE_HS_, HSW_ChangeLanguageCleanCache, 0);
	}
	if (nLastvalue != nClearScrCache && 3 == nLastvalue){
		CHMIScreens::get_instance().setLoadScreenFalg(true);
	}
	if (nLastvalue != nClearScrCache){
		nLastvalue = nClearScrCache;
	}
	return 0;
}

void HswAddrMonitor() {
	DateAddrMonitor();          // 时间
	updateUserOptTime();
#ifndef WIN32
	FlashMemeryAddrMonitor();   // flash
	NandFlashReadAddrMonitor(); // nandflash读
	NandFlashShowAddrMonitor(); // nandflash显示
#endif
	IntoBackgroundMonitor();
	CpuAddrMonitor();           // cpu
	ScreenJumpMonitor();        // 画面切换
	ClickStausMonitor();
	monitorFastCollect();	/* monitor fast collect */
	monitorClearScrCache();//清理画面缓存
}