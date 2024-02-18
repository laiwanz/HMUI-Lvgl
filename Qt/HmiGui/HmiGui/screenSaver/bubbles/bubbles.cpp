#include "bubbles.h"
#include <stdlib.h>
#include <math.h>
#include <QPainter>
#include "../common/common.h"
#include "utility/utility.h"
#include "platform/platform.h"
#include "../../EventMgr/EventMgr.h"

using namespace UTILITY;
using namespace PLATFORM;

#define MIN_SEED            (1)
#define BALL_POP_INTERVAL   (1000)  /* 泡泡弹出间隔时间 */

#ifndef WIN32
#define BUBBLES_IMG     "/mnt/wecon/wecon/run/project/img/"  // 泡泡图片路径 xqh 20221009
#endif

typedef struct _tagPOSITION_T {
    double x;
    double y;
} POSITION_T;

static int genRandomSpeed(int nMin, int nMax);

CBubbles::CBubbles() {

    m_bAllShow = false;
    m_bInit = false;
    m_ullPopTime = 0;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
    this->hide();
}

CBubbles::~CBubbles() {

    if (nullptr != m_timer) {
        delete m_timer;
    }

    for (int i = 0; i < m_vecBubbl.size(); ++i) {

        if (m_vecBubbl[i]) {
            delete m_vecBubbl[i];
        }
    }
}

int CBubbles::start(QWidget* widget) {

    if (nullptr == widget) {
        return -1;
    }

    if ((false == m_bInit) &&
        (0 != init(widget->width(), widget->height()))) {
        return -2;
    }

   if (true == isRunning()) {
        return 0;
    }
	;
    this->setParent(widget);
    this->setFixedSize(widget->size());

	//resetBallState();
    m_bAllShow = false;
    m_ullPopTime = CTime::get_instance().getTickCount();
    m_timer->start(m_nInterval);
    this->show();
    return 0;
}

bool CBubbles::isRunning() {

    if (nullptr == m_timer) {
        return false;
    }

    if (true == this->isHidden()) {
        return false;
    }

    return true;
}


int CBubbles::stop() {

    if (nullptr == m_timer) {
        return -1;
    }

    if (true == m_timer->isActive()) {
        m_timer->stop();
    }

    if (false == this->isHidden()) {
        this->hide();
        this->setParent(nullptr);
    }

    EventMgr::sendCloseScrSaver();
    return 0;
}

void CBubbles::paintEvent(QPaintEvent*) {

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::transparent);

    for (int i = 0; i < m_nBubbleNum; ++i) {

        if (false == m_vecBubbl[i]->getDisp()) {
            continue;
        }

        m_vecBubbl[i]->paint(p);
    }

    return;
}

int CBubbles::init(int w, int h) {

    if (0 != loadConf(w, h)) {
        return -1;
    }

    if (0 != loadImg()) {
        return -2;
    }

    if (0 != createBubbl()) {
        return -3;
    }
	resetBallState();
    m_bInit = true;
    return 0;
}

int CBubbles::loadConf(int w, int h) {

    switch (CPlatCpu::getType()) {
        case CPU_TYPE_A7: {
            m_nInterval = 200;
            m_nMaxSpeedX = 6;
            m_nMaxSpeedY = 4;
        }
        break;
        case CPU_TYPE_A8:
        case CPU_TYPE_A9:
        case CPU_TYPE_RK3308: {
            m_nInterval = 100;
            m_nMaxSpeedX = 7;
            m_nMaxSpeedY = 5;
        }
        break;
        default: {
            m_nInterval = 100;
            m_nMaxSpeedX = 7;
            m_nMaxSpeedY = 5;
        }
        break;
    }

    int nWidth = w;
    if (w < h) {
        nWidth = h;
        int nTemp = m_nMaxSpeedX;
        m_nMaxSpeedX = m_nMaxSpeedY;
        m_nMaxSpeedY = nTemp;
    }

    if ((nWidth <= 320)) { /* 3035 */
        m_nBubbleNum = 5;
        m_nRadius = 25;
    }
    else if (nWidth <= 480) { /* 3043 */
        m_nBubbleNum = 5;
        m_nRadius = 30;
    }
    else if (nWidth <= 800) { /* 3070 */
        m_nBubbleNum = 5;
        m_nRadius = 50;
    }
    else if (nWidth <= 1024) { /* 3102 */
        m_nBubbleNum = 6;
        m_nRadius = 65;
    }
    else if (nWidth <= 1280) { /* 8150 */
        m_nBubbleNum = 7;
        m_nRadius = 75;
    }
    else { /* 9150 */
        m_nBubbleNum = 10;
        m_nRadius = 111;
    }

    return 0;
}


int CBubbles::createBubbl() {

    for (int i = 0; i < m_nBubbleNum; ++i) {

        CBall* bubbl = new CBall(m_listPix);
        if (nullptr == bubbl) {
            return -5;
        }

        bubbl->setSpeedX(0);
        bubbl->setSpeedY(0);
        bubbl->setRadius(m_nRadius);
        bubbl->resetOpa();
        m_vecBubbl.push_back(bubbl);
    }

    return 0;
}

