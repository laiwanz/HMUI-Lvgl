#include "DispEventHandle.h"
#ifdef WITH_UI
#include "HmiGui/HmiGui/HmiGui.h"
#endif
#include "HmiBase/HmiBase.h"
#include <list>
#include <string>
#include <sstream>
#include "macrosplatform/calculateRunTime.h"
#include "GlobalModule/GlobalModule.h"
#include <utility/utility.h>
#include "RegisterOpt/RegisterOpt.h"
#include "macrosplatform/registerAddr.h"
#include "../webserverCallback/webserverCallback.h"
#include "webserver/webserver.h"
#include "hmiScreens/hmiScreens.h"
#include "CSlaveModule/CSlaveModule.h"

using namespace UTILITY_NAMESPACE;
using namespace ProtoXML;
using namespace std;

CDispEventHandle::CDispEventHandle() {

    m_bGuiQuit = false;
    m_bShow = false;
}

CDispEventHandle::~CDispEventHandle() {

}

CDispEventHandle& CDispEventHandle::getInstance() {
    static CDispEventHandle    eventHandle;
    return eventHandle;
}

bool CDispEventHandle::dispStates() {
    return m_bShow;
}

int CDispEventHandle::initScreen(hmiproto::hmievent& event) {
	LPPROJSETUP				pps = GetProjInfo();
    hmiproto::hmiact        act;
    hmiproto::hmievent*     pmsgEvent = act.mutable_event();
	int nReadType = CRegisterRW::readWord(REG_TYPE_HS_, HSW_REMOTE_ADAPTATION_TYPE);
    pmsgEvent->set_scrno(event.scrno());
    pmsgEvent->set_type(HMIEVENT_INITSCRPART);
    pmsgEvent->set_clientid(event.clientid());
	pmsgEvent->set_degrees(pps->nRotateMode);
	pmsgEvent->set_adaptivetype(nReadType);
	pmsgEvent->set_connecttype(event.connecttype());
	pmsgEvent->set_clienttopic(event.clienttopic());
    if (0 > CHMIScreens::get_instance().getScreenAttr(act)) {
        return -1;
    }
    updateUserInfo(event.clientid(), pmsgEvent->scrno());
	hmiproto::hmiredis* redisdata = act.mutable_redis();
	redisdata->set_topic(event.clienttopic());
    sendScreenData(act, event.clientid());
    if (false == m_bShow) {
        m_bShow = true;
        CRegisterRW::writeWord(REG_TYPE_HS_, HSW_SCRJMPID, (unsigned short)pmsgEvent->scrno());
        SET_TIME_POW_MAIN;  // 进入main函数时间
        SET_TIME_POW_SHOW;  // 开机第一次显示画面时间
    }
    return 0;
}

int CDispEventHandle::reloadScreen(hmiproto::hmievent& event) {
	int nRtn = 0;
	int nClientId = 0;
	hmiproto::hmiact act;
	hmiproto::hmievent *pHMIEvent = act.mutable_event();
	if (pHMIEvent == nullptr) {
		return -1;
	}
	nClientId = event.clientid();
	pHMIEvent->set_scrno(event.scrno());
	pHMIEvent->set_type(HMIEVENT_CHANGESCR);
	pHMIEvent->set_clientid(nClientId);
	if ((nRtn = CHMIScreens::get_instance().quitClient(nClientId)) < 0) {
		printf("%s:quit client error:%d\r\n", __FUNCTION__, nRtn);
		return -2;
	}
	if ((nRtn = CHMIScreens::get_instance().getScreenAttr(act)) < 0) {
		printf("%s:get screen attr error:%d\r\n", __FUNCTION__, nRtn);
		return -3;
	}
	if ((nRtn = sendScreenData(act, event.clientid())) < 0) {
		printf("%s:send screen data error:%d\r\n", __FUNCTION__, nRtn);
		return -4;
	}
	return 0;
}

