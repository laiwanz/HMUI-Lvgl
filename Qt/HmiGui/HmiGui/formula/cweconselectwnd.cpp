#include "cweconselectwnd.h"
#include <QDir>
#include <QPushButton>
#include <QBoxLayout>
#include <QListWidget>
#include "csvgbutton.h"
#include "cformulapart.h"

#ifndef HMIPREVIEW
#include "utility/utility.h"
#ifdef _LINUX_
#include "btl/btl.h"
#endif // _LINUX_
#endif // !HMIPREVIEW

CWeconSelectWnd::CWeconSelectWnd(const int &nWidth, const int &nHeight, const WndMode wndType, const QString & sPath, const int & nWndType, const QString & sInitName, const QString & sName2, const QString & sPath2, QWidget * parent)
	:QDialog(parent->parentWidget(), Qt::FramelessWindowHint)
{
    m_nCurrentPos = 0;
	m_showPath = "/";
	m_nWndType = nWndType;
	QDir dir(sPath);
	m_sUPath = dir.path();
	m_sInitPath = m_sUPath;
	bIsSDExit = false;
	if (nWndType == TypeFile) {
		this->setWindowTitle("select file");
	}
	else {
		this->setWindowTitle("select document");
	}
	auto parentptr = (CWeconTable *)parent;
	if (nullptr != parentptr) {
		m_languageType = parentptr->getLanguageType();
	}
	else {
		m_languageType = 1;
	}
    m_sImgPath = CWeconTableModel::g_sFormulaImagePath;
	m_nFontSize = nHeight * 0.03;
	if (nWidth < nHeight) {
		m_nFontSize = nWidth * 0.03;
	}
	if (m_nFontSize < 8) {
		m_nFontSize = 8;
	}
	QString sStyle = "QDialog{background-color: rgb(255, 255, 255);}";
	sStyle += "QLabel{font-size:" + QString::number(m_nFontSize) + "px;}";
	sStyle += "QPushButton{font-size:" + QString::number(m_nFontSize) + "px;}";
	sStyle += "QListWidget{font-size:" + QString::number(m_nFontSize) + "px;}";
	this->setStyleSheet(sStyle);
	m_dlgFillInFileName = new QDialog(parent->parentWidget(), Qt::FramelessWindowHint);
	m_dlgFillInFileName->setStyleSheet("background-color: rgb(226,234,245)");
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
	auto nWndWidth = m_dWidthScale * 460;
	auto nWndHeight = m_dHeightScale * 360;
	this->setFixedSize(nWndWidth, nWndHeight);
	this->move(ponint.x() + (nWidth - nWndWidth) / 2, ponint.y() + (nHeight - nWndHeight) / 2);
	// 移动新建文件夹画面 
	nWndWidth = m_dWidthScale * 300;
	nWndHeight = m_dHeightScale * 100;
	m_dlgFillInFileName->setFixedSize(nWndWidth, nWndHeight);
	m_dlgFillInFileName->move(ponint.x() + (nWidth - nWndWidth) / 2, ponint.y() + (nHeight - nWndHeight) / 2);

    m_mainLayout = new QVBoxLayout(this);
    m_headLayout = new QHBoxLayout;
	m_pathLayout = new QHBoxLayout;
    m_fileLayout = new QHBoxLayout;
    m_nameLayout = new QHBoxLayout;
    m_selectLayout = new QHBoxLayout;

    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0,0,0,0);
    m_mainLayout->setMargin(0);
    QSizePolicy sizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    // header
    m_labHead = new QLabel(this);
	QString sTitleM;
	if (ImportWnd == wndType) {
		sTitleM = "  " + CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_languageType][CFormulaPart::Import];
	} 
	else if (ExportWnd == wndType) {
		sTitleM = "  " + CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_languageType][CFormulaPart::Export];
	}
    m_labHead->setText(sTitleM);
    m_labHead->setStyleSheet("background-color: rgb(226,234,245)");
	m_labHead->setMinimumSize(1,1);
    m_headLayout->addWidget(m_labHead);
    m_headLayout->setSpacing(0);
    m_mainLayout->addLayout(m_headLayout);
    // spacer
    QSpacerItem * spacer = new QSpacerItem(1,1,QSizePolicy::Expanding);
    m_mainLayout->addSpacerItem(spacer);
    // path
    spacer = new QSpacerItem(1,1,QSizePolicy::Expanding);
    m_pathLayout->addSpacerItem(spacer);
    m_widgetPath = new QWidget();
    m_widgetPath->setStyleSheet("background-color: rgb(239,244,248)");
    m_pathShowLayout = new QHBoxLayout(m_widgetPath);
    m_labPathImg = new QLabel(this);
    m_labPathImg->setPixmap(QPixmap(QString::fromStdString(m_sImgPath + "/filePathIcon.png")));
	m_labPathImg->setMinimumSize(15,15);
    m_labPathImg->setScaledContents(true);
    m_pathShowLayout->addWidget(m_labPathImg);
    m_labPath = new QLabel(this);
    m_labPath->setText(m_showPath);
	m_labPath->setMinimumSize(1, 15);
    m_pathShowLayout->addWidget(m_labPath);
    m_pathShowLayout->setStretch(0, 1);
    m_pathShowLayout->setStretch(1, 12);
	m_pathShowLayout->setContentsMargins(0,0,0,0);
    m_pathLayout->addWidget(m_widgetPath);
	m_widgetPath->setMinimumSize(1,1);
    spacer = new QSpacerItem(1,1,QSizePolicy::Expanding);
    m_pathLayout->addSpacerItem(spacer);
	QIcon ico;
    m_btnStandBack = new QPushButton(this);
    sizePolicy.setHeightForWidth(m_btnStandBack->sizePolicy().hasHeightForWidth());
    m_btnStandBack->setSizePolicy(sizePolicy);
	ico.addPixmap(QPixmap(QString::fromStdString(m_sImgPath + "/standBack.png")));
	m_btnStandBack->setIcon(ico);
    m_btnStandBack->setStyleSheet("border:none");
    m_btnStandBack->setEnabled(false);
	m_btnStandBack->setIconSize(QSize(30 * m_dHeightScale, 30 * m_dHeightScale));
	m_btnStandBack->setMinimumSize(30 * m_dHeightScale, 30 * m_dHeightScale);
    m_pathLayout->addWidget(m_btnStandBack);
    m_btnForward = new QPushButton(this);
    sizePolicy.setHeightForWidth(m_btnForward->sizePolicy().hasHeightForWidth());
    m_btnForward->setSizePolicy(sizePolicy);
	ico.addPixmap(QPixmap(QString::fromStdString(m_sImgPath + "/forward.png")));
	m_btnForward->setIcon(ico);
    m_btnForward->setStyleSheet("border:none");
    m_btnForward->setEnabled(false);
	m_btnForward->setIconSize(QSize(30 * m_dHeightScale, 30 * m_dHeightScale));
	m_btnForward->setMinimumSize(30 * m_dHeightScale, 30 * m_dHeightScale);
    m_pathLayout->addWidget(m_btnForward);
    spacer = new QSpacerItem(1,1,QSizePolicy::Expanding);
    m_pathLayout->addSpacerItem(spacer);
    m_btnCreate = new CSvgButton(QString::fromStdString(CWeconTableModel::g_sFormulaImagePath + "/create.svg"), this);
    m_btnCreate->setStyleSheet("border:none");
    m_pathLayout->addWidget(m_btnCreate);
	m_btnCreate->setMinimumSize(60 * m_dHeightScale, 30 * m_dHeightScale);
    spacer = new QSpacerItem(1,1,QSizePolicy::Expanding);
    m_pathLayout->addSpacerItem(spacer);
    m_pathLayout->setStretch(0,1);
    m_pathLayout->setStretch(1,16);
    m_pathLayout->setStretch(2,1);
    m_pathLayout->setStretch(3,2);
    m_pathLayout->setStretch(4,2);
    m_pathLayout->setStretch(5,1);
    m_pathLayout->setStretch(6,6);
    m_pathLayout->setStretch(7,1);
    m_mainLayout->addLayout(m_pathLayout);
    // spacer
    spacer = new QSpacerItem(1,1,QSizePolicy::Expanding);
    m_mainLayout->addSpacerItem(spacer);
    // file
    m_fileCutLayout = new QVBoxLayout;
    m_btnUDisk = new QPushButton();
    sizePolicy.setHeightForWidth(m_btnUDisk->sizePolicy().hasHeightForWidth());
    m_btnUDisk->setSizePolicy(sizePolicy);
    m_btnUDisk->setStyleSheet("background-color: rgb(239,244,248); border:none");
    m_uDiskLayout = new QHBoxLayout(m_btnUDisk);
    m_labDiskImg = new QLabel(this);
    if(sInitName == "USB FLASH"){
        m_labDiskImg->setPixmap(QPixmap(QString::fromStdString(m_sImgPath + "/UDisk.png")));
    }
    else{
        m_labDiskImg->setPixmap(QPixmap(QString::fromStdString(m_sImgPath + "/SDCard.png")));
    }
    m_labDiskImg->setScaledContents(true);
    m_uDiskLayout->addWidget(m_labDiskImg);
    m_labDiskText = new QLabel(this);
    m_labDiskText->setText(sInitName);
    m_uDiskLayout->addWidget(m_labDiskText);
    m_uDiskLayout->setStretch(0,3);
    m_uDiskLayout->setStretch(1,8);
	m_btnUDisk->setMinimumSize(1,1);
    m_fileCutLayout->addWidget(m_btnUDisk);
    if(sPath2 != ""){
        m_btnSDCard = new QPushButton();
        sizePolicy.setHeightForWidth(m_btnSDCard->sizePolicy().hasHeightForWidth());
        m_btnSDCard->setSizePolicy(sizePolicy);
        m_btnSDCard->setStyleSheet("background-color: rgb(255,255,255); border:none");
        m_SDLayout = new QHBoxLayout(m_btnSDCard);
        m_labSDImg = new QLabel(this);
        m_labSDImg->setPixmap(QPixmap(QString::fromStdString(m_sImgPath + "/SDCard.png")));
        m_labSDImg->setScaledContents(true);
        m_SDLayout->addWidget(m_labSDImg);
        m_labSDText = new QLabel(this);
        m_labSDText->setText(sName2);
        m_SDLayout->addWidget(m_labSDText);
        m_SDLayout->setStretch(0,3);
        m_SDLayout->setStretch(1,8);
		m_btnSDCard->setMinimumSize(1, 1);
        m_fileCutLayout->addWidget(m_btnSDCard);
    }
    spacer = new QSpacerItem(1, 1 * m_dHeightScale, QSizePolicy::Expanding);
    m_fileCutLayout->addSpacerItem(spacer);
    if(sPath2 != ""){
        m_fileCutLayout->setStretch(0,1);
        m_fileCutLayout->setStretch(1,1);
        m_fileCutLayout->setStretch(2,2);
    }
    else{
        m_fileCutLayout->setStretch(0,1);
        m_fileCutLayout->setStretch(1,3);
    }
    m_fileLayout->addLayout(m_fileCutLayout);
    m_list = new QListWidget(this);
	sStyle = "QListWidget{border:0px}QListWidget::item{border-bottom: 1px solid rgb(239,244,248);height:" + QString::number(30 * m_dHeightScale) + "px;}QListWidget::item:selected{color:black;background-color:rbg(204,232,255);}";
	m_list->setStyleSheet(sStyle);
    m_fileLayout->addWidget(m_list);
	m_list->setMinimumSize(1,1);
	m_list->setIconSize(QSize(20 * m_dHeightScale, 20 * m_dHeightScale));
    spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
    m_fileLayout->addSpacerItem(spacer);
    m_fileLayout->setStretch(0,9);
    m_fileLayout->setStretch(1,28);
    m_fileLayout->setStretch(2,1);
    m_mainLayout->addLayout(m_fileLayout);
    // spacer
    spacer = new QSpacerItem(1,1,QSizePolicy::Expanding);
    m_mainLayout->addSpacerItem(spacer);
    // name
    spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
    m_nameLayout->addSpacerItem(spacer);
    m_labFileName = new QLabel(this);
    m_labFileName->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_languageType][CFormulaPart::Folder]);
    m_nameLayout->addWidget(m_labFileName);
	m_labFileName->setMinimumHeight(1);
    m_labFileText= new QLabel(this);
    m_labFileText->setStyleSheet("background-color: rgb(239,244,248)");
	m_labFileText->setMinimumSize(1,1);
    m_nameLayout->addWidget(m_labFileText);
    spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
    m_nameLayout->addSpacerItem(spacer);
    m_nameLayout->setStretch(0,9);
    m_nameLayout->setStretch(1,5);
    m_nameLayout->setStretch(2,22);
    m_nameLayout->setStretch(3,1);
    m_mainLayout->addLayout(m_nameLayout);
    // spacer
    spacer = new QSpacerItem(1,1,QSizePolicy::Expanding);
    m_mainLayout->addSpacerItem(spacer);
    // select
	spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
    m_selectLayout->addSpacerItem(spacer);
    m_btnClose = new QPushButton(this);
    sizePolicy.setHeightForWidth(m_btnClose->sizePolicy().hasHeightForWidth());
    m_btnClose->setSizePolicy(sizePolicy);
    m_btnClose->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_languageType][CFormulaPart::Cancel]);
    m_btnClose->setStyleSheet("background-color: rgb(255,255,255); ");
    m_selectLayout->addWidget(m_btnClose);
	m_btnClose->setMinimumSize(30,15);
    spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
    m_selectLayout->addSpacerItem(spacer);
    m_btnSelect = new QPushButton(this);
	m_btnSelect->setMinimumSize(30, 15);
    sizePolicy.setHeightForWidth(m_btnSelect->sizePolicy().hasHeightForWidth());
    m_btnSelect->setSizePolicy(sizePolicy);
    m_btnSelect->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_languageType][CFormulaPart::Determine]);
    m_btnSelect->setStyleSheet("background-color: rgb(9,100,183);");
    m_btnSelect->setEnabled(false);
    m_selectLayout->addWidget(m_btnSelect);
    spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
    m_selectLayout->addSpacerItem(spacer);
    m_selectLayout->setStretch(0,33);
    m_selectLayout->setStretch(1,10);
    m_selectLayout->setStretch(2,1);
    m_selectLayout->setStretch(3,10);
    m_selectLayout->setStretch(4,3);
    m_mainLayout->addLayout(m_selectLayout);
    // spacer
    spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
    m_mainLayout->addSpacerItem(spacer);
    m_mainLayout->setStretch(0,9);
    m_mainLayout->setStretch(1,1);
    m_mainLayout->setStretch(2,12);
    m_mainLayout->setStretch(3,1);
    m_mainLayout->setStretch(4,52);
    m_mainLayout->setStretch(5,1);
    m_mainLayout->setStretch(6,6);
    m_mainLayout->setStretch(7,2);
    m_mainLayout->setStretch(8,6);
    m_mainLayout->setStretch(9,2);

    m_listFilePathSelete.push_back(m_labPath->text().toStdString());
    m_nCurrentPos ++;
    init();
    connect(m_btnCreate, &QPushButton::clicked, this, &CWeconSelectWnd::OnclickCreateFile);
    connect(m_btnStandBack, &QPushButton::clicked, this, &CWeconSelectWnd::OnClickPathStandBack);
    connect(m_btnForward, &QPushButton::clicked, this, &CWeconSelectWnd::OnClickPathForward);
    if(sPath2 != ""){
		bIsSDExit = true;
        connect(m_btnSDCard, &QPushButton::clicked, this, &CWeconSelectWnd::OnClickSDCardCheck);
    }
    connect(m_btnUDisk, &QPushButton::clicked, this, &CWeconSelectWnd::OnClickUDiskCheck);
	connect(m_btnClose, &QPushButton::clicked, this, &CWeconSelectWnd::OnClickCloseWndBtn);
	connect(m_btnSelect, &QPushButton::clicked, this, &CWeconSelectWnd::OnClickSelectBtn);
	connect(m_list, &QListWidget::itemClicked, this, &CWeconSelectWnd::OnClickListItem);
    connect(m_list, &QListWidget::itemDoubleClicked, this, &CWeconSelectWnd::OnDbClickListItem);
	// 新建文件夹
	QVBoxLayout *layout = new QVBoxLayout(m_dlgFillInFileName);
	QHBoxLayout *laTillte = new QHBoxLayout();
	QLabel* label = new QLabel();
	label->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_languageType][CFormulaPart::CreateFolder]);
	label->setFont(QFont("simsun", m_nFontSize));
	CSvgButton * btnClose = new CSvgButton(QString::fromStdString(CWeconTableModel::g_sFormulaImagePath + "/closeWnd.svg"), this);
	btnClose->setStyleSheet("QPushButton{border:none}");
	btnClose->setMinimumHeight(30 * m_dHeightScale);
	laTillte->addWidget(label);
	laTillte->addWidget(btnClose);
	laTillte->setStretch(0, 6);
	laTillte->setStretch(1, 1);
	m_lineFileName = new QLineEdit();
	m_lineFileName->setMinimumHeight(30 * m_dHeightScale);
	m_lineFileName->setFont(QFont("simsun", m_nFontSize));
	connect(m_lineFileName, &QLineEdit::returnPressed, this, &CWeconSelectWnd::OnClickAddNewFile);
	layout->addLayout(laTillte);
	layout->addWidget(m_lineFileName);
	layout->setStretch(0, 1);
	layout->setStretch(2, 3);
	connect(btnClose, &QPushButton::clicked, this, &CWeconSelectWnd::OnClickCancelAddFileName);
}

