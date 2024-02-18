#include <QVector>
#include <QTimer>
#include "stripe.h"

#define STRIPE_ITEM_NUM         (10)
#define STRIPE_ANIMATION_TIME   (10000) /* 动画时长1000ms */
#define STRIPE_FRAME_RATE       (2000)  /* 帧率200ms显示一次图片 */
#define STRIPE_FRAME_NUM        (STRIPE_ANIMATION_TIME/STRIPE_FRAME_RATE) /* 帧数 */

CStripe::CStripe()
    : m_brushWhite(Qt::white),
    m_brushBlack(Qt::black) {

    m_bInit = false;
    m_nItemLen = 0;
    m_pTimer = nullptr;
    m_nFrameCnt = 0;
    m_mode = MODE_IN;
    m_type = TYPE_STRIPE;
    init();
}

CStripe::~CStripe() {

    if (nullptr != m_pTimer) {
        delete m_pTimer;
    }

    foreach(CItem* item, m_vecItem) {

        if (item) {
            delete item;
        }
    }
}

int CStripe::init() {

    for (int i = 0; i < STRIPE_ITEM_NUM; ++i) {

        CItem* item = new CItem();
        if (nullptr == item) {
            return -1;
        }

        m_vecItem.push_back(item);
        if (0 == (i % 2)) {
            item->setDirect(LEFT_TO_RIGHT);
        }
        else {
            item->setDirect(RIGHT_TO_LEFT);
        }
    }

    m_pTimer = new QTimer(this);
    if (nullptr == m_pTimer) {
        return -2;
    }

    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(slotTimer()));

    m_bInit = true;
    this->hide();
    return 0;
}

void CStripe::resetItemArea() {

    int nHeight = this->height() / STRIPE_ITEM_NUM;

    for (int i = 0; i < STRIPE_ITEM_NUM - 1; ++i) {
        m_vecItem[i]->setGeometry(0, i * nHeight, this->width(), nHeight);
    }

    m_vecItem[STRIPE_ITEM_NUM - 1]->setGeometry(0, (STRIPE_ITEM_NUM - 1) * nHeight,
                                this->width(), nHeight + this->height() % nHeight);

    return;
}

void CStripe::updateItemLenth() {

    if (STRIPE_FRAME_NUM == m_nFrameCnt) {
        if (MODE_IN == m_mode) {
            m_nItemLen = this->width();
        }
        else {
            m_nItemLen = 0;
        }
    }
    else {
        m_nItemLen = this->width() / STRIPE_FRAME_NUM * m_nFrameCnt;
        if (MODE_OUT == m_mode) {
            m_nItemLen = this->width() - m_nItemLen;
        }
    }

    return;
}

void CStripe::swapItemPosition() {

    int nHeight = this->height() / STRIPE_ITEM_NUM;
    
    for (int i = 0; i < STRIPE_ITEM_NUM - 2; i+=2) {

        CItem*  item1 = m_vecItem[i];
        item1->setGeometry(0, (i + 1) * nHeight, this->width(), nHeight);

        CItem*  item2 = m_vecItem[i + 1];
        item2->setGeometry(0, i * nHeight, this->width(), nHeight);
    }

    m_vecItem[STRIPE_ITEM_NUM - 2]->setGeometry(0, (STRIPE_ITEM_NUM - 1) * nHeight,
                            this->width(), nHeight + this->height() % nHeight);
    m_vecItem[STRIPE_ITEM_NUM - 1]->setGeometry(0, (STRIPE_ITEM_NUM - 2) * nHeight, this->width(), nHeight);

    return;
}

void CStripe::slotTimer() {
    
    ++m_nFrameCnt;
    if ((m_nFrameCnt > STRIPE_FRAME_NUM) && (MODE_OUT == m_mode)) {
        this->stop();
        return;
    }

    if (MODE_MOVE == m_mode) {
        m_mode = MODE_OUT;
        m_nFrameCnt = 1;
    }

    if ((m_nFrameCnt > STRIPE_FRAME_NUM) && (MODE_IN == m_mode)) {
        swapItemPosition();
        m_mode = MODE_MOVE;
    }
    else {
        updateItemLenth();
    }
       
    this->update();
    return;
}

int CStripe::start(QWidget* widget) {

    if (nullptr == widget) {
        return -1;
    }

    if (false == m_bInit) {
        return -2;
    }

    if (true == m_pTimer->isActive()) {
        return -3;
    }

    m_nFrameCnt = 0;
    m_mode = MODE_IN;
    m_nItemLen = 0;
    this->setParent(widget);
    this->setFixedSize(widget->size());
    this->resetItemArea();
    this->show();
    m_pTimer->start(STRIPE_FRAME_RATE);

    return 0;
}

bool CStripe::isRunning() {

    if (false == m_bInit) {
        return false;
    }

    if (nullptr == m_pTimer) {
        return false;
    }

    if (true == this->isHidden()) {
        return false;
    }

    if (false == m_pTimer->isActive()) {
        return false;
    }

    return true;
}

int CStripe::stop() {

    if (nullptr == m_pTimer) {
        return -1;
    }

    if (true == m_pTimer->isActive()) {
        m_pTimer->stop();
    }

    if (false == this->isHidden()) {
        this->hide();
        this->setParent(nullptr);
    }

    return 0;
}

void CStripe::paintEvent(QPaintEvent*) {

    if (false == m_bInit) {
        return;
    }

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(m_brushWhite);

    for (int i = 0; i < STRIPE_ITEM_NUM; ++i) {

        if (0 == i % 2) {
            CItem*  item = m_vecItem[i];
            item->paint(p, m_nItemLen);
        }
    }

    p.setBrush(m_brushBlack);
    for (int i = 0; i < STRIPE_ITEM_NUM; ++i) {

        if (1 == i % 2) {
            CItem*  item = m_vecItem[i];
            item->paint(p, m_nItemLen);
        }
    }

    return;
}
