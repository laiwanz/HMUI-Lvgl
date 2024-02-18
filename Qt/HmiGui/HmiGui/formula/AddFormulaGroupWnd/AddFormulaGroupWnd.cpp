#include "AddFormulaGroupWnd.h"
#include "ui_AddFormulaGroupWnd.h"
#include "../csvgbutton.h"
#include "../cwecontablemodel.h"
#include "../cformulapart.h"
#include <QKeyEvent>
#include <QSpacerItem>
#include <QMouseEvent>
#include "../../HmiGui/HmiGui.h"
#ifdef _LINUX_
#include "btl/btl.h"
#include "platform/platform.h"
#endif // _LINUX_
CAddFormulaGroupWnd::CAddFormulaGroupWnd(QWidget *parent) :
	QDialog(parent->parentWidget(), Qt::FramelessWindowHint)
	, ui(new Ui_AddFormulaGroupWnd)
{
	ui->setupUi(this);
	int nWidth = 0, nHeight = 0;
	auto parentWidget = qobject_cast<CWeconTable *>(parent);
	if (nullptr != parentWidget) {
		m_nLanguage = parentWidget->getLanguageType();
	}
	else {
		m_nLanguage = 1;
	}
	this->setWindowTitle("Add");
	ui->groupNameEdit->setMaxLength(31);
#ifndef _LINUX_
#ifndef HMIPREVIEW
	HmiGui::GetScaleWH(nWidth, nHeight);
#else
	nWidth = 800;
	nHeight = 480;
#endif
#else
	int nBitPerPixel = 0;
	UTILITY::CState::getLcdResolutionByHMIUIRot(nWidth, nHeight, nBitPerPixel);
#endif
	// 使样式设置的字体生效
	this->setAttribute(Qt::WA_StyledBackground);
	this->setFont(QFont("simsun", m_nFontSize));
#ifndef _LINUX_
	QPoint point = parent->parentWidget()->mapToGlobal(QPoint(0, 0));
	if (nWidth > nHeight) {
		auto nWndWidth = nWidth * 300 / 800 ,nWndHeight = nHeight * 310 / 480;
		this->setFixedSize(nWndWidth, nWndHeight);
		this->move(point.x() + (nWidth - nWndWidth) / 2, point.y() + (nHeight - nWndHeight) / 2);
	}
	else {
		auto nWndWidth = nWidth * 300 / 480, nWndHeight = nHeight * 310 / 800;
		this->setFixedSize(nWidth * 300 / 480, nHeight * 310 / 800);
		this->move(point.x() + (nWidth - nWndWidth) / 2, point.y() + (nHeight - nWndHeight) / 2);
	}
#else
	if (nWidth > nHeight) {
		auto nWndWidth = nWidth * 300 / 800, nWndHeight = nHeight * 350 / 480;
		this->setFixedSize(nWndWidth, nWndHeight);
		this->move((nWidth - nWndWidth) / 2, (nHeight - nWndHeight) / 2);
}
	else {
		auto nWndWidth = nWidth * 300 / 480, nWndHeight = nHeight * 350 / 800;
		this->setFixedSize(nWndWidth, nWndHeight);
		this->move((nWidth - nWndWidth) / 2, (nHeight - nWndHeight) / 2);
	}
#endif // !_LINUX_
	m_nFontSize = nHeight * 0.03;
	if (nWidth < nHeight) { 
		m_nFontSize = nWidth * 0.03;
	}
	if (m_nFontSize < 8) {
		m_nFontSize = 8;
	}
	m_nPosition = 2;
	ui->closeBtn->setSvgPath(QString::fromStdString(CWeconTableModel::g_sFormulaImagePath + "/closeWnd.svg"));
	m_currentPushButton = ui->labTop;
	selectLabel(true, ui->labTop);
	selectLabel(false, ui->labUp);
	selectLabel(false, ui->labDown);
	selectLabel(false, ui->labBottom);
	QString sStyle = "QDialog{background-color: rgb(255, 255, 255);}";
	sStyle += "QLabel{font-size:" + QString::number(m_nFontSize) + "px;}";
	sStyle += "QPushButton{font-size:" + QString::number(m_nFontSize) + "px;}";
	sStyle += "QLineEdit{font-size:" + QString::number(m_nFontSize) + "px;}";
	this->setStyleSheet(sStyle);
	ui->groupNameEdit->setPlaceholderText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::pleaseInput]);
	ui->labDesc->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::pleaseSelect]);
	ui->labCurentChoose->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::intop]);
	ui->labTop->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::intop]);
	ui->labUp->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::preline]);
	ui->labDown->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::nextline]);
	ui->labBottom->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::inbottom]);
	ui->btnAdd->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::add]);
	connect(ui->closeBtn, &CSvgButton::clicked, this, &CAddFormulaGroupWnd::clickCloseButton);
	connect(ui->labTop, &QPushButton::clicked, this, &CAddFormulaGroupWnd::clickPositionButton);
	connect(ui->labUp, &QPushButton::clicked, this, &CAddFormulaGroupWnd::clickPositionButton);
	connect(ui->labDown, &QPushButton::clicked, this, &CAddFormulaGroupWnd::clickPositionButton);
	connect(ui->labBottom, &QPushButton::clicked, this, &CAddFormulaGroupWnd::clickPositionButton);
	connect(ui->btnAdd, &QPushButton::clicked, this, &CAddFormulaGroupWnd::clickAddButton);
	ui->groupNameEdit->installEventFilter(this);
}

