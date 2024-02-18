#include <string>
#include <vector>
#include <map>
#include <QApplication>
#include <QPainter>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QStyleOption>
#include <QPixmapCache>
#include "msgTip.h"
#include "../tipWidget/timeoutTip.h"
#include "../tipWidget/commonTip.h"
#include "../tipWidget/debugTip.h"
#include "utility/utility.h"
#include "platform/platform.h"
#include "HmiBase/HmiBase.h"

using namespace std;
using namespace UTILITY;
using namespace PLATFORM;

#ifdef WIN32
#pragma execution_character_set("utf-8")
#else
#include "btl/btl.h"
#endif

#define CLOSE_TIME_INTERVAL 3 * 1000    /* 关闭倒计时3秒 */

#define PNG_PATH_CLOSE      "img/close.png"
#define PNG_PATH_OPEN       "img/show_more.png"
#define PNG_PATH_FOLD       "img/show_less.png"

#define BTN_DOWN_STYLE      "QPushButton{border:2px solid rgb(180,180,180);border-bottom-color:transparent;background-color:transparent;}"
#define	BTN_UP_STYLE        "QPushButton{border:2px solid rgb(180,180,180);background-color:transparent;}"
#define	FUNC_BTN_STYLE      "QPushButton{border:2px solid rgb(180,180,180);border-right-color:rgb(0,0,0); \
							border-bottom-color:rgb(0,0,0);background-color:transparent;}QPushButton:pressed{border-color: rgb(180,180,180);\
							background-color:transparent;border-left-color:rgb(0,0,0);border-top-color:rgb(0,0,0);}"

#define VER_BAR_STYLE       "QScrollBar:vertical{width:15px;background:rgba(98,205,241,0%);color:rgb(98,205,241);margin:0px,0px,0px,0px;}" \
                            "QScrollBar::handle:vertical{width:8px;background:rgba(0,0,0,30%);border-radius:2px;min-height:20;}" \
                            "QScrollBar::add-line:vertical{height:0px;width:0px;subcontrol-position:bottom;}" \
                            "QScrollBar::sub-line:vertical{height:0px;width:0px;subcontrol-position:top;}" \
                            "QScrollBar::add-page:vertical{background:rgba(0,0,0,10%);border-radius:2px;}" \
                            "QScrollBar::sub-page:vertical{background:rgba(0,0,0,10%);border-radius:2px;}"

#define HOR_BAR_STYLE       "QScrollBar:horizontal{height:15px;background:rgba(98,205,241,0%);color:rgb(98,205,241);margin:0px,0px,0px,0px;}" \
                            "QScrollBar::handle:horizontal{width:8px;background:rgba(0,0,0,30%);border-radius:2px;min-height:20;}" \
                            "QScrollBar::add-line:horizontal{height:0px;width:0px;subcontrol-position:bottom;}" \
                            "QScrollBar::sub-line:horizontal{height:0px;width:0px;subcontrol-position:top;}" \
                            "QScrollBar::add-page:horizontal{background:rgba(0,0,0,10%);border-radius:2px;}" \
                            "QScrollBar::sub-page:horizontal{background:rgba(0,0,0,10%);border-radius:2px;}"

enum {
	English = 1,
	Chinese = 2,
};

enum {
    ID_TOP_TIP          = 0,
    ID_BTN_ALL          = 1,
    ID_BTN_TIMEOUT      = 2,
    ID_BTN_COMMON       = 3,
    ID_BTN_DEBUG        = 4,
    ID_NULL_MSG         = 5,
    ID_MAX,
};

enum {
    MSG_TYPE_COMMON = 0,
    MSG_TYPE_TIMEOUT,
    MSG_TYPE_DEBUG,
};

typedef struct _tagSIZE_CONF_T {
    int nWidth;         /* 宽 */
    int nHeight;        /* 高 */
    int nFontSize;      /* 字体大小 */
    int nBtnWidth;      /* 按钮宽 */
    int nBtnHeight;     /* 按钮高 */
    int nMargin;        /* 边距 */
    int nBorderWidth;   /* 边框 */
} SIZE_CONF_T;

static void* msgTipTimer(void* params);
static int getHorizontalConf(const QWidget* parent, SIZE_CONF_T& conf);
static int getVerticalConf(const QWidget* parent, SIZE_CONF_T& conf);