int CBubbles::loadImg() {
	string sImgPath;
	string sImgPathTmp;
#ifndef WIN32
	sImgPath = BUBBLES_IMG;
#else
	char    szTmpPath[MAX_PATH] = { 0 };
	CState::GetTempPathCos(MAX_PATH, szTmpPath);
	sImgPath = szTmpPath;
	sImgPath.append("HMIEmulation/img/");
#endif

	sImgPathTmp = sImgPath + "bubble.png";
	QImage img(sImgPathTmp.c_str());

    if (true == img.isNull()) {
        printf("-----------load img error\n");
        return -1;
    }

    QPixmap pix;
    pix.convertFromImage(img);
    pix = pix.scaled(m_nRadius * 2, m_nRadius * 2,
        Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_listPix.push_back(pix);

	sImgPathTmp = sImgPath + "blue.png";
	img.load(sImgPathTmp.c_str());

    if (true == img.isNull()) {
        return -2;
    }

    pix.convertFromImage(img);
    pix = pix.scaled(m_nRadius * 2, m_nRadius * 2,
        Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_listPix.push_back(pix);

	sImgPathTmp = sImgPath + "green.png";
	img.load(sImgPathTmp.c_str());

    if (true == img.isNull()) {
        return -3;
    }

    pix.convertFromImage(img);
    pix = pix.scaled(m_nRadius * 2, m_nRadius * 2,
        Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_listPix.push_back(pix);

	sImgPathTmp = sImgPath + "red.png";
	img.load(sImgPathTmp.c_str());

    if (true == img.isNull()) {
        return -4;
    }
    pix.convertFromImage(img);
    pix = pix.scaled(m_nRadius * 2, m_nRadius * 2,
        Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_listPix.push_back(pix);

    return 0;
}

void CBubbles::resetBallState() {

    QPoint  pt(0, 0);

    for (int i = 0; i < m_nBubbleNum; ++i) {

        m_vecBubbl[i]->setDisp(false);
        switch (i % 8) {
            case 0: { /* 左上角 */
                m_vecBubbl[i]->setCenter(QPoint(0, 0));
            }
            break;
            case 1: { /* 左下角 */
                m_vecBubbl[i]->setCenter(QPoint(0, this->height()));
            }
            break;
            case 2: { /* 右下角 */
                m_vecBubbl[i]->setCenter(QPoint(this->width(), this->height()));
            }
            break;
            case 3: { /* 右上角 */
                m_vecBubbl[i]->setCenter(QPoint(this->width(), 0));
            }
            break;
            case 4: { /* 上中 */
                m_vecBubbl[i]->setCenter(QPoint(this->width() / 2, 0));
            }
            break;
            case 5: { /* 左中 */
                m_vecBubbl[i]->setCenter(QPoint(0, this->height() / 2));
            }
            break;
            case 6: { /* 下中 */
                m_vecBubbl[i]->setCenter(QPoint(this->width() / 2, this->height()));
            }
            break;
            case 7: { /* 右中 */
                m_vecBubbl[i]->setCenter(QPoint(this->width(), this->height() / 2));
            }
            break;
            default: {
                m_vecBubbl[i]->setCenter(QPoint(0, 0));
            }
            break;
        }
        
    }

    return;
}

int CBubbles::slotTimer() {

    if (false == m_bAllShow) {
        popBubble();
    }
    
    checkStop();
    updateBubblePos();
	this->show();
	this->update();
    return 0;
}

void CBubbles::popBubble() {

    if (CTime::get_instance().getTickCount() - m_ullPopTime < BALL_POP_INTERVAL) {
        return;
    }

    m_ullPopTime = CTime::get_instance().getTickCount();

    for (int i = 0; i < m_nBubbleNum; ++i) {

        if (true == m_vecBubbl[i]->getDisp()) {
            continue;
        }

        m_vecBubbl[i]->setDisp(true);
        m_vecBubbl[i]->setSpeedX(genRandomSpeed(MIN_SEED, m_nMaxSpeedX));
        m_vecBubbl[i]->setSpeedY(genRandomSpeed(MIN_SEED, m_nMaxSpeedY));

        if (i == m_nBubbleNum - 1) {
            m_bAllShow = true;
        }

        break;
    }

    return;
}

void CBubbles::updateBubblePos() {

    QPoint  pt;

    for (int i = 0; i < m_nBubbleNum; ++i) {

        CBall* bub = m_vecBubbl[i];
        if (false == bub->getDisp()) {
            continue;
        }

        pt = bub->getCenter();
        float mv = bub->getSpeedX() / (100.0f / m_nInterval);
        pt.setX(pt.x() + mv);
        
        mv = bub->getSpeedY() / (60.0 / m_nInterval);
        pt.setY(pt.y() + mv);

        bub->setCenter(pt);
        checkWalls(*bub);
    }

    for (int i = 0; i < m_nBubbleNum - 1; ++i) {

        CBall* bub0 = m_vecBubbl[i];

        for (int j = i + 1; j < m_nBubbleNum; ++j) {

            CBall* bub1 = m_vecBubbl[j];
            if (true == bub1->getDisp()) {
                checkCollision(*bub0, *bub1);
            }
        }
    }

    return;
}

void CBubbles::checkWalls(CBall& bub) {

    QPoint  pt = bub.getCenter();
    bool    bUpdate = false;

    if (pt.x() < 0) {
        pt.setX(0);
        bub.setSpeedX(0 - bub.getSpeedX());
        bub.resetOpa();
        bUpdate = true;
    }
    else if (pt.x() > this->width()) {
        pt.setX(this->width());
        bub.setSpeedX(0 - bub.getSpeedX());
        bub.resetOpa();
        bUpdate = true;
    }

    if (pt.y() < 0) {
        pt.setY(0);
        bub.setSpeedY(0 - bub.getSpeedY());
        bub.resetOpa();
        bUpdate = true;
    }
    else if (pt.y() > this->height()) {
        pt.setY(this->height());
        bub.setSpeedY(0 - bub.getSpeedY());
        bub.resetOpa();
        bUpdate = true;
    }

    if (true == bUpdate) {
        bub.setCenter(pt);
    }

    return;
}

void CBubbles::checkCollision(CBall& bub0, CBall& bub1) {

    QPoint  pt0 = bub0.getCenter(), pt1 = bub1.getCenter();
    int dx = pt1.x() - pt0.x(), dy = pt1.y() - pt0.y();

    double dist = sqrt(dx * dx + dy * dy);
    if (dist > m_nRadius * 2) {
        return;
    }

    if (dist < 1) {
        dist = 1;
    }

    double py1 = (dy * dy) * bub1.getSpeedY() / dist / dist +
                 (dy * dx) * bub1.getSpeedX() / dist / dist;

    double px1 = (dx * dx) * bub1.getSpeedX() / dist / dist +
                 (dx * dy) * bub1.getSpeedY() / dist / dist;

    double py2 = (dy * dy) * bub0.getSpeedY() / dist / dist +
                 (dy * dx) * bub0.getSpeedX() / dist / dist;

    double px2 = (dx * dx) * bub0.getSpeedX() / dist / dist +
                 (dx * dy) * bub0.getSpeedY() / dist / dist;

    float fSpeed = bub0.getSpeedX() + px1 - px2;
    if ((fSpeed > 0) && (fSpeed < 1)) {
        bub0.setSpeedX(1);
    }
    else if ((fSpeed < 0) && (fSpeed > -1)) {
        bub0.setSpeedX(-1);
    }
    else {
        bub0.setSpeedX(fSpeed);
    }
    
    fSpeed = bub0.getSpeedY() + py1 - py2;
    if ((fSpeed > 0) && (fSpeed < 1)) {
        bub0.setSpeedY(1);
    }
    else if ((fSpeed < 0) && (fSpeed > -1)) {
        bub0.setSpeedY(-1);
    }
    else {
        bub0.setSpeedY(fSpeed);
    }

    fSpeed = bub1.getSpeedX() - px1 + px2;
    if ((fSpeed > 0) && (fSpeed < 1)) {
        bub1.setSpeedX(1);
    }
    else if ((fSpeed < 0) && (fSpeed > -1)) {
        bub1.setSpeedX(-1);
    }
    else {
        bub1.setSpeedX(fSpeed);
    }

    fSpeed = bub1.getSpeedY() - py1 + py2;
    if ((fSpeed > 0) && (fSpeed < 1)) {
        bub1.setSpeedY(1);
    }
    else if ((fSpeed < 0) && (fSpeed > -1)) {
        bub1.setSpeedY(-1);
    }
    else {
        bub1.setSpeedY(fSpeed);
    }

    //移动第二个圆到延长线上去。
    float d2 = m_nRadius * 2 - dist + 1;
    float dx2 = d2 * (dx / dist);
    float dy2 = d2 * (dy / dist);
    pt1.setX(pt1.x() + dx2);
    pt1.setY(pt1.y() + dy2);
    bub1.setCenter(pt1);

    bub0.resetOpa();
    bub1.resetOpa();

    return;
}

void CBubbles::checkStop() {

    bool bStop = true;

    for (int i = 0; i < m_nBubbleNum; ++i) {

        CBall* bub = m_vecBubbl[i];
        int nSpeed = bub->getSpeedX() * bub->getSpeedX();
        nSpeed += bub->getSpeedY() * bub->getSpeedY();
        if (nSpeed > 2) {
            bStop = false;
            break;
        }
    }

    if (false == bStop) {
        return;
    }

    for (int i = 0; i < m_nBubbleNum; ++i) {

        CBall* bub = m_vecBubbl[i];
        bub->setSpeedX(genRandomSpeed(MIN_SEED, m_nMaxSpeedX));
        bub->setSpeedY(genRandomSpeed(MIN_SEED, m_nMaxSpeedY));
    }

    return;
}

static int genRandomSpeed(int nMin, int nMax) {

    int nVal = genRandomNumber(0, 1);
    int nSpeed = genRandomNumber(nMin, nMax);
    if (1 == nVal) {
        nSpeed = 0 - nSpeed;
    }

    return nSpeed;
}