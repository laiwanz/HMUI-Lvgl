#include "HSWAddrOperate.h"
#include "HmiBase/HmiBase.h"
#include "GlobalModule/GlobalModule.h"
#include "GlobalModule/HMINet/HMINet.h"
#include "../EventDispatcher/EventDispatch.h"
#include "platform/platform.h"
#include <secure/secure.h>
#include <utility/utility.h>
#include "RegisterOpt/RegisterOpt.h"
#include "../webserverCallback/webserverCallback.h"
#include "webserver/webserver.h"
#include "hmiScreens/hmiScreens.h"
#include "macrosplatform/registerAddr.h"
#include "macrosplatform/path.h"
#include "serialmanager.h"
#include "../cloud/cloud.h"
#ifdef _LINUX_
#include <sys/reboot.h>
#include <unistd.h>
#include "btl/btl.h"
#endif

using namespace std;
using namespace UTILITY_NAMESPACE;
using namespace SECURE_NAMESPACE;
using namespace PLATFORM_NAMESPACE;

typedef struct tagCustom_Static_Ip_T{
	std::string         sIp;					//IP,如"192.168.52.102"
	std::string         sNetMask;				//子网掩码，如"255.255.255.0"
	std::string         sGateWay;				//网关，如"192.168.50.1"
	std::string         sDnsPrimary;			//域名服务器地址，如"192.168.32.3" 
	std::string         sDnsSecondary;			//备用域名服务器地址
	std::string			sMac;					//mac地址，如"E6:AA:50:02:DA:40" 
	unsigned short		wValue[12] = { 0 };		//用来放从寄存器读出的ip、子网掩码、网关的值
	unsigned short		wDnsValue[8] = { 0 };	//用来放从寄存器读出的两个DNS的值	
}Custom_Static_Ip_T;

static bool IsCrash(const unsigned int dwCrash, const unsigned int dwCrashLen, const unsigned int dwTest, const unsigned int dwTestLen) {
	if (dwCrash <= dwTest && dwCrash + dwCrashLen - 1 >= dwTest)    // 左端碰撞
	{
		return true;
	}

	if (dwCrash <= dwTest + dwTestLen - 1 && dwCrash + dwCrashLen >= dwTest + dwTestLen) // 右端碰撞
	{
		return true;
	}

	if (dwCrash >= dwTest && dwCrash + dwCrashLen <= dwTest + dwTestLen)    // 完全包含
	{
		return true;
	}

	return false;
}

static void BeepAddrSet(const unsigned int dwAddrOffset, const unsigned int dwLen) {
#ifdef _LINUX_
	bool        bEnable = true;
	unsigned short        wClose = 0;
	unsigned short        wTime = 0;
	unsigned short        wType = 0;

	if (IsCrash(dwAddrOffset, dwLen, HSW_USEBEEPTIME, 1) == true)    // 蜂鸣控制地址
	{
		wClose = CRegisterRW::readWord(REG_TYPE_HS_, HSW_USEBEEPTIME);
		if (wClose == 1) {
			bEnable = false;
		}

		btl::beepEnable(bEnable);
		CRegisterRW::writeBit(REG_TYPE_HS_, HSW_USEBEEPTIMESAVE, 0, wClose);
	}

	if (IsCrash(dwAddrOffset, dwLen, HSW_BEEPTIME, 1) == true )   // 蜂鸣时间地址
	{
		wTime = CRegisterRW::readWord(REG_TYPE_HS_, HSW_BEEPTIME);
		btl::beepDurationMs(wTime);
		CRegisterRW::writeWord(REG_TYPE_HS_, HSW_BEEPTIME, wTime);
	}

	if (IsCrash(dwAddrOffset, dwLen, HSW_BEEPTYPE, 1) == true )   // 蜂鸣类型地址
	{
		wType = CRegisterRW::readWord(REG_TYPE_HS_, HSW_BEEPTYPE);
		btl::beepBuzzerType(wType);
		CRegisterRW::writeWord(REG_TYPE_HS_, HSW_BEEPTYPE, wType);
	}
#endif
}

static void BacklighAddrSet(const unsigned int dwAddrOffset, const unsigned int dwLen) {
#ifdef _LINUX_
	unsigned short		nBgSpanTime = 0;
	unsigned short		wBgBrightness = 0;
	unsigned short		wSetValue = 0;

	if (IsCrash(dwAddrOffset, dwLen, HSW_SETBACKLIGHTTIME, 1) == true)   // 背光灯打开时间地址
	{
		nBgSpanTime = CRegisterRW::readWord(REG_TYPE_HS_, HSW_SETBACKLIGHTTIME);
		if (0 == nBgSpanTime)
		{
			btl::backlightEnable(false);
		}
		else {
			btl::backlightEnable(true);
			btl::backlightTime(nBgSpanTime);
		}
		
	}

	if (IsCrash(dwAddrOffset, dwLen, HSW_SETBACKLIGHTNESS, 1) == true)   // 背光灯亮度地址
	{
		wBgBrightness = CRegisterRW::readWord(REG_TYPE_HS_, HSW_SETBACKLIGHTNESS);
		btl::backlightBrightness(wBgBrightness);
	}

	if (IsCrash(dwAddrOffset, dwLen, HSW_SETBACKLIGHT, 1) == true)   // 背光灯控制地址
	{
		wSetValue = CRegisterRW::readWord(REG_TYPE_HS_, HSW_SETBACKLIGHT);
		if (1 == wSetValue) {
			btl::backlightOn(true);
		}
		else if (2 == wSetValue) {
			btl::backlightOn(false);
		}

		if (0 != wSetValue) {
			wSetValue = 0;
			CRegisterRW::writeWord(REG_TYPE_HS_, HSW_SETBACKLIGHT, wSetValue);
		}
	}
#endif
}