void CWeconSelectWnd::init()
{
	QString sCurPath = m_labPath->text();
	QStringList items;
	if ("/" != sCurPath) {
	
		items.push_back("..");
	}
	sCurPath = m_sInitPath + sCurPath;
	if (m_nWndType == TypeFile) {
	
		getAllFiles(sCurPath, items);
	}
	else {
	
		getAllDocuments(sCurPath, items);
		m_btnSelect->setEnabled(true);
	}

	m_list->clear();

	for (int i = 0; i < items.size(); i++)
	{
		QFileInfo  fileInfo(sCurPath + "/" + items[i]);

        QListWidgetItem *ite = new QListWidgetItem;
        ite->setText(items[i]);
		QIcon ico;
		if (!fileInfo.isDir()) {
			ico.addPixmap(QPixmap(QString::fromStdString(m_sImgPath + "/file.png")));
		}
		else {
			ico.addPixmap(QPixmap(QString::fromStdString(m_sImgPath + "/folder.png")));
		}
		ite->setIcon(ico);
        m_list->addItem(ite);
	}
}

void CWeconSelectWnd::OnClickCloseWndBtn()
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
	this->close();
}

void CWeconSelectWnd::OnClickSelectBtn()
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
	QString sText = "";
	auto item = m_list->currentItem();
	if (nullptr == item) {
	
		if (m_nWndType == TypeFile) {
		
            CWeconTable::showMsgWnd("warning", "please select csv file", QMessageBox::NoButton, this);
			return;
		}	
	}
	else {
		sText = item->text();
	}
	QString sPath = m_sInitPath + m_labPath->text() + sText;
	if (sPath != "") {
	
		OnSelectResult(sPath);
		this->close();
	}
	return;
}

