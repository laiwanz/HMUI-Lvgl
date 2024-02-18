#ifndef MONITORSCREEN_H_20230630
#define MONITORSCREEN_H_20230630
#include <QApplication>
#include <QWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QTableWidget>
#include <QPropertyAnimation>
#include "platform/platform.h"
#include "utility/utility.h"
#ifdef _LINUX_
#include "btl/btl.h"
#endif

class MyApplication : public QApplication
{
public:
	MyApplication(int& argc, char** argv) : QApplication(argc, argv) {
		m_originalPos = QPoint(-1, -1);
		m_pWidget = nullptr;
		m_pAnimation = nullptr;
		m_nAnimationTimes = 300;
	}
	bool notify(QObject* receiver, QEvent* event) override
	{
        QWidget* pWidget = qobject_cast<QWidget*>(receiver);
        QWidget* pMainWin = qApp->activeWindow();
		if (pWidget && pMainWin) {
			if (0 == pWidget->objectName().compare("Qt5KeyBoard")) {
				if (event->type() == QEvent::Show) {
					int nWidth = 800, nHeight = 480, nBitPerPixel = 0;
#ifdef WIN32
					nWidth = 1920;
					nHeight = 1080;
					nBitPerPixel = 32;
#else
					UTILITY::CState::getLcdResolutionByHMIUIRot(nWidth, nHeight, nBitPerPixel);
#endif 
					if (nWidth > nHeight){
						pWidget->setGeometry(0, nHeight / 2, nWidth, nHeight / 5 * 3);
					}
					else{
						pWidget->setGeometry(0, nHeight / 2, nWidth, nHeight / 5 * 2);
					}
					this->animateStop();
					QWidget* pCurrentWidget = qobject_cast<QWidget*>(QApplication::focusObject());
					QPoint globalPos = pCurrentWidget->mapToGlobal(QPoint(0, 0));
					QRect curRect = pCurrentWidget->geometry();
					int nFouceHeight = curRect.height() + globalPos.y();
					int nKeyBoardHeight = QGuiApplication::inputMethod()->keyboardRectangle().height();
					nFouceHeight = nHeight - nFouceHeight;
					if (nFouceHeight < nKeyBoardHeight) {
						m_originalPos = pMainWin->pos();
						m_pWidget = pMainWin;
						animateMove(pMainWin, QPoint(pMainWin->x(), pMainWin->y() - (nKeyBoardHeight - nFouceHeight)));
					}
				}
				else if (event->type() == QEvent::Hide) {
					this->animateStop();
					if (m_originalPos != QPoint(-1, -1) && 
						m_pWidget != nullptr) {
						animateMove(m_pWidget, m_originalPos);
						m_originalPos = QPoint(-1, -1);
					}
				}
			}
			QObject* pObject = pWidget->parent();
			if (pObject != nullptr &&
				QApplication::inputMethod()->isVisible() &&
				QApplication::inputMethod()->objectName() == pObject->objectName()) {
				if (event->type() == QEvent::MouseButtonPress) {
#ifdef _LINUX_
					btl::beep(true);
#endif
				}
			}
		}
		return QApplication::notify(receiver, event);
	}
private:
	void animateStop() {
		if (m_pAnimation &&
			m_pAnimation->state() == QAbstractAnimation::Running) {
			m_pAnimation->setCurrentTime(m_nAnimationTimes);
		}
	}
	void animateMove(QWidget *pWidget, const QPoint &dstPos) {
		if (pWidget == nullptr) {
			return;
		}
		if (m_pAnimation != nullptr) {
			delete m_pAnimation;
			m_pAnimation = nullptr;
		}
		m_pAnimation = new QPropertyAnimation(pWidget, "pos");
		m_pAnimation->setDuration(m_nAnimationTimes);
		m_pAnimation->setStartValue(pWidget->pos());
		m_pAnimation->setEndValue(dstPos);
		connect(m_pAnimation, &QPropertyAnimation::finished, [this]() {
			if (m_pAnimation != nullptr) {
				delete m_pAnimation;
				m_pAnimation = nullptr;
			}
		});
		m_pAnimation->start();
	}
private:
	QPoint m_originalPos;
	QWidget* m_pWidget;
	QPropertyAnimation *m_pAnimation;
	int m_nAnimationTimes;
};
#endif