static void LanguageAddrSet(const unsigned int dwAddrOffset, const unsigned int dwLen) {
	unsigned short  nLangNum = 0;
	PROJSETUP       *pps = GetProjInfo();

	if (IsCrash(dwAddrOffset, dwLen, HSW_LANGNUM, 1) == true)    // 语言地址
	{
		nLangNum = CRegisterRW::readWord(REG_TYPE_HS_, HSW_LANGNUM);
		if (nLangNum == pps->AssisSet.byLangType) {
			return;
		}

		if (nLangNum < MAX_MultiLangNum && nLangNum >= 0) {
			CRegisterRW::writeWord(REG_TYPE_HS_, HSW_LANGSAVE, nLangNum);//语言发生变化时,将当前语言写到HSW10609(掉电保存)寄存器，用于下次开机使用 Lizh 20181207
			pps->AssisSet.byLangType = (BYTE) nLangNum;
			CMessageCenter::get_instance().setLanguage(nLangNum + 1);

			int nStopCleanCache = CRegisterRW::readWord(REG_TYPE_HS_, HSW_ChangeLanguageCleanCache);
			if (1 != nStopCleanCache)//控制是否切换语言时，要清缓存 Linzhen 20221229
			{
				CHMIScreens::get_instance().removeScreen(true);
			}
		}
		else//目前只支持8种语言（0~7），所以超过7时不允许进行设置寄存器值 Lizh 20190123
		{
			CRegisterRW::writeWord(REG_TYPE_HS_, HSW_LANGNUM, pps->AssisSet.byLangType);
		}
		CAlarmCollect::get_instance().setLanguage(pps->AssisSet.byLangType);
		CTraditionalRecipe::get_instance().setLanguage(pps->AssisSet.byLangType);
		CHMIScreens::get_instance().setLanguage(pps->AssisSet.byLangType);
		CCloudOperator::get_instance().setLanguage(pps->AssisSet.byLangType);
		CSecureCtrl::getInstance().getAuthority().setLanguage(pps->AssisSet.byLangType);	// 设置角色语言配置 chenGH 2022.10.13
	}
}

static void DateAddrSet(const unsigned int dwAddrOffset, const unsigned int dwLen, unsigned short *pwData) {
	DATETIME_T      stNewTime { 0, 0, 0, 0, 0, 0, 0, 0 };
	DATETIME_T      stCurTime { 0, 0, 0, 0, 0, 0, 0, 0 };
	bool            bTimeErr = false;
	int             nMax = min((unsigned int) (HSW_LOCALSECOND + 1), dwAddrOffset + dwLen);//修复时间部件，秒不能改的问题_wangzq_20230815
	int             nMin = max((unsigned int) HSW_LOCALYEAR, dwAddrOffset);
	int				nYear = HSW_LOCALYEAR;
	if (VBOX_MODE == PLATFORM::CPlatBoard::getDeviceMode()) {
		nMax = min((unsigned int)(HSW_LOCALSECOND + 1), dwAddrOffset + dwLen);
		nMin = max((unsigned int)HSW_LOCALYEAR_R, dwAddrOffset);
		nYear = HSW_LOCALYEAR_R;
	}
	if (NULL == pwData ||
		false == IsCrash(dwAddrOffset, dwLen, nYear, 6)) {// 时间地址
		return;
	}
	CTime::get_instance().getLocal(stCurTime);
	stNewTime = stCurTime;
	for (int i = nMin; i < nMax; i++) {
		switch (i) {
		case HSW_LOCALYEAR:
		case HSW_LOCALYEAR_R:
			stNewTime.nYear = pwData[i - dwAddrOffset];
			break;
		case HSW_LOCALMONTH:
		case HSW_LOCALMONTH_R:
			stNewTime.nMonth = pwData[i - dwAddrOffset];
			break;
		case HSW_LOCALDAY:
		case HSW_LOCALDAY_R:
			stNewTime.nDay = pwData[i - dwAddrOffset];
			break;
		case HSW_LOCALHOUR:
		case HSW_LOCALHOUR_R:
			stNewTime.nHour = pwData[i - dwAddrOffset];
			break;
		case HSW_LOCALMINUTE:
		case HSW_LOCALMINUTE_R:
			stNewTime.nMinute = pwData[i - dwAddrOffset];
			break;
		case HSW_LOCALSECOND:
		case HSW_LOCALSECOND_R:
			stNewTime.nSecond = pwData[i - dwAddrOffset];
			break;
		default:
			break;
		}
	}

	if (0 == CTime::get_instance().compare(stCurTime, stNewTime)) {   // 时间未修改
		return;
	}

	do {
		if (CTime::get_instance().isValid(stNewTime) < 0) {  // 时间异常
			bTimeErr = true;
			break;
		}
		if (VBOX_MODE == PLATFORM::CPlatBoard::getDeviceMode()) {
			CTime::get_instance().setLocal(stNewTime);
		}
		else if (HMI_MODE == PLATFORM::CPlatBoard::getDeviceMode()) {
			if (CFile::ExistFile(FORBIDCHANGETIME) == false &&
				true == Installment_SetTime(stNewTime)) {  // 分期付款允许设置时间 xqh 20200514  //标志文件存在，说明设置了禁止修改时间 Linzhen 20211210
				CTime::get_instance().setLocal(stNewTime);
			}
			else {
				bTimeErr = true;
			}
		}
	} while (0);

	if (true == bTimeErr) {// 时间异常，时间需要还原
		for (int i = nMin; i < nMax; i++) {
			switch (i) {
			case HSW_LOCALYEAR:
			case HSW_LOCALYEAR_R:
				pwData[i - dwAddrOffset] = stCurTime.nYear;
				break;
			case HSW_LOCALMONTH:
			case HSW_LOCALMONTH_R:
				pwData[i - dwAddrOffset] = stCurTime.nMonth;
				break;
			case HSW_LOCALDAY:
			case HSW_LOCALDAY_R:
				pwData[i - dwAddrOffset] = stCurTime.nDay;
				break;
			case HSW_LOCALHOUR:
			case HSW_LOCALHOUR_R:
				pwData[i - dwAddrOffset] = stCurTime.nHour;
				break;
			case HSW_LOCALMINUTE:
			case HSW_LOCALMINUTE_R:
				pwData[i - dwAddrOffset] = stCurTime.nMinute;
				break;
			case HSW_LOCALSECOND:
			case HSW_LOCALSECOND_R:
				pwData[i - dwAddrOffset] = stCurTime.nSecond;
				break;
			default:
				break;
			}
		}
	}
}

