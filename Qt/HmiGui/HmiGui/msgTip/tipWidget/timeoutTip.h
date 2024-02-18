#ifndef _TIMEOUTTIP_H__20220621
#define _TIMEOUTTIP_H__20220621

#include <QPainter>
#include <string>
#include <list>
#include <mutex>
#include "tipWidget.h"

typedef struct _tagMSG_TIMEOUT_T {
    std::string         sPos;
    std::string         sAddr;
    int                 nTimeout;
    unsigned long long  ullUpdate;

    _tagMSG_TIMEOUT_T() {
        sPos = "";
        sAddr = "";
        nTimeout = 0;
        ullUpdate = 0;
    }
} MSG_TIMEOUT_T;

typedef std::list<MSG_TIMEOUT_T>    LIST_TIMEOUT_T;

class CTimeoutTip : public CTipWidget {
    Q_OBJECT
public:
    explicit CTimeoutTip(QWidget* pParent = nullptr);
	~CTimeoutTip();
    
    int  doEvent(void* msg);
    int  timer();
    int  getMsgNum();
    int  getNewMsg(std::string& sMsg, uint64_t& ullTime, int& nLevel);
    void clearMsg();

private:
    int  getFormat(std::string& sFormat);
    int  getMsg(QString& strMsg);
    void updateMsgArea();
    
signals:
    void sigUpdateMsg();

private slots:
    void slotUpdateMsg();

private:
    LIST_TIMEOUT_T  m_listMsg;
    std::mutex      m_mutex;
    std::string     m_sFormat;
};
#endif
