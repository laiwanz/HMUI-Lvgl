#ifndef _EVENTDISPATCH_H__20220613
#define _EVENTDISPATCH_H__20220613
#include "ProtoXML/ProtoXML.h"
#include "macrosplatform/event.h"
#include <list>
#include <mutex>
#include <condition_variable>

typedef struct tagHMI_EVENT_T {
    int                     nType;
    hmiproto::hmievent      scrEvent;
    EVENT_T                 event;
} HMI_EVENT_T;

typedef struct _tagREDUCESPEEDSTRATEGY {
	unsigned long long ullTimestamp;
	unsigned long long ullPeriod;
	EVENT_T event;
} REDUCESPEEDSTRATEGY_T;

typedef struct tagREDUCESPEEDNODE {

	float fClickReductionTime;                  /* 单次点击降速时间 */
	float fRefreshReductionTime;                /* 刷新时间 */
	float fChangeReductionTime;                 /* 修改时间 */
	float fClickReductionMaxTime;               /* 最大延时时间 */
	float fCurrentClickReductionTime;           /* 最终点击延时时间 */
	float fMaxReductionTime;                    /* 总画面延时时间 */
	int   nStartReductionScreenCount;           /* 开始限速画面数 */
	float fScreenReductionTime;                 /* 每增加1个画面增加的延时时间 */
	float fMaxScreenReductionTime;              /* 最大画面切换时间 */
	int   nFirstAdjustReductionScreenCount;     /* 首次调整降速的画面数 */
	int   nSecondAdjustReductionScreenCount;    /* 第二次调整降速的画面数 */
	int   nThirdAdjustReductionScreenCount;     /* 第三次调整降速的画面数 */
	float fFirstAdjustAddClickExtraDelayTime;   /* 首次调整降速的画面的额外延时时间 */
	float fSecondAdjustAddClickExtraDelayTime;  /* 第二次调整降速的画面的额外延时时间 */
	float fThirdAdjustAddClickExtraDelayTime;   /* 第三次调整降速的画面的额外延时时间 */

	tagREDUCESPEEDNODE() {

		fClickReductionTime = 0;             
		fRefreshReductionTime = 0;         
		fChangeReductionTime = 0;               
		fClickReductionMaxTime = 0;             
		fCurrentClickReductionTime = 0;          
		fMaxReductionTime = 0;                    
		nStartReductionScreenCount = 0;         
		fScreenReductionTime = 0;              
		fMaxScreenReductionTime = 0;             
		nFirstAdjustReductionScreenCount = -1;     
		nSecondAdjustReductionScreenCount = -1;   
		nThirdAdjustReductionScreenCount = -1;    
		fFirstAdjustAddClickExtraDelayTime = 0;  
		fSecondAdjustAddClickExtraDelayTime = 0;  
		fThirdAdjustAddClickExtraDelayTime = 0;  
	}

}REDUCESPEEDNODE_T;

class CEventDispatch {
public:
    static CEventDispatch& getInstance();

	void initReductionConfig();
    int start();
    int stop();
    int sendEvent(const hmiproto::hmievent& event);
    int postEvent(const hmiproto::hmievent& event, bool bFilter);
    int postEvent(const hmiproto::hmievent& event);
    int postEvent(const EVENT_T& event, bool bFilter);
    int postEvent(const EVENT_T& event);
	int setReductionFlag(bool bFlag);
	int setClickReductionTime(int nClickTime);
	int setRefreshReductionTime(int nRefreshTime);
	int setChangeReductionTime(int nChangeTime);
private:
    CEventDispatch();
    ~CEventDispatch();
    bool    exist(const EVENT_T& event);
    bool    exist(const hmiproto::hmievent& event);
    int     getEvent(hmiproto::hmievent& event);
    int     getEvent(EVENT_T& event);
    int     getEvent(HMI_EVENT_T& event);
    int     wakeUpThread();
    void    doEvent();
    void    workThread();
private:
    std::list<hmiproto::hmievent>   m_listScrEvent;             /* screen event list */
    std::list<EVENT_T>              m_listEvent;                /* common event list */
    std::mutex						m_mutex;                    /* mutex for event list*/
	std::mutex                      m_mutexCv;                   /* lock for cv */
    std::condition_variable         m_cvThread;                 /* condition variable for work thread */
    int                             m_nRunStates;               /* work thread run state */
	bool							m_bReduceSpeedStrategy;
	std::mutex						m_mutexReduceSpeedStrategy;
	std::list<REDUCESPEEDSTRATEGY_T> m_listReduceSpeedStrategy;
	REDUCESPEEDNODE_T				m_reduceSpeedNode;
};
#endif