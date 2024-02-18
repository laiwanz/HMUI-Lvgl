#include "commonTip.h"
#include "ProtoXML/ProtoXML.h"
#include "utility/utility.h"

using namespace std;
using namespace hmiproto;
using namespace UTILITY;

#define COMMON_UPDATE_TIME      5 * 1000    /* 每条消息的最小更新时间 */
#define MAX_COMMON_MSG_NUM      50          /* 最大记录数量 */
typedef google::protobuf::RepeatedPtrField<messages_message_general>    REPEATED_COMMON_MSG;

CCommonTip::CCommonTip(QWidget* pParent)
    :CTipWidget(pParent){

    connect(this, SIGNAL(sigUpdateMsg()), this, SLOT(slotUpdateMsg()));
}

CCommonTip::~CCommonTip() {

}

int CCommonTip::doEvent(void* msg) {

    if (nullptr == msg) {
        return -1;
    }

    REPEATED_COMMON_MSG* pMsg = (REPEATED_COMMON_MSG*)msg;
    for (int i = 0; i < pMsg->size(); ++i) {

        const messages_message_general& Msg = pMsg->Get(i);
        switch (Msg.type()) {
            case message_type_insert: {
                addMsg(&Msg);
            }
            break;
            case message_type_delete: {
                rmMsg(Msg.id());
            }
            break;
            case message_type_clear: {
                clearMsg();
            }
            break;
            default: {
                return -2;
            }
            break;
        }
    }

    updateMsgArea();
    update();

    return 0;
}

int CCommonTip::timer() {

    unsigned long long  ullTime = CTime::get_instance().getTickCount();
    bool                bRemove = false;
    unique_lock<mutex>  lock(m_mutex);

    for (auto it = m_listMsg.begin(); it != m_listMsg.end();) {

        if ((message_level_common != (*it).nLevel) ||
            (ullTime - (*it).ullUpdate < COMMON_UPDATE_TIME)) {
            it++;
        }
        else {
            it = m_listMsg.erase(it);
            bRemove = true;
        }
    }

    lock.unlock();
    if (true == bRemove) {
        emit sigUpdateMsg();
    }

    return 0;
}

int CCommonTip::getMsgNum() {

    unique_lock<mutex>  lock(m_mutex);
    return (int)m_listMsg.size();
}

int CCommonTip::getNewMsg(std::string& sMsg, uint64_t& ullTime, int& nLevel) {

    unique_lock<mutex>  lock(m_mutex);
    if (0 == m_listMsg.size()) {
        return -1;
    }

    auto& msg = m_listMsg.front();
    sMsg = msg.sMsg;
    ullTime = msg.ullUpdate;
    nLevel = msg.nLevel;

    return 0;
}

void CCommonTip::clearMsg() {

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

int CCommonTip::addMsg(const void* pMsg) {

    if (nullptr == pMsg) {
        return -1;
    }

    const messages_message_general* msg = (const messages_message_general*)pMsg;
    if (0 == msg->buffer().length()) {
        return -2;
    }

    MSG_COMMON_T    common;
    common.nId = msg->id();
    common.nLevel = msg->level();
    common.sMsg += msg->buffer();
    common.ullUpdate = CTime::get_instance().getTickCount();

    unique_lock<mutex>  lock(m_mutex);
    m_listMsg.push_front(common);
    if (m_listMsg.size() > MAX_COMMON_MSG_NUM) {
        m_listMsg.pop_back();
    }

    return 0;
}

int CCommonTip::rmMsg(int nId) {

    unique_lock<mutex>  lock(m_mutex);
    if (0 == m_listMsg.size()) {
        return -1;
    }

    for (auto it = m_listMsg.begin(); it != m_listMsg.end(); ++it) {

        if ((*it).nId != nId) {
            continue;
        }

        m_listMsg.erase(it);
        return 0;
    }

    return -2;
}

int CCommonTip::getMsg(QString& strMsg) {

    unique_lock<mutex>  lock(m_mutex);
    if (0 == m_listMsg.size()) {
        return -1;
    }

    strMsg.clear();
    for (auto& msg : m_listMsg) {

        if (0 == msg.sMsg.length()) {
            continue;
        }

        if (0 != strMsg.length()) {
            strMsg.append("\n");
        }

        strMsg.append(QString::fromLocal8Bit("· "));
        strMsg.append(QString::fromStdString(msg.sMsg));
    }

    return 0;
}

void CCommonTip::updateMsgArea() {

    m_sizeMsg.setWidth(0);
    m_sizeMsg.setHeight(0);

    unique_lock<mutex>  lock(m_mutex);
    if (0 == m_listMsg.size()) {
        return;
    }

    QRect           area(0, 0, this->width(), 16777215);
    QFontMetrics    fm(m_font);

    for (auto& msg : m_listMsg) {

        QString     strMsg = QString::fromLocal8Bit( "· ");
        strMsg.append(QString::fromStdString(msg.sMsg));

        QRect   rc = fm.boundingRect(area, Qt::TextWrapAnywhere, strMsg);

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

void CCommonTip::slotUpdateMsg() {

    updateMsgArea();
    update();
    return;
}
