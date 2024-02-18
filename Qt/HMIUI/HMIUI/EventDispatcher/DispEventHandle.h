#ifndef _DISPEVENTHANDLE__H
#define _DISPEVENTHANDLE__H

#include <string>
#include "ProtoXML/ProtoXML.h"
#include "macrosplatform/event.h"

class CDispEventHandle {
public:
    static CDispEventHandle& getInstance();
    bool dispStates();
	int sendScreenData(hmiproto::hmiact& act, int nClientId, bool bBroadcast = false);
private:
    CDispEventHandle();
    ~CDispEventHandle();
    
    int initScreen(hmiproto::hmievent& event);
    int updateScreen();
    int changeScreen(hmiproto::hmievent& event);
	int reloadScreen(hmiproto::hmievent& event);
    int popScreen(hmiproto::hmievent& event);
    int closeScreen(hmiproto::hmievent& event);
    int dispEmbedScreen(hmiproto::hmievent& event);
    int dispHtml(hmiproto::hmievent& event);
    int captureScreen(hmiproto::hmievent& event);
    int repaintScreen(hmiproto::hmievent& event);
    int dispCursor(hmiproto::hmievent& event);
    int getCursorPos(hmiproto::hmievent& event);
    int setCursorPos(hmiproto::hmievent& event);
    
    int freeScreenCache(hmiproto::hmievent& event);
    int dispMsg(hmiproto::hmievent& event);
    int clearLcd(hmiproto::hmievent& event);
    int clientVisitLimit(hmiproto::hmievent& event);
    int quit(hmiproto::hmievent& event);
    int popKeyboard(hmiproto::hmievent& event);
    int closeKeyboard(hmiproto::hmievent& event);
    int showScreenSaver(hmiproto::hmievent& event);
	int showSysSetScreen(hmiproto::hmievent& event);
	int sendAdaptiveType(hmiproto::hmievent& event);
    int doEvent(hmiproto::hmievent& event);

    int updateUserInfo(int nClientId, int nScreenNo);
    bool isQtClient(int nClientId);
	bool isSlaveClient(int nClientId);
    bool isWebClient(int nClientId);
    void getClientID(hmiproto::hmievent& msgEvent, std::list<int>& listClientId);  
private:
	int updateSelectMode(hmiproto::hmievent& event);
	int quitScreenSaver(hmiproto::hmievent& event);

private:
    friend class CEventDispatch;       // 允许事件处理模块直接使用私有方法
private:
    bool    m_bGuiQuit; /* 是否已退出 */
    bool    m_bShow;    /* 是否在显示 */
};

#endif