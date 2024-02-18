#include <QVector>
#include <QTimer>
#include <thread>
#include "blinds.h"
#include "../../EventMgr/EventMgr.h"

using namespace std;
using namespace std::chrono;

#define LEAF_NUM                (10)    /* 叶片数量 */
#define BLINDS_TIMER_INTERVAL   (200)  /* 定时器间隔 */
#define BLINDS_FRAME_NUM        (5)    /* 帧数 */

CBlinds::CBlinds() {

    m_bInit = false;
    m_pTimer = nullptr;
    m_nLeafHeight = 0;
    m_bFinishFlag = false;
    m_pTimer = nullptr;
    m_nFrameCnt = 0;
    m_nBrushType = 0;
    m_type = TYPE_BLINDS;
    m_bFill = false;
    init();
}

CBlinds::~CBlinds() {

    if (nullptr != m_pTimer) {
        delete m_pTimer;
    }

    foreach(CLeaf* leaf, m_vecLeaf) {
        if (leaf) {
            delete leaf;
        }
    }
}

int CBlinds::init() {

    for (int i = 0; i < LEAF_NUM; ++i) {

        CLeaf* leaf = new CLeaf();
        if (nullptr == leaf) {
            return -1;
        }

        m_vecLeaf.push_back(leaf);
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

void CBlinds::resetArea() {

    int nLeafHeight = this->height() / LEAF_NUM;

    for (int i = 0; i < LEAF_NUM - 1; ++i) {
        m_vecLeaf[i]->setGeometry(0, i * nLeafHeight, this->width(), nLeafHeight);
    }
    
    m_vecLeaf[LEAF_NUM - 1]->setGeometry(0, (LEAF_NUM - 1) * nLeafHeight,
                            this->width(), this->height() - (LEAF_NUM - 1) * nLeafHeight);

    return;
}

void CBlinds::slotTimer() {

    /* 判断是否已经完全显示了 */
    if ((true == m_bFill) && (BLINDS_FRAME_NUM == m_nFrameCnt)) {
        m_bFill = false;
        resetDirect(TOP_TO_BOTTOM);
    }

    /* 判断是否已经完全消失了 */
    if ((false == m_bFill) && (0 == m_nFrameCnt)) {

        if (1 == m_nBrushType) { /* 黑色百叶窗已经动画结束 */
            this->stop();
			return;
        }
        else { /* 白色百叶窗动画结束，开始黑色百叶窗动画 */
            m_nBrushType = 1;
            m_bFill = true;
            resetDirect(BOTTOM_TO_TOP);
        }
    }

    if (true == m_bFill) { /* 叶片升起 */
        ascentLeaf();
    }
    else { /* 叶片下降 */
        descentLeaf();
    }
    
    this->update();
    return;
}

void CBlinds::resetDirect(DIRECT_T direct) {

    for (int i = 0; i < LEAF_NUM; ++i) {
        m_vecLeaf[i]->setDirect(direct);
    }
    
    return;
}

void CBlinds::ascentLeaf() {

    int nHeight = this->height() / LEAF_NUM;

    m_nLeafHeight = nHeight / BLINDS_FRAME_NUM * (++m_nFrameCnt);
    if (BLINDS_FRAME_NUM == m_nFrameCnt) {
        m_nLeafHeight = this->height() - (LEAF_NUM - 1) * nHeight;
    }

    return;
}

void CBlinds::descentLeaf() {

    int nHeight = this->height() / LEAF_NUM;

    m_nLeafHeight = nHeight / BLINDS_FRAME_NUM * (--m_nFrameCnt);
    
    return;
}

int CBlinds::start(QWidget* widget) {

    if (nullptr == widget) {
        return -1;
    }

    if (false == m_bInit) {
        return -2;
    }

    if (true == isRunning()) {
        return 0;
    }

    m_bFinishFlag = false;
    m_nBrushType = 0;
    m_nFrameCnt = 0;
    m_nLeafHeight = 0;
    m_bFill = true;
    this->setParent(widget);
    this->setFixedSize(widget->size());
    resetArea();
    resetDirect(BOTTOM_TO_TOP);
    this->show();
    m_pTimer->start(BLINDS_TIMER_INTERVAL);
    return 0;
}

bool CBlinds::isRunning() {

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

int CBlinds::stop() {

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

void CBlinds::paintEvent(QPaintEvent*) {

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

    for (int i = 0; i < LEAF_NUM; ++i) {

        CLeaf*  leaf = m_vecLeaf[i];
        leaf->paint(p, m_nLeafHeight);
    }

    return;
}