static void ComPortAddrSet(const unsigned int dwAddrOffset, const unsigned int dwLen) {
	if (true == IsCrash(dwAddrOffset, dwLen, HSW_HMIPORTINFO, 100)) // 串口相关
	{
		//HMI、PLC 站号 运行时变更
		CSerialManager::get_instance().addrtobtl();
	}
}

static void RemoteAddrSet(const unsigned int dwAddrOffset, const unsigned int dwLen) {

	int nRemoteSNReg = HSW_REMOTESN;
	if (PLATFORM::VBOX_MODE == PLATFORM::CPlatBoard::getDeviceMode()) {
		nRemoteSNReg = HSW_REMOTESN_R;
	}

	if (false == IsCrash(dwAddrOffset, dwLen, nRemoteSNReg, 65)) {
		return;
	}

	if (HMI_MODE == PLATFORM::CPlatBoard::getDeviceMode()) {
		if (CUserMgr::getInstances().getMaxUserNum() < MAX_SessionID_High) { // 非高端屏
			return;
		}
	}

	WMachine    wMachine;
	std::string sDeviceId;
	if (wMachine.getDeviceId(sDeviceId) < 0) {
		return;
	}

	char    szRemoteSN[128 * 4] = { 0 };
	CRegisterRW::read(RW_FORMAT_STR, REG_TYPE_HS_, DATA_TYPE_WORD, nRemoteSNReg, 0, szRemoteSN, 64);
	if (sDeviceId != szRemoteSN) {
		CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, nRemoteSNReg, 0, (void *) sDeviceId.c_str(), 64);
	}

	return;
}

static void ResetHmi() {
#ifndef WIN32
	sync();
	std::this_thread::sleep_for(std::chrono::milliseconds(500));   //软重启时间延迟太长。改成500ms  20180126  chenfl
	sync();

	PRINTAPP("ResetHmi()\r\n");
	if (CPlatCpu::enableReboot()) {

		if (HMI_MODE == PLATFORM::CPlatBoard::getDeviceMode()) {
			CRegisterRW::writeWord(REG_TYPE_HS_, HSW_RESET, 0);
		}
		else if (VBOX_MODE == PLATFORM::CPlatBoard::getDeviceMode()) {
			CRegisterRW::writeWord(REG_TYPE_HS_, HSW_RESET_R, 0);
		}
		reboot(RB_AUTOBOOT);
	}
	else {

		EVENT_T  event;

		event.nType = EVENT_EXITHMI;
        CEventDispatch::getInstance().postEvent(event);
	}
#endif
}

static void SystemAddrSet(const unsigned int dwAddrOffset, const unsigned int dwLen) {
	unsigned short                wValue = 0;
	static   unsigned long long  ullTime = CTime::get_instance().getTickCount();

	int nReg = HSW_RESET;
	if (VBOX_MODE == PLATFORM::CPlatBoard::getDeviceMode()) {
		nReg = HSW_RESET_R;
	}

	if (true == IsCrash(dwAddrOffset, dwLen, nReg, 1)) {
		wValue = CRegisterRW::readWord(REG_TYPE_HS_, nReg);
		if (wValue != 1) {
			return;
		}

		if (CTime::get_instance().getTickCount() - ullTime > 10 * 1000) //开机10s后才允许HSW24进行软重启 Lizh 20190909)
		{
			ResetHmi();
		}
		else {
			CRegisterRW::writeWord(REG_TYPE_HS_, nReg, 0);
		}
	}
}

static void UserOptAddrSet(const unsigned int dwAddrOffset, const unsigned int dwLen) {

	if (false == IsCrash(dwAddrOffset, dwLen, HSW_DOUSEROPERATE, 1)) {
		return;
	}

	unsigned short    wValue = CRegisterRW::readWord(REG_TYPE_HS_, HSW_DOUSEROPERATE);
	if (0 == wValue) {
		return;
	}

	CAuthority& authority = CSecureCtrl::getInstance().getAuthority();
	authority.doFunc(wValue, QTCLIENT_T);
	CRegisterRW::writeWord(REG_TYPE_HS_, HSW_DOUSEROPERATE, 0);

	return;
}

static void	SyncAddrSet(const unsigned int dwAddrOffset, const unsigned int dwLen) {

	unsigned short               wDoSync = 0;
	static unsigned long long    ullTime = CTime::get_instance().getTickCount();

	if (true == IsCrash(dwAddrOffset, dwLen, HSW_DATA_DOSYNC, 1)) {
		wDoSync = CRegisterRW::readBit(REG_TYPE_HS_, HSW_DATA_DOSYNC, 0);
		if (wDoSync != 1) {
			return;
		}

		if (CTime::get_instance().getTickCount() - ullTime > 5 * 6 * 1000)    // 触发间隔必须大于5分钟
		{
			ullTime = CTime::get_instance().getTickCount();
#ifndef WIN32
			sync();
#endif
		}

		CRegisterRW::writeBit(REG_TYPE_HS_, HSW_DATA_DOSYNC, 0, 0);
	}
}

static void ComCtrlAddrSet(const unsigned int dwAddrOffset, const unsigned int dwLen) {
	unsigned short      wData[256] = { 0 };
	int                 i = 0;

	if (false == IsCrash(dwAddrOffset, dwLen, HSW_COMMUCONTRL, 300) &&
		false == IsCrash(dwAddrOffset, dwLen, HSW_COMMUCONTRL2, 300)) {
		return;
	}

	// 从原来地址中读取通讯控制位信息
	for (i = 0; i < 16; i++) {
		CRegisterRW::read(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_COMMUCONTRL + i * 16, 0, &wData[i * 8], 8);
	}

	for (i = 0; i < 16; i++) {
		CRegisterRW::read(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_COMMUCONTRL2 + i * 16, 0, &wData[( i + 16 ) * 8], 8);
	}

	// 同步数据掉电地址中
	CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_COMCTRLSAVEADDR, 0, &wData, 256);
	return;
}

static void MsgTipSet(const unsigned int dwAddrOffset, const unsigned int dwLen) {
	if (false == IsCrash(dwAddrOffset, dwLen, 2000, 1)) {
		return;
	}
	unsigned short  usAddrPos = CRegisterRW::readWord(REG_TYPE_HS_, 2000);
	if (0 == usAddrPos) {
		CMessageCenter::get_instance().insert(hmiproto::message_type_clear, "");
		return ;
	}
	CMessageCenter::get_instance().insert(hmiproto::message_type_insert, usAddrPos);
	CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_DWORD, 2000, 0, 0, 1);
}

