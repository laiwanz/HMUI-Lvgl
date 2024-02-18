#include "csvgbutton.h"
#include <QPainter>
#include <QSvgRenderer>
#include "../imgcachemgr.h"

CSvgButton::CSvgButton(const QString & sPath, QWidget *parent)
	:QPushButton(parent) {
	this->setSvgPath(sPath);
}

CSvgButton::CSvgButton(QWidget *parent) : QPushButton(parent) {
	this->setSvgPath("");
}

int CSvgButton::setSvgPath(const QString &sPath) {
	m_sOriginalPath = m_sImagePath = sPath;
	return 0;
}

void CSvgButton::paintEvent(QPaintEvent *event)
{
#if defined(WIN32) && !defined(HMIPREVIEW)
	QByteArray  byteArray;
	ImgCacheMgr::loadSvg(m_sImagePath.toStdString(), byteArray);
	QSvgRenderer renderer(byteArray, this);
	QPainter p(this);
	renderer.render(&p);
#else
	QSvgRenderer renderer(this);
	renderer.load(m_sImagePath);
	QPainter p(this);
	renderer.render(&p);
#endif
}

void CSvgButton::mousePressEvent(QMouseEvent * e) {
	m_sImagePath = m_sImagePath.left(m_sImagePath.size() - 4 > 0 ? m_sImagePath.size() - 4 : m_sImagePath.size()) + QString("-click.svg");
	QPushButton::mousePressEvent(e);
}

void CSvgButton::mouseReleaseEvent(QMouseEvent * e) {
	m_sImagePath = m_sOriginalPath;
	QPushButton::mouseReleaseEvent(e);
}
