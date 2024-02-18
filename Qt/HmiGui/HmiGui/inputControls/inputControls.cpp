#include "inputControls.h"
#include <QFont>
#include <QPalette>
#include <QDebug>
#include <QGuiApplication>
#include "../EventMgr/EventMgr.h"
#include "../imgcachemgr.h"
#include "../hmiutility.h"
#include <QPainter>

CInputControls::CInputControls(const hmiproto::hmipartinput &partInput, QWidget *parent) : QWidget(parent) {
	m_pLineEdit = nullptr;
	m_pTextEdit = nullptr;
	m_bLoadLockImg = false;
	m_nLockImgIndex = -1;
	m_nScreenNo = partInput.basic().scrno();
	m_sPartName = partInput.basic().name();
	m_nTextColor = partInput.basic().textcolor();
	m_bTextGray = partInput.gray();
	m_pBackGroundWidget = new CBackgroundWidget(this);
	m_pLockWidget = new QWidget(this);
	m_pLockWidget->hide();
	QRect rc;
	rc.setLeft(partInput.basic().left());
	rc.setRight(partInput.basic().right());
	rc.setTop(partInput.basic().top());
	rc.setBottom(partInput.basic().bottom());
	this->setGeometry(rc);
	m_pBackGroundWidget->setGeometry(QRect(0, 0, rc.width(), rc.height()));
	if (!partInput.ismultiline()) {
		m_pLineEdit = new CInputLineEdit(this);
		connect(m_pLineEdit, &CInputLineEdit::setvalue, this, [=](const QString &sbuffer) {
			EventMgr::SendDataTransEvent(m_nScreenNo, m_sPartName, sbuffer.toStdString());
		});
		this->setInputGeometry(QRect(0, 0, rc.width(), rc.height()));
		if (partInput.star()) {
			m_pLineEdit->setEchoMode(QLineEdit::Password);
			QString sStyleSheet = m_pLineEdit->styleSheet() + QString("QLineEdit[echoMode=\"2\"] { lineedit-password-character: 42; }");
			this->setStyleSheet(sStyleSheet);
		}
		m_pLineEdit->config(partInput.dataformat(), partInput.min_(), partInput.max_(), partInput.length(), partInput.decimal());
		if (partInput.has_displayrange()) {
			m_pLineEdit->setDispalyRange(partInput.displayrange());
		}
	}
	else {
		m_pTextEdit = new CInputTextEdit(this);
		this->setInputGeometry(QRect(0, 0, rc.width(), rc.height()));
		connect(m_pTextEdit, &CInputTextEdit::setvalue, this, [=](const QString &sbuffer) {
			EventMgr::SendDataTransEvent(m_nScreenNo, m_sPartName, sbuffer.toStdString());
		});
		m_pTextEdit->config(partInput.length());
	}
	this->setLanguage(partInput.basic().msgtiplanguage());
	this->setInputText(QString(partInput.basic().text().c_str()));
	this->saveLastText(QString(partInput.basic().text().c_str()));
	this->setInputHide(partInput.basic().hide());
	this->setInputFont(partInput.basic().font().c_str());
	this->setInputAlignment(partInput.basic().align(), partInput.basic().verticalalign());
	if (m_bTextGray) {
		this->setInputTextColor(QColor(220, 220, 220));
	}
	else {
		this->setInputTextColor(m_nTextColor);
	}
	m_nBackgroundImg = partInput.basic().imgindex();
	m_sBackgroundImg = partInput.basic().imgpath();
	m_pBackGroundWidget->setBackgroudImage(m_nBackgroundImg, m_sBackgroundImg.c_str());
	m_nLockWidth = partInput.lockimgwidth();
	if (partInput.has_lockimg()) {
		m_nLockImgIndex = partInput.lockimg();
		this->setLockImage(partInput.lockimgpath());
	}
	this->setConfirmFlag(partInput.operateconfirm(), partInput.operatewaittime());
}