static void KeyBoardAddrSet(const unsigned int dwAddrOffset, const unsigned int dwLen) {
	if (false == IsCrash(dwAddrOffset, dwLen, HSW_CURSPEED, 1) &&
		false == IsCrash(dwAddrOffset, dwLen, HSW_XYREGUPDATEORENTERMODE, 1)) {
		return;
	}
	short		nCursorSpeed = 0;
	int			nXYRegUpdate = 0;
	short		nPos_x = 0;
	short		nPos_y = 0;
	LPPROJSETUP pps = GetProjInfo();

	if (pps &&
		true == pps->BaseSet.bIsShowCursor) {
		CRegisterRW::read(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_CURSPEED, 0, &nCursorSpeed, 1);
		if (nCursorSpeed > 100) {//光标速度限制最大为100
			nCursorSpeed = 100;
			CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_CURSPEED, 0, &nCursorSpeed, 1);
		}
		if (nCursorSpeed < 0) {//光标速度限制最小为0
			nCursorSpeed = 0;
			CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_CURSPEED, 0, &nCursorSpeed, 1);
		}
		nXYRegUpdate = CRegisterRW::readBit(REG_TYPE_HS_, HSW_XYREGUPDATEORENTERMODE, 0);
		if (nXYRegUpdate == 1) {//通过光标位置寄存器值设置光标位置
            CRegisterRW::writeBit(REG_TYPE_HS_, HSW_XYREGUPDATEORENTERMODE, 0, 0);
			CRegisterRW::read(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_CURSORX, 0, &nPos_x, 1);//保存光标坐标
			CRegisterRW::read(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_CURSORY, 0, &nPos_y, 1);
            hmiproto::hmievent  event;
            event.set_type(HMIEVENT_SETCURSOR);
            event.set_x(nPos_x);
            event.set_y(nPos_y);
            CEventDispatch::getInstance().postEvent(event);
		}
	}
	return;
}

static void MediaAddrSet(const unsigned int dwAddrOffset, const unsigned int dwLen) {
	if (false == IsCrash(dwAddrOffset, dwLen, HSW_FORMATUDISK, 1) &&
		false == IsCrash(dwAddrOffset, dwLen, HSW_DELUSBORSD, 1)) {
		return;
	}

	int nSafeExit = 0;
	int nFormat = 0;

	CRegisterRW::read(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_DELUSBORSD, 0, &nSafeExit, 1);
	CRegisterRW::read(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_FORMATUDISK, 0, &nFormat, 1);
	if (nSafeExit & 1) {
		CHmiMedia::safeExitUdisk();
	}
	if (nSafeExit & ( 1 << 1 )) {
		CHmiMedia::safeExitSD();
	}
	if (nFormat == 1) {
		CHmiMedia::format();
	}

	return;
}

static void TrafficAddrSet(const unsigned int dwAddrOffset, const unsigned int dwLen) {

	unsigned short  wTemp = 0;

	if (true == IsCrash(dwAddrOffset, dwLen, HSW_NETDEVTYPE, 1)) {
		wTemp = CRegisterRW::readWord(REG_TYPE_HS_, HSW_NETDEVTYPE);
		Traffic::GetInstance().SetNetType(wTemp);
	}

	if (true == IsCrash(dwAddrOffset, dwLen, HSW_USETRIFFICFUNC, 1)) {
		wTemp = CRegisterRW::readWord(REG_TYPE_HS_, HSW_USETRIFFICFUNC);
		if (1 == wTemp) {
			Traffic::GetInstance().SetStates(true);
		}
		else {
			Traffic::GetInstance().SetStates(false);
		}
	}

	if (true == IsCrash(dwAddrOffset, dwLen, HSW_TRAFFICCORRECT, 1)) {
		wTemp = CRegisterRW::readWord(REG_TYPE_HS_, HSW_TRAFFICCORRECT);
		if (1 == wTemp) {
			CRegisterRW::writeWord(REG_TYPE_HS_, HSW_TRAFFICCORRECT, 0);
		}
	}

	if (true == IsCrash(dwAddrOffset, dwLen, HSW_TRAFFICRESET, 1)) {
		wTemp = CRegisterRW::readWord(REG_TYPE_HS_, HSW_TRAFFICRESET);
		if (1 == wTemp) {
			Traffic::GetInstance().ResetTraffic();
			CRegisterRW::writeWord(REG_TYPE_HS_, HSW_TRAFFICRESET, 0);
		}
	}

	return;
}

static void WebLimitAddrSet(const unsigned int dwAddrOffset, const unsigned int dwLen) {

	if (false == IsCrash(dwAddrOffset, dwLen, HSW_CLIENTVISITLIMITTYPE, 1)) {

		return;
	}

	unsigned short    wTemp = CRegisterRW::readWord(REG_TYPE_HS_, HSW_CLIENTVISITLIMITTYPE);
	if (wTemp > REMOTE_OFF) {
		return;
	}

	std::list<int> listID;
	CUserMgr::getInstances().getAllUserID(listID);
	disconnectWebClient(listID, wTemp);
	return;
}

