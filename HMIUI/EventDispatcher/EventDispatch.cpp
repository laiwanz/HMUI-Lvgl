#include <thread>
#include <algorithm>
#include "EventDispatch.h"
#include "DispEventHandle.h"
#include "HandleEvent.h"
#include "utility/utility.h"
#include "platform/platform.h"
#include "hmiScreens/hmiScreens.h"
#include "CSlaveModule/CSlaveModule.h"
#ifndef WIN32
#include <sys/prctl.h>
#include "btl/btl.h"
#endif

using namespace UTILITY_NAMESPACE;
using namespace PLATFORM_NAMESPACE;
using namespace std;

#define MAX_EVENT_NUM           128         /* 每个链表中最大的事件数量 */
#define ON_PLC_READ_TIMER       50          /* plc addr read loop time 50ms */
#define SYS_REG_READ_TIMER      200         /* systerm addr loop time 200ms */
#define TIMEOUT_TIME            50          /* timeout time 50ms */

enum {
    EVENT_SCR = 0,
    EVENT_HMI = 1,
};

enum {
    stateExited = 0,
    stateRunging = 1,
    stateExiting = 2,
};

/* CEventDispatch
**/
CEventDispatch::CEventDispatch() {
 
    m_listScrEvent.clear();
    m_listEvent.clear();
    m_nRunStates = stateExited;
	m_bReduceSpeedStrategy = false;

	initReductionConfig();
}

/* ~CEventDispatch
**/
CEventDispatch::~CEventDispatch() {
    stop();
	{
		std::unique_lock<mutex> lock(m_mutex);
		m_listEvent.clear();
		m_listScrEvent.clear();
	}
}

CEventDispatch& CEventDispatch::getInstance() {
    static CEventDispatch   dispatch;
    return dispatch;
}

