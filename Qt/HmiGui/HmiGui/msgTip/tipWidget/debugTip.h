#ifndef _DEBUGTIP_H__20220621
#define _DEBUGTIP_H__20220621

#include <QPainter>
#include <string>
#include <list>
#include "tipWidget.h"

typedef struct _tagMSG_DEBUG_T {
    std::string         sMsg;
    uint64_t            ullUpdate;

    _tagMSG_DEBUG_T() {

        sMsg = "";
        ullUpdate = 0;
    }
} MSG_DEBUG_T;

typedef std::list<MSG_DEBUG_T>  LIST_DEBUG__T;

class CDebugTip : public CTipWidget {
public:
    CDebugTip(QWidget* pParent = nullptr);
	~CDebugTip();
    
    int  doEvent(void* msg);
    int  getMsgNum();
    int  getNewMsg(std::string& sMsg, uint64_t& ullTime, int& nLevel);
    void clearMsg();

private:
    int  getMsg(QString& strMsg);
    void updateMsgArea();

private:
    LIST_DEBUG__T   m_listMsg;
};
#endif
