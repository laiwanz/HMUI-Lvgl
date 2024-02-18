#ifndef _COMMONTIP_H__20220621
#define _COMMONTIP_H__20220621

#include <QPainter>
#include <string>
#include <list>
#include <mutex>
#include "tipWidget.h"

typedef struct _tagMSG_COMMON_T {
    int                 nId;
    int                 nLevel;
    std::string         sMsg;
    uint64_t            ullUpdate;

    _tagMSG_COMMON_T() {

        nId = -1;
        nLevel = -1;
        sMsg = "";
        ullUpdate = 0;
    }
} MSG_COMMON_T;

typedef std::list<MSG_COMMON_T>     LIST_COMMON_T;

class CCommonTip : public CTipWidget {
    Q_OBJECT
public:
    CCommonTip(QWidget* pParent = nullptr);
	~CCommonTip();
    
    int  doEvent(void* msg);
    int  timer();
    int  getMsgNum();
    int  getNewMsg(std::string& sMsg, uint64_t& ullTime, int& nLevel);
    void clearMsg();

private:
    int  addMsg(const void* pMsg);
    int  rmMsg(int nId);
    int  getMsg(QString& strMsg);
    void updateMsgArea();

signals:
    void sigUpdateMsg();

private slots:
    void slotUpdateMsg();

private:
    LIST_COMMON_T   m_listMsg;
    std::mutex      m_mutex;
};
#endif