static void OnTrdRcpDownLoadAndUpLoadMonitor(const unsigned int dwAddrOffset, const unsigned int dwLen) {
	unsigned short            wDownLoad = 0;
	unsigned short            wUpLoad = 0;

	if (IsCrash(dwAddrOffset, dwLen, HSW_TRDRCP_DOWNORUPLOAD, 1) == true)    // 配方下载
	{
		wDownLoad = CRegisterRW::readBit(REG_TYPE_HS_, HSW_TRDRCP_DOWNORUPLOAD, 0);

		if (1 == wDownLoad)//下载
		{
			unsigned short	wGroupNo = 0;

			CRegisterRW::writeBit(REG_TYPE_HS_, HSW_TRDRCP_DOWNORUPLOAD, 0, 0);
			wGroupNo = CRegisterRW::readWord(REG_TYPE_HS_, HSW_TRDRCP_GROUPID);
			CRegisterRW::writeWord(REG_TYPE_HS_, HSW_RECIPE_RESULT, RECIPE_RESULT_UPLOAD_BEGIN);  //正在下载
			CTraditionalRecipe::get_instance().download(wGroupNo);
		}
	}

	if (IsCrash(dwAddrOffset, dwLen, HSW_TRDRCP_DOWNORUPLOAD, 1) == true)    //配方上传
	{
		wUpLoad = CRegisterRW::readBit(REG_TYPE_HS_, HSW_TRDRCP_DOWNORUPLOAD, 1);
		if (1 == wUpLoad)//上传
		{
			unsigned short	wGroupNo = 0;

			CRegisterRW::writeBit(REG_TYPE_HS_, HSW_TRDRCP_DOWNORUPLOAD, 1, 0);
			wGroupNo = CRegisterRW::readWord(REG_TYPE_HS_, HSW_TRDRCP_GROUPID);
			CRegisterRW::writeWord(REG_TYPE_HS_, HSW_RECIPE_RESULT, RECIPE_RESULT_DOWNLOAD_BEGIN);  //正在上传
			CTraditionalRecipe::get_instance().upload(wGroupNo);
		}
	}
}

static void UdiskDownloadMonitor(const unsigned int dwAddrOffset, const unsigned int dwLen) {
	unsigned short      nSetValue = 0;
	std::string			strReturn;
	std::string			strCmd;

	if (IsCrash(dwAddrOffset, dwLen, HSW_UDISKDOWNLOAD, 1) == true)    // U盘下载操作
	{
		nSetValue = CRegisterRW::readWord(REG_TYPE_HS_, HSW_UDISKDOWNLOAD);     //读取进入后台寄存器的数值，1  进入后台   0  不进入
		if (nSetValue != 1 && nSetValue != 2 && nSetValue != 3) // 未触发一键进后台
		{
			return;
		}

		switch (nSetValue) {
		case 1:
			{
				GMExtern::showSysSetScreen(1);
				break;
			}
		case 2:
			{
				GMExtern::showSysSetScreen(1);
				break;
			}
		case 3:
			{
				GMExtern::showSysSetScreen(0);
				break;
			}
		default: break;
		}

		nSetValue = 0;
		CRegisterRW::writeWord(REG_TYPE_HS_, HSW_UDISKDOWNLOAD, nSetValue);	//还原寄存器的值，防止一直进入后台
		CRegisterRW::writeBit(REG_TYPE_HS_, HSW_UDISKDOWNLOADSCREEN + 2, 0, 1);//关闭U盘弹窗子画面
	}
}

static void swapHiLowWordAddrSet(const unsigned int dwWordPos, const unsigned int dwLen) {

	if (false == IsCrash(dwWordPos, dwLen, HSW_HILOWREVERTYPE, 1)) {
		return;
	}

	unsigned short    wVal = CRegisterRW::readWord(REG_TYPE_HS_, HSW_HILOWREVERTYPE);
	CRegisterRW::setSwapHiLowType(wVal);
	return;
}

static void screenProtectSet(const unsigned int dwWordPos, const unsigned int dwLen) {

	CScreenProtect& scrProtect = CSecureCtrl::getInstance().getScrProtect();

    if ((false == IsCrash(dwWordPos, dwLen, HSW_USEPROTECTSCREEN, 1)) &&
        (false == IsCrash(dwWordPos, dwLen, HSW_PROTECTSCREENTIME, 1)) &&
        (false == IsCrash(dwWordPos, dwLen, HSW_PROTECTSCREENNO, 1))) {
        return;
    }

	unsigned short wVal = CRegisterRW::readWord(REG_TYPE_HS_, HSW_USEPROTECTSCREEN);

	if (1 == wVal) {
        wVal = CRegisterRW::readWord(REG_TYPE_HS_, HSW_PROTECTSCREENTIME);
        if (wVal < 10) {
            wVal = 10;
            CRegisterRW::writeWord(REG_TYPE_HS_, HSW_PROTECTSCREENTIME, wVal);
        }
        else if (wVal > 43200) {
            wVal = 43200;
            CRegisterRW::writeWord(REG_TYPE_HS_, HSW_PROTECTSCREENTIME, wVal);
        }

        scrProtect.setWaitTime(wVal);
        wVal = CRegisterRW::readWord(REG_TYPE_HS_, HSW_PROTECTSCREENNO);
        scrProtect.setScrID(wVal);
        scrProtect.setFuncStatus(true);
	}
	else {
		scrProtect.setFuncStatus(false);
	}

	return;
}

static void partGradeSet(const unsigned int dwWordPos, const unsigned int dwLen) {

	if (false == IsCrash(dwWordPos, dwLen, HSW_PARTGRADESAVESET, 1)) {
		return;
	}

	CPartGrade& partGrade = CSecureCtrl::getInstance().getPartGrade();
	partGrade.changeGradeConf();

	return;
}

static int deleteLuaExtern(const unsigned long ulAddrOffset, const unsigned long ulLen) {

	if (!IsCrash(ulAddrOffset, ulLen, HSW_DELETELUAEXTERN, 1)) {

		return -1;
	}
	if (1 != CRegisterRW::readWord(REG_TYPE_HS_, HSW_DELETELUAEXTERN)) {

		return -2;
	}

#ifdef WIN32
	cbl::removeDir(HMIEXLUAPATH);
#else
	cbl::removeDir(HMIEXLUAPATH, true);
#endif // WIN32


	CRegisterRW::writeWord(REG_TYPE_HS_, HSW_DELETELUAEXTERN, 0);
	return 0;
}

static int setNetCardPriority(const unsigned long ulAddrOffset, const unsigned long ulLen) {
	unsigned short		nPriority = 0;
	int nReg = HSW_PRIORITYCLOUDNETDEV;
	if (PLATFORM::VBOX_MODE == PLATFORM::CPlatBoard::getDeviceMode()) {
		nReg = HSW_PRIORITYCLOUDNETDEV_R;
	}
	if (false == IsCrash(ulAddrOffset, ulLen, nReg, 1)) {
		return -1;
	}
	nPriority = CRegisterRW::readWord(REG_TYPE_HS_, nReg);
	if (CHMINet::get_instance().setNetCardPriority(nPriority) < 0) {
		CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_SETDEV_ERR,"setDevPriority Error");
	}
	return 0;
}

