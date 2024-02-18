#include <QPainter>
#include <QTimer>
#include "lattice.h"
#include "../../EventMgr/EventMgr.h"

#define LATTICE_ANIMATION_TIME   (500) /* 动画时长1000ms */

CLattice::CLattice() {

    m_type = TYPE_LATTICE;
    m_bInit = false;
    m_pTimer = nullptr;
    m_bDispOddLine = false;
    m_nBrushType = 0;
    init();
}

CLattice::~CLattice() {

    if (nullptr != m_pTimer) {
        delete m_pTimer;
    }
}

int CLattice::init() {

    m_pTimer = new QTimer(this);
    if (nullptr == m_pTimer) {
        return -2;
    }

    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(slotTimer()));

    m_bInit = true;
    this->hide();
    return 0;
}

void CLattice::slotTimer() {
    
    /* 动画效果为(白:奇,黑:偶,白:偶,黑:奇) */
    if ((1 == m_nBrushType) && (true == m_bDispOddLine)) {
        this->stop();
        return;
    }

    if ((0 == m_nBrushType) && (true == m_bDispOddLine)) {
        m_nBrushType = 1;
        m_bDispOddLine = false;
    }
    else if ((1 == m_nBrushType) && (false == m_bDispOddLine)) {
        m_nBrushType = 0;
    }
    else if ((0 == m_nBrushType) && (false == m_bDispOddLine)) {
        m_nBrushType = 1;
        m_bDispOddLine = true;
    }
    
    this->update();
    return;
}

int CLattice::start(QWidget* widget) {

    if (nullptr == widget) {
        return -1;
    }

    if (false == m_bInit) {
        return -2;
    }

    if (true == m_pTimer->isActive()) {
        return -3;
    }

    m_bDispOddLine = true;
    m_nBrushType = 0;
    this->setParent(widget);
    this->setFixedSize(widget->size());
    this->show();
    m_pTimer->start(LATTICE_ANIMATION_TIME);

    return 0;
}

bool CLattice::isRunning() {

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

int CLattice::stop() {

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

    EventMgr::sendCloseScrSaver();
    return 0;
}

void CLattice::paintEvent(QPaintEvent*) {

    if (false == m_bInit) {
        return;
    }

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);

    if (0 == m_nBrushType) {
        p.setBrush(Qt::white);
    }
    else {
        p.setBrush(Qt::black);
    }

    int nWidth = this->width() / 8;
    int nHeight = this->height() / 8;
    bool bDispOdd = m_bDispOddLine;

    for (int i = 0; i < 8; ++i) {
        
        for (int j = 0; j < 8; ++j) {

            if (((true == bDispOdd) && (1 == j % 2)) || /* 奇数行显示 */
                ((false == bDispOdd) && (0 == j % 2))) { /* 偶数行显示 */
                p.drawRect(nWidth * i, nHeight * j, nWidth, nHeight);
            }
        }

        bDispOdd = !bDispOdd;
    }

    return;
}