int CDispEventHandle::updateScreen() {
    std::list<int>       listID;
    CUserMgr::getInstances().getAllUserID(listID);
    for (auto& id : listID) {
        std::list<hmiproto::hmiact> listHmiact;
        CHMIScreens::get_instance().getScreenAttr(listHmiact, id);
        for (auto &iterAct : listHmiact) {
            hmiproto::hmiact        &act = iterAct;
            hmiproto::hmievent*     pHmievent = act.mutable_event();
            pHmievent->set_type(HMIEVENT_UPDATEPART);
            pHmievent->set_clientid(id);
            sendScreenData(act, id);
        }
    }
    CHMIScreens::get_instance().resetAllScreenAttrFlag();
    return 0;
}

static bool enableJump(hmiproto::hmievent& event) {
    if ((true == CHMIScreens::get_instance().isSubscreen(event.scrno())) || /* 目标画面是子画面不允许跳转 */
        (true == Installment_GetStatus()) || /* 分期付款到期不允许跳转 */
        (false == CHMIScreens::get_instance().isScreenEnableVisit(event))) { /* 画面使用了用户权限功能不允许跳转 */
        return false;
    }
    /* 目标画面不是当前画面允许跳转 */
    if (false == CHMIScreens::get_instance().isScreenRunning(event.scrno(), event.clientid())) {
        return true;
    }
    /* 限制了跳转到当前画面 */
    if (1 == CRegisterRW::readWord(REG_TYPE_HS_, HSW_JUMPCLOSECHILD)) {
        return false;
    }
    return true;
}

int CDispEventHandle::changeScreen(hmiproto::hmievent& event) {

    if (false == enableJump(event)) {
        return -1;
    }
    std::list<int>  listClientId;
    getClientID(event, listClientId);
	for (auto& id : listClientId) {
		// 预加载客户端信息（提前至所有客户端之前，是由于web端不执行脚本，若在关闭脚本之前，QT端执行的脚本内容会被web客户端预加载同步，画面寄存器监控无法监控跳转事件）
		updateUserInfo(id, event.scrno()); 
	}
	// 所有客户端画面预加载完毕后更新系统画面寄存器
	CRegisterRW::writeWord(REG_TYPE_HS_, HSW_SCRJMPID, (unsigned short)event.scrno());

	//clock_t start, end;
	//start = clock();

	if (event.alljump()) {
	
		auto qtid = QTCLIENT_T;
		hmiproto::hmiact        act;
		hmiproto::hmievent*     pEvent = act.mutable_event();
		pEvent->CopyFrom(event);
		pEvent->set_clientid(qtid);
		CHMIScreens::get_instance().closeAllRunningScreen(listClientId);
		CHMIScreens::get_instance().getScreenAttr(act, listClientId);
        bool bIsFirstSlave = true;

		for (auto& id : listClientId) {
            
			pEvent->set_clientid(id);
            if (true == isSlaveClient(id)) {
            
                if (bIsFirstSlave) {
                
                    sendScreenData(act, id, true);
                    bIsFirstSlave = false;
                }
            }
            else {
            
                sendScreenData(act, id);
            }
			
			/* 切换画面的时候清除临时角色权限等待操作，防止画面销毁无法还原操作 chenGH 20230110 */
			CSecureCtrl::getInstance().getAuthority().clearWaitOpt(id);
		}

		//end = clock();
		//printf("[%s-%d] change page use time=%f\n", __FUNCTION__, __LINE__, (double)(end - start) / CLOCKS_PER_SEC);
	}
	else {
	
		for (auto& id : listClientId) {
			hmiproto::hmiact        act;
			hmiproto::hmievent*     pEvent = act.mutable_event();
			pEvent->CopyFrom(event);
			pEvent->set_clientid(id);
			CHMIScreens::get_instance().closeAllRunningScreen(id);
			CHMIScreens::get_instance().getScreenAttr(act);
			sendScreenData(act, id);
			/* 切换画面的时候清除临时角色权限等待操作，防止画面销毁无法还原操作 chenGH 20230110 */
			CSecureCtrl::getInstance().getAuthority().clearWaitOpt(id);
		}

		//end = clock();
		//printf("[%s-%d] change page use time=%f\n", __FUNCTION__, __LINE__, (double)(end - start) / CLOCKS_PER_SEC);
	}
    
    return 0;
}