CAddFormulaGroupWnd::~CAddFormulaGroupWnd() {
}

int CAddFormulaGroupWnd::getPosition() {
	return m_nPosition;
}

void CAddFormulaGroupWnd::changePosition(int &nPosition) {
	if (nPosition < 0 || nPosition > 3) {
		return;
	}
	if (nullptr != m_currentPushButton) {
		selectLabel(false, m_currentPushButton);
	}
	m_nPosition = nPosition;
	if (0 == m_nPosition) {
		m_currentPushButton = ui->labUp;
		selectLabel(true, ui->labUp);
		ui->labCurentChoose->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::preline]);
	}
	else if (1 == m_nPosition) {
		m_currentPushButton = ui->labDown;
		selectLabel(true, ui->labDown);
		ui->labCurentChoose->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::nextline]);
	}
	else if (2 == m_nPosition) {
		m_currentPushButton = ui->labTop;
		selectLabel(true, ui->labTop);
		ui->labCurentChoose->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::intop]);
	}
	else if (3 == m_nPosition) {
		m_currentPushButton = ui->labBottom;
		selectLabel(true, ui->labBottom);
		ui->labCurentChoose->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::inbottom]);
	}
}

void CAddFormulaGroupWnd::clickAddButton() {
#ifdef _LINUX_
	btl::beep(true);
#endif
	QString sText = ui->groupNameEdit->text();
	if (sText.isEmpty()) {
		CWeconTable::showMsgWnd("warning", CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::groupIsEmpty], QMessageBox::NoButton, this);
		return;
	}
	emit addGroup(m_nPosition, sText);
	this->close();
}

void CAddFormulaGroupWnd::clickPositionButton() {
#ifdef _LINUX_
	btl::beep(true);
#endif
	if (nullptr == m_currentPushButton) {
		return;
	}
	QPushButton *pushButton = qobject_cast<QPushButton*>(sender());
	if (nullptr != pushButton && 
		pushButton != m_currentPushButton) {
		selectLabel(false, m_currentPushButton);
		QString sText = pushButton->text();
		if (CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::preline] == sText) {
			m_currentPushButton = ui->labUp;
			m_nPosition = 0;
			selectLabel(true, ui->labUp);
			ui->labCurentChoose->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::preline]);
		}
		else if (CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::nextline] == sText) {
			m_currentPushButton = ui->labDown;
			m_nPosition = 1;
			selectLabel(true, ui->labDown);
			ui->labCurentChoose->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::nextline]);
		}
		else if (CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::intop] == sText) {
			m_currentPushButton = ui->labTop;
			m_nPosition = 2;
			selectLabel(true, ui->labTop);
			ui->labCurentChoose->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::intop]);
		}
		else if (CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::inbottom] == sText) {
			m_currentPushButton = ui->labBottom;
			m_nPosition = 3;
			selectLabel(true, ui->labBottom);
			ui->labCurentChoose->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_nLanguage][CFormulaPart::inbottom]);
		}
		else {
			selectLabel(true, m_currentPushButton);
		}
	}
}

void CAddFormulaGroupWnd::clickCloseButton() {
#ifdef _LINUX_
	btl::beep(true);
#endif
	this->close();
}

int CAddFormulaGroupWnd::selectLabel(bool bSelected, QPushButton *pushButton) {
	if (bSelected) {
		pushButton->setStyleSheet("border:1px solid rgb(6,97,179);background-color:white;color:rgb(6,97,179);font-size:" + QString::number(m_nFontSize) + "px;");
	}
	else {
		pushButton->setStyleSheet("border:0px solid rgb(0,0,255);background-color:rgb(239,244,248);font-size:" + QString::number(m_nFontSize) + "px;");
	}
	return 0;
}

bool CAddFormulaGroupWnd::eventFilter(QObject *obj, QEvent *ev) {
	if (ev->type() == QEvent::KeyPress) {
		QKeyEvent *pKeyEvent = static_cast<QKeyEvent *>(ev);//将事件转化为键盘事件
		if (pKeyEvent->key() == Qt::Key_Close ||
			pKeyEvent->key() == Qt::Key_Escape) {
			ui->groupNameEdit->setText(m_sOriginal);
			ui->groupNameEdit->clearFocus();
			ev->accept();
			return true;
		}
		else if (pKeyEvent->key() == Qt::Key_Return ||
			pKeyEvent->key() == Qt::Key_Enter) {
			ui->groupNameEdit->clearFocus();
			ev->accept();
			return true;
		}
	}
	else if (ev->type() == QEvent::FocusIn) {
		ui->groupNameEdit->selectAll();
		m_sOriginal = ui->groupNameEdit->text();
	}
	return QObject::eventFilter(obj, ev);
}