void CWeconSelectWnd::OnClickListItem(QListWidgetItem * item)
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
	if (nullptr != item) {
	
		QString sText = item->text();
		if (item == m_lastSelectItem) {

			OnDbClickListItem(item);
			m_lastSelectItem = nullptr;
			return;
		}
		else {

			m_lastSelectItem = item;
		}
		if ("" != sText && ".." != sText) {
		
			QString sPath = m_labPath->text();
            m_labFileText->setText(sText);
			QFileInfo  fileInfo(m_sInitPath + sPath + "/" + sText);
			
			if ((fileInfo.isFile() && m_nWndType == TypeFile) || (fileInfo.isDir() && m_nWndType == TypeDocument)) {
			
				m_btnSelect->setEnabled(true);
				return;
			}
		}
	}
	m_btnSelect->setEnabled(false);
}

void CWeconSelectWnd::OnDbClickListItem(QListWidgetItem * item)
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
	if (nullptr != item) {
	
		QString sPath = m_labPath->text();
		QString sText = item->text();
		QFileInfo  fileInfo(m_sInitPath + sPath + "/" + sText);
		if (!fileInfo.isDir()) {
		
			return;
		}
		if (".." == sText) {
		
			
			QDir dir(m_sInitPath + sPath);
			dir.cdUp();
			sPath = dir.path();
			sPath = sPath.mid(m_sInitPath.size(), sPath.size() - m_sInitPath.size()) + "/";
			m_labPath->setText(sPath);
			init();
		} else if ("" != sText) {

			m_labPath->setText(sPath + sText + "/");
			init();
        }
        if(m_listFilePathSelete.size() > 0){
            // 删除当前选中的后面几位
            auto first = m_listFilePathSelete.begin();
            for(int i = 1; i < m_nCurrentPos; i++){
                ++first;
            }
            auto last = m_listFilePathSelete.end();
            --last;
            m_listFilePathSelete.erase(first, last);
        }
        //添加选中的
        m_listFilePathSelete.push_back(m_labPath->text().toStdString());
        m_btnStandBack->setEnabled(true);
        m_btnForward->setEnabled(false);
        m_nCurrentPos ++;

	}
}