int CDispEventHandle::popScreen(hmiproto::hmievent& event) {
    if (false == CHMIScreens::get_instance().isSubscreen(event.scrno())) {
        return -1;
    }
    std::list<int>  listClientId;
    getClientID(event, listClientId);
    for (auto& id : listClientId) {
        if (true == CHMIScreens::get_instance().isScreenRunning(event.scrno(), id)) {
            continue;
        }
		CHMIScreens::get_instance().pauseVideo(event.scrno(), event.x(), event.y(), id);
		if (id == QTCLIENT_T && CHMIScreens::get_instance().isInSelectMode() == true){
			CHMIScreens::get_instance().setSelectPartShow(false); 
		}
        hmiproto::hmiact        act;
        hmiproto::hmievent*     pHmievent = act.mutable_event();
        pHmievent->CopyFrom(event);
        pHmievent->set_clientid(id);
        CHMIScreens::get_instance().getScreenAttr(act);
        sendScreenData(act, id);
    }
    return 0;
}

int CDispEventHandle::closeScreen(hmiproto::hmievent& event) {

    if (false == CHMIScreens::get_instance().isSubscreen(event.scrno())) {
        return -1;
    }

    std::list<int>  listClientId;
    getClientID(event, listClientId);

    for (auto& id : listClientId) {

        hmiproto::hmiact        act;
        hmiproto::hmievent*     pHmievent = act.mutable_event();

        pHmievent->CopyFrom(event);
        pHmievent->set_clientid(id);

		CHMIScreens::get_instance().resumeVideo(id);
        if (0 == CHMIScreens::get_instance().closeRunningScreen(event.scrno(), id)) {
            sendScreenData(act, id);
        }

        if (SCREEN_USERLOGIN == event.scrno()) {

            CRegister   addr;
            genLocalRegister(REG_TYPE_HU_, DATA_TYPE_WORD, HUW_PASSWORD, 0, addr);

            uint16_t    wClear[200] = { 0 };
            RW_PARAM_T  rw = { RW_MODE_CACHE_DEV, 0, 0, id, -1 };
            CRegisterRW::write(RW_FORMAT_STR, addr, wClear, MAX_PSW_LEN, rw);
        }
    }

    return 0;
}

int CDispEventHandle::dispEmbedScreen(hmiproto::hmievent& event) {

    if (false == CHMIScreens::get_instance().isSubscreen(event.scrno())) {
        return -1;
    }

    std::list<int>  listClientId;
    getClientID(event, listClientId);

    for (auto& id : listClientId) {

        hmiproto::hmiact        act;
        hmiproto::hmievent*     pEvent = act.mutable_event();

        pEvent->CopyFrom(event);
        pEvent->set_clientid(id);
        CHMIScreens::get_instance().getScreenAttr(act);
        sendScreenData(act, id);
    }

    return 0;
}

int CDispEventHandle::dispHtml(hmiproto::hmievent& event) {

    if (true == isQtClient(event.clientid())) {
        return -1;
    }

    hmiproto::hmiact        act;
    hmiproto::hmievent*     pEvent = act.mutable_event();

    pEvent->CopyFrom(event);
    sendScreenData(act, event.clientid());

    return 0;
}

int CDispEventHandle::captureScreen(hmiproto::hmievent& event) {

    hmiproto::hmiact        act;
    hmiproto::hmievent*     pEvent = act.mutable_event();

    pEvent->CopyFrom(event);
    sendScreenData(act, QTCLIENT_T);

    return 0;
}

int CDispEventHandle::repaintScreen(hmiproto::hmievent& event) {
    hmiproto::hmiact        act;
    hmiproto::hmievent*     pEvent = act.mutable_event();
    pEvent->CopyFrom(event);
    sendScreenData(act, QTCLIENT_T);
    return 0;
}

int CDispEventHandle::dispCursor(hmiproto::hmievent& event) {

    hmiproto::hmiact        act;
    hmiproto::hmievent*     pEvent = act.mutable_event();

    pEvent->CopyFrom(event);
    sendScreenData(act, QTCLIENT_T);

    return 0;
}