CMsgTip::CMsgTip(QWidget* pParent)
	: QWidget(pParent) {

    m_btnFold = NULL;
    m_btnClose = NULL;
    m_labTopTip = NULL;
    m_btnAll = NULL;
    m_btnTimeout = NULL;
    m_btnCommon = NULL;
    m_btnDebug = NULL;
    m_widgetScroll = NULL;
    m_verBar = NULL;
    m_tipTimeout = NULL;
    m_tipCommon = NULL;
    m_tipDebug = NULL;
    m_line = NULL;
    m_nLanguage = -1;
    m_nFontSize = 0;
    m_nWidth = 0;
    m_nHeight = 0;
    m_nMargin = 0;
    m_nBorderWidth = 0;
    m_bFold = true;
    m_bTopMsgElide = false;
    m_ullUpdateTime = 0;
    m_scrClick = "";

    setStyleSheet("background-color:rgba(255, 255, 255, 200)");
#ifdef WIN32
    setWindowFlags(Qt::FramelessWindowHint);
#else
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
#endif

    init();
}

CMsgTip::~CMsgTip() {
	
    CRateTimer::removeTimer((int)this);
}

int CMsgTip::init() {

    QWidget* pParent = this->parentWidget();
    if (NULL == pParent) {
        return -1;
    }

    SIZE_CONF_T sizeConf = { 0 };
    if (pParent->width() > pParent->height()) {
        getHorizontalConf(pParent, sizeConf);
    }
    else {
        getVerticalConf(pParent, sizeConf);
    }

    int nBtnWidth = sizeConf.nBtnWidth;
    int nBtnHeight = sizeConf.nBtnHeight;
    m_nWidth = sizeConf.nWidth;
    m_nHeight = sizeConf.nHeight;
    m_nFontSize = sizeConf.nFontSize;
    m_nMargin = sizeConf.nMargin;
    m_nBorderWidth = sizeConf.nBorderWidth;

    this->setFixedWidth(m_nWidth);
    this->setFixedHeight(m_nHeight);
    this->move((pParent->width() - m_nWidth) / 2, 0);

    m_btnFold = new QPushButton(this);
    m_btnFold->setGeometry(m_nMargin, m_nBorderWidth + height() / 72, height() / 12, height() / 12);
    m_btnFold->setStyleSheet(FUNC_BTN_STYLE);
    m_btnFold->hide();

    m_labTopTip = new QLabel(this);
    m_labTopTip->setGeometry(m_nMargin + height() / 12, m_nBorderWidth, width() - 2 * (m_nMargin + height() / 12) , height() / 9);
    m_labTopTip->setAlignment(Qt::AlignCenter);
    m_labTopTip->setStyleSheet("background-color: transparent;");

    m_btnClose = new QPushButton(this);
    m_btnClose->setGeometry(width() - m_nMargin - height() / 12, m_nBorderWidth + height() / 72, height() / 12, height() / 12);
    m_btnClose->setStyleSheet(FUNC_BTN_STYLE);
    m_btnClose->hide();

    m_btnAll = new QPushButton(this);
    m_btnAll->setGeometry(m_nMargin, height() / 9 + m_nBorderWidth * 2, nBtnWidth, nBtnHeight);
    m_btnAll->setStyleSheet(BTN_DOWN_STYLE);

    m_btnTimeout = new QPushButton(this);
    m_btnTimeout->setGeometry(m_nMargin + nBtnWidth, height() / 9 + m_nBorderWidth * 2, nBtnWidth, nBtnHeight);
    m_btnTimeout->setStyleSheet(BTN_UP_STYLE);

    m_btnCommon = new QPushButton(this);
    m_btnCommon->setGeometry(m_nMargin + nBtnWidth * 2, height() / 9 + m_nBorderWidth * 2, nBtnWidth, nBtnHeight);
    m_btnCommon->setStyleSheet(BTN_UP_STYLE);

    m_btnDebug = new QPushButton(this);
    m_btnDebug->setGeometry(m_nMargin + nBtnWidth * 3, height() / 9 + m_nBorderWidth * 2, nBtnWidth, nBtnHeight);
    m_btnDebug->setStyleSheet(BTN_UP_STYLE);
    m_btnDebug->hide();

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setGeometry(m_nMargin,
                            m_nMargin + m_nBorderWidth + m_labTopTip->height() + nBtnHeight,
                            width() - m_nMargin * 2 - 2,
                            height() - m_labTopTip->height() - nBtnHeight - m_nMargin * 2 - 2 * m_nBorderWidth);

    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("background-color:transparent;");

    m_verBar = scrollArea->verticalScrollBar();
    m_verBar->setStyleSheet(VER_BAR_STYLE);

    QScrollBar* horBar = scrollArea->horizontalScrollBar();
    horBar->setStyleSheet(HOR_BAR_STYLE);

    m_widgetScroll = new QWidget(scrollArea);
    m_widgetScroll->setStyleSheet("background-color:transparent;");
    scrollArea->setWidget(m_widgetScroll);

    QVBoxLayout* layoutVec = new QVBoxLayout(m_widgetScroll);

    m_tipCommon = new CCommonTip(m_widgetScroll);
    m_tipCommon->setMinimumSize(QSize(0, 0));
    m_tipCommon->setFixedWidth(scrollArea->width() - m_verBar->height() - 6);
    layoutVec->addWidget(m_tipCommon);

    m_line = new CLineShap(m_widgetScroll);
    m_line->setColor(QColor(Qt::black));
    m_line->setWidth(2);
    m_line->setFixedWidth(scrollArea->width() - m_verBar->height() - 6);
    layoutVec->addWidget(m_line);

    m_tipTimeout = new CTimeoutTip(m_widgetScroll);
    m_tipTimeout->setMinimumSize(QSize(0, 0));
    m_tipTimeout->setFixedWidth(scrollArea->width() - m_verBar->height() - 6);
    layoutVec->addWidget(m_tipTimeout);

    m_tipDebug = new CDebugTip(m_widgetScroll);
    m_tipDebug->setMinimumSize(QSize(0, 0));
    m_tipDebug->setFixedWidth(scrollArea->width() - m_verBar->height() - 6);
    layoutVec->addWidget(m_tipDebug);
    m_tipDebug->hide();

    QSpacerItem* item = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    layoutVec->addItem(item);
    layoutVec->setSpacing(m_nMargin);
    layoutVec->setContentsMargins(m_nMargin, m_nMargin, m_nMargin, m_nMargin);
    layoutVec->setStretch(0, 1);
    layoutVec->setStretch(1, 1);
    layoutVec->setStretch(2, 1);
    layoutVec->setStretch(3, 1);

    connect(m_btnFold, SIGNAL(clicked()), this, SLOT(slotBtnFoldClick()));
    connect(m_btnClose, SIGNAL(clicked()), this, SLOT(slotBtnCloseClick()));
    connect(m_btnAll, SIGNAL(clicked()), this, SLOT(slotBtnAllClick()));
    connect(m_btnCommon, SIGNAL(clicked()), this, SLOT(slotBtnCommonClick()));
    connect(m_btnTimeout, SIGNAL(clicked()), this, SLOT(slotBtnTimeoutClick()));
    connect(m_btnDebug, SIGNAL(clicked()), this, SLOT(slotBtnDebugClick()));
    connect(this, SIGNAL(sigUpdate()), this, SLOT(slotUpdate()));
    connect(m_verBar, SIGNAL(rangeChanged(int, int)), this, SLOT(slotUpdateVerBar(int, int)));

    updateHeight();
    CRateTimer::addTimer(msgTipTimer, (void*)this, (int)this, 500);
    return 0;
}