void CEventDispatch::initReductionConfig() {
	std::string sDeviceId = "", sHmiModel = "", sHmiType = "", sSN = "";
	SECURE::WMachine cMachineT;
	cMachineT.GetSN(sSN);
	if (cMachineT.getDeviceId(sDeviceId) < 0) {
		return;
	}
	if (3 > sDeviceId.size()) {
		return;
	}
	sDeviceId = cbl::lcase(sDeviceId);
	sHmiModel = sDeviceId.substr(0, 1);
	if ("n" == sHmiModel) {
		//if is illegal mac will so slow
		sHmiType = sDeviceId.substr(1, 1);
		if ("1" == sHmiType) {
			m_bReduceSpeedStrategy = true;
			sHmiType = sDeviceId.substr(2, 1);
			if ("7" == sHmiType) {
				this->m_reduceSpeedNode.fClickReductionTime = 2;
				this->m_reduceSpeedNode.fRefreshReductionTime = 100;
				m_reduceSpeedNode.fClickReductionMaxTime = 180;
				m_reduceSpeedNode.fChangeReductionTime = 2.1;
				m_reduceSpeedNode.nStartReductionScreenCount = 30;
				m_reduceSpeedNode.fScreenReductionTime = 79;
				m_reduceSpeedNode.fMaxScreenReductionTime = 790;
				m_reduceSpeedNode.fMaxReductionTime = 900;
				m_reduceSpeedNode.nFirstAdjustReductionScreenCount = 30;
				m_reduceSpeedNode.nSecondAdjustReductionScreenCount = 40;
				m_reduceSpeedNode.nThirdAdjustReductionScreenCount = 50;
				m_reduceSpeedNode.fFirstAdjustAddClickExtraDelayTime = 50;
				m_reduceSpeedNode.fSecondAdjustAddClickExtraDelayTime = 100;
				m_reduceSpeedNode.fThirdAdjustAddClickExtraDelayTime = 140;
			}
			else if ("8" == sHmiType) {
				sHmiType = sSN.substr(4, 1);
				if ("s" == sHmiType || "S" == sHmiType) {
					this->m_reduceSpeedNode.fClickReductionTime = 0;
					this->m_reduceSpeedNode.fRefreshReductionTime = 0;
					m_reduceSpeedNode.fClickReductionMaxTime = 0;
					m_reduceSpeedNode.fChangeReductionTime = 0;
					m_reduceSpeedNode.nStartReductionScreenCount = 0;
					m_reduceSpeedNode.fScreenReductionTime = 0;
					m_reduceSpeedNode.fMaxScreenReductionTime = 0;
					m_reduceSpeedNode.fMaxReductionTime = 0;
					m_reduceSpeedNode.fFirstAdjustAddClickExtraDelayTime = 0;
					m_reduceSpeedNode.fSecondAdjustAddClickExtraDelayTime = 0;
					m_reduceSpeedNode.fThirdAdjustAddClickExtraDelayTime = 0;
				}
				else {
					this->m_reduceSpeedNode.fClickReductionTime = 2;
					this->m_reduceSpeedNode.fRefreshReductionTime = 100;
					m_reduceSpeedNode.fClickReductionMaxTime = 180;
					m_reduceSpeedNode.fChangeReductionTime = 1.8;
					m_reduceSpeedNode.nStartReductionScreenCount = 0;
					m_reduceSpeedNode.fScreenReductionTime = 0;
					m_reduceSpeedNode.fMaxScreenReductionTime = 0;
					m_reduceSpeedNode.fMaxReductionTime = 0;
					m_reduceSpeedNode.fFirstAdjustAddClickExtraDelayTime = 0;
					m_reduceSpeedNode.fSecondAdjustAddClickExtraDelayTime = 0;
					m_reduceSpeedNode.fThirdAdjustAddClickExtraDelayTime = 0;
				}
			}
			else {
				//if is illegal mac will so slow
				this->m_reduceSpeedNode.fClickReductionTime = 100;
				this->m_reduceSpeedNode.fRefreshReductionTime = 1000;
				m_reduceSpeedNode.fClickReductionMaxTime = 2000;
				m_reduceSpeedNode.fChangeReductionTime = 200;
				m_reduceSpeedNode.nStartReductionScreenCount = 30;
				m_reduceSpeedNode.fScreenReductionTime = 75;
				m_reduceSpeedNode.fMaxScreenReductionTime = 750;
				m_reduceSpeedNode.fMaxReductionTime = 2000;
			}
		}
		else {
			m_bReduceSpeedStrategy = true;
			this->m_reduceSpeedNode.fClickReductionTime = 100;
			this->m_reduceSpeedNode.fRefreshReductionTime = 1000;
			m_reduceSpeedNode.fClickReductionMaxTime = 2000;
			m_reduceSpeedNode.fChangeReductionTime = 200;
			m_reduceSpeedNode.nStartReductionScreenCount = 30;
			m_reduceSpeedNode.fScreenReductionTime = 75;
			m_reduceSpeedNode.fMaxScreenReductionTime = 750;
			m_reduceSpeedNode.fMaxReductionTime = 2000;
		}
	}
	else if ("y" == sHmiModel) {
		sHmiType = sDeviceId.substr(1, 1);
		if ("1" == sHmiType) {
			m_bReduceSpeedStrategy = true;
			sHmiType = sDeviceId.substr(2, 1);
			if ("7" == sHmiType) {
				this->m_reduceSpeedNode.fClickReductionTime = 2;
				this->m_reduceSpeedNode.fRefreshReductionTime = 100;
				m_reduceSpeedNode.fClickReductionMaxTime = 180;
				m_reduceSpeedNode.fChangeReductionTime = 2.3;
				m_reduceSpeedNode.nStartReductionScreenCount = 30;
				m_reduceSpeedNode.fScreenReductionTime = 80;
				m_reduceSpeedNode.fMaxScreenReductionTime = 800;
				m_reduceSpeedNode.fMaxReductionTime = 1000;
				m_reduceSpeedNode.nFirstAdjustReductionScreenCount = 30;
				m_reduceSpeedNode.nSecondAdjustReductionScreenCount = 40;
				m_reduceSpeedNode.nThirdAdjustReductionScreenCount = 50;
				m_reduceSpeedNode.fFirstAdjustAddClickExtraDelayTime = 100;
				m_reduceSpeedNode.fSecondAdjustAddClickExtraDelayTime = 150;
				m_reduceSpeedNode.fThirdAdjustAddClickExtraDelayTime = 200;
			}
			else {
				
				//In IE module only special models do decelerate limit
				this->m_reduceSpeedNode.fClickReductionTime = 0;
				this->m_reduceSpeedNode.fRefreshReductionTime = 0;
				m_reduceSpeedNode.fClickReductionMaxTime = 0;
				m_reduceSpeedNode.fChangeReductionTime = 0;
				m_reduceSpeedNode.nStartReductionScreenCount = 0;
				m_reduceSpeedNode.fScreenReductionTime = 0;
				m_reduceSpeedNode.fMaxScreenReductionTime = 0;
				m_reduceSpeedNode.fMaxReductionTime = 0;
			}
		}
	}
}

/* start
**/
int CEventDispatch::start() {
	printf("%s---%d--%d\r\n", __FUNCTION__, __LINE__, m_nRunStates);
    if (stateExited != m_nRunStates) {
        return -1;
    }
    std::thread thread([this] {
        workThread();
    });
	thread.detach();
    return 0;
}