void CWeconSelectWnd::OnClickUDiskCheck()
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
    m_btnUDisk->setStyleSheet("background-color: rgb(239,244,248); border:none");
	if (bIsSDExit) {
		m_btnSDCard->setStyleSheet("background-color: rgb(255,255,255); border:none");
	}
	m_sInitPath = m_sUPath;
	m_labPath->setText("/");
	init();
}

void CWeconSelectWnd::OnClickSDCardCheck()
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
    m_btnSDCard->setStyleSheet("background-color: rgb(239,244,248); border:none");
    m_btnUDisk->setStyleSheet("background-color: rgb(255,255,255); border:none");
	m_sInitPath = m_sSdPath;
	m_labPath->setText("/");
	init();
}

int CWeconSelectWnd::getAllFiles(const QString & sPath, QStringList & files)
{
	QDir dir(sPath);

	dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
	dir.setSorting(QDir::DirsFirst);

	QFileInfoList list = dir.entryInfoList();
	for (int i = 0; i < list.size(); i++)
	{
		if ((list[i].isFile() && list[i].fileName().right(4).toLower() == ".csv") || list[i].isDir()) {
		
			files.push_back(list[i].fileName());
		}
		
	}
	return 0;
}

int CWeconSelectWnd::getAllDocuments(const QString & sPath, QStringList & documents)
{
	QDir dir(sPath);

	dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	QFileInfoList list = dir.entryInfoList();
	for (int i = 0; i < list.size(); i++)
	{
		if (list[i].isDir()) {
		
			documents.push_back(list[i].fileName());
		}
	}
	return 0;
}