void CMsgTip::setLanguage(int nLanguage) {

    if (m_nLanguage == nLanguage) {
        return;
    }

    m_nLanguage = nLanguage;

    QString strText;

    if (false == m_bFold){
        setTopTipText(ID_TOP_TIP);
    }
    
    getText(ID_BTN_ALL, strText);
    m_btnAll->setText(strText);

    getText(ID_BTN_TIMEOUT, strText);
    m_btnTimeout->setText(strText);
    m_tipTimeout->setTitle(strText);

    getText(ID_BTN_COMMON, strText);
    m_btnCommon->setText(strText);
    m_tipCommon->setTitle(strText);

    getText(ID_BTN_DEBUG, strText);
    m_btnDebug->setText(strText);

    QFont   ft;
    ft.setPixelSize(m_nFontSize);
    if (Chinese == m_nLanguage) {
        ft.setFamily(QString::fromLocal8Bit("SimSun"));
    }
    else {
        ft.setFamily(QString::fromLocal8Bit("Calibri"));
    }

    m_labTopTip->setFont(ft);
    m_btnAll->setFont(ft);
    m_btnTimeout->setFont(ft);
    m_btnCommon->setFont(ft);
    m_btnDebug->setFont(ft);
    m_tipTimeout->setFont(ft);
    m_tipCommon->setFont(ft);
    m_tipDebug->setFont(ft);

    return;
}

