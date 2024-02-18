#include "remarksWidget.h"
#include <QHBoxLayout>
#include <QPainter>
#include "../HmiGui/HmiGui.h"

CRemarkWidget::CRemarkWidget(QWidget *parent) : QWidget(parent) {
	m_nOffset = 10;
	m_nTriangleWidth = 20;
	m_nTriangleHeight = 10;
	m_nRadius = 5;
	this->setWindowFlags(Qt::FramelessWindowHint);
	this->setAttribute(Qt::WA_TranslucentBackground);
	m_pLabel = new QLabel(this);
	QHBoxLayout *pHLayout = new QHBoxLayout(this);
	pHLayout->addWidget(m_pLabel);
	m_pLabel->setAlignment(Qt::AlignCenter);
	int nWidth = 0, nHeight = 0;
#ifdef _WIN32
	HmiGui::GetScaleWH(nWidth, nHeight);
#else
	int nBitPerPixel = 0;
	if (UTILITY::CState::getLcdResolutionByHMIUIRot(nWidth, nHeight, nBitPerPixel, true) < 0) {
		nWidth = 800;
		nHeight = 480;
	}
#endif
	m_pLabel->setStyleSheet(QString("QLabel{font-size: %1px; padding: 16px;padding: 16px 16px 16px 16px;}").arg((int)(nWidth*0.025)));
	m_bWarning = false;
	connect(this, &CRemarkWidget::commitMode, [=](bool isWarning) {
		if (isWarning) {
			this->setStyleSheet("QLabel{ color: white;}");
		}
		else {
			this->setStyleSheet("QLabel{ color: black;}");
		}
		m_bWarning = isWarning;
		return 0;
	});
}

CRemarkWidget::~CRemarkWidget() {

}

int CRemarkWidget::setTriangeOffset(int nOffset) {
	if (m_nOffset == nOffset) {
		return 0;
	}
	m_nOffset = nOffset;
	this->repaint();
	return 0;
}

int CRemarkWidget::setTriangeSize(unsigned int nWidth, unsigned int nHeight) {
	m_nTriangleHeight = nHeight;
	m_nTriangleWidth = nWidth;
	this->repaint();
	return 0;
}

int CRemarkWidget::setText(const QString &sText) {
	m_pLabel->setText(sText);
	this->adjustSize();
	return 0;
}

int CRemarkWidget::setDirection(eWIDGETDIRECTION_T eDirection) {
	m_eDirection = eDirection;
	this->repaint();
	return 0;
}

void CRemarkWidget::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setPen(Qt::NoPen);
	if (m_bWarning) {
		painter.setBrush(QColor(255, 0, 0, 200));
	}
	else {
		painter.setBrush(QColor(255, 255, 255, 200));
	}
	QPainterPath painterPath;
	QPolygon polygon;
	QRect rect(m_pLabel->x(), m_pLabel->y(), m_pLabel->width(), m_pLabel->height());
	int nX = 0, nY = 0;
	switch (m_eDirection)
	{
	case eDirection_bottom_right:
		{
			nX = rect.x() + m_nOffset;
			nY = rect.y();
			polygon << QPoint(nX, nY);
			polygon << QPoint(nX + m_nTriangleWidth, nY);
			polygon << QPoint(nX + m_nTriangleWidth / 2, nY - m_nTriangleHeight);
		}
		break;
	case eDirection_bottom_left:
		{
			nX = rect.x() + rect.width() - m_nOffset;
			nY = rect.y();
			polygon << QPoint(nX, nY);
			polygon << QPoint(nX - m_nTriangleWidth, nY);
			polygon << QPoint(nX - m_nTriangleWidth / 2, nY - m_nTriangleHeight);
		}
		break;
	case eDirection_top_right:
		{
			nX = rect.x() + m_nOffset;
			nY = rect.y() + rect.height();
			polygon << QPoint(nX, nY);
			polygon << QPoint(nX + m_nTriangleWidth / 2, nY + m_nTriangleHeight);
			polygon << QPoint(nX + m_nTriangleWidth, nY);
		}
		break;
	case eDirection_top_left:
		{
			nX = rect.x() + rect.width() - m_nOffset;
			nY = rect.y() + rect.height();
			polygon << QPoint(nX, nY);
			polygon << QPoint(nX - m_nTriangleWidth / 2, nY + m_nTriangleHeight);
			polygon << QPoint(nX - m_nTriangleWidth, nY);
		}
		break;
	case eDirection_right:
		{
			nX = rect.x();
			nY = rect.y() + m_nOffset;
			polygon << QPoint(nX, nY);
			polygon << QPoint(nX - m_nTriangleHeight, nY + m_nTriangleWidth / 2);
			polygon << QPoint(nX, nY + m_nTriangleWidth);
		}
		break;
	case eDirection_left:
		{
			nX = rect.x() + rect.width();
			nY = rect.y() + m_nOffset;
			polygon << QPoint(nX, nY);
			polygon << QPoint(nX + m_nTriangleHeight, nY + m_nTriangleWidth / 2);
			polygon << QPoint(nX, nY + m_nTriangleWidth);
		}
		break;
	default:
		break;
	}
	painterPath.addRoundedRect(rect, m_nRadius, m_nRadius);
	painterPath.addPolygon(polygon);
	painter.drawPath(painterPath);
}