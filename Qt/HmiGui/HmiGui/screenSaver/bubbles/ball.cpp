#include "ball.h"
#include "../common/common.h"

CBall::CBall(QList<QPixmap>& listPix)
    : m_ptCenter(0, 0), m_listPix(listPix) {

    m_nRadius = 0;
    m_nSpeedX = 0;
    m_nSpeedY = 0;
    m_bDisp = false;
    memset(m_fOpa, 0, sizeof(m_fOpa));
    memset(m_fWeight, 0, sizeof(m_fWeight));
    init();
}

CBall::~CBall() {

}

void CBall::setCenter(const QPoint& pt) {

    m_ptCenter = pt;
    return;
}

const QPoint& CBall::getCenter() {

    return m_ptCenter;
}

void CBall::setRadius(int nRadius) {

    m_nRadius = nRadius;
    m_pix = QPixmap(2 * m_nRadius, 2 * m_nRadius);
    m_pix.fill(Qt::transparent);

    return;
}

int CBall::getRadius() {

    return m_nRadius;
}

void CBall::setSpeedX(int speed) {

    m_nSpeedX = speed;
}

int CBall::getSpeedX() {

    return m_nSpeedX;
}

void CBall::setSpeedY(int speed) {

    m_nSpeedY = speed;
}

int CBall::getSpeedY() {

    return m_nSpeedY;
}

void CBall::setDisp(bool bDisp) {

    m_bDisp = bDisp;
    return;
}

bool CBall::getDisp() {

    return m_bDisp;
}

int CBall::init() {

    m_fWeight[0] = 1.0f;
    m_fWeight[1] = 0.8f;
    m_fWeight[2] = 0.8f * 0.8f;

    return 0;
}

void CBall::paint(QPainter& p) {

    if (false == m_bDisp) {
        return;
    }

    p.drawPixmap(m_ptCenter.x() - m_nRadius, m_ptCenter.y() - m_nRadius,
                m_nRadius * 2, m_nRadius * 2, m_pix);

    return;
}

void CBall::resetOpa() {

    for (int i = 0; i < 3; ++i) {
        m_fOpa[i] = genRandomNumber(0, 10) / 10.0;
    }

    m_pix.fill(Qt::transparent);

    QPainter p(&m_pix);
    p.setPen(Qt::transparent);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.drawPixmap(0, 0, m_nRadius * 2, m_nRadius * 2, m_listPix[0]);

    for (int i = 0; i < 3; ++i) {
        p.setOpacity(m_fOpa[i] * m_fWeight[i]);
        p.drawPixmap(0, 0, m_nRadius * 2, m_nRadius * 2, m_listPix[i + 1]);
    }

    return;
}
