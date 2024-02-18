#include "EventMgr.h"
#include <QString>
#include <QPoint>
#include <QCursor>
#include "../hmiutility.h"
#include "cbl/cbl.h"
#include "multiplatform/multiplatform.h"

int EventMgr::m_nClientId = QTCLIENT_T;
FUNCTION_SENDEVENT EventMgr::m_pSendEvent = NULL;


void EventMgr::SetSendFunc(FUNCTION_SENDEVENT func) {
	m_pSendEvent = func;
	return;
}

void EventMgr::SendInitScrEvent(int nScrNo) {
	EVENT_T event;
	event.nClientId = m_nClientId;
	event.nType = EVENT_SCRINIT;
	event.nScreenID = nScrNo;
	SendEvent(event);
	return;
}

void EventMgr::SendMouseEvent(int nScrNo, int x, int y, bool bDown) {
	EVENT_T    event;
	event.nClientId = m_nClientId;
	event.nScreenID = nScrNo;
	event.sEventbuffer = cbl::format("%d,%d", x, y);
	if (true == bDown) {
		event.nType = EVENT_CLICKDOWN;
	}
	else {
		event.nType = EVENT_CLICKUP;
	}
	SendEvent(event);
	return;
}

void EventMgr::SendDataTransEvent(int nScrNo, const std::string &sPartName, const std::string &sBuffer)  {
	EVENT_T event;
	event.nClientId = QTCLIENT_T;
	event.nType = EVENT_JSON;
	event.nScreenID = nScrNo;
	event.sPartName = sPartName;
	event.sEventbuffer = sBuffer;
	SendEvent(event);
	return;
}

void EventMgr::SendDataTransEvent(int nScrNo, const QString &sPartName, const QString &sBuffer) {
	EVENT_T event;
	event.nClientId = m_nClientId;
	event.nType = EVENT_DATATRANS;
	event.nScreenID = nScrNo;
	event.sPartName = sPartName.toStdString();
	event.sEventbuffer = sBuffer.toStdString();
	SendEvent(event);
	return;
}

void EventMgr::SendScreenhotEvent(QString strInfo) {
	EVENT_T event;

	event.nType = EVENT_PRINTPART;
	event.nClientId = m_nClientId;
	event.sEventbuffer = strInfo.toStdString();
	SendEvent(event);
	return;
}

void EventMgr::SendKeyBoardEvent(int nScrNo, int nKeyVal, bool bDown) {
	EVENT_T event;

	event.nClientId = m_nClientId;
	event.nScreenID = nScrNo;
	event.sEventbuffer = cbl::format("%d", nKeyVal);
	if (true == bDown) {
		event.nType = EVENT_KEYDOWN;
	}
	else {
		event.nType = EVENT_KEYUP;
	}

	SendEvent(event);
	return;
}

void EventMgr::SendExitEvent() {
	EVENT_T    event;
	event.nType = EVENT_EXITHMI;
	SendEvent(event);
	return;
}

void EventMgr::SendCursorPosEvent(const QString& strBuf) {
	EVENT_T		event;
	event.nClientId = m_nClientId;
	event.nScreenID = -1;
	event.nType = EVENT_CURSOREVENT;
	event.sEventbuffer = strBuf.toStdString();
	SendEvent(event);
	return;
}

void EventMgr::SendMsgTipPop(bool bPop, bool bClose) {
	EVENT_T	event;
	event.nClientId = m_nClientId;
	event.nScreenID = -1;
	event.nType = EVENT_DISPLAYMSG;
	event.sEventbuffer = cbl::format("%d %d", bPop, bClose);
	SendEvent(event);
	return;
}

void EventMgr::sendCloseScrSaver() {
    EVENT_T	event;
    event.nClientId = m_nClientId;
    event.nType = EVENT_CLOSE_SCRSAVER;
    SendEvent(event);
    return;
}

void EventMgr::SendEvent(const EVENT_T& event) {
	if (NULL == m_pSendEvent) {
		return;
	}
	m_pSendEvent(event);
	return;
}

void EventMgr::SendUpdateTopScrEvent(const int nTopScr, int x, int y) {
	EVENT_T		event;
	event.nClientId = m_nClientId;
	event.nScreenID = -1;
	event.nType = EVENT_UPDATETOPSCR;
	event.sEventbuffer = cbl::format("%d,%d,%d", nTopScr, x, y);
	SendEvent(event);
	return;
}

void EventMgr::SendClickIndirectScrEvent(const int nTopScr, const std::string &sPartName) {
	EVENT_T		event;

	event.nClientId = m_nClientId;
	event.nScreenID = nTopScr;
	event.nType = EVENT_CLICKINDIRSCRPART;
	event.sPartName = sPartName;
	SendEvent(event);
	return;
}

void EventMgr::SendExitSysSet() {
	EVENT_T    event;
	event.nType = EVENT_EXITSYSSET;
	SendEvent(event);
}
