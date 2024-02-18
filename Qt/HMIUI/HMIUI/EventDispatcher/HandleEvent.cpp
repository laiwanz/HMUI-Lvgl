#include "HandleEvent.h"
#include "EventDispatch.h"
#include "../PluginDLL/PluginDLL.h"
#include "GlobalModule/GlobalModule.h"
#ifndef WIN32
#include <unistd.h>
#include "btl/btl.h"
#endif
#include "RegisterOpt/RegisterOpt.h"
#include "webserver/webserver.h"
#include "hmiScreens/hmiScreens.h"
#include "utility/utility.h"
#include "macrosplatform/calculateRunTime.h"
#include "macrosplatform/registerAddr.h"
#include "../task/common.h"
using namespace UTILITY_NAMESPACE;
#ifndef WITH_UI
static EXIT_HMIUI m_funcExit = nullptr;
int setEventConfig(EXIT_HMIUI funcExit) {
	m_funcExit = funcExit;
	return 0;
}
#endif

static bool EventValid(const EVENT_T& event) {
	if (event.nClientId <= 0 ||   // 非画面事件
		QTCLIENT_T == event.nClientId) {// qtqt客户端事件不限制
		return true;
	}
	bool bRtn = true;
	unsigned short wType = CRegisterRW::readWord(REG_TYPE_HS_, HSW_CLIENTOPTLIMITTYPE);

	switch (event.nType) {
	case EVENT_CLICKDOWN:
	case EVENT_CLICKUP:
	case EVENT_DATATRANS:
		{
			// 客户端只允许看，忽略点击和部件消息传递
			if (2 == wType) {
				bRtn = false;
			}
			if (bRtn && event.nClientId < 70000) {
				if (REMOTE_NO_OPERATE == CWebServer::get_instance().getPermission(event.nClientId)) {
					bRtn = false;
				}
			}
		}
		break;
	default:
		break;
	}
	return bRtn;
}

static int initScreen(EVENT_T& event) {
	if (0 != CUserMgr::getInstances().userLogin(event.nClientId)) {
		hmiproto::hmievent  msgEvent;
		msgEvent.set_html(CWebServer::get_instance().getHtml(REMOTE_NOMORE_CLIENT));
		msgEvent.set_type(HMIEVENT_JUMPHTML);
		msgEvent.set_clientid(event.nClientId);
		CEventDispatch::getInstance().postEvent(msgEvent);
		return 0;
	}
	CEngineMgr::getInstance().initUserReg(event.nClientId);
	CSecureCtrl::getInstance().initUserSecure(event.nClientId);
	if (QTCLIENT_T == event.nClientId) {
		event.nScreenID = CHMIScreens::get_instance().getInitialScreen();
	}
	else { // web客户端要获取QTQT用户当前正在显示的画面号起始画面号
		CUser*  pUser = CUserMgr::getInstances().getUser(QTCLIENT_T);
		if (NULL == pUser) {
			return -1;
		}
		event.nScreenID = pUser->getCurBaseScrID();
	}
    {
        hmiproto::hmievent  msgEvent;
        msgEvent.set_type(HMIEVENT_INITSCRPART);
        msgEvent.set_clientid(event.nClientId);
        msgEvent.set_scrno(event.nScreenID);
		msgEvent.set_clienttopic(event.sClientTopic);
        CEventDispatch::getInstance().sendEvent(msgEvent);
    }
	if (true == Installment_GetStatus()) { // 分期付款到期了，再客户端登录时就要弹出到期画面
		hmiproto::hmievent  msgEvent;
		msgEvent.set_type(HMIEVENT_POPSCR);
		msgEvent.set_clientid(event.nClientId);
		msgEvent.set_scrno(SCREEN_INSTALLMENT);
		msgEvent.set_x(0);
		msgEvent.set_y(0);
		msgEvent.set_title(false);
        CEventDispatch::getInstance().sendEvent(msgEvent);
	}
	return 0;
}

static int reloadScreen(int nClientId) {
	int nScreenId = 0;
	CUser*  pUser = CUserMgr::getInstances().getUser(QTCLIENT_T);
	if (nullptr == pUser) {
		return -1;
	}
	nScreenId = pUser->getCurBaseScrID();
	{
		hmiproto::hmievent  msgEvent;
		msgEvent.set_type(HMIEVENT_RELOAD_SCREEN);
		msgEvent.set_clientid(nClientId);
		msgEvent.set_scrno(nScreenId);
		CEventDispatch::getInstance().sendEvent(msgEvent);
	}
	return 0;
}

