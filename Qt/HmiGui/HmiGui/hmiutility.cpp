#include "hmiutility.h"
#ifdef _LINUX_
#include <sys/time.h>
#include <iconv.h>
#else
#include <winbase.h>
#endif
#include <QtDebug>
#include <QString>
#include <QColor>
#include <QResizeEvent>
#include <QSize>
#include <QRect>
#include <QPoint>

QString HMISTR(const char *pStr) {
	if (pStr == NULL) {
		return QString();
	}

	return QString::fromUtf8(pStr, strlen(pStr));
}

void QStringToChar(const QString &strIn, char *psOut, const int &nMaxLen) {
	if (psOut == NULL) {
		return;
	}

	std::string			cstr = strIn.toStdString();
	int					nLen = strlen(cstr.c_str());

	if (nLen > nMaxLen) {
		nLen = nMaxLen;
	}
	strncpy(psOut, cstr.c_str(), nLen);

	return;
}

HMIUtility::HMIUtility() {
}

#if ((defined WIN32) && (defined WIN32SCALE))
void HMIUtility::SetScaling(QResizeEvent *event) {
	if (NULL == event)
		return;

	m_dWidthScale = (double) ( event->size().width() ) / (double) ( m_nInitWidth );
	m_dHeightScale = (double) ( event->size().height() ) / (double) ( m_nInitHeight );
}

void HMIUtility::SetScaling(int nScaleWidth, int nScaleHeight) {
	m_dWidthScale = (double) ( nScaleWidth ) / (double) ( m_nInitWidth );
	m_dHeightScale = (double) ( nScaleHeight ) / (double) ( m_nInitHeight );
}

void HMIUtility::CountScaleRect(RECT *rect) {
	if (NULL == rect)
		return;

	rect->left = (double) rect->left*(double) m_dWidthScale;
	rect->right = (double) rect->right*(double) m_dWidthScale + 0.5;
	rect->top = (double) rect->top*(double) m_dHeightScale;
	rect->bottom = (double) rect->bottom*(double) m_dHeightScale + 0.5;
}

void HMIUtility::CountScaleRect(QRect *rect) {
	if (NULL == rect)
		return;

	rect->setLeft((double) rect->left()*(double) m_dWidthScale);
	rect->setRight((double) rect->right()*(double) m_dWidthScale + 0.5);
	rect->setTop((double) rect->top()*(double) m_dHeightScale);
	rect->setBottom((double) rect->bottom()*(double) m_dHeightScale + 0.5);
}

void HMIUtility::CountScaleSize(QSize *size) {
	if (NULL == size)
		return;

	size->setWidth((double) size->width()*(double) m_dWidthScale + 0.5);
	size->setHeight((double) size->height()*(double) m_dHeightScale + 0.5);
}

void HMIUtility::CountScaleWH(int *pnWidth, int *pnHeight) {
	if (NULL != pnWidth) {
		*pnWidth = (double) ( *pnWidth )*(double) m_dWidthScale + 0.5;
	}

	if (NULL != pnHeight) {
		*pnHeight = (double) ( *pnHeight )*(double) m_dHeightScale + 0.5;
	}
}

void HMIUtility::CountMousePoint(QPoint *point) {
	if (NULL == point)
		return;

	point->setX((double) point->x() / (double) m_dWidthScale);
	point->setY((double) point->y() / (double) m_dHeightScale);
}
#endif

#if ((defined WIN32) && (defined WIN32SCALE))
int HMIUtility::m_nInitWidth = 0;
int HMIUtility::m_nInitHeight = 0;

double HMIUtility::m_dWidthScale = 1;
double HMIUtility::m_dHeightScale = 1;
#endif