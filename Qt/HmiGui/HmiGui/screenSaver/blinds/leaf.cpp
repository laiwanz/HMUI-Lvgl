#include "leaf.h"

CLeaf::CLeaf() {

    m_direct = LEFT_TO_RIGHT;
}

CLeaf::~CLeaf() {

}

void CLeaf::setDirect(DIRECT_T direct) {

    m_direct = direct;
    return;
}

void CLeaf::setGeometry(int x, int y, int width, int height) {

    m_area.setLeft(x);
    m_area.setTop(y);
    m_area.setWidth(width);
    m_area.setHeight(height);

    return;
}

void CLeaf::paint(QPainter& p, int nHeight) {

    if (nHeight <= 0) {
        return;
    }

    if (nHeight > m_area.height()) {
        nHeight = m_area.height();
    }

    int y = 0;

    if (TOP_TO_BOTTOM == m_direct) { /* 从上往下 */
        y = m_area.y();
    }
    else if (BOTTOM_TO_TOP == m_direct) { /* 从下往上 */
        y = m_area.y() + m_area.height() - nHeight;
    }

    p.drawRect(m_area.x(), y, m_area.width(), nHeight);
    return;
}
