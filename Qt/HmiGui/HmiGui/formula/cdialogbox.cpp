#include "cdialogbox.h"
#include "cformulapart.h"

CDialogBox::CDialogBox(const int& nWidth, const int& nHeight, QWidget *parent) : QDialog(parent->parentWidget(), Qt::FramelessWindowHint) {
	
	this->setAttribute(Qt::WA_TranslucentBackground);
	auto parentptr = (CWeconTable *)parent;
	if (nullptr != parentptr) {
		m_languageType = parentptr->getLanguageType();
	}
	else {
		m_languageType = 1;
	}
	QPoint ponint = QPoint(0, 0);
#ifndef _LINUX_
	ponint = parent->parentWidget()->mapToGlobal(QPoint(0, 0));
#endif // !_LINUX_

	// 计算长宽缩放比
	if (nWidth > nHeight) {
		m_dWidthScale = (double)nWidth / 800;
		m_dHeightScale = (double)nHeight / 480;
	}
	else {
		m_dWidthScale = (double)nWidth / 480;
		m_dHeightScale = (double)nHeight / 800;
	}
	// 移动当前画面 
	auto nWndWidth = m_dWidthScale * 300;
	auto nWndHeight = m_dHeightScale * 200;
	this->setFixedSize(nWndWidth, nWndHeight);
	this->move(ponint.x() + (nWidth - nWndWidth) / 2, ponint.y() + (nHeight - nWndHeight) / 2);
	m_nFontSize = nHeight * 0.03;
	if (nWidth < nHeight) {
		m_nFontSize = nWidth * 0.03;
	}
	if (m_nFontSize < 8) {
		m_nFontSize = 8;
	}
}

int CDialogBox::config(const std::string& sTitle, const std::string& sNote) {
	initState();
	initWarn(sTitle, sNote);
	return 0;
}

void CDialogBox::initState(){
   
    this->setStyleSheet("background-color:rgb(255,255,255);");
	m_layoutMain = new QHBoxLayout(this);
	m_layoutMain->setContentsMargins(0, 0, 0, 0);
	m_frameMain = new QFrame;
	m_frameMain->setStyleSheet("QFrame{border-radius:5px; border: 1px solid #828790;}");
	QString sStyle = "QLabel{font-size:" + QString::number(m_nFontSize) + "px;}";
	sStyle += "QPushButton{font-size:" + QString::number(m_nFontSize) + "px;}";
	m_frameMain->setStyleSheet(sStyle);
	m_layoutMain->addWidget(m_frameMain);
    m_labNote = new QLabel(this);
    m_labTitle = new QLabel(this);
    m_btnFork = new CSvgButton(QString::fromStdString(CWeconTableModel::g_sFormulaImagePath + "/closeWnd.svg"), this);
    m_btnSure = new QPushButton(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_languageType][CFormulaPart::Determine],this);
    m_btnCancle = new QPushButton(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_languageType][CFormulaPart::Cancel],this);
    connect(m_btnCancle,&QPushButton::clicked,this,&CDialogBox::dealbtnCancelClicked);
    connect(m_btnSure,&QPushButton::clicked,this,&CDialogBox::dealbtnSureClicked);
    connect(m_btnFork,&QPushButton::clicked,this,&CDialogBox::dealbtnForkClicked);
}

void CDialogBox::initWarn(const std::string &sTitle, const std::string &sNote){

	QSizePolicy sizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	m_layoutFrame = new QVBoxLayout(m_frameMain);
	m_layoutFrame->setContentsMargins(0, 0, 0, 0);
	QWidget * widget = new QWidget();
	widget->setStyleSheet("QWidget{background-color: rgb(226,234,245);border-top-left-radius: 5px; border-top-right-radius: 5px; }");
	QHBoxLayout* layoutTitle = new QHBoxLayout(widget);
	QSpacerItem * spacer;
	QString sTitleM = "  " + QString::fromStdString(sTitle);
    m_labTitle->setText(sTitleM);
	m_labTitle->setStyleSheet("color:black; border:none;");
	layoutTitle->addWidget(m_labTitle);
	m_btnFork->setStyleSheet("QPushButton{border:none;}");
	m_btnFork->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	layoutTitle->addWidget(m_btnFork);
	layoutTitle->setStretch(0,40);
	layoutTitle->setStretch(1,6);
	m_layoutFrame->addWidget(widget);
	spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
	m_layoutFrame->addSpacerItem(spacer);
	QHBoxLayout* layoutNote = new QHBoxLayout;
	spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
	layoutNote->addSpacerItem(spacer);
    m_labNote->setStyleSheet("color:black; border:none;");
    m_labNote->setAlignment(Qt::AlignCenter);
    m_labNote->setText(QString::fromStdString(sNote));
	layoutNote->addWidget(m_labNote);
	spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
	layoutNote->addSpacerItem(spacer);
	layoutNote->setStretch(0,1);
	layoutNote->setStretch(1,15);
	layoutNote->setStretch(2,1);
	m_layoutFrame->addLayout(layoutNote);
	spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
	m_layoutFrame->addSpacerItem(spacer);
	QHBoxLayout* layoutBtn = new QHBoxLayout;
	spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
	layoutBtn->addSpacerItem(spacer);
    m_btnSure->setStyleSheet("QPushButton{border:1px groove gray;border-radius:4px;border-color: rgb(139,159,185);}"
                           "QPushButton:hover{background-color: rgb(119,146,183);}"
                           "QPushButton:pressed{background-color: rgb(139,159,185);}");
	sizePolicy.setHeightForWidth(m_btnSure->sizePolicy().hasHeightForWidth());
	m_btnSure->setSizePolicy(sizePolicy);
	layoutBtn->addWidget(m_btnSure);
	spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
	layoutBtn->addSpacerItem(spacer);
    m_btnCancle->setStyleSheet("QPushButton{border:1px groove gray;border-radius:4px;border-color: rgb(139,159,185);}"
                             "QPushButton:hover{background-color: rgb(119,146,183);}"
                            "QPushButton:pressed{background-color: rgb(139,159,185);}");
	sizePolicy.setHeightForWidth(m_btnCancle->sizePolicy().hasHeightForWidth());
	m_btnCancle->setSizePolicy(sizePolicy);
	layoutBtn->addWidget(m_btnCancle);
	spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
	layoutBtn->addSpacerItem(spacer);
	layoutBtn->setStretch(0,3);
	layoutBtn->setStretch(1,9);
	layoutBtn->setStretch(2,3);
	layoutBtn->setStretch(3,9);
	layoutBtn->setStretch(4,3);
	m_layoutFrame->addLayout(layoutBtn);
	spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
	m_layoutFrame->addSpacerItem(spacer);
	m_layoutFrame->setStretch(0,6);
	m_layoutFrame->setStretch(1,3);
	m_layoutFrame->setStretch(2,5);
	m_layoutFrame->setStretch(3,3);
	m_layoutFrame->setStretch(4,4);
	m_layoutFrame->setStretch(5,3);
}


void CDialogBox::dealbtnSureClicked(){
    this->accept();
}

void CDialogBox::dealbtnCancelClicked(){
    this->reject();
}

void CDialogBox::dealbtnForkClicked(){
    this->reject();
}