void CMsgTip::setDispDebugTip(bool enabled) {

    if (enabled != m_btnDebug->isHidden()) {
        return;
    }

    if (true == enabled) {
        m_btnDebug->show();
        dispDebugMsg(false);
        setFold(false);
        setDisp(true);
    }
    else {

        m_btnDebug->hide();
        m_tipDebug->clearMsg();

        if (false == m_tipDebug->isHidden()) {
            dispAllMsg(false);
        }
    }

    return;
}

void CMsgTip::setDispCloseBtn(bool enabled) {

    if (enabled != m_btnClose->isHidden()) {
        return;
    }

    if (true == enabled) {
        m_btnClose->show();
    }
    else {
        m_btnClose->hide();
    }

    updateTopTipArea();

    return;
}

void CMsgTip::setDisp(bool enabled) {

    if (enabled != this->isHidden()) {
        return;
    }

    if (true == enabled) {
        this->show();
    }
    else {
        this->hide();
    }

    return;
}

void CMsgTip::setFold(bool enabled) {

    if (enabled == m_bFold) {
        return;
    }

    m_bFold = enabled;
    if (false == m_bFold) {
        setDispFoldBtn(true);
        setTopTipText(ID_TOP_TIP);
    }
    else {
        m_ullUpdateTime = CTime::get_instance().getTickCount();
        slotUpdate();
    }

    updateHeight();
    return;
}

void CMsgTip::setDispFoldBtn(bool enabled) {

    if (enabled != m_btnFold->isHidden()) {
        return;
    }

    if (true == enabled) {
        m_btnFold->show();
    }
    else {
        m_btnFold->hide();
    }

    updateTopTipArea();
    return;
}

void CMsgTip::setTopTipText(int nTextId) {

    QString strText;

    if (0 != getText(nTextId, strText)) {
        return;
    }

    setTopTipText(strText);
    return;
}

void CMsgTip::setTopTipText(const QString& strText) {

    if (0 == strText.length()) {
        return;
    }

    QString         strTemp;
    QFontMetrics    fm(m_labTopTip->font());

    if (fm.width(strText) >= m_labTopTip->width()) {

        while (1) {
            strTemp = fm.elidedText(strText, Qt::ElideRight, m_labTopTip->width());
            m_bTopMsgElide = true;
            if (false == m_btnFold->isHidden()) {
                break;
            }

            /* m_btnFold显示后会改变m_labTopTip的大小，所以要重新获取显示的文本 */
            setDispFoldBtn(true);
            continue;
        }
    }
    else {
        strTemp = strText;
        m_bTopMsgElide = false;
    }
    

    if (strTemp == m_labTopTip->text()) {
        return;
    }

    m_labTopTip->setText(strTemp);
    return;
}

int CMsgTip::msgNum() {

    int nNum = 0;
    
    nNum = m_tipCommon->getMsgNum();
    nNum += m_tipTimeout->getMsgNum();
    nNum += m_tipDebug->getMsgNum();

    return nNum;
}


void CMsgTip::getNewMsg(QString& strMsg, int& nType, int& nLeve) {

    std::string sTemp, sMsg;
    uint64_t    ullTemp = 0, ullTime = 0;
    int         nTemp = -1;

    nType = -1;

    if (0 == m_tipCommon->getNewMsg(sTemp, ullTemp, nTemp)) {
        sMsg = sTemp;
        ullTime = ullTemp;
        nLeve = nTemp;
        nType = MSG_TYPE_COMMON;
    }

    if ((0 == m_tipTimeout->getNewMsg(sTemp, ullTemp, nTemp)) &&
        (ullTemp > ullTime + 1000)) {
        sMsg = sTemp;
        ullTime = ullTemp;
        nLeve = nTemp;
        nType = MSG_TYPE_TIMEOUT;
    }

    if ((-1 == nType) && (0 == m_tipDebug->getNewMsg(sTemp, ullTemp, nTemp))) {
        sMsg = sTemp;
        ullTime = ullTemp;
        nLeve = nTemp;
        nType = MSG_TYPE_DEBUG;
    }

    strMsg = QString::fromStdString(sMsg);

    return;
}

