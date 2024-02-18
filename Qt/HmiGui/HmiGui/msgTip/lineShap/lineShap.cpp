#include <QPainter>
#include "lineShap.h"

CLineShap::CLineShap(QWidget* parent)
    :QWidget(parent){

    m_color = QColor(Qt::black);
    m_nWidth = 1;
}

CLineShap::~CLineShap() {

}

void CLineShap::setColor(const QColor& color) {

    m_color = color;
    return;
}

void CLineShap::setWidth(int nWidth) {

    m_nWidth = nWidth;
    setFixedHeight(m_nWidth);
    return;
}

void CLineShap::paintEvent(QPaintEvent*) {

    QPen    pen;
    pen.setColor(m_color);
    pen.setWidth(m_nWidth);

    QPainter p(this);
    p.setPen(pen);

    p.drawLine(0, 0, width(), 0);
    return;
}