//采集坐标等数据
static void DoAutoClickRecord(const char *pBuf, int nScreenID, int nType) {
	if (!CPluginDLL::IsLoad()) {
		return;
	}
	PluginAutoClick&        autoClick = CPluginDLL::GetClickFunc();
	if (true == autoClick.bLoad &&
		NULL != autoClick.Record) {
		autoClick.Record(pBuf, nScreenID, nType);
	}
}

static void ClickDownEvent(EVENT_T& event) {
	CUser*  pUser = CUserMgr::getInstances().getUser(event.nClientId);
	if (NULL == pUser) {
		return;
	}
	SET_TIME_CLICK;
	pUser->setOptTime(CTime::get_instance().getTickCount()); //hongw 20180606 保存点击的时间
	if (QTCLIENT_T == event.nClientId) {  // 是QTQT用户
		CClearLcd::stop();
	}
	DoAutoClickRecord(event.sEventbuffer.c_str(), event.nScreenID, 1);
	{
		POINT		pt;
		int			nUnBeep = 0;
		bool		bBeep = true;
		if (!event.sEventbuffer.empty()) {
			sscanf(event.sEventbuffer.c_str(), "%ld,%ld,%d", &pt.x, &pt.y, &nUnBeep);
		}
		bBeep = nUnBeep == 1 ? false : true;
		CHMIScreens::get_instance().onClick(event.nScreenID, event.nClientId, event.sPartName, CT_CLICKDOWN, pt, bBeep);
	}
	return;
}

static void ClickUpEvent(EVENT_T& event) {
	CUser*  pUser = CUserMgr::getInstances().getUser(event.nClientId);
	if (NULL == pUser) {
		return;
	}
	pUser->setOptTime(CTime::get_instance().getTickCount()); //hongw 20180606 保存点击的时间
	DoAutoClickRecord(event.sEventbuffer.c_str(), event.nScreenID, 0);
	{
		POINT		pt;
		int			nUnBeep = 0;
		bool		bBeep = true;
		if (!event.sEventbuffer.empty()) {
			sscanf(event.sEventbuffer.c_str(), "%ld,%ld,%d", &pt.x, &pt.y, &nUnBeep);
		}
		bBeep = nUnBeep == 1 ? false : true;
		CHMIScreens::get_instance().onClick(event.nScreenID, event.nClientId, event.sPartName, CT_CLICKUP, pt, bBeep);
	}
	return;
}

static void LuaClickEvent(EVENT_T& event) {
	int					nScreenId = -1;
	POINT				pt = { 0 };
	CLICKPARTTYPE_T		nType = CT_CLICKDOWN;
	if (EVENT_LUACLICKDOWN == event.nType) {
		nType = CT_CLICKDOWN;
	}
	else {
		nType = CT_CLICKUP;
	}
	if (!event.sPartName.empty()) {
		SMATCH result;
		if (REGEX_MATCH(event.sPartName, result, REGEX("^([0-9]+)_([a-zA-Z]+)_([0-9]+)")) != true) {
			return;
		}
		try
		{
			if (result.size() < 2){
				return;
			}
			nScreenId = std::stoi(result[1].str().c_str());
		}
		catch (const std::exception&) {
			return;
		}
	}
	CUser*  pUser = CUserMgr::getInstances().getUser(QTCLIENT_T);
	if (pUser) {
		pUser->setOptTime(CTime::get_instance().getTickCount()); //chenGH 2023.07.28 保存点击的时间
	}
	CHMIScreens::get_instance().onClick(nScreenId, QTCLIENT_T, event.sPartName, nType, pt);
	return;
}

static void PrintEvent(const EVENT_T& event) {
	char		szImagePath[MAX_PATH] = { 0 };
	bool		bSuccess = false;
	int			nType = 0;
	int			nPrintType = 0;
	if (!event.sEventbuffer.empty()) {
		sscanf(event.sEventbuffer.c_str(), "%d %260s %d", &nType, szImagePath, &nPrintType);
	}
	bSuccess = (bool) nType;
	if (bSuccess) {
		if (0 == nPrintType) {
			PrintModule_Add(szImagePath, (unsigned short) nPrintType);
		}
#ifdef _LINUX_
		sync();
#endif
	}
	return;
}

static void UpdateTopScrEvent(EVENT_T& event) {
	int					nTopScreenId = -1;
	POINT				pt = { 0 };
	if (event.sEventbuffer.empty()) {
		return;
	}
	sscanf(event.sEventbuffer.c_str(), "%d,%ld,%ld", &nTopScreenId, &pt.x, &pt.y);
	CHMIScreens::get_instance().setTopScreenNo(nTopScreenId, pt);
	return;
}

