#include "inputLineEdit.h"
#include <QMouseEvent>
#include <QGuiApplication>
#include <QApplication>
#include <QDebug>
#include "multiplatform/multiplatform.h"
#include "../common/MessageTip/messagetip.h"
#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif

CInputLineEdit::CInputLineEdit(QWidget *parent) : QLineEdit(parent) {
	m_pValidator = nullptr;
	m_bMouseButtonPress = m_bConfirmFlag = m_bConfirmStatRunning = false;
	m_sCurrentText.clear();
	m_sMin = m_sMax = "";
	m_nLanguage = m_nConfirmTimeout = 0;
	m_pRemarkWidget = nullptr;
	this->setStyleSheet("QLineEdit { border: none; background-color: transparent; }");
	this->setFocusPolicy(Qt::StrongFocus);
	this->setReadOnly(true);
	this->installEventFilter(this);
}

CInputLineEdit::~CInputLineEdit() {
	if (m_pValidator) {
		delete m_pValidator;
		m_pValidator = nullptr;
	}
	if (m_pRemarkWidget) {
		delete m_pRemarkWidget;
		m_pRemarkWidget = nullptr;
	}
}

int CInputLineEdit::config(unsigned int nDataFormat, const std::string &sMin, const std::string &sMax, unsigned int nLength, unsigned int nDecimal) {
	m_pValidator = new CTextValidator(this);
	m_pValidator->config(nDataFormat, sMin, sMax, nLength, nDecimal);
	m_sMin = sMin;
	m_sMax = sMax;
	if (_String != nDataFormat) {
		this->setInputMethodHints(Qt::ImhDigitsOnly);
	}
	this->setValidator(m_pValidator);
	return 0;
}

int CInputLineEdit::setDispalyRange(bool bDisPlayRange) {
	m_bDisplayRange = bDisPlayRange;
	if (m_bDisplayRange) {
		initRemarkWidget();
	}
	return 0;
}

int CInputLineEdit::setRect(const QRect &rc) {
	this->setGeometry(rc);
	this->calculate();
	return 0;
}

int CInputLineEdit::setMinRange(const std::string &sbuffer) {
	if (m_sMin == sbuffer) {
		return 0;
	}
	if (m_pValidator) {
		m_pValidator->setMinRange(sbuffer);
	}
	this->setRemarkWidgetRange();
	return 0;
}

int CInputLineEdit::setMaxRange(const std::string &sbuffer) {
	if (m_sMax == sbuffer) {
		return 0;
	}
	if (m_pValidator) {
		m_pValidator->setMaxRange(sbuffer);
	}
	this->setRemarkWidgetRange();
	return 0;
}

int CInputLineEdit::setLanguage(unsigned int nLanguage) {
	m_nLanguage = nLanguage;
	this->setRemarkWidgetRange();
	return 0;
}

void CInputLineEdit::mousePressEvent(QMouseEvent *event) {
	if (this->isReadOnly()) {
		this->clearFocus();
		m_bMouseButtonPress = true;
		event->ignore();
		return;
	}
	QLineEdit::mousePressEvent(event);
}

void CInputLineEdit::mouseReleaseEvent(QMouseEvent *event) {
	if (m_bMouseButtonPress) {
		m_bMouseButtonPress = false;
		event->ignore();
		return;
	}
	QLineEdit::mouseReleaseEvent(event);
}

void CInputLineEdit::mouseMoveEvent(QMouseEvent *event) {
	if (event->buttons() & Qt::LeftButton) {
		return;
	}
	QLineEdit::mouseMoveEvent(event);
}

int CInputLineEdit::reset() {
	this->deselect();
	this->setText(m_sCurrentText);
	this->setReadOnly(true);
	if (m_pRemarkWidget) {
		m_pRemarkWidget->hide();
	}
	this->clearFocus();
	return 0;
}

int CInputLineEdit::save() {
	this->deselect();
	emit setvalue(this->text());
	this->setReadOnly(true);
	if (m_pRemarkWidget) {
		m_pRemarkWidget->hide();
	}
	this->clearFocus();
	return 0;
}

int CInputLineEdit::initRemarkWidget() {
	if (m_pRemarkWidget != nullptr) {
		return 0;
	}
	m_pRemarkWidget = new CRemarkWidget(this->parentWidget()->parentWidget());
	this->calculate();
	this->setRemarkWidgetRange();
	m_pRemarkWidget->hide();
	return 0;
}

int CInputLineEdit::setRemarkWidgetRange() {
	if (m_pRemarkWidget == nullptr) {
		return -1;
	}
	QString sbuffer;
	if (m_nLanguage == 2) {
		sbuffer = QString("范围:%1 到 %2").arg(m_sMin.c_str(), m_sMax.c_str());
	}
	else {
		sbuffer = QString("Range:%1 to %2").arg(m_sMin.c_str(), m_sMax.c_str());
	}
	m_pRemarkWidget->setText(sbuffer);
	return this->calculate();
}

