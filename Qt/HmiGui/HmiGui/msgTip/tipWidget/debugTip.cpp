#include "debugTip.h"
#include "ProtoXML/ProtoXML.h"
#include "utility/utility.h"

using namespace hmiproto;
using namespace UTILITY;

#define MAX_DEBUG_MSG_NUM     50    /* 最大记录数量 */
typedef google::protobuf::RepeatedPtrField<std::string> REPEATED_DEBUG_MSG;

CDebugTip::CDebugTip(QWidget* pParent)
    :CTipWidget(pParent){

}

CDebugTip::~CDebugTip() {

    m_listMsg.clear();
}

int CDebugTip::doEvent(void* msg) {

    if (nullptr == msg) {
        return -1;
    }

    REPEATED_DEBUG_MSG* pMsg = (REPEATED_DEBUG_MSG*)msg;
    for (int i = 0; i < pMsg->size(); ++i) {

        const std::string& sMsg = pMsg->Get(i);
        if (0 == sMsg.length()) {
            continue;
        }

        MSG_DEBUG_T Msg;

        Msg.sMsg = sMsg;
        if ('\n' == Msg.sMsg[sMsg.length() - 1]) {
            Msg.sMsg.pop_back();
        }

        Msg.ullUpdate = CTime::get_instance().getTickCount();

        m_listMsg.push_back(Msg);
        if (m_listMsg.size() > MAX_DEBUG_MSG_NUM) {
            m_listMsg.pop_front();
        }
    }

    updateMsgArea();
    update();
    return 0;
}

int CDebugTip::getMsgNum() {

    return (int)m_listMsg.size();
}

int CDebugTip::getNewMsg(std::string& sMsg, uint64_t& ullTime, int& nLevel) {

    if (0 == m_listMsg.size()) {
        return -1;
    }

    auto& msg = m_listMsg.back();
    sMsg = msg.sMsg;
    ullTime = msg.ullUpdate;
    nLevel = -1;
    return 0;
}

void CDebugTip::clearMsg() {

    m_listMsg.clear();
    updateMsgArea();
    return;
}

int CDebugTip::getMsg(QString& strMsg) {

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

        strMsg.append(QString::fromStdString(msg.sMsg));
    }

    return 0;
}

void CDebugTip::updateMsgArea() {

    m_sizeMsg.setWidth(0);
    m_sizeMsg.setHeight(0);

    if (0 == m_listMsg.size()) {
        return;
    }

    QRect           area(0, 0, this->width(), 16777215);
    QFontMetrics    fm(m_font);

    for (auto& msg : m_listMsg) {

        QString     strMsg = QString::fromStdString(msg.sMsg);
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