int CDispEventHandle::getCursorPos(hmiproto::hmievent& event) {
	unsigned int	nSelectMode = CHMIScreens::get_instance().getSelectMode();
	int				nEventType = -1;
	int				nOptType = -1;

	const string&               strData = event.eventbuffer();
	sscanf(strData.c_str(), "%d,%d", &nEventType, &nOptType);

	switch (nEventType)
	{
	case Cursor_Move: {
		if ((Cursor_Update != nOptType) &&
			(CurSorMode_Rect == nSelectMode ||
			CurSorMode_Cursor == nSelectMode ||
			CurSorMode_GrayPlacement == nSelectMode)) {
			CHMIScreens::get_instance().selectPart(nOptType);
			POINT	ptCursor = { -1,-1 };
			CHMIScreens::get_instance().getSelectCusor(ptCursor);
			hmiproto::hmievent  event;
			event.set_type(HMIEVENT_SETCURSOR);
			event.set_x(ptCursor.x);
			event.set_y(ptCursor.y);
			setCursorPos(event);
		}
		else{
			hmiproto::hmiact        act;
			hmiproto::hmievent*     pEvent = act.mutable_event();

			pEvent->CopyFrom(event);
			sendScreenData(act, QTCLIENT_T);
		}
		
		break;
	}
	case Cursor_Click: {
		if ((CurSorMode_Rect == nSelectMode ||
			CurSorMode_GrayPlacement == nSelectMode)&&
			Cursor_Press == nOptType) {
			POINT	ptCursor = { -1,-1 };
			CHMIScreens::get_instance().getSelectCusor(ptCursor);
			hmiproto::hmievent  event;
			event.set_type(HMIEVENT_SETCURSOR);
			event.set_x(ptCursor.x);
			event.set_y(ptCursor.y);
			setCursorPos(event);
		}

		hmiproto::hmiact        act;
		hmiproto::hmievent*     pEvent = act.mutable_event();

		pEvent->CopyFrom(event);
		sendScreenData(act, QTCLIENT_T);
		break;
	}
	default:
		break;
	}

   
    return 0;
}

int CDispEventHandle::setCursorPos(hmiproto::hmievent& event) {

    hmiproto::hmiact        act;
    hmiproto::hmievent*     pEvent = act.mutable_event();

    pEvent->CopyFrom(event);
    sendScreenData(act, QTCLIENT_T);

    return 0;
}

int CDispEventHandle::freeScreenCache(hmiproto::hmievent& event) {

    hmiproto::hmiact        act;
    hmiproto::hmievent*     pEvent = act.mutable_event();

    pEvent->CopyFrom(event);
    sendScreenData(act, QTCLIENT_T);

    return 0;
}

int CDispEventHandle::dispMsg(hmiproto::hmievent& event) {

	std::list<int>				listClientId;
	std::list<MESSAGES_ALL_T>	messages;


	getClientID(event, listClientId);
	if (0 == CMessageCenter::get_instance().get(listClientId, messages)) {

		for (auto &iter : messages) {

			hmiproto::hmiact			act;
			hmiproto::hmievent*			pEvent = act.mutable_event();
			hmiproto::messages*			pMessage = act.mutable_message();

			pEvent->CopyFrom(event);
			pEvent->set_clientid(iter.nClientId);
			pMessage->CopyFrom(iter.message);
			sendScreenData(act, iter.nClientId);
		}
	}
    return 0;
}

int CDispEventHandle::clearLcd(hmiproto::hmievent& event) {
    hmiproto::hmiact        act;
    hmiproto::hmievent*     pEvent = act.mutable_event();
    pEvent->CopyFrom(event);
    sendScreenData(act, QTCLIENT_T);
    return 0;
}

int CDispEventHandle::quit(hmiproto::hmievent& event) {
    hmiproto::hmiact        act;
    hmiproto::hmievent*     pEvent = act.mutable_event();
    pEvent->CopyFrom(event);
    sendScreenData(act, QTCLIENT_T);
    m_bGuiQuit = true;
    return 0;
}