int CMsgTip::getText(int id, QString& strText) {

    if ((id < ID_TOP_TIP) || (id >= ID_MAX)) {
        return -1;
    }

    std::map<int, vector<string>> mapText;
    mapText[ID_TOP_TIP] = { "消息列表", "Message List" };
    mapText[ID_BTN_ALL] = { "所有消息", "All" };
    mapText[ID_BTN_TIMEOUT] = { "超时列表", "Timeout" };
    mapText[ID_BTN_COMMON] = { "消息提示", "Message" };
    mapText[ID_BTN_DEBUG] = { "调试信息", "Debug" };
    mapText[ID_NULL_MSG] = { "无最新消息，窗口即将关闭", "No update, the window will close soon" };
   
    vector<string>& vec = mapText[id];
    if (Chinese == m_nLanguage) {
        strText = QString::fromStdString(vec[0]);
    }
    else {
        strText = QString::fromStdString(vec[1]);
    }

    return 0;
}

int CMsgTip::handleEvent(const hmiproto::messages& msg) {

    if (true == msg.has_language()) {
        setLanguage(msg.language());
    }

    if (msg.common_size() > 0) {
        m_tipCommon->doEvent((void *)(&(msg.common())));
    }

    if (msg.timeout_size() > 0) {
        m_tipTimeout->doEvent((void *)(&(msg.timeout())));
    }

    if (true == msg.has_isdebug()) {
        setDispDebugTip(msg.isdebug());
    }

    if (msg.debug_size() > 0) {
        m_tipDebug->doEvent((void *)(&(msg.debug())));
    }

    slotUpdate();

    return 0;
}

void CMsgTip::timer() {

    if (true == isHidden()) {
        return;
    }

    if (false == m_bFold) { /* 展开状态不进行消息清除处理 */
        return;
    }

    m_tipTimeout->timer();
    m_tipCommon->timer();
    emit sigUpdate();
    return;
}

void CMsgTip::setNewParent(QWidget* pParent) {

    if (NULL == pParent) {
        return;
    }

    this->setParent(pParent, this->windowFlags());

#if (QT_VERSION <= QT_VERSION_CHECK(5, 0, 0))
    int nW = pParent->width();
    this->move((nW - m_nWidth) / 2, 0);
#endif

    return;
}

void CMsgTip::paintEvent(QPaintEvent *) {
	
    QStyleOption opt;
    opt.init(this);

    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QColor  color(0, 0, 0, 100);
    p.setPen(color);

    for (int i = 0; i <= m_nBorderWidth; i++) {
        p.drawRoundedRect(m_nBorderWidth - i, m_nBorderWidth - i,
                          width() - (m_nBorderWidth - i) * 2,
                          height() - (m_nBorderWidth - i) * 2,
                          m_nBorderWidth, m_nBorderWidth);
    }

    if (false == m_bFold) {
        drawTipAreaFram(p);
    }

    drawBtnImg(p);

    return;
}

void CMsgTip::drawTipAreaFram(QPainter& p) {

    QPen    pen;
    pen.setWidth(2);
    pen.setColor(QColor(180, 180, 180));

    p.setPen(pen);

    int nBtnW = m_btnAll->width();
    int nBtnH = m_btnAll->height();

    /* 左竖线 */
    p.drawLine(m_nMargin + 1, m_labTopTip->height() + nBtnH, 
                m_nMargin + 1, height() - m_nMargin);

    /* 上横线 */
    if (false == m_btnDebug->isHidden()) {
        p.drawLine(m_nMargin + nBtnW * 4,
                    m_labTopTip->height() + nBtnH + m_nBorderWidth * 2 - 1,
                    width() - m_nMargin,
                    m_labTopTip->height() + nBtnH + m_nBorderWidth * 2 - 1);
    }
    else{
        p.drawLine(m_nMargin + nBtnW * 3,
                    m_labTopTip->height() + nBtnH + m_nBorderWidth * 2 - 1,
                    width() - m_nMargin,
                    m_labTopTip->height() + nBtnH + m_nBorderWidth * 2 - 1);
    }

    pen.setColor(QColor(0, 0, 0, 200));
    p.setPen(pen);

    /* 右竖线 */
    p.drawLine(width() - m_nMargin, m_labTopTip->height() + nBtnH + m_nBorderWidth * 2,
            width() - m_nMargin, height() - m_nMargin);

    /* 下横线 */
    p.drawLine(m_nMargin + 1, height() - m_nMargin,
                width() - m_nMargin, height() - m_nMargin);
    
    return;
}