int CEventDispatch::stop() {
    if (stateExited == m_nRunStates) {
        return -1;
    }
    if (stateRunging == m_nRunStates) {
        m_nRunStates = stateExiting;
        wakeUpThread();
        while (m_nRunStates != stateExited) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
    return 0;
}

/* postEvent
**/
int CEventDispatch::sendEvent(const hmiproto::hmievent& event) {
    hmiproto::hmievent  hmievent;
	hmievent.CopyFrom(event);
	if (m_bReduceSpeedStrategy &&
		(hmievent.type() == HMIEVENT_CHANGESCR ||
		hmievent.type() == HMIEVENT_POPSCR ||
		hmievent.type() == HMIEVENT_CLOSECHILDSCR ||
		hmievent.type() == HMIEVENT_INDIRECTSCRINIT)) {
		int nPeriod = (unsigned long long)CHMIScreens::get_instance().getScreenPartSize(hmievent.scrno()) * m_reduceSpeedNode.fChangeReductionTime;
		nPeriod -= m_reduceSpeedNode.fCurrentClickReductionTime;
		if (0 > nPeriod) {
			nPeriod = 0;
		}
		int nCurrentScreenCount = CHMIScreens::get_instance().getTotalScreenNum();
		float tempReductionTime = 0;
		if (nCurrentScreenCount > m_reduceSpeedNode.nStartReductionScreenCount) {
			tempReductionTime = (nCurrentScreenCount - m_reduceSpeedNode.nStartReductionScreenCount) * m_reduceSpeedNode.fScreenReductionTime;
		}
		if (tempReductionTime > m_reduceSpeedNode.fMaxScreenReductionTime) {
			tempReductionTime = m_reduceSpeedNode.fMaxScreenReductionTime;
		}
		nPeriod += tempReductionTime;
		if (nPeriod > m_reduceSpeedNode.fMaxReductionTime) {
			nPeriod = m_reduceSpeedNode.fMaxReductionTime;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds((unsigned long long)nPeriod));
	}
	if (CDispEventHandle::getInstance().doEvent(hmievent) < 0) {
		return -1;
	}
    return 0;
}


/* postEvent
**/
int CEventDispatch::postEvent(const hmiproto::hmievent& event, bool bFilter) {
    if ((true == bFilter) &&
        (true == exist(event))) {
        printf("---------add scr event error [%d]\n", event.type());
        return -1;
    }
    {
        lock_guard<mutex>   lock(m_mutex);
        m_listScrEvent.push_back(event);
        if (m_listScrEvent.size() > 10) {
            printf("---------dispatch is busy [scr event total size %d]\n", m_listScrEvent.size());
            if (m_listScrEvent.size() >= MAX_EVENT_NUM) {
                m_listScrEvent.pop_front();
                printf("---------scr event list full\n");
            }
        }
    }
    wakeUpThread();
    return 0;
}

/* postEvent
**/
int CEventDispatch::postEvent(const hmiproto::hmievent& event) {
    if (0 != postEvent(event, false)) {
        return -1;
    }
    return 0;
}

/* postEvent
**/
int CEventDispatch::postEvent(const EVENT_T& event, bool bFilter) {

	// std::cout << "postEvent ------- " << std::endl;
	
    if ((true == bFilter) &&
        (true == exist(event))) {
        printf("---------add event error [%d]\n", event.nType);
        return -1;
    }
	bool bReduceSpeedStrategy = false;
	if (m_bReduceSpeedStrategy) {
		if (event.nType == EVENT_CLICKDOWN ||
			event.nType == EVENT_CLICKUP) {
			bReduceSpeedStrategy = true;
		}
	}
	if (bReduceSpeedStrategy) {
		lock_guard<mutex> lock(m_mutexReduceSpeedStrategy);
		REDUCESPEEDSTRATEGY_T reduceSpeedStrategy;
		reduceSpeedStrategy.ullTimestamp = CTime::get_instance().getTickCount();
		m_reduceSpeedNode.fCurrentClickReductionTime = min((unsigned int)(CHMIScreens::get_instance().getScreenPartSize(event.nScreenID) * m_reduceSpeedNode.fClickReductionTime), (unsigned int)m_reduceSpeedNode.fClickReductionMaxTime);
		int nCurrentScreenCount = CHMIScreens::get_instance().getTotalScreenNum();
		int fClickExtraDelayTime = 0;
		if (nCurrentScreenCount > m_reduceSpeedNode.nThirdAdjustReductionScreenCount && m_reduceSpeedNode.nThirdAdjustReductionScreenCount != -1) {
			fClickExtraDelayTime = m_reduceSpeedNode.fThirdAdjustAddClickExtraDelayTime;
		}
		else if (nCurrentScreenCount > m_reduceSpeedNode.nSecondAdjustReductionScreenCount && m_reduceSpeedNode.nSecondAdjustReductionScreenCount != -1) {
			fClickExtraDelayTime = m_reduceSpeedNode.fSecondAdjustAddClickExtraDelayTime;
		}
		else if (nCurrentScreenCount > m_reduceSpeedNode.nFirstAdjustReductionScreenCount && m_reduceSpeedNode.nFirstAdjustReductionScreenCount != -1) {
			fClickExtraDelayTime = m_reduceSpeedNode.fFirstAdjustAddClickExtraDelayTime;
		}

		if (nCurrentScreenCount > m_reduceSpeedNode.nStartReductionScreenCount && fClickExtraDelayTime > 0) {

			m_reduceSpeedNode.fCurrentClickReductionTime += fClickExtraDelayTime;
		}
		
		reduceSpeedStrategy.ullPeriod = (unsigned long long)m_reduceSpeedNode.fCurrentClickReductionTime;
		reduceSpeedStrategy.event = event;
		m_listReduceSpeedStrategy.push_back(reduceSpeedStrategy);
	}
	else {
		lock_guard<mutex> lock(m_mutex);
		m_listEvent.push_back(event);
		if (m_listEvent.size() > 10) {
			printf("---------dispatch is busy [event total size %d]\n", m_listEvent.size());
            if (m_listEvent.size() >= MAX_EVENT_NUM) {
                m_listEvent.pop_front();
                printf("---------event list full\n");
            }
		}
		wakeUpThread();
	}
    return 0;
}

/* postEvent
**/
int CEventDispatch::postEvent(const EVENT_T& event) {
	if (0 != postEvent(event, false)) {
		return -1;
	}
    return 0;
}

int CEventDispatch::setReductionFlag(bool bFlag) {
	this->m_bReduceSpeedStrategy = bFlag;
	return 0;
}

int CEventDispatch::setClickReductionTime(int nClickTime) {
	this->m_reduceSpeedNode.fClickReductionTime = nClickTime;
	return 0;
}

int CEventDispatch::setRefreshReductionTime(int nRefreshTime) {
	this->m_reduceSpeedNode.fRefreshReductionTime = nRefreshTime;
	return 0;
}

int CEventDispatch::setChangeReductionTime(int nChangeTime) {
	this->m_reduceSpeedNode.fChangeReductionTime = nChangeTime;
	return 0;
}

/* exist
**/
bool CEventDispatch::exist(const EVENT_T& event) {
	lock_guard<mutex> lock(m_mutex);
    for (auto& it : m_listEvent) {
        if (it.nType == event.nType) {
            return true;
        }
    }
    return false;
}

/* exist
**/
bool CEventDispatch::exist(const hmiproto::hmievent& event) {
    lock_guard<mutex> lock(m_mutex);
    for (auto& it : m_listScrEvent) {
        if (it.type() == event.type()) {
            return true;
        }
    }
    return false;
}

/* getEvent
**/
int CEventDispatch::getEvent(hmiproto::hmievent& event) {
	lock_guard<mutex> lock(m_mutex);
    if (true == m_listScrEvent.empty()) {
        return -1;
    }
    event = m_listScrEvent.front();
    m_listScrEvent.pop_front();
    return 0;
}

/* getEvent
**/
int CEventDispatch::getEvent(EVENT_T& event) {
    lock_guard<mutex> lock(m_mutex);
    if (true == m_listEvent.empty()) {
        return -1;
    }
    event = m_listEvent.front();
    m_listEvent.pop_front();
    return 0;
}

/* getEvent
**/
int CEventDispatch::getEvent(HMI_EVENT_T& event) {
    if (0 == getEvent(event.scrEvent)) {
        event.nType = EVENT_SCR;
        return 0;
    }
    if (0 == getEvent(event.event)) {
        event.nType = EVENT_HMI;
        return 0;
    }
    return -1;
}

/* doEvent
**/
void CEventDispatch::doEvent() {
    while (stateRunging == m_nRunStates) {
        HMI_EVENT_T event;
        if (0 != getEvent(event)) {
            break;
        }

        if (EVENT_SCR == event.nType) {
			if (m_bReduceSpeedStrategy &&
				(event.scrEvent.type() == HMIEVENT_CHANGESCR ||
				event.scrEvent.type() == HMIEVENT_POPSCR ||
				event.scrEvent.type() == HMIEVENT_CLOSECHILDSCR ||
				event.scrEvent.type() == HMIEVENT_INDIRECTSCRINIT)) {
				int nPeriod = (unsigned long long)CHMIScreens::get_instance().getScreenPartSize(event.scrEvent.scrno()) * m_reduceSpeedNode.fChangeReductionTime;
				nPeriod -= m_reduceSpeedNode.fCurrentClickReductionTime;
				if (0 > nPeriod) {

					nPeriod = 0;
				}
				int nCurrentScreenCount = CHMIScreens::get_instance().getTotalScreenNum();
				float tempReductionTime = 0;
				if (nCurrentScreenCount > m_reduceSpeedNode.nStartReductionScreenCount) {

					tempReductionTime = (nCurrentScreenCount - m_reduceSpeedNode.nStartReductionScreenCount) * m_reduceSpeedNode.fScreenReductionTime;
				}
				if (tempReductionTime > m_reduceSpeedNode.fMaxScreenReductionTime) {

					tempReductionTime = m_reduceSpeedNode.fMaxScreenReductionTime;
				}
				nPeriod += tempReductionTime;
				if (nPeriod > m_reduceSpeedNode.fMaxReductionTime) {

					nPeriod = m_reduceSpeedNode.fMaxReductionTime;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds((unsigned long long)nPeriod));
			}
			CDispEventHandle::getInstance().doEvent(event.scrEvent);
        }
        else {

			if (event.event.nClientId >= 70000 && CSlaveModule::get_instance().isSlaveMode()) {
			
				hmiproto::hmiact hmiact;
				auto hmievent = hmiact.mutable_event();
				hmievent->set_scrno(event.event.nScreenID);
				hmievent->set_partname(event.event.sPartName);
				hmievent->set_type(event.event.nType);
				hmievent->set_eventbuffer(event.event.sEventbuffer);
				hmievent->set_clientid(event.event.nClientId);

				auto hmiredis = hmiact.mutable_redis();
				hmiredis->set_type(2);
				hmiredis->set_topic(event.scrEvent.clienttopic());

				CSlaveModule::get_instance().sendtoMasterClient(hmiact);
#ifdef _LINUX_
				if (event.event.nType == EVENT_CLICKDOWN) {
			
					btl::beep(true);
				}
				
#endif
			}
			else {
			
				HmiEvent_Do(event.event);
			}
        }
    }
    return;
}

/* workThread
**/
void CEventDispatch::workThread() {
#ifdef _LINUX_
	prctl(PR_SET_NAME, "hmiEventWork");
#endif
			std::cout << "HMIUI workThread _------------ " << std::endl;

    m_nRunStates = stateRunging;
    unsigned long long  ullPlcRead = CTime::get_instance().getTickCount();
    unsigned long long  ullSysRegRead = ullPlcRead;
    while (stateRunging == m_nRunStates) {
		unsigned long long ullPeriod = ON_PLC_READ_TIMER;
        {
            unique_lock<mutex> lock(m_mutexCv);
            m_cvThread.wait_for(lock, std::chrono::milliseconds(TIMEOUT_TIME));
        }
		if (m_bReduceSpeedStrategy) {
			ullPeriod = m_reduceSpeedNode.fRefreshReductionTime;
			lock_guard<mutex> lock(m_mutexReduceSpeedStrategy);
			for (auto iter = m_listReduceSpeedStrategy.begin(); iter != m_listReduceSpeedStrategy.end();) {
				if (CTime::get_instance().getTickCount() - iter->ullTimestamp >= iter->ullPeriod) {
					lock_guard<mutex> lock(m_mutex);
					m_listEvent.push_back(iter->event);
					iter = m_listReduceSpeedStrategy.erase(iter);
				}
				else {
					iter++;
				}
			}
		}
		if (CPlatBoard::getDeviceMode() == HMI_MODE) {
			if (CTime::get_instance().getTickCount() - ullPlcRead > ullPeriod) {
				hmiproto::hmievent  event;
				event.set_type(HMIEVENT_UPDATEPART);
				postEvent(event, true);
				ullPlcRead = CTime::get_instance().getTickCount();
			}
		}
		if (CTime::get_instance().getTickCount() - ullSysRegRead > SYS_REG_READ_TIMER) {
            EVENT_T event;
			event.nType = EVENT_SYSREGTIMER;
			postEvent(event, true);
			ullSysRegRead = CTime::get_instance().getTickCount();
        }
        doEvent();
    }
    m_nRunStates = stateExited;
    return;
}

/* wakeUpThread
**/
int CEventDispatch::wakeUpThread() {
	unique_lock<mutex> lock(m_mutexCv);
	m_cvThread.notify_one();
    return 0;
}