int CDispEventHandle::popKeyboard(hmiproto::hmievent& event) {

	char		szFormat[MAX_INPUTFORMATLEN] = { 0 };
	RW_PARAM_T	rw = { RW_MODE_CACHE_DEV, 0, 0, event.clientid(), REG_SRC_NULL };
    if (false == CHMIScreens::get_instance().isSubscreen(event.scrno())) {
        return -1;
    }

    CUser*  user = CUserMgr::getInstances().getUser(event.clientid());
    if (NULL == user) {
        return -2;
    }
    
    hmiproto::hmievent  eventTemp;
	/* 在进入此函数前，会给huw801写值，此时关闭键盘不能把弹出键盘的值清零，所以重新写回 zhuolq20230615 */
	CRegisterRW::read(RW_FORMAT_STR, REG_TYPE_HU_, DATA_TYPE_WORD, HUW_INPUTFORMAT, 0, szFormat, MAX_INPUTFORMATLEN, &rw);
    if (true == user->getKbStatus()) {

        eventTemp.set_type(HMIEVENT_CLOSE_KEYBOARD);
        eventTemp.set_scrno(user->getKeyboard());
        eventTemp.set_clientid(event.clientid());
        closeKeyboard(eventTemp);
        eventTemp.Clear();
    }
    else if(true == CHMIScreens::get_instance().isScreenRunning(event.scrno(), event.clientid())){

        eventTemp.set_type(HMIEVENT_CLOSECHILDSCR);
        eventTemp.set_scrno(event.scrno());
        eventTemp.set_clientid(event.clientid());
        closeScreen(eventTemp);
        eventTemp.Clear();
    }
	CRegisterRW::write(RW_FORMAT_STR, REG_TYPE_HU_, DATA_TYPE_WORD, HUW_INPUTFORMAT, 0, szFormat, MAX_INPUTFORMATLEN, &rw);

    eventTemp.CopyFrom(event);
    eventTemp.set_type(HMIEVENT_POPSCR);
    if (0 != popScreen(eventTemp)) {
        return -3;
    }

    user->setKbStatus(true);
    user->setKeyboard(event.scrno());
    return 0;
}

int CDispEventHandle::closeKeyboard(hmiproto::hmievent& event) {

    CUser*  user = CUserMgr::getInstances().getUser(event.clientid());
    if (NULL == user) {
        return -1;
    }

    user->setKbStatus(false);

    hmiproto::hmievent  eventTemp;
    eventTemp.CopyFrom(event);
    eventTemp.set_type(HMIEVENT_CLOSECHILDSCR);

    closeScreen(eventTemp);
    return 0;
}


int CDispEventHandle::showScreenSaver(hmiproto::hmievent& event) {

    hmiproto::hmiact        act;
    hmiproto::hmievent*     pHmievent = act.mutable_event();

    pHmievent->CopyFrom(event);
    //pHmievent->set_clientid(QTCLIENT_T);
    sendScreenData(act, QTCLIENT_T);

    return 0;
}

int CDispEventHandle::showSysSetScreen(hmiproto::hmievent & event) {
	hmiproto::hmiact        act;
	hmiproto::hmievent     *pHmievent = act.mutable_event();
	CHMIScreens::get_instance().pauseVideo(event.scrno(), event.x(), event.y(), QTCLIENT_T);
	pHmievent->CopyFrom(event);
	sendScreenData(act, QTCLIENT_T);
	return 0;
}

int CDispEventHandle::quitScreenSaver(hmiproto::hmievent& event) {

	hmiproto::hmiact        act;
	hmiproto::hmievent*     pHmievent = act.mutable_event();

	pHmievent->CopyFrom(event);
	sendScreenData(act, QTCLIENT_T);

	return 0;
}
int CDispEventHandle::sendAdaptiveType(hmiproto::hmievent& event) {

	std::list<int>  listClientId;
	CUserMgr::getInstances().getAllUserID(listClientId);

	for (auto& id : listClientId) {

		if (QTCLIENT_T == id) {
			continue;
		}
		hmiproto::hmiact        act;
		hmiproto::hmievent*     pHmievent = act.mutable_event();
		pHmievent->CopyFrom(event);
		pHmievent->set_clientid(id);
		sendtoWebClient(act);
	}

	return 0;
}