static int IpVerification(const Custom_Static_Ip_T ipInfo) {
	int Ret = 0;
	for (int nI = 0; nI < 12;nI++) {
		if (ipInfo.wValue[nI] > 255 || ipInfo.wValue[nI] < 0) {
			if (nI < 4) {
				CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_SETIP_ERR,"IP setting failed");
				CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_IPFMTERR,"IP format error, range 0~255");
				Ret = -1;
			}
			if (nI >= 4 && nI < 8) {
				CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_SETSYBMASK_ERR,"Subnet mask setting failed");
				CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_SUBMASKFMT_ERR,"Subnet mask format error, range 0~255");
				Ret = -2;
			}
			if (nI >= 8 && nI < 12) {
				CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_SETGATEWAY_ERR,"Gateway setting failed");
				CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_GATEWAYFMT_ERR, "Gateway format error, range 0~255");
				Ret = -3;
			}	
		}
	}
	for (int nI = 0; nI < 8; nI++) {
		if (ipInfo.wDnsValue[nI] > 255 || ipInfo.wDnsValue[nI] < 0) {
			if (nI < 4) {
				CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_SETDNS1_ERR,"DNS1 setting failed");
				CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_DNS1FMT_ERR, "DNS1 format error, range 0~255");
				Ret = -4;
			}
			if (nI >= 4 && nI < 8) {
				CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_SETDNS2_ERR, "DNS2 setting failed");
				CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_DNS2FMT_ERR, "DNS2 format error, range 0~255");
				Ret = -5;
			}
		}
	}
	return Ret;
}

static void ConfirmIpSettings(const unsigned int dwAddrOffset, const unsigned int dwLen) {
	unsigned short      nSetValue = 0;
	unsigned short      nSetConfirm = 0;
	std::string         m_strDevName;
	CRegister			addrNetAddr, addrNetDnsAddr;
	RW_PARAM_T			rw;
	if (IsCrash(dwAddrOffset, dwLen, HSW_NETLOCALIPSetConfirm, 1) == true) {
		nSetConfirm = CRegisterRW::readWord(REG_TYPE_HS_, HSW_NETLOCALIPSetConfirm);
		if (nSetConfirm != 1) {
			nSetConfirm = 0;
			CRegisterRW::writeWord(REG_TYPE_HS_, HSW_NETLOCALIPSetConfirm, nSetConfirm);	//将确认按钮置为0
			return;
		}
		nSetConfirm = 0;
		CRegisterRW::writeWord(REG_TYPE_HS_, HSW_NETLOCALIPSetConfirm, nSetConfirm);	//将确认按钮置为0
		std::map < std::string, std::string > networkCardName;
		CHMINet::get_instance().getNetworkCardName(networkCardName);
		std::map < std::string, std::string >::iterator iter = networkCardName.find("ETH");
		if (networkCardName.end() != iter) {
			m_strDevName = iter->second;
		}
		else{
			CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_ETH1_NOTEXIST, "Setup failed, Ethernet1 does not exist");
			return;
		}
		nSetValue = CRegisterRW::readBit(REG_TYPE_HS_, HSW_NETLOCALIPType, 0);    //读取切换动静态寄存器的值，0静态，1动态
		if (nSetValue == 0 ) // 静态IP
		{
			Custom_Static_Ip_T ipInfo;
			netT::stopDhcp(m_strDevName, false);
			CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_STATIC_IPSWITCH_SUCCESS, "IP Switch success, the current IP is static");
			if (PLATFORM::HMI_MODE == PLATFORM::CPlatBoard::getDeviceMode()) {
				genLocalRegister(REG_TYPE_HS_, DATA_TYPE_WORD, HSW_NETLOCALIPADDR1, 0, addrNetAddr);
			}
			else if (PLATFORM::VBOX_MODE == PLATFORM::CPlatBoard::getDeviceMode()) {
				genLocalRegister(REG_TYPE_HS_, DATA_TYPE_WORD, HSW_NETLOCALIPADDR1_R, 0, addrNetAddr);
			}
			genLocalRegister(REG_TYPE_HS_, DATA_TYPE_WORD, HSW_NETLOCALIPDNS1, 0, addrNetDnsAddr);
			CRegisterRW::read(RW_FORMAT_INT, addrNetAddr, ipInfo.wValue, 12, rw);
			CRegisterRW::read(RW_FORMAT_INT, addrNetDnsAddr, ipInfo.wDnsValue, 8, rw);
			if (0 > IpVerification(ipInfo)) {
				return;
			}
			ipInfo.sIp = cbl::format("%d.%d.%d.%d", ipInfo.wValue[0], ipInfo.wValue[1], ipInfo.wValue[2], ipInfo.wValue[3]);
			ipInfo.sNetMask = cbl::format("%d.%d.%d.%d", ipInfo.wValue[4], ipInfo.wValue[5], ipInfo.wValue[6], ipInfo.wValue[7]);
			ipInfo.sGateWay = cbl::format("%d.%d.%d.%d", ipInfo.wValue[8], ipInfo.wValue[9], ipInfo.wValue[10], ipInfo.wValue[11]);
			ipInfo.sDnsPrimary = cbl::format("%d.%d.%d.%d", ipInfo.wDnsValue[0], ipInfo.wDnsValue[1], ipInfo.wDnsValue[2], ipInfo.wDnsValue[3]);//增加前台可显示DNS1 wangzq 20221021
			ipInfo.sDnsSecondary = cbl::format("%d.%d.%d.%d", ipInfo.wDnsValue[4], ipInfo.wDnsValue[5], ipInfo.wDnsValue[6], ipInfo.wDnsValue[7]);//增加前台可显示DNS2 wangzq 20221021
			if (0 > netT::setIp(m_strDevName, ipInfo.sIp, ipInfo.sNetMask, ipInfo.sGateWay, false)) {
				CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_SETIP_ERR, "IP setting failed");
				return;
			}
			if (0 > netT::setDns(m_strDevName, ipInfo.sDnsPrimary, ipInfo.sDnsSecondary, false)) {
				CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_SETDNS_ERR, "DNS setting failed");
				return;
			}
			CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_SET_SUCCESS, "Setting succeeded");
		}
		else if (nSetValue == 1) //动态IP
		{
			if (0 > netT::startDhcp(m_strDevName, false)) {
				CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_DHCPSWITCH_ERR, "Switching to DHCP failed");
			}
			else {
				CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_SET_SUCCESS, "Setting succeeded");
			}
		}
	}
	return;
}