int CInputLineEdit::calculate() {
	// calculate
	eWIDGETDIRECTION_T eDirection = eDirection_NONE;
	if (m_pRemarkWidget == nullptr) {
		return -1;
	}
	QRect currentrc = this->parentWidget()->geometry();
	QRect parentrc = this->parentWidget()->parentWidget()->geometry();
	int nWidth = m_pRemarkWidget->width(), nHeight = m_pRemarkWidget->height();
	{
		if (currentrc.top() - nHeight >= 0) {
			if (currentrc.left() + nWidth <= parentrc.width()) {
				eDirection = eDirection_top_right;
			}
			else {
				eDirection = eDirection_top_left;
			}
		}
		else if (currentrc.right() + nWidth <= parentrc.width()) {
			eDirection = eDirection_right;
		}
		else if (currentrc.left() - nWidth >= 0) {
			eDirection = eDirection_left;
		}
		else if (currentrc.bottom() + nHeight <= parentrc.height()) {
			if (currentrc.left() + nWidth <= parentrc.width()) {
				eDirection = eDirection_bottom_right;
			}
			else {
				eDirection = eDirection_bottom_left;
			}
		}
	}
	if (eDirection == eDirection_NONE) {
		return -1;
	}
	switch (eDirection) {
	case eDirection_top_right:
		{
			m_pRemarkWidget->setGeometry(currentrc.left(), currentrc.top() - nHeight, nWidth, nHeight);
		}
		break;
	case eDirection_top_left:
		{
			m_pRemarkWidget->setGeometry(currentrc.right() - nWidth, currentrc.top() - nHeight, nWidth, nHeight);
		}
		break;
	case eDirection_bottom_right:
		{
			m_pRemarkWidget->setGeometry(currentrc.left(), currentrc.bottom(), nWidth, nHeight);
		}
		break;
	case eDirection_bottom_left:
		{
			m_pRemarkWidget->setGeometry(currentrc.right() - nWidth, currentrc.bottom(), nWidth, nHeight);
		}
		break;
	case eDirection_left:
		{
			m_pRemarkWidget->setGeometry(currentrc.left() - nWidth, currentrc.top() + abs(nHeight - currentrc.height()) / 2, nWidth, nHeight);
		}
		break;
	case eDirection_right:
		{
			m_pRemarkWidget->setGeometry(currentrc.right(), currentrc.top() + abs(nHeight - currentrc.height()) / 2, nWidth, nHeight);
		}
		break;
	default:
		break;
	}
	m_pRemarkWidget->setDirection(eDirection);
	return 0;
}

bool CInputLineEdit::eventFilter(QObject *obj, QEvent *ev) {
	if (ev->type() == QEvent::KeyPress) {
		QKeyEvent *pKeyEvent = static_cast<QKeyEvent *>(ev);//将事件转化为键盘事件
		if (pKeyEvent->key() == Qt::Key_Close ||
			pKeyEvent->key() == Qt::Key_Escape) {
			this->reset();
			ev->accept();
		}
		else if (pKeyEvent->key() == Qt::Key_Return || 
			pKeyEvent->key() == Qt::Key_Enter) {
			QString sbuffer = this->text();
			if (m_pValidator->isValid(sbuffer)) {
				if (this->confirmWidget() != 0) {
					this->reset();
				}
				else {
					this->save();
				}
			}
			else {
				if (m_pRemarkWidget) {
					emit m_pRemarkWidget->commitMode(true);
				}
			}
			ev->accept();
		}
	}
	else if (ev->type() == QEvent::FocusOut &&
		!m_bConfirmStatRunning) {
		this->reset();
		ev->accept();
	}
	else if (ev->type() == QEvent::FocusIn) {
		if (!this->isReadOnly() &&
			m_pRemarkWidget) {
			emit m_pRemarkWidget->commitMode(false);
			m_pRemarkWidget->show();
			m_pRemarkWidget->raise();
		}
	}
	return QObject::eventFilter(obj, ev);
}

int CInputLineEdit::confirmWidget() {
	int nRtn = 0;
	if (!m_bConfirmFlag) {
		return 0;
	}
	m_bConfirmStatRunning = true;
	QWidget maskLayer(this->window());
	maskLayer.setFixedSize(this->window()->width(), this->window()->height());
	maskLayer.setStyleSheet("background-color: rgba(0, 0, 0, 0.5);"); // 设置蒙版层的背景色和透明度
	maskLayer.show();
	MessageTip tip(this->parentWidget()->parentWidget());
	tip.setWindowModality(Qt::WindowModal);
	if (m_nConfirmTimeout != 0) {
		tip.confirmParam(m_bConfirmFlag, m_nConfirmTimeout);
	}
	QString sbuffer = "", sOkBuffer = "", sCancelBuffer = "";
	if (m_nLanguage == 2) {
		sbuffer = QString("您确认是否执行该操作？");
		sOkBuffer = QString("确认");
		sCancelBuffer = QString("取消");
	}
	else {
		sbuffer = QString("Please confirm the operation!");
		sOkBuffer = QString("Confirm");
		sCancelBuffer = QString("Cancel");
	}
	tip.setText(sbuffer);
	tip.setOKText(sOkBuffer);
	tip.setCancelText(sCancelBuffer);
	if (tip.exec() == QDialog::Rejected) {
		nRtn =  -1;
	}
	m_bConfirmStatRunning = false;
	return nRtn;
}

int CInputLineEdit::setConfirmParam(bool bConfirmFlag, unsigned int nTimeout) {
	m_bConfirmFlag = bConfirmFlag;
	m_nConfirmTimeout = nTimeout;
	return 0;
}

int CInputLineEdit::saveLastText(const QString &sbuffer) {
	m_sCurrentText = sbuffer;
	return 0;
}