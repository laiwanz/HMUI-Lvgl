#include "screenSaver.h"
#include <QMouseEvent>
#include <QApplication>

CScreenSaver::CScreenSaver()
    :QWidget(nullptr) {

    m_type = TYPE_INVALID;
    m_scrClick = "";
    this->setAttribute(Qt::WA_TranslucentBackground, true);
#ifdef WIN32
    this->setWindowFlags(Qt::FramelessWindowHint);
#else
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
#endif
	this->setAttribute(Qt::WA_TransparentForMouseEvents, true);//允许鼠标点击穿透
}

CScreenSaver::~CScreenSaver() {

}

void CScreenSaver::mousePressEvent(QMouseEvent* ev) {

    if (nullptr == ev) {
        return;
    }

    QWidget* pParent = this->parentWidget();
    if (nullptr == pParent) {
        return;
    }

    this->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    QPoint      pt = this->mapTo(pParent, ev->pos());
    QWidget*    recv = pParent->childAt(pt);

    if (nullptr == recv) {
        recv = pParent;
    }
    else {
        pt = recv->mapFrom(pParent, pt);
    }

    m_scrClick = recv->objectName();
    QMouseEvent me(ev->type(), pt, ev->button(), ev->buttons(), ev->modifiers());
    QApplication::sendEvent(recv, &me);
    this->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    return;
}

void CScreenSaver::mouseReleaseEvent(QMouseEvent* ev) {

    if (nullptr == ev) {
        return;
    }

    QWidget* pParent = this->parentWidget();
    if (nullptr == pParent) {
        return;
    }

    QPoint      pt = this->mapTo(pParent, ev->pos());
    QWidget*    recv = pParent;

    if (pParent->objectName() != m_scrClick) {

        recv = pParent->findChild<QWidget*>(m_scrClick);
        if (nullptr != recv) {
            pt = recv->mapFrom(pParent, pt);
        }
    }

    if (nullptr == recv) {
        return;
    }

    this->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    QMouseEvent me(ev->type(), pt, ev->button(), ev->buttons(), ev->modifiers());
    QApplication::sendEvent(recv, &me);
    this->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    this->stop();
    return;
}

CScreenSaver::TYPE_T CScreenSaver::type() {

    return m_type;
}
