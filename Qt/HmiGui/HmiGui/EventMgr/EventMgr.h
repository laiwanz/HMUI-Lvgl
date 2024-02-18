#ifndef _EVENTMGR_H_20201110
#define _EVENTMGR_H_20201110
#include "macrosplatform/event.h"
#include <string>
#include <functional>
using namespace std;

using FUNCTION_SENDEVENT = std::function<int(const EVENT_T&)>;
class QString;
class EventMgr {
public:
	static void     SetSendFunc(FUNCTION_SENDEVENT func);
	static void     SendInitScrEvent(int nScrNo);  // 发送初始化画面事件
	static void     SendMouseEvent(int nScrNo, int x, int y, bool bDown);   // 发送鼠标事件
	static void     SendDataTransEvent(int nScrNo, const QString &sPartName, const QString &sBuffer);  // 发送数据传输事件
	static void     SendDataTransEvent(int nScrNo, const std::string &sPartName, const std::string &sBuffer);  // 发送数据传输事件
	static void     SendScreenhotEvent(QString strInfo); // 截图结果返回
	static void     SendKeyBoardEvent(int nScrNo, int nKeyVal, bool bDown); // 发送键盘事件
	static void     SendExitEvent();    // 退出事件
	static void     SendCursorPosEvent(const QString& strBuf);
	static void		SendMsgTipPop(bool bPop, bool bClose);
    static void     sendCloseScrSaver();
	static void     SendUpdateTopScrEvent(const int nTopScr,int x ,int y);
	static void		SendClickIndirectScrEvent(const int nTopScr, const std::string &sPartName);
	static void		SendExitSysSet(); //退出后台画面
private:
	static void     SendEvent(const EVENT_T& event);
private:
	static int					m_nClientId;
	static FUNCTION_SENDEVENT	m_pSendEvent;
};

#endif // HMISENDEVENT_H