void CMsgTip::drawBtnImg(QPainter& p) {

    QString strPath = QString::fromStdString(getPrjDir());
    if (0 == strPath.length()) {
        return;
    }

    if (false == m_btnFold->isHidden()) {
        if (true == m_bFold) {
            QPixmap pixmap(strPath + PNG_PATH_OPEN);
            p.drawPixmap(m_btnFold->geometry(), pixmap);
        }
        else {
            QPixmap pixmap(strPath + PNG_PATH_FOLD);
            p.drawPixmap(m_btnFold->geometry(), pixmap);
        }

        QPixmapCache::clear();
    }
    else {
        p.fillRect(m_btnFold->geometry(), Qt::NoBrush);
    }

    if (false == m_btnClose->isHidden()) {
        QPixmap pixmap(strPath + PNG_PATH_CLOSE);
        p.drawPixmap(m_btnClose->geometry(), pixmap);
        QPixmapCache::clear();
    }
    else {
        p.fillRect(m_btnClose->geometry(), Qt::NoBrush);
    }

    return;
}

void CMsgTip::updateHeight() {

    if (true == m_bFold) {
        setFixedHeight(m_nBorderWidth * 2 + m_labTopTip->height());
    }
    else {
        setFixedHeight(m_nHeight);
    }

    return;
}

void CMsgTip::updateNewMsg() {

    if ((false == m_bFold)) {
        setTopTipText(ID_TOP_TIP);
        return;
    }

    int     nTextId = -1, nLeve = -1, nType = -1;
    QString strMsg;

    if (msgNum() <= 0) { /* 无消息 */
        if (false == m_btnDebug->isHidden()) { /* 打开了调试窗口 */
            nTextId = ID_TOP_TIP;
        }
        else {
            nTextId = ID_NULL_MSG;
        }
    }
    else {
        getNewMsg(strMsg, nType, nLeve);
    }

    if (-1 != nTextId) {
        setTopTipText(nTextId);
        setDispCloseBtn(false);
    }
    else {

        setTopTipText(strMsg);

        if (hmiproto::message_level_senior == nLeve) {
            setDispCloseBtn(true);
        }
        else {
            setDispCloseBtn(false);
        }
        /*else if(hmiproto::message_level_modal == nLeve) {
            showModalWidget();
        }*/
    }
    
    return;
}

void CMsgTip::updateTopTipArea() {

    int nX = m_nMargin;
    int nY = m_labTopTip->y();
    int nWidth = this->width() - 2 * m_nMargin;
    int nHeight = m_labTopTip->height();

    if (false == m_btnFold->isHidden()) {
        nX += m_btnFold->width();
        nWidth -= m_btnFold->width();
    }

    if (false == m_btnClose->isHidden()) {
        nWidth -= m_btnClose->width();
    }

    if ((nX != m_labTopTip->x()) ||
        (nWidth != m_labTopTip->width())) {
        m_labTopTip->setGeometry(nX, nY, nWidth, nHeight);
    }

    return;
}

void CMsgTip::updateBtnFold() {

    bool bShow = false;

    if ((false == m_bFold) || /* 展开状态 */
        (true == m_bTopMsgElide) || /* 顶部消息显示不全 */
        (msgNum() > 1) ||   /* 有消息 */
        (false == m_btnDebug->isHidden())) { /* 显示了调试消息 */
        bShow = true;
    }

    setDispFoldBtn(bShow);

    return;
}

void CMsgTip::updateDispState() {

    uint64_t    ullTime = CTime::get_instance().getTickCount();

    if (false == m_bFold) {
        m_ullUpdateTime = ullTime;
        return;
    }

    bool        bShow = false;
    
    /* 有消息和启用了调试情况消息提示不能关闭 */
    if ((msgNum() > 0) || (false == m_btnDebug->isHidden())) {
        
        bShow = true;
        m_ullUpdateTime = ullTime;
    }
    else if (ullTime - m_ullUpdateTime < CLOSE_TIME_INTERVAL) {
        return;
    }

    setDisp(bShow);
    return;
}

void CMsgTip::mousePressEvent(QMouseEvent* pEvent) {

    if (nullptr == pEvent) {
        return;
    }

    QWidget* pParent = this->parentWidget();
    if (nullptr == pParent) {
        return;
    }

    this->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    QPoint      pt = this->mapTo(pParent, pEvent->pos());
    QWidget*    recv = pParent->childAt(pt);

    if (nullptr == recv) {
        recv = pParent;
    }
    else {
        pt = recv->mapFrom(pParent, pt);
    }

    m_scrClick = recv->objectName();
    QMouseEvent me(pEvent->type(), pt, pEvent->button(), pEvent->buttons(), pEvent->modifiers());
    QApplication::sendEvent(recv, &me);
    this->setAttribute(Qt::WA_TransparentForMouseEvents, false);

    return;
}