CInputControls::~CInputControls() {
	if (m_pLineEdit) {
		delete m_pLineEdit;
		m_pLineEdit = nullptr;
	}
	if (m_pTextEdit) {
		delete m_pTextEdit;
		m_pTextEdit = nullptr;
	}
	if (m_pLockWidget) {
		delete m_pLockWidget;
		m_pLockWidget = nullptr;
	}
}

int CInputControls::setLockImage(const std::string &sPath) {
	if (m_pLockWidget == nullptr ||
		m_nLockImgIndex == -1 ||
		m_nLockWidth == 0) {
		return -1;
	}
	if (sPath.find("NoFace.png") != std::string::npos) {
		m_pLockWidget->hide();
		return 0;
	}
	else if (m_bLoadLockImg) {
		m_pLockWidget->show();
		return 0;
	}
	IMG_T *pIMG = ImgCacheMgr::GetImgInfo(m_nLockImgIndex);
	if (pIMG == nullptr) {
		return -2;
	}
	QRect rc(0, 0, m_nLockWidth, m_nLockWidth);
	m_pLockWidget->setGeometry(rc);
	pIMG->mutex.lock();
	QBrush brush = QBrush(QPixmap(*pIMG->pix).scaled(m_pLockWidget->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	QPalette palette = m_pLockWidget->palette();
	palette.setBrush(QPalette::Background, brush);
	m_pLockWidget->setPalette(palette);
	m_pLockWidget->setAutoFillBackground(true);
	pIMG->mutex.unlock();
	m_bLoadLockImg = true;
	m_pLockWidget->show();
	return 0;
}

void CInputControls::Init(QWidget *pWidget) {
	this->raise();
#if ((defined WIN32) && (defined WIN32SCALE))
	RECT rc;
	rc.left = this->geometry().left();
	rc.right = this->geometry().right();
	rc.top = this->geometry().top();
	rc.bottom = this->geometry().bottom();
	m_rect.setLeft(rc.left);
	m_rect.setRight(rc.right);
	m_rect.setTop(rc.top);
	m_rect.setBottom(rc.bottom);
	memcpy(&m_OriginalRect, &rc, sizeof(RECT));
	this->ZoomPartSelf();
#endif
}

void CInputControls::Update(const hmiproto::hmipartinput &partInput, QWidget *pWidget) {
	if (partInput.basic().has_text()) {
		if (this->isInputReadOnly()) {
			this->setInputText(QString(partInput.basic().text().c_str()));
		}
		this->saveLastText(QString(partInput.basic().text().c_str()));
	}
	if (partInput.basic().has_hide()) {
		this->setInputHide(partInput.basic().hide());
	}
	if (partInput.basic().has_left() ||
		partInput.basic().has_right() ||
		partInput.basic().has_top() ||
		partInput.basic().has_bottom()) {
#if ((defined WIN32) && (defined WIN32SCALE))
		if (partInput.basic().has_left()) {
			m_rect.setLeft(partInput.basic().left());
		}
		if (partInput.basic().has_right()) {
			m_rect.setRight(partInput.basic().right());
		}
		if (partInput.basic().has_top()) {
			m_rect.setTop(partInput.basic().top());
		}
		if (partInput.basic().has_bottom()) {
			m_rect.setBottom(partInput.basic().bottom());
		}
		memcpy(&m_OriginalRect, &m_rect, sizeof(RECT));
		this->ZoomPartSelf();
#else
		QRect rc = this->geometry();
		if (partInput.basic().has_left()) {
			rc.setLeft(partInput.basic().left());
		}
		if (partInput.basic().has_right()) {
			rc.setRight(partInput.basic().right());
		}
		if (partInput.basic().has_top()) {
			rc.setTop(partInput.basic().top());
		}
		if (partInput.basic().has_bottom()) {
			rc.setBottom(partInput.basic().bottom());
		}
		this->setGeometry(rc);
		this->setInputGeometry(QRect(0, 0, rc.width(), rc.height()));
		m_pBackGroundWidget->setGeometry(QRect(0, 0, rc.width(), rc.height()));
		m_pBackGroundWidget->setBackgroudImage(m_nBackgroundImg, m_sBackgroundImg.c_str());
#endif
	}
	if (partInput.basic().has_textcolor()) {
		m_nTextColor = partInput.basic().textcolor();
	}
	if (partInput.has_gray()) {
		m_bTextGray = partInput.gray();
	}
	if (m_bTextGray) {
		this->setInputTextColor(QColor(220, 220, 220));
	}
	else {
		this->setInputTextColor(m_nTextColor);
	}
	if (partInput.has_lockimgwidth()) {
		m_nLockWidth = partInput.lockimgwidth();
	}
	if (partInput.has_lockimg()) {
		m_nLockImgIndex = partInput.lockimg();
		this->setLockImage(partInput.lockimgpath());
	}
	if (partInput.has_max_()) {
		if (m_pLineEdit) {
			m_pLineEdit->setMaxRange(partInput.max_());
		}
	}
	if (partInput.has_min_()) {
		if (m_pLineEdit) {
			m_pLineEdit->setMinRange(partInput.min_());
		}
	}
	if (partInput.basic().has_msgtiplanguage()) {
		this->setLanguage(partInput.basic().msgtiplanguage());
	}
}

void CInputControls::Draw(QWidget *pWidget, int drawtype) {

}

int CInputControls::setInputText(const QString &sText) {
	if (m_pLineEdit) {
		m_pLineEdit->setText(sText);
	}
	else if (m_pTextEdit) {
		m_pTextEdit->setText(sText);
	}
	return 0;
}

int CInputControls::setInputGeometry(const QRect &rc) {
	if (m_pLineEdit) {
		m_pLineEdit->setRect(rc);
	}
	else if (m_pTextEdit) {
		m_pTextEdit->setGeometry(rc);
	}
	return 0;
}

QRect CInputControls::getInputGeometry() {
	if (m_pLineEdit) {
		return m_pLineEdit->geometry();
	}
	else if (m_pTextEdit) {
		return m_pTextEdit->geometry();
	}
	return QRect();
}

int CInputControls::setInputReadOnly(bool enabled) {
	if (m_pLineEdit) {
		m_pLineEdit->setReadOnly(enabled);
	}
	else if (m_pTextEdit) {
		m_pTextEdit->setReadOnly(enabled);
	}
	return 0;
}

bool CInputControls::isInputReadOnly() {
	if (m_pLineEdit) {
		return m_pLineEdit->isReadOnly();
	}
	else if (m_pTextEdit) {
		return m_pTextEdit->isReadOnly();
	}
	return false;
}

int CInputControls::setInputFont(const QString &sFont) {
	if (sFont.isEmpty()) {
		return -1;
	}
	QFont font;
	font.setItalic(false);
	font.setBold(false);
	QStringList fontParts = sFont.split(" ");
	for (auto &iter : fontParts) {
		QRegExp sizeRegex("(\\d+)px");
		if (iter == "italic") {
			font.setItalic(true);
		}
		else if (iter == "bold") {
			font.setBold(true);
		}
		else if (sizeRegex.indexIn(iter) != -1) {
			font.setPixelSize(sizeRegex.cap(1).toInt());
		}
		else {
			font.setFamily(iter);
		}
	}
	if (m_pLineEdit) {
		m_pLineEdit->setFont(font);
	}
	else if (m_pTextEdit) {
		m_pTextEdit->setFont(font);
	}
	return 0;
}

int CInputControls::setInputAlignment(unsigned int nHorizontalAlign, unsigned int nVerticalAlign) {
	if (m_pLineEdit) {
		Qt::Alignment eFlag = Qt::AlignCenter;
		switch (nHorizontalAlign)
		{
		case 0:
			{
				eFlag = Qt::AlignCenter;
			}
			break;
		case 1:
			{
				eFlag = Qt::AlignLeft;
			}
			break;
		case 2:
			{
				eFlag = Qt::AlignRight;
			}
			break;
		default:
			break;
		}
		m_pLineEdit->setAlignment(eFlag);
	}
	else if (m_pTextEdit) {
		m_pTextEdit->setHVAlign(nHorizontalAlign, nVerticalAlign);
	}
	return 0;
}

int CInputControls::setInputTextColor(unsigned int nColor) {
	if (m_pLineEdit) {
		QPalette palette = m_pLineEdit->palette();
		palette.setColor(QPalette::Text, QColor(QRgb(nColor)));
		m_pLineEdit->setPalette(palette);
	}
	else if (m_pTextEdit) {
		m_pTextEdit->setTextColor(QColor(QRgb(nColor)));
	}
	return 0;
}

int CInputControls::setInputTextColor(const QColor &color) {
	if (m_pLineEdit) {
		QPalette palette = m_pLineEdit->palette();
		palette.setColor(QPalette::Text, QColor(color));
		m_pLineEdit->setPalette(palette);
	}
	else if (m_pTextEdit) {
		m_pTextEdit->setTextColor(QColor(color));
	}
	return 0;
}

int CInputControls::setInputHide(bool bHide) {
	if (bHide) {
		this->hide();
	}
	else {
		this->show();
	}
	return 0;
}

int CInputControls::setInputFocus() {
	if (m_pLineEdit) {
		m_pLineEdit->setFocus();
	}
	else if (m_pTextEdit) {
		m_pTextEdit->setFocus();
	}
	QGuiApplication::inputMethod()->show();
	return 0;
}

int CInputControls::selectInputAll() {
	if (m_pLineEdit) {
		m_pLineEdit->selectAll();
	}
	else if (m_pTextEdit) {
		m_pTextEdit->selectAll();
	}
	return 0;
}

int CInputControls::setConfirmFlag(bool bConfirmFlag, unsigned int nTimeout) {
	if (m_pLineEdit) {
		return m_pLineEdit->setConfirmParam(bConfirmFlag, nTimeout);
	}
	else if (m_pTextEdit) {
		return m_pTextEdit->setConfirmParam(bConfirmFlag, nTimeout);
	}
	return 0;
}

int CInputControls::setLanguage(unsigned int nLanguage) {
	if (m_pLineEdit) {
		return m_pLineEdit->setLanguage(nLanguage);
	}
	else if (m_pTextEdit) {
		return m_pTextEdit->setLanguage(nLanguage);
	}
	return 0;
}

int CInputControls::parseJson(const std::string &sbuffer) {
	int nRtn = 0;
	Json::Value root;
	if ((nRtn = cbl::loadJsonString(sbuffer, root)) < 0) {
		printf("%s:%d load json error:%d\r\n", __FUNCTION__, __LINE__, nRtn);
		return -1;
	}
	if (root.isNull() ||
		!root.isMember("input") ||
		!root["input"].isMember("current")) {
		return -2;
	}
	if (!this->isInputReadOnly()) {
		return -3;
	}
	std::string sText = "";
	if (root["input"]["current"].isString()) {
		sText = root["input"]["current"].asString();
	}
	this->setInputText(QString(sText.c_str()));
	this->setInputReadOnly(false);
	this->setInputFocus();
	this->selectInputAll();
	return 0;
}

int CInputControls::saveLastText(const QString &sbuffer) {
	if (m_pLineEdit) {
		return m_pLineEdit->saveLastText(sbuffer);
	}
	else if (m_pTextEdit) {
		return m_pTextEdit->saveLastText(sbuffer);
	}
	return 0;
}

bool CInputControls::hasInputFocus() {
	if (m_pLineEdit) {
		return m_pLineEdit->hasFocus();
	}
	else if (m_pTextEdit) {
		return m_pTextEdit->hasFocus();
	}
	return false;
}

#if ((defined WIN32) && (defined WIN32SCALE))
void CInputControls::ZoomPartSelf() {
	RECT rc = m_OriginalRect;
	HMIUtility::CountScaleRect(&rc);
	QRect rcGeometry;
	rcGeometry.setLeft(rc.left);
	rcGeometry.setRight(rc.right);
	rcGeometry.setTop(rc.top);
	rcGeometry.setBottom(rc.bottom);
	this->setGeometry(rcGeometry);
	m_pBackGroundWidget->setGeometry(QRect(0, 0, rcGeometry.width(), rcGeometry.height()));
	this->setInputGeometry(QRect(0, 0, rcGeometry.width(), rcGeometry.height()));
}
#endif