static void ConfirmIp2Settings(const unsigned int dwAddrOffset, const unsigned int dwLen) {
	unsigned short      nSetValue = 0;
	unsigned short      nSetConfirm = 0;
	std::string         m_strDevName;
	CRegister			addrNetAddr, addrNetDnsAddr;
	RW_PARAM_T			rw;
	if (IsCrash(dwAddrOffset, dwLen, HSW_NETLOCALIP2SetConfirm, 1) == true) {

		nSetConfirm = CRegisterRW::readWord(REG_TYPE_HS_, HSW_NETLOCALIP2SetConfirm);
		if (nSetConfirm != 1) {
		
			nSetConfirm = 0;
			CRegisterRW::writeWord(REG_TYPE_HS_, HSW_NETLOCALIP2SetConfirm, nSetConfirm);	//将确认按钮置为0
			return;
		}
		nSetConfirm = 0;
		CRegisterRW::writeWord(REG_TYPE_HS_, HSW_NETLOCALIP2SetConfirm, nSetConfirm);	//将确认按钮置为0
		std::map < std::string, std::string > networkCardName;
		CHMINet::get_instance().getNetworkCardName(networkCardName);
		std::map < std::string, std::string >::iterator iter = networkCardName.find("ETH1");
		if (networkCardName.end() != iter) {
			m_strDevName = iter->second;
		}
		else {
			CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_ETH2_NOTEXIST, "Setup failed, Ethernet2 does not exist");
			return;
		}
		nSetValue = CRegisterRW::readBit(REG_TYPE_HS_, HSW_NETLOCALIP2Type, 0);    //读取切换动静态寄存器的值，0静态，1动态
		if (nSetValue == 0) // 静态IP
		{
			Custom_Static_Ip_T ipInfo;
			netT::stopDhcp(m_strDevName, false);
			CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_STATIC_IPSWITCH_SUCCESS, "IP Switch success, the current IP is static");
			genLocalRegister(REG_TYPE_HS_, DATA_TYPE_WORD, HSW_NETLOCALIP2ADDR1, 0, addrNetAddr);
			genLocalRegister(REG_TYPE_HS_, DATA_TYPE_WORD, HSW_NETLOCALIP2DNS1, 0, addrNetDnsAddr);
			CRegisterRW::read(RW_FORMAT_INT, addrNetAddr, ipInfo.wValue, 12, rw);
			CRegisterRW::read(RW_FORMAT_INT, addrNetDnsAddr, ipInfo.wDnsValue, 8, rw);
			if (0 > IpVerification(ipInfo)) {
				return;
			}
			ipInfo.sIp = cbl::format("%d.%d.%d.%d", ipInfo.wValue[0], ipInfo.wValue[1], ipInfo.wValue[2], ipInfo.wValue[3]);
			ipInfo.sNetMask = cbl::format("%d.%d.%d.%d", ipInfo.wValue[4], ipInfo.wValue[5], ipInfo.wValue[6], ipInfo.wValue[7]);
			ipInfo.sGateWay = cbl::format("%d.%d.%d.%d", ipInfo.wValue[8], ipInfo.wValue[9], ipInfo.wValue[10], ipInfo.wValue[11]);
			ipInfo.sDnsPrimary = cbl::format("%d.%d.%d.%d", ipInfo.wDnsValue[0], ipInfo.wDnsValue[1], ipInfo.wDnsValue[2], ipInfo.wDnsValue[3]);//增加前台可显示DNS1 wangzq 20221021
			ipInfo.sDnsSecondary = cbl::format("%d.%d.%d.%d", ipInfo.wDnsValue[4], ipInfo.wDnsValue[5], ipInfo.wDnsValue[6], ipInfo.wDnsValue[7]);//增加前台可显示DNS2 wangzq 20221021
			if (0 > netT::setIp(m_strDevName, ipInfo.sIp, ipInfo.sNetMask, ipInfo.sGateWay, false)) {
				CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_SETIP_ERR, "IP setting failed");
				return;
			}
			if (0 > netT::setDns(m_strDevName, ipInfo.sDnsPrimary, ipInfo.sDnsSecondary, false)) {
				CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_SETDNS_ERR, "DNS setting failed");
				return;
			}
			CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_SET_SUCCESS, "Setting succeeded");
		}
		else if (nSetValue == 1) //动态IP
		{
			if (0 > netT::startDhcp(m_strDevName, false))
			{
				CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_DHCPSWITCH_ERR, "Switching to DHCP failed");
			}
			else
			{
				CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_SET_SUCCESS, "Setting succeeded");
			}
		}
	}
	return;
}

static void screenSaverSet(const unsigned int nAddrPos, const unsigned int nLen) {

    CScrSaver& scrSaver = CSecureCtrl::getInstance().getScrSaver();

    if ((false == IsCrash(nAddrPos, nLen, HSW_SCREENSAVER_CTRL, 1)) &&
        (false == IsCrash(nAddrPos, nLen, HSW_SCREENSAVER_TIME, 1)) &&
        (false == IsCrash(nAddrPos, nLen, HSW_SCREENSAVER_TYPE, 1))) {
        return;
    }

    unsigned short wVal = CRegisterRW::readBit(REG_TYPE_HS_, HSW_SCREENSAVER_CTRL, 1);

    if (1 == wVal) {
        wVal = CRegisterRW::readWord(REG_TYPE_HS_, HSW_SCREENSAVER_TIME);
        if (wVal < 1) {
            wVal = 1;
            CRegisterRW::writeWord(REG_TYPE_HS_, HSW_SCREENSAVER_TIME, wVal);
        }
        else if (wVal > 720) {
            wVal = 720;
            CRegisterRW::writeWord(REG_TYPE_HS_, HSW_SCREENSAVER_TIME, wVal);
        }

        scrSaver.setWaitTime(wVal);
        wVal = CRegisterRW::readWord(REG_TYPE_HS_, HSW_SCREENSAVER_TYPE);
        scrSaver.setType(wVal);
        scrSaver.setFuncStatus(true);
    }
    else {
        scrSaver.setFuncStatus(false);
    }

    return;
}