int CDispEventHandle::doEvent(hmiproto::hmievent& event) {
    if (true == m_bGuiQuit) {
        return -1;
    }
    switch (event.type()) {
        case HMIEVENT_INITSCRPART: {
            initScreen(event);
        }
        break;
        case HMIEVENT_UPDATEPART: { /* 画面更新 */
            CHMIScreens::get_instance().runAllScreen();
            updateScreen();
            SET_TIME_PLCREAD;
            CHMIScreens::get_instance().runScreenCache();
        }
        break;
        case HMIEVENT_CHANGESCR: { /* 画面跳转 */
            CLEAR_SCREEN_TIME
            changeScreen(event);
            SET_TIME_SCR_SHOW;  /* 画面跳转时间 */
			CHMIScreens::get_instance().runScreenCache();
        }
        break;
        case HMIEVENT_POPSCR: { /* 弹出子画面 */
            popScreen(event);
        }
        break;
        case HMIEVENT_CLOSECHILDSCR: { /* 关闭子画面 */
            closeScreen(event);
        }
        break;
        case HMIEVENT_INDIRECTSCRINIT: { /* 间接画面显示 */
            dispEmbedScreen(event);
        }
        break;
        case HMIEVENT_UPDATESCREEN: { /* 重绘画面 */
            repaintScreen(event);
        }
        break;
        case HMIEVENT_PRINTPART: { /* 截图打印 */
            captureScreen(event);
        }
        break;
        case HMIEVENT_SHOWCURSOR: { /* 显示光标 */
            dispCursor(event);
        }
        break;
        case HMIEVENT_SETCURSOR: { /* 设置光标坐标 */
            setCursorPos(event);
        }
        break;
        case HMIEVENT_GETCURSOR: { /* 获取光标坐标 */
            getCursorPos(event);
        }
        break;
        case HMIEVENT_FREESCRCACHE: { /* 释放画面缓存 */
            freeScreenCache(event);
        }
        break;
        case HMIEVENT_JUMPHTML: { /* 跳转到html画面 */
            dispHtml(event);
        }
        break;
        case HMIEVENT_EXITUI: { /* 退出ui */
            quit(event);
        }
        break;
        case HMIEVENT_DISPLAYMSG: { /* 消息提示 */
            dispMsg(event);
        }
        break;
        case HMIEVENT_CLICKSTAUS: { /* 访问限制 */
            clientVisitLimit(event);
        }
        break;
        case HMIEVENT_CLEAR_LCD: { /* 清除lcd */
            clearLcd(event);
        }
        break;
        case HMIEVENT_POP_KEYBOARD: { /* 弹出键盘 */
            popKeyboard(event);
        }
        break;
        case HMIEVENT_CLOSE_KEYBOARD: { /* 关闭键盘 */
            closeKeyboard(event);
        }
        break;
        case HMIEVENT_SHOW_SCREENSAVER: { /* 显示屏保 */
            showScreenSaver(event);
        }
        break;
		case HMIEVENT_GO_SYSSET: { /*显示后台画面*/
			showSysSetScreen(event);
		}
		break;
		case HMIEVENT_UPDATESELECTMODE: {
			updateSelectMode(event);
		}
		break;
		case HMIEVENT_ADAPTIVE_TYPE: {	/* 设置远程画面适应类型 */
			sendAdaptiveType(event);
		}
		break;
		case HMIEVENT_CLOSE_SCREENSAVER: { /* 退出屏保 */
			quitScreenSaver(event);
		}
		break;
		case HMIEVENT_JSON: {
			hmiproto::hmiact act;
			hmiproto::hmievent *pHmievent = act.mutable_event();
			pHmievent->CopyFrom(event);
			pHmievent->set_clientid(event.clientid());
			this->sendScreenData(act, event.clientid());
		}
		break;
		case HMIEVENT_RELOAD_SCREEN: {
			reloadScreen(event);
		}
		break;
        default: {
        }
        break;
    }
    return 0;
}