void CMsgTip::mouseReleaseEvent(QMouseEvent* pEvent) {

    if (nullptr == pEvent) {
        return;
    }

    QWidget* pParent = this->parentWidget();
    if (nullptr == pParent) {
        return;
    }

    QPoint      pt = this->mapTo(pParent, pEvent->pos());
    QWidget*    recv = pParent;

    if (pParent->objectName() != m_scrClick) {

        recv = pParent->findChild<QWidget*>(m_scrClick);
        if (nullptr != recv) {
            pt = recv->mapFrom(pParent, pt);
        }
    }

    if (nullptr == recv) {
        return;
    }

    this->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    QMouseEvent me(pEvent->type(), pt, pEvent->button(), pEvent->buttons(), pEvent->modifiers());
    QApplication::sendEvent(recv, &me);
    this->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    
    return;
}

/*void CMsgTip::showModalWidget() {

    if (NULL != m_widgetModal) {
        return;
    }

    m_widgetModal = new QWidget(this->parentWidget());
    m_widgetModal->setGeometry(0, 0, m_nWidth, m_nHeight);
    m_widgetModal->setWindowFlags(Qt::FramelessWindowHint);
    m_widgetModal->setStyleSheet("background-color: transparent;");
    m_widgetModal->show();
    this->raise();
    return;
}*/

void CMsgTip::slotBtnFoldClick() {

#ifdef _LINUX_
	btl::beep(true);
#endif

    setFold(!m_bFold);
    return;
}

void CMsgTip::slotBtnAllClick() {

    dispAllMsg(true);
    return;
}

void CMsgTip::dispAllMsg(bool beep) {

    if (true == beep) {
#ifdef _LINUX_
		btl::beep(true);
#endif
    }

    m_btnAll->setStyleSheet(BTN_DOWN_STYLE);
    m_btnTimeout->setStyleSheet(BTN_UP_STYLE);
    m_btnCommon->setStyleSheet(BTN_UP_STYLE);
    m_btnDebug->setStyleSheet(BTN_UP_STYLE);

    m_tipTimeout->setDispTitle(true);
    m_tipTimeout->show();
    m_tipCommon->setDispTitle(true);
    m_tipCommon->show();
    m_line->show();
    m_tipDebug->hide();
    
    return;
}

void CMsgTip::slotBtnCommonClick() {

#ifdef _LINUX_
	btl::beep(true);
#endif

    m_btnAll->setStyleSheet(BTN_UP_STYLE);
    m_btnTimeout->setStyleSheet(BTN_UP_STYLE);
    m_btnCommon->setStyleSheet(BTN_DOWN_STYLE);
    m_btnDebug->setStyleSheet(BTN_UP_STYLE);

    m_tipDebug->hide();
    m_line->hide();
    m_tipTimeout->hide();
    m_tipCommon->setDispTitle(false);
    m_tipCommon->show();
    return;
}

void CMsgTip::slotBtnTimeoutClick() {

#ifdef _LINUX_
	btl::beep(true);
#endif

    m_btnAll->setStyleSheet(BTN_UP_STYLE);
    m_btnTimeout->setStyleSheet(BTN_DOWN_STYLE);
    m_btnCommon->setStyleSheet(BTN_UP_STYLE);
    m_btnDebug->setStyleSheet(BTN_UP_STYLE);

    m_tipCommon->hide();
    m_tipDebug->hide();
    m_line->hide();
    m_tipTimeout->setDispTitle(false);
    m_tipTimeout->show();
    
    return;
}

void CMsgTip::slotBtnDebugClick() {

    dispDebugMsg(true);
    return;
}

void CMsgTip::dispDebugMsg(bool beep) {

    if (true == beep) {
#ifdef _LINUX_
		btl::beep(true);
#endif
    }

    m_btnAll->setStyleSheet(BTN_UP_STYLE);
    m_btnTimeout->setStyleSheet(BTN_UP_STYLE);
    m_btnCommon->setStyleSheet(BTN_UP_STYLE);
    m_btnDebug->setStyleSheet(BTN_DOWN_STYLE);

    m_tipCommon->hide();
    m_line->hide();
    m_tipTimeout->hide();
    m_tipDebug->show();
    return;
}

void CMsgTip::slotUpdate() {

    updateNewMsg();
    updateBtnFold();
    updateDispState();

    return;
}

