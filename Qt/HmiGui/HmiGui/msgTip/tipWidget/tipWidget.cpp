#include "tipWidget.h"

CTipWidget::CTipWidget(QWidget* parent)
    :QWidget(parent){

    setStyleSheet("background-color: transparent;");
#ifndef WIN32
    setWindowFlags(Qt::WindowStaysOnTopHint);
#endif

    m_bDispTitle = true;
    m_strTitle = "";
}

CTipWidget::~CTipWidget() {

}

int CTipWidget::setTitle(const QString& sTitle) {

    m_strTitle = sTitle;
    return 0;
}

void CTipWidget::setDispTitle(bool enabled) {

    if (m_bDispTitle == enabled) {
        return;
    }

    m_bDispTitle = enabled;
    updateTitleArea();
    update();
    return;
}

void CTipWidget::setFont(const QFont& font) {

    m_font = font;
    updateTitleArea();
    updateMsgArea();
    update();
    return;
}

int CTipWidget::doEvent(void*) {

    return 0;
}

int CTipWidget::timer() {

    return 0;
}

int CTipWidget::getMsgNum() {

    return 0;
}

int CTipWidget::getNewMsg(std::string&, uint64_t&, int&) {

    return -1;
}

void CTipWidget::clearMsg() {

    return;
}

void CTipWidget::updateTitleArea() {

    m_sizeTitle.setWidth(0);
    m_sizeTitle.setHeight(0);

    if ((false == m_bDispTitle) ||
        (0 == m_strTitle.length())) {
        return;
    }

    QFont   font = m_font;
    font.setBold(true);

    QSize           size = this->maximumSize();
    QRect           rect(0, 0, size.width(), size.height());
    QFontMetrics    fm(font);
    QRect           rc = fm.boundingRect(rect, Qt::AlignLeft, m_strTitle);

    m_sizeTitle.setWidth(rc.width());
    m_sizeTitle.setHeight(rc.height() + fm.ascent());

    return;
}

void CTipWidget::updateMsgArea() {

    return;
}

void CTipWidget::updateArea() {

    if (this->height() != m_sizeTitle.height() + m_sizeMsg.height()) {
        
        if (m_sizeTitle.height() + m_sizeMsg.height() < 2) {
            this->setFixedHeight(1);
        }
        else {
            this->setFixedHeight(m_sizeTitle.height() + m_sizeMsg.height());
        }
    }

    return;
}

void CTipWidget::drawTitle(QPainter& p) {

    if ((false == m_bDispTitle) ||
        (0 == m_strTitle.length())) {
        return;
    }

    QFont font = m_font;
    font.setBold(true);

    p.setFont(font);
    p.setPen(QColor(Qt::black));

    QTextOption option(Qt::AlignLeft);

    QRect   rc(0, 0, m_sizeTitle.width(), m_sizeTitle.height());
    p.drawText(rc, m_strTitle, option);

    return;
}

void CTipWidget::drawMsg(QPainter& p) {

    QString strMsg;
    if ((0 != getMsg(strMsg)) ||
        (0 == strMsg.length())) {
        return;
    }

    p.setFont(m_font);
    p.setPen(QColor(Qt::black));

    QTextOption option(Qt::AlignLeft);
    option.setWrapMode(QTextOption::WrapAnywhere);

    QRect   rc(0, m_sizeTitle.height(), m_sizeMsg.width(), m_sizeMsg.height());
    p.drawText(rc, strMsg, option);
    return;
}

void CTipWidget::paintEvent(QPaintEvent*) {

    QPainter p(this);

    updateArea();
    drawTitle(p);
    drawMsg(p);
    return;
}