bool hswRegWriteCheck(const CRegister& addr, void* pData, unsigned int size, const RW_PARAM_T& rw) {
	if (NULL == pData || 0 == size) {
		return false;
	}

	unsigned int   dwLen = size;
	if (DATA_TYPE_BIT == addr.getDataType()) { // 位操作
		dwLen = (addr.getBitAddr() + size + 14) / 15;
	}

	DateAddrSet(addr.getAddr(), dwLen, (unsigned short*)pData); // 时间
	return true;
}
static int refreshNetSignal(const unsigned long ulAddrOffset, const unsigned long ulLen) {
	if (!IsCrash(ulAddrOffset, ulLen, HSW_REFRESHNETSINAL, 1)) {
		return -1;
	}
	if (1 != CRegisterRW::readWord(REG_TYPE_HS_, HSW_REFRESHNETSINAL)) {
		return -2;
	}
	if (0 != CHMINet::get_instance().refreshNetSignal()) {
		CMessageCenter::get_instance().insert(hmiproto::message_type_insert, MSG_SYS_REFRESH_ERR, "Refresh failed");
	}
	CRegisterRW::writeWord(REG_TYPE_HS_, HSW_REFRESHNETSINAL, 0);
	return 0;
}

static void KeyProcessRuntimeSet(const unsigned int dwAddrOffset, const unsigned int dwLen) {
	if (false == IsCrash(dwAddrOffset, dwLen, HSW_KeyBorderRunTimeType, 1)) {
		return;
	}
	unsigned short nRunTimeType = CRegisterRW::readWord(REG_TYPE_HS_, HSW_KeyBorderRunTimeType);
	if (true != PlatKeyBoardDev::setKeyProcessRuntime(nRunTimeType)){
		CRegisterRW::writeWord(REG_TYPE_HS_, HSW_KeyBorderRunTimeType, nRunTimeType);
	}
	return;
}
static void CursorModeSet(const unsigned int dwAddrOffset, const unsigned int dwLen) {
	if (false == IsCrash(dwAddrOffset, dwLen, HSW_CursorMode, 1)) {
		return;
	}
	unsigned short nCursorType = CRegisterRW::readWord(REG_TYPE_HS_, HSW_CursorMode);
	PlatKeyBoardDev::setCursorType(nCursorType);
	return;
}

static int screenAdaptationTypeSet(const unsigned long ulAddrOffset, const unsigned long ulLen) {
	static int nAdaptationType = -1;//不设置0，防止判断是否使用工程时冲突，导致修改不成功
	unsigned short nReadType;
	if (!IsCrash(ulAddrOffset, ulLen, HSW_REMOTE_ADAPTATION_TYPE, 1)) {
		return -1;
	}
	nReadType = CRegisterRW::readWord(REG_TYPE_HS_, HSW_REMOTE_ADAPTATION_TYPE);
	if (nReadType > 3) {
		CRegisterRW::writeWord(REG_TYPE_HS_, HSW_REMOTE_ADAPTATION_TYPE, nAdaptationType);
		return -2;
	}
	if (nReadType != nAdaptationType) {
		hmiproto::hmievent  event;
		event.set_type(HMIEVENT_ADAPTIVE_TYPE);
		event.set_adaptivetype(nReadType);
		CEventDispatch::getInstance().postEvent(event);
		nAdaptationType = nReadType;
	}
	return 0;
}

void hswRegChangeNotify(const CRegister& addr, unsigned int size, const RW_PARAM_T& rw) {
	if (0 == size) {
		return;
	}
	unsigned long   dwLen = size;
	if (DATA_TYPE_BIT == addr.getDataType()) { // 位操作
		dwLen = ( addr.getBitAddr() + size + 14 ) / 15;
	}
	unsigned long   dwWordOffset = addr.getAddr();
#ifndef WIN32
	RemoteAddrSet(dwWordOffset, dwLen);     // 远程
	SystemAddrSet(dwWordOffset, dwLen);     // 系统
	BeepAddrSet(dwWordOffset, dwLen);       // 蜂鸣
	BacklighAddrSet(dwWordOffset, dwLen);   // 背光灯
	SyncAddrSet(dwWordOffset, dwLen);       // sync
	ConfirmIpSettings(dwWordOffset, dwLen); //确认ip设置 20221019 Wangzq
	ConfirmIp2Settings(dwWordOffset, dwLen); //确认ip2设置 20221026 Wangzq
#endif
	ComPortAddrSet(dwWordOffset, dwLen);    // com口
	LanguageAddrSet(dwWordOffset, dwLen);   // 语言
	KeyBoardAddrSet(dwWordOffset, dwLen);	//键盘
	MediaAddrSet(dwWordOffset, dwLen);		//U盘、SD卡
	UserOptAddrSet(dwWordOffset, dwLen);    // 用户权限
	ComCtrlAddrSet(dwWordOffset, dwLen);    // 通讯控制
	TrafficAddrSet(dwWordOffset, dwLen);    // 网卡控制
	WebLimitAddrSet(dwWordOffset, dwLen);   // 远程监控控制
	OnTrdRcpDownLoadAndUpLoadMonitor(dwWordOffset, dwLen);// 监控传统配方上传下载
	UdiskDownloadMonitor(dwWordOffset, dwLen);	// U盘下载操作
	swapHiLowWordAddrSet(dwWordOffset, dwLen);  // 高低字反转设置 
	screenProtectSet(dwWordOffset, dwLen);  // 屏保相关设置
	partGradeSet(dwWordOffset, dwLen);		// 部件等级密码相关设置
	deleteLuaExtern(dwWordOffset, dwLen);	//删除lua扩展模块 
	setNetCardPriority(dwWordOffset, dwLen);
    screenSaverSet(dwWordOffset, dwLen);
	refreshNetSignal(dwWordOffset, dwLen);//刷新网络信号
	KeyProcessRuntimeSet(dwWordOffset, dwLen);
	CursorModeSet(dwWordOffset, dwLen);
	screenAdaptationTypeSet(dwWordOffset, dwLen);//远程监控画面适应类型设置
	MsgTipSet(dwWordOffset, dwLen);
}