void CMsgTip::slotBtnCloseClick() {

#ifdef _LINUX_
	btl::beep(true);
#endif
    m_tipCommon->clearMsg();
    m_tipTimeout->clearMsg();
    m_tipDebug->clearMsg();
    setDispCloseBtn(false);
    setFold(true);
    setDisp(false);

    return;
}

void CMsgTip::slotUpdateVerBar(int, int) {

    if ((true == m_tipDebug->isHidden()) ||
        (true == m_verBar->isHidden()) ||
        (true == m_verBar->isSliderDown())) {
        return;
    }

    m_verBar->setSliderPosition(m_verBar->maximum());
    return;
}

static void* msgTipTimer(void* params) {

    if (NULL == params) {
        return NULL;
    }

    CMsgTip*    msgTip = (CMsgTip*)params;
    msgTip->timer();

    return NULL;
}

static int getHorizontalConf(const QWidget* parent, SIZE_CONF_T& conf) {

    if (NULL == parent) {
        return -1;
    }

    int nW = parent->width();
    int nH = parent->height();

    conf.nWidth = nW * 1 / 2;
    conf.nHeight = nH * 3 / 4;

    if (nW <= 320) { /* 3035 */
        conf.nWidth = nW * 4 / 5;
        conf.nFontSize = 13;
        conf.nBtnWidth = 60;
        conf.nBtnHeight = 24;
        conf.nMargin = 4;
        conf.nBorderWidth = 2;
    }
    else if (nW <= 480) { /* 3043 */
        conf.nWidth = nW * 2 / 3;
        conf.nFontSize = 13;
        conf.nBtnWidth = 60;
        conf.nBtnHeight = 24;
        conf.nMargin = 6;
        conf.nBorderWidth = 2;
    }
    else if (nW <= 1024) { /* 3070、3102 */
        conf.nFontSize = 16;
        conf.nBtnWidth = 80;
        conf.nBtnHeight = 30;
        conf.nMargin = 10;
        conf.nBorderWidth = 3;
    }
    else if (nW <= 1280) { /* 8150 */
        conf.nFontSize = 22;
        conf.nBtnWidth = 120;
        conf.nBtnHeight = 40;
        conf.nMargin = 15;
        conf.nBorderWidth = 4;
    }
    else { /* 9150 */
        conf.nFontSize = 24;
        conf.nBtnWidth = 160;
        conf.nBtnHeight = 60;
        conf.nMargin = 20;
        conf.nBorderWidth = 6;
    }

    return 0;
}

static int getVerticalConf(const QWidget* parent, SIZE_CONF_T& conf) {

    if (NULL == parent) {
        return -1;
    }

    int nW = parent->width();
    int nH = parent->height();

    conf.nHeight = nH * 1 / 2;

    if (nW <= 240) {  /* 3035 */
        conf.nWidth = nW - 8;
        conf.nFontSize = 12;
        conf.nBtnWidth = 56;
        conf.nBtnHeight = 24;
        conf.nMargin = 4;
        conf.nBorderWidth = 2;
    }
    else if (nW <= 272) { /* 3043 */
        conf.nWidth = nW - 10;
        conf.nFontSize = 12;
        conf.nBtnWidth = 56;
        conf.nBtnHeight = 24;
        conf.nMargin = 4;
        conf.nBorderWidth = 2;
    }
    else if (nW <= 480) { /* 3070 */
        conf.nWidth = nW * 4 / 5;
        conf.nFontSize = 16;
        conf.nBtnWidth = 80;
        conf.nBtnHeight = 30;
        conf.nMargin = 6;
        conf.nBorderWidth = 2;
    }
    else if (nW <= 600) { /* 3102 */
        conf.nWidth = nW * 4 / 5;
        conf.nFontSize = 16;
        conf.nBtnWidth = 80;
        conf.nBtnHeight = 30;
        conf.nMargin = 8;
        conf.nBorderWidth = 3;
    }
    else if (nW <= 720) { /* 8150 */
        conf.nWidth = nW * 5 / 6;
        conf.nFontSize = 20;
        conf.nBtnWidth = 100;
        conf.nBtnHeight = 40;
        conf.nMargin = 10;
        conf.nBorderWidth = 3;
    }
    else if (nW <= 1080) { /* 9150 */
        conf.nWidth = nW * 6 / 7;
        conf.nFontSize = 22;
        conf.nBtnWidth = 120;
        conf.nBtnHeight = 40;
        conf.nMargin = 15;
        conf.nBorderWidth = 4;
    }

    return 0;
}