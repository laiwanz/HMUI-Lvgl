#include "timeoutTip.h"
#include "ProtoXML/ProtoXML.h"
#include "utility/utility.h"
#include "cbl/cbl.h"

using namespace std;
using namespace hmiproto;
using namespace UTILITY;

#define TIMEOUT_UPDATE_TIME     5 * 1000    /* 每条消息的最小更新时间 */
#define MAX_TIMEOUT_MSG_NUM     50          /* 最大记录数量 */

typedef google::protobuf::RepeatedPtrField<messages_message_timeout>    REPEATED_TIMOUT_MSG;

CTimeoutTip::CTimeoutTip(QWidget* pParent)
    :CTipWidget(pParent){

    m_sFormat = "";
    connect(this, SIGNAL(sigUpdateMsg()), this,  SLOT(slotUpdateMsg()));
}

CTimeoutTip::~CTimeoutTip() {

    disconnect(this, SIGNAL(sigUpdateMsg()), this, SLOT(slotUpdateMsg()));

    lock_guard<mutex>   lock(m_mutex);
    m_listMsg.clear();
}

int CTimeoutTip::doEvent(void* msg) {

    if (nullptr == msg) {
        return -1;
    }

    REPEATED_TIMOUT_MSG* pMsg = (REPEATED_TIMOUT_MSG*)msg;
    for (int i = 0; i < pMsg->size(); ++i) {

        const messages_message_timeout& timeoutMsg = pMsg->Get(i);
        bool                bExist = false;
        lock_guard<mutex>   lock(m_mutex);

        for (auto it = m_listMsg.begin(); it != m_listMsg.end(); ++it) {

            if (((*it).sPos != timeoutMsg.position()) ||
                ((*it).sAddr != timeoutMsg.addr())) {
                continue;
            }

            if ((*it).nTimeout != timeoutMsg.period()) {
                (*it).nTimeout = timeoutMsg.period();
                (*it).ullUpdate = CTime::get_instance().getTickCount();
            }

            bExist = true;
            break;
        }

        if (false == bExist) {
            MSG_TIMEOUT_T   Msg;
            Msg.sPos = timeoutMsg.position();
            Msg.sAddr = timeoutMsg.addr();
            Msg.nTimeout = timeoutMsg.period();
            Msg.ullUpdate = CTime::get_instance().getTickCount();
            m_listMsg.push_front(Msg);
        }

        if (m_listMsg.size() > MAX_TIMEOUT_MSG_NUM) {
            m_listMsg.pop_back();
        }
    }

    updateMsgArea();
    update();
    return 0;
}

int CTimeoutTip::timer() {

    unsigned long long  ullTime = CTime::get_instance().getTickCount();
    bool                bRemove = false;
    unique_lock<mutex>  lock(m_mutex);

    for (auto it = m_listMsg.begin(); it != m_listMsg.end();) {

        if ( ullTime - (*it).ullUpdate > TIMEOUT_UPDATE_TIME) {
            it = m_listMsg.erase(it);
            bRemove = true;
        }
        else {
            ++it;
        }
    }

    lock.unlock();
    if (true == bRemove) {
        emit sigUpdateMsg();
    }

    return 0;
}

int CTimeoutTip::getMsgNum() {

    unique_lock<mutex>  lock(m_mutex);
    return (int)m_listMsg.size();
}

int CTimeoutTip::getNewMsg(std::string& sMsg, uint64_t& ullTime, int& nLevel) {

    unique_lock<mutex>  lock(m_mutex);

    if (0 == m_listMsg.size()) {
        return -1;
    }

    if (0 == m_sFormat.length()) {
        return -2;
    }

    auto& msg = m_listMsg.front();
    sMsg = cbl::format(m_sFormat.c_str(), msg.sPos.c_str(), msg.sAddr.c_str(), msg.nTimeout);
    ullTime = msg.ullUpdate;
    nLevel = -1;
    return 0;
}

void CTimeoutTip::clearMsg() {

    {
        unique_lock<mutex>  lock(m_mutex);
        if (0 == m_listMsg.size()) {
            return;
        }

        m_listMsg.clear();
    }

    updateMsgArea();
    return;
}

int CTimeoutTip::getFormat(std::string& sFormat) {

    unique_lock<mutex>  lock(m_mutex);

    if (0 == m_listMsg.size()) {
        return -1;
    }

    size_t nMaxPos = 0, nMaxAddr = 0, nMaxTime = 0;
    
    for (auto& msg : m_listMsg) {

        if (msg.sPos.length() > nMaxPos) {
            nMaxPos = msg.sPos.length();
        }

        if (msg.sAddr.length() > nMaxAddr) {
            nMaxAddr = msg.sAddr.length();
        }

        if (msg.nTimeout > (int)nMaxTime) {
            nMaxTime = msg.nTimeout;
        }
    }

    lock.unlock();

    nMaxTime = std::to_string(nMaxTime).length();
    sFormat = cbl::format("[%%-%ds] [%%-%ds] [%%-%dds]", nMaxPos, nMaxAddr, nMaxTime);
    return 0;
}

int CTimeoutTip::getMsg(QString& strMsg) {

    if (0 == m_sFormat.length()) {
        return -1;
    }

    unique_lock<mutex>  lock(m_mutex);
    if (0 == m_listMsg.size()) {
        return -2;
    }

    strMsg.clear();

    for (auto& msg : m_listMsg) {

        if (strMsg.length() > 0) {
            strMsg.append("\n");
        }

        std::string sMsg = cbl::format(m_sFormat.c_str(), msg.sPos.c_str(),
                                        msg.sAddr.c_str(), msg.nTimeout);
        strMsg.append(QString::fromStdString(sMsg));
    }

    return 0;
}

void CTimeoutTip::updateMsgArea() {

    m_sizeMsg.setWidth(0);
    m_sizeMsg.setHeight(0);
    m_sFormat = "";

    if ((0 != getFormat(m_sFormat)) ||
        (0 == m_sFormat.length())) {
        return;
    }

    unique_lock<mutex>  lock(m_mutex);

    if (0 == m_listMsg.size()) {
        return;
    }

    QRect           area(0, 0, this->width(), 16777215);
    QFontMetrics    fm(m_font);
    
    for (auto& msg : m_listMsg) {

        std::string sMsg = cbl::format(m_sFormat.c_str(), msg.sPos.c_str(),
                                        msg.sAddr.c_str(), msg.nTimeout);
        QString     strMsg = QString::fromStdString(sMsg);
        QRect       rc = fm.boundingRect(area, Qt::TextWrapAnywhere, strMsg);
        
        m_sizeMsg.setHeight(m_sizeMsg.height() + rc.height());
        if (m_sizeMsg.width() < rc.width()) {
            m_sizeMsg.setWidth(rc.width());
        }
    }
    
    if (m_listMsg.size() > 1) {
        m_sizeMsg.setHeight(m_sizeMsg.height() + (m_listMsg.size() - 1) * fm.leading());
    }

    return;
}

void CTimeoutTip::slotUpdateMsg() {

    updateMsgArea();
    update();
    return;
}
