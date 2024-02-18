#include "inputTextEdit.h"
#include <QMouseEvent>
#include <QTextFrame>
#include <QScroller>
#include <QDebug>
#include "HmiBase/HmiBase.h"
#include "../common/MessageTip/messagetip.h"
#include "../HmiGui/HmiGui.h"

CInputTextEdit::CInputTextEdit(QWidget *parent) : QTextEdit(parent) {
	m_sCurrentText.clear();
	m_sFrameText.clear();
	m_bFrameRepaint = false;
	m_bMouseButtonPress = m_bConfirmFlag = m_bConfirmStatRunning = false;
	m_nLanguage = m_nLength = m_nVerticalAlign = m_nConfirmTimeout = 0;
	this->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	this->setFocusPolicy(Qt::StrongFocus);
	this->setReadOnly(true);
	this->setStyleSheet("QTextEdit{ background-color: transparent; }");
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	{ // 拖动
		QTextOption textOption = this->document()->defaultTextOption();
		textOption.setWrapMode(QTextOption::NoWrap);
		this->document()->setDefaultTextOption(textOption);
		QScroller::grabGesture(this, QScroller::LeftMouseButtonGesture);
	}
	this->installEventFilter(this);
	connect(this, &QTextEdit::textChanged, [=]() {
		std::string sText = this->toPlainText().toStdString();
		if (sText.size() > m_nLength) {
			QTextCursor cursor = this->textCursor();
			cursor.deletePreviousChar();
			this->setTextCursor(cursor);
		}
		return 0;
	});
#ifdef _WIN32
	HmiGui::GetScaleWH(m_nWidgetWidth, m_nWidgetHeight);
#else
	int nBitPerPixel = 0;
	if (UTILITY::CState::getLcdResolutionByHMIUIRot(m_nWidgetWidth, m_nWidgetHeight, nBitPerPixel, true) < 0) {
		m_nWidgetWidth = 800;
		m_nWidgetHeight = 480;
}
#endif
	m_pWidget = new QWidget(this);
	m_pWidget->setGeometry(0, 0, (int)(m_nWidgetWidth*0.065), (int)(m_nWidgetWidth*0.065));
	std::string sPath = getPrjDir() + std::string("img/multiline-confirm.png");
	QPixmap pixmap(QString(sPath.c_str()));
	pixmap = pixmap.scaled(m_pWidget->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	m_pWidget->setFocusPolicy(Qt::NoFocus);
	QPalette palette = m_pWidget->palette();
	palette.setBrush(QPalette::Background, QBrush(pixmap));
	m_pWidget->setPalette(palette);
	m_pWidget->setAutoFillBackground(true);
	m_pWidget->hide();
	m_pWidget->installEventFilter(this);
}

CInputTextEdit::~CInputTextEdit() {
	if (m_pWidget) {
		delete m_pWidget;
		m_pWidget = nullptr;
	}
}

int CInputTextEdit::config(unsigned int nLength) {
	m_nLength = nLength;
	return 0;
}

int CInputTextEdit::setHVAlign(unsigned int nHorizontalAlign, unsigned int nVerticalAlign) {
	QTextOption textOption = this->document()->defaultTextOption();
	switch (nHorizontalAlign)
	{
	case 0:
		{	
			textOption.setAlignment(Qt::AlignCenter);
		}
		break;
	case 1:
		{
			textOption.setAlignment(Qt::AlignLeft);
		}
		break;
	case 2:
		{
			textOption.setAlignment(Qt::AlignRight);
		}
		break;
	default:
		break;
	}
	this->document()->setDefaultTextOption(textOption);
	m_nVerticalAlign = nVerticalAlign;
	return 0;
}

void CInputTextEdit::mousePressEvent(QMouseEvent *event) {
	if (this->isReadOnly()) {
		this->clearFocus();
		m_bMouseButtonPress = true;
		event->ignore();
		return;
	}
	QTextEdit::mousePressEvent(event);
}

void CInputTextEdit::mouseMoveEvent(QMouseEvent *event) {
	if (event->buttons() & Qt::LeftButton) {
		return;
	}
	QTextEdit::mouseMoveEvent(event);
}

void CInputTextEdit::mouseReleaseEvent(QMouseEvent *event) {
	if (m_bMouseButtonPress) {
		m_bMouseButtonPress = false;
		event->ignore();
		return;
	}
	QTextEdit::mouseReleaseEvent(event);
}

void CInputTextEdit::paintEvent(QPaintEvent *event) {
	if (this->toPlainText() != m_sFrameText ||
		m_bFrameRepaint) {
		bool bFlag = false;
		m_sFrameText = this->toPlainText();
		m_bFrameRepaint = false;
		QTextCursor cursor = this->textCursor();
		if (this->document()->isEmpty()) {
			cursor.insertText(QString(" "));
			this->setTextCursor(cursor);
			bFlag = true;
		}
		int nCtrlHeight = this->height();// the TextEdit control height
		// access the document top frame and its format
		QTextFrame *pFrame = this->document()->rootFrame();
		QTextFrameFormat frameFmt = pFrame->frameFormat();
		int nTopMargin = 0;
		if (!this->document()->isEmpty()) {
			int nDocHeight = (int)this->document()->size().height();
			// get current top margin and compute the 'black' height of the document
			nTopMargin = (int)frameFmt.topMargin();// the frame top margin
			int nBBDocH = nDocHeight - nTopMargin;// the height of the document 'bounding box'
			// compute and set appropriate frame top margin
			switch (m_nVerticalAlign)
			{
			case 1:
				{
					if (nCtrlHeight <= nBBDocH) {// if the control is shorter than the document
						nTopMargin = 2;
					}// set a nominal top margin
					else {// if the control is taller than the document
						nTopMargin = (nCtrlHeight - nBBDocH) / 2 - 2;// set half of the excess as top margin
					}
				}
				break;
			case 2:
				{
					if (nCtrlHeight <= nBBDocH) {// if the control is shorter than the document
						nTopMargin = 2;
					}// set a nominal top margin
					else {							// if the control is taller than the document
						nTopMargin = (nCtrlHeight - nBBDocH) - 2;		// set half of the excess as top margin
					}
				}
				break;
			default:
				{
					nTopMargin = 2;
				}
				break;
			}
		}
		frameFmt.setTopMargin(nTopMargin);
		pFrame->setFrameFormat(frameFmt);// apply the new format
		if (bFlag) {
			cursor.deletePreviousChar();
			this->setTextCursor(cursor);
		}
	}
	if (this->hasFocus() &&
		!this->isReadOnly()) {
		QRect cursorRect = this->cursorRect();
		int nY = cursorRect.y();
		if (nY + m_pWidget->height() > this->geometry().height()) {
			nY = this->geometry().height() - m_pWidget->height();
		}
		else if (nY < 0) {
			nY = 0;
		}
		m_pWidget->setGeometry(this->geometry().width() - m_pWidget->geometry().width(), nY, (int)(m_nWidgetWidth*0.065), (int)(m_nWidgetWidth*0.065));
		m_pWidget->show();
	}
	else {
		m_pWidget->hide();
	}
	QTextEdit::paintEvent(event);
}

int CInputTextEdit::selectNone() {
	QTextCursor cursor = this->textCursor();
	cursor.clearSelection();
	this->setTextCursor(cursor);
	return 0;
}

int CInputTextEdit::reset() {
	this->selectNone();
	this->setText(m_sCurrentText);
	this->setReadOnly(true);
	this->clearFocus();
	m_bFrameRepaint = true;
	return 0;
}

int CInputTextEdit::save() {
	this->selectNone();
	emit setvalue(this->toPlainText());
	this->setReadOnly(true);
	this->clearFocus();
	m_bFrameRepaint = true;
	return 0;
}

bool CInputTextEdit::eventFilter(QObject *obj, QEvent *ev) {
	if (obj == this) {
		if (ev->type() == QEvent::KeyPress) {
			QKeyEvent *pKeyEvent = static_cast<QKeyEvent *>(ev);//将事件转化为键盘事件
			if (pKeyEvent->key() == Qt::Key_Close ||
				pKeyEvent->key() == Qt::Key_Escape) {
				this->reset();
				ev->accept();
			}
		}
		else if (ev->type() == QEvent::FocusOut &&
			!m_bConfirmStatRunning) {
			this->reset();
			ev->accept();
		}
		else if (ev->type() == QEvent::FocusIn) {
			m_bFrameRepaint = true;
		}
	}
	if (obj == m_pWidget &&
		ev->type() == QEvent::MouseButtonPress) {
		if (this->confirmWidget() == 0) {
			this->save();
		}
		else {
			this->reset();
		}
		ev->accept();
		return true;
	}
	return QObject::eventFilter(obj, ev);
}

int CInputTextEdit::setConfirmParam(bool bConfirmFlag, unsigned int nTimeout) {
	m_bConfirmFlag = bConfirmFlag;
	m_nConfirmTimeout = nTimeout;
	return 0;
}

int CInputTextEdit::setLanguage(unsigned int nLanguage) {
	m_nLanguage = nLanguage;
	return 0;
}

int CInputTextEdit::confirmWidget() {
	int nRtn = 0;
	if (!m_bConfirmFlag) {
		return 0;
	}
	m_bConfirmStatRunning = true;
	QWidget maskLayer(this->window());
	maskLayer.setFixedSize(this->window()->width(), this->window()->height());
	maskLayer.setStyleSheet("background-color: rgba(0, 0, 0, 0.5);"); // 设置蒙版层的背景色和透明度
	maskLayer.show();
	MessageTip tip(this->window());
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
		nRtn = -1;
	}
	m_bConfirmStatRunning = false;
	return nRtn;
}

int CInputTextEdit::saveLastText(const QString &sbuffer) {
	m_sCurrentText = sbuffer;
	return 0;
}