int CDispEventHandle::clientVisitLimit(hmiproto::hmievent& event) {
    if (false == event.has_eventbuffer()) {
        return -1;
    }
    std::list<int>  listClientId;
    CUserMgr::getInstances().getAllUserID(listClientId);
    for (auto& id : listClientId) {
        if (QTCLIENT_T == id) {
            continue;
        }
        int                 nPermission = CWebServer::get_instance().getPermission(id);
        std::string         sVal = event.eventbuffer();
        hmiproto::hmiact    act;
        hmiproto::hmievent* pEvent = act.mutable_event();
        if ("ClickOn" == sVal &&
            (REMOTE_ONLY_READ == nPermission ||
             REMOTE_NO_OPERATE == nPermission)) {
            sVal = "ClickOff";
        }
        pEvent->set_type(HMIEVENT_CLICKSTAUS);
        pEvent->set_clientid(id);
        pEvent->set_eventbuffer(sVal);
        sendScreenData(act, id);
    }
    return 0;
}

int CDispEventHandle::updateUserInfo(int nClientId, int nScreenNo) {
    PROJSETUP*  pps = GetProjInfo();
    if (NULL == pps) {
        return -1;
    }
    CUser*  pUser = CUserMgr::getInstances().getUser(nClientId);
    if (nullptr == pUser) {
        return -2;
    }
    pUser->setCurBaseScrID(nScreenNo);
    pUser->setKbStatus(false);
    CRegisterRW::writeWord(REG_TYPE_HU_, HUW_SCRJUMPID, (unsigned short)nScreenNo, nClientId);
    if ((true == pps->BaseSet.bUseSysContrl) &&
        (QTCLIENT_T == nClientId)) {
        RW_PARAM_T    rw;
        CRegisterRW::write(RW_FORMAT_INT, pps->BaseSet.addrSysContrl, &nScreenNo, 1, rw);
    }
    return 0;
}

bool CDispEventHandle::isQtClient(int nClientId) {

    if (QTCLIENT_T == nClientId) {
        return true;
    }

    return false;
}

bool CDispEventHandle::isSlaveClient(int nClientId)
{
	if (nClientId >= 70000) {
	
		return true;
	}
	return false;
}

bool CDispEventHandle::isWebClient(int nClientId) {
    if (QTCLIENT_T != nClientId) {
        return true;
    }
    return false;
}

void CDispEventHandle::getClientID(hmiproto::hmievent& event, std::list<int>& listClientId) {
    if ((true == event.has_clientid()) && 
        (event.clientid() > 0) &&
        (false == event.alljump())) {
        listClientId.push_back(event.clientid());
    }
    else {
        CUserMgr::getInstances().getAllUserID(listClientId);
    }
    return;
}

int CDispEventHandle::sendScreenData(hmiproto::hmiact& act, int nClientId, bool bBroadcast) {
    if (true == m_bGuiQuit) { // 已经退出显示
        return -1;
    }

	//printf("[%s-%d] clientId:%d\n", __FUNCTION__, __LINE__, nClientId);
	if (true == isQtClient(nClientId)) {
#ifdef WITH_UI
		HmiGui::doHmiEventQuickly(act);
#endif
	} 
	else if (true == isSlaveClient(nClientId)) {

		//sendtoSlaveClient(act);
        CSlaveModule::get_instance().sendtoSlaveClient(act, bBroadcast);
	}
	else if(true == isWebClient(nClientId)) {
		sendtoWebClient(act);
	}

    return 0;
}

int CDispEventHandle::updateSelectMode(hmiproto::hmievent& event) {
	int				nSelectMode = 0;
	const string&   strData = event.eventbuffer();
	sscanf(strData.c_str(), "%d", &nSelectMode);
	CHMIScreens::get_instance().updateSelectMode(nSelectMode);
	if (CurSorMode_Rect == nSelectMode ||
		CurSorMode_GrayPlacement == nSelectMode||
		CurSorMode_Cursor == nSelectMode) {
		POINT	ptCursor = { -1,-1 };
		CHMIScreens::get_instance().getSelectCusor(ptCursor);
		hmiproto::hmievent  event;
		event.set_type(HMIEVENT_SETCURSOR);
		event.set_x(ptCursor.x);
		event.set_y(ptCursor.y);
		setCursorPos(event);
	}
	return 0;
}