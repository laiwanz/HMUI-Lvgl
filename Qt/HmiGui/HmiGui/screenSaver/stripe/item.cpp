#include "item.h"

CItem::CItem() {

}

CItem::~CItem() {

}

void CItem::setDirect(DIRECT_T direct) {

    m_direct = direct;
    return;
}

void CItem::setGeometry(int x, int y, int width, int height) {

    m_area.setLeft(x);
    m_area.setTop(y);
    m_area.setWidth(width);
    m_area.setHeight(height);

    return;
}

void CItem::move(int x, int y) {

    m_area.setLeft(x);
    m_area.setTop(y);
    return;
}

void CItem::paint(QPainter& p, int nLen) {

    if (nLen < 0) {
        return;
    }

    if (((LEFT_TO_RIGHT == m_direct) || (RIGHT_TO_LEFT == m_direct)) &&
        (nLen > m_area.width())) {
        nLen = m_area.width();
    }
    else if (((TOP_TO_BOTTOM == m_direct) || (BOTTOM_TO_TOP == m_direct)) &&
        (nLen > m_area.height())) {
        nLen = m_area.height();
    }

    if (LEFT_TO_RIGHT == m_direct) {
        p.drawRect(m_area.x(), m_area.y(), nLen, m_area.height());
    }
    else if (RIGHT_TO_LEFT == m_direct) {
        p.drawRect(m_area.width() - nLen, m_area.y(), nLen, m_area.height());
    }
    else if (TOP_TO_BOTTOM == m_direct) {
        p.drawRect(m_area.x(), m_area.y(), m_area.width(), nLen);
    }
    else {
        p.drawRect(m_area.x(), m_area.y() - nLen, m_area.width(), nLen);
    }

    return;
}