void CWeconSelectWnd::OnClickPathForward(){
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
    m_nCurrentPos++;
    if (m_nCurrentPos == (int)m_listFilePathSelete.size() - 1){
        m_btnForward->setEnabled(false);
    }
    if (m_nCurrentPos != 0){
        m_btnStandBack->setEnabled(true);
    }
    auto first = m_listFilePathSelete.begin();
    for(int i = 1; i < m_nCurrentPos; i++){
        ++first;
    }
    std::string sText = *first;
    m_labPath->setText(QString::fromStdString(sText));
    init();
}

void CWeconSelectWnd::OnClickPathStandBack(){
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
    m_btnForward->setEnabled(true);
    m_nCurrentPos --;
    if (m_nCurrentPos == 0){
        m_btnStandBack->setEnabled(false);
    }
    auto first = m_listFilePathSelete.begin();
    for(int i = 1; i < m_nCurrentPos; i++){
        ++first;
    }
    std::string sText = *first;
    m_labPath->setText(QString::fromStdString(sText));
    init();
}

void CWeconSelectWnd::OnclickCreateFile(){
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
	m_dlgFillInFileName->setWindowModality(Qt::WindowModal);
	m_dlgFillInFileName->raise();
    m_dlgFillInFileName->show();
}

void CWeconSelectWnd::OnClickAddNewFile(){
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
	bool bIsTrue = false;
    m_dlgFillInFileName->accept();
    QString sPath = m_labPath->text();
    QDir dir(m_sInitPath + sPath);
	if ("" == m_lineFileName->text()) {
		CWeconTable::showMsgWnd("warning", "NULL  !", QMessageBox::NoButton, this);
		return;
	}
	bIsTrue = dir.mkdir(m_lineFileName->text());
	if (bIsTrue) {
		QListWidgetItem *ite = new QListWidgetItem;
		ite->setText(m_lineFileName->text());
		QIcon ico;
		ico.addPixmap(QPixmap(QString::fromStdString(m_sImgPath + "/folder.png")));
		ite->setIcon(ico);
		m_list->addItem(ite);
		UTILITY_NAMESPACE::CDir::DirSync((m_sInitPath + sPath).toStdString());
	}
	else {
		CWeconTable::showMsgWnd("warning", CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_languageType][CFormulaPart::CreateFailed], QMessageBox::NoButton, this);
	}
	m_lineFileName->clear();
}

void CWeconSelectWnd::OnClickCancelAddFileName(){
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
    m_dlgFillInFileName->accept();
}