static void ClickIndirectScrPart(EVENT_T& event) {
	CHMIScreens::get_instance().ClickIndirectScrPart(event.nScreenID, event.sPartName);
	return;
}

static int doJsonEvent(EVENT_T& event) {
	int nScreenNo = event.nScreenID;
	int nClientId = event.nClientId;
	std::string sName = event.sPartName;
	std::string sEventbuffer = event.sEventbuffer;
	std::string srspbuffer = "";
	int nRtn = CHMIScreens::get_instance().doJsonbuffer(nScreenNo, nClientId, sName, sEventbuffer, srspbuffer);
	if (!srspbuffer.empty()) {
		hmiproto::hmievent event;
		event.set_scrno(nScreenNo);
		event.set_partname(sName);
		event.set_type(HMIEVENT_JSON);
		event.set_clientid(nClientId);
		event.set_eventbuffer(srspbuffer);
		CEventDispatch::getInstance().sendEvent(event);
	}
	if (0 == nRtn) {
		hmiproto::hmievent  event;
		event.set_type(HMIEVENT_UPDATEPART);
		CEventDispatch::getInstance().sendEvent(event);
	}
	return 0;
}

bool HmiEvent_Do(EVENT_T& event) {
	if (false == EventValid(event)) { // 事件无效
		return false;
	}
	switch (event.nType) {
	case EVENT_SCRINIT:
		{
			initScreen(event);
		}
		break;
	case EVENT_FREESCRCACHE: // 释放画面缓存
		{

		}
		break;
	case EVENT_WEBQUIT: // web用户退出
		{
			CSecureCtrl::getInstance().destoryUserSecure(event.nClientId);
			CUserMgr::getInstances().userQuit(event.nClientId);
            CHMIScreens::get_instance().quitClient(event.nClientId);
            CEngineMgr::getInstance().resetUserReg(event.nClientId);
		}
		break;
	case EVENT_CLICKDOWN:
		{
			ClickDownEvent(event);
		}
		break;
	case EVENT_CLICKUP:
		{
			ClickUpEvent(event);
		}
		break;
	case EVENT_LUACLICKDOWN:
	case EVENT_LUACLICKUP:
		{
			LuaClickEvent(event);
		}
		break;
	case EVENT_CURSOREVENT://保存光标坐标 Lizh 20180518
		{
			PlatKeyBoardDev::RevCursorInfo(( const std::string )event.sEventbuffer);
		}
		break;
	case EVENT_KEYDOWN://接收UI发来的键盘按下消息 Lizh 20181221
		{
			PlatKeyBoardDev::RevKeyInfo(event.sEventbuffer, true);
		}
		break;
	case EVENT_KEYUP://接收UI发来的键盘释放消息 Lizh 20181221
		{
			PlatKeyBoardDev::RevKeyInfo(event.sEventbuffer, false);
		}
		break;
	case EVENT_DATATRANS:
		{
			CHMIScreens::get_instance().setScreenAttr(event.nScreenID, event.nClientId, event.sPartName, event.sEventbuffer);
		}
		break;
	case EVENT_EXITHMI: {
#ifdef WITH_UI
            hmiproto::hmievent  hmievent;
			hmievent.set_type(HMIEVENT_EXITUI);
            CEventDispatch::getInstance().sendEvent(hmievent);
#else
		if (m_funcExit) {
			m_funcExit();
		}
#endif
		}
		break;
	case EVENT_PRINTPART:
		{
			PrintEvent(event);
		}
		break;
	case EVENT_DISPLAYMSG:
		{
			
		}
		break;
	case EVENT_SYSREGTIMER:
		{
			runSystemTimer();
		}
		break;
    case EVENT_CLOSE_SCRSAVER: /* 关闭屏保 */
        {
            CSecureCtrl::getInstance().getScrSaver().close();
        }
        break;
	case EVENT_UPDATETOPSCR: /* 更新置顶画面 */
		{
			UpdateTopScrEvent(event);
		}
		break;
	case EVENT_JSON:
		{
			doJsonEvent(event);
		}
		break;
	case EVENT_CLICKINDIRSCRPART: /* 点击间接画面 */
		{
			ClickIndirectScrPart(event);
		}
		break;
	case EVENT_EXITSYSSET: 
		{
			reloadScreen(QTCLIENT_T);
			CHMIScreens::get_instance().resumeVideo(QTCLIENT_T);
		}
		break;
	default:
		break;
	}
	return true;
}