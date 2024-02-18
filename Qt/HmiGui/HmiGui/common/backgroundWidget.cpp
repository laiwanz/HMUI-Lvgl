#include "backgroundWidget.h"
#include <QPainter>
#include "../imgcachemgr.h"

CBackgroundWidget::CBackgroundWidget(QWidget *parent) : QWidget(parent) {
	
}

CBackgroundWidget::~CBackgroundWidget() {

}

void CBackgroundWidget::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	if (m_nIndex != BITMAPDISPLAY_INDEX) {
		IMG_T *pIMG = ImgCacheMgr::GetImgInfo(m_nIndex);
		if (pIMG == nullptr) {
			printf("%s:%d get image info error\r\n", __FUNCTION__, __LINE__);
			return;
		}
		pIMG->mutex.lock();
		if (pIMG->nImgType == ImgCacheMgr::NORMALIMG &&
			pIMG->pix != nullptr) {
			painter.drawPixmap(this->rect(), *pIMG->pix);
		}
		else if (pIMG->nImgType == ImgCacheMgr::SVGIMG &&
			pIMG->svg != nullptr) {
			pIMG->svg->render(&painter);
		}
		else if (pIMG->nImgType == ImgCacheMgr::GIFIMG) {
			if (m_pMovie == nullptr) {
				m_pMovie = new QMovie(this);
				m_pMovie->setFileName(pIMG->strImgPath);
				m_sGifpath = pIMG->strImgPath;
				if (m_pMovie->speed() > 50) {
					m_pMovie->setSpeed(50);//这里先限速50
				}
				connect(m_pMovie, &QMovie::updated, [=](const QRect &) {
					this->update();
				});
			}
			else {
				if (0 != m_sGifpath.compare(pIMG->strImgPath)) {
					m_pMovie->stop();
					m_pMovie->setFileName(pIMG->strImgPath);
					m_sGifpath = pIMG->strImgPath;
				}
			}
			m_pMovie->start();
			painter.drawPixmap(this->rect(), m_pMovie->currentPixmap());
		}
		pIMG->mutex.unlock();
	}
	else {
		if (m_sPath.isEmpty()) {
			printf("%s:%d image path is empty!\r\n", __FUNCTION__, __LINE__);
			return;
		}
		QImage image;
		RECT rc;
		QRect rect = this->geometry();
		rc.left = rect.left();
		rc.right = rect.right();
		rc.top = rect.top();
		rc.bottom = rect.bottom();
		ImgCacheMgr::LoadCustomIMG(m_sPath, image, rc);
		if (m_pMovie != nullptr) {
			m_pMovie->stop();
		}
		painter.drawImage(rect, image);
	}
}

int CBackgroundWidget::setBackgroudImage(int nIndex, const QString &sPath) {
	m_nIndex = nIndex;
	m_sPath = sPath;
	return 0;
}