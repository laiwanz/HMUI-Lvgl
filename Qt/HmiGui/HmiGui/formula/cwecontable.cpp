#include "cwecontable.h"
#include <QScrollBar>
#include <QLayout>
#include <QComboBox>
#include <QTableView>
#include <QDebug>
#include "csvgbutton.h"
#include "cchangepageedit.h"
#include "cwecontablemodel.h"
#include "ctoastdialog.h"
#include "HmiBase/HmiBase.h"
#include "./cwecontabledelegate.h"
#ifdef _LINUX_
#include "btl/btl.h"
#endif // _LINUX_

int CWeconTable::m_nWidth = 800;
int CWeconTable::m_nHeight = 480;

CWeconTable::CWeconTable(QWidget *parent)
{
	getScaleWH();
}

CWeconTable::CWeconTable(TableSpace::TABLESTYLE & style, QWidget *parent) : QWidget(parent)
{
	init(style);
	getScaleWH();
}

int CWeconTable::init(TableSpace::TABLESTYLE &style)
{
	m_languageType = ChineseLanguage;
	static bool initFormulaStatic = false;
	if (style.m_nPrjHeight != 0 && style.m_nPrjWidth != 0) {
		m_nWidth = style.m_nPrjWidth;
		m_nHeight = style.m_nPrjHeight;
	}
	int nFontSize = m_nHeight * 0.03;
	if (m_nWidth < m_nHeight) {

		nFontSize = m_nWidth * 0.03;
	}
	if (nFontSize < 8) {
		nFontSize = 8;
	}
	style.m_nTableFontSize = nFontSize; // 临时方案
	double dWidthScale, dHeightScale;
	if (m_nWidth > m_nHeight) {
		dWidthScale = (double)m_nWidth / 800;
		dHeightScale = (double)m_nHeight / 480;
	}
	else {
		dWidthScale = (double)m_nWidth / 480;
		dHeightScale = (double)m_nHeight / 800;
	}
	if (!initFormulaStatic) {

		initLanguageInfo();
		initFormulaStatic = true;
	}
	
	m_bIsDisablesCellEditing = style.m_bIsDisablesCellEditing;
	m_nPositionId = -1;
	QSpacerItem * tempSpaceItem = nullptr;
	CSvgButton * tempBtn = nullptr;
	QString sSvgName;
	QString sStyle;
	m_bIsExpand = true;
	m_nVHeadWidth = 0;

	if (style.m_sBackgroundImage != "") {

		setAutoFillBackground(true);
		QPalette pal = this->palette();
		pal.setBrush(QPalette::Background, QBrush(QPixmap(QString::fromStdString(style.m_sBackgroundImage))));
		setPalette(pal);
		sStyle = "QLabel{font-size:" + QString::number(nFontSize) + "px;}";
		sStyle += "QComboBox{font-size:" + QString::number(nFontSize) + "px;}";
		this->setStyleSheet(sStyle);
	}
	else {

		setAttribute(Qt::WA_StyledBackground);
		this->setProperty("name", "table");
		sStyle = "QWidget[name=\"table\"]{background:" + IntToRGB(style.m_nBackgroundColor, style.m_nTransparent) + ";border:1px solid block;}";
		sStyle += "QLabel{font-size:" + QString::number(nFontSize) + "px;}";
		sStyle += "QComboBox{font-size:" + QString::number(nFontSize) + "px;}";
		this->setStyleSheet(sStyle);
	}

	if (style.m_sImgPath[style.m_sImgPath.size() - 1] != '\\') {
		m_svgPath = QString::fromStdString(style.m_sImgPath) + "/";
	}
	else if (style.m_sImgPath[style.m_sImgPath.size() - 1] != '/') {
		m_svgPath = QString::fromStdString(style.m_sImgPath.substr(0, style.m_sImgPath.size() - 1)) + "/";
	}
	else {
		m_svgPath = QString::fromStdString(style.m_sImgPath);
	}

	this->setGeometry(style.m_nLeft, style.m_nTop, style.m_nWidth, style.m_nHeight);

	//new table
	m_table = new QTableView(this);
	m_model = new CWeconTableModel(m_table);
	m_model->setFontSize(style.m_nTableFontSize);
	m_model->setDisablesCellEditing(style.m_bIsDisablesCellEditing);
	m_table->setModel(m_model);

	auto tableDelegate = new CWeconTableDelegate(this);
	m_table->setItemDelegate(tableDelegate);
	m_table->setEditTriggers(QTableView::DoubleClicked);
	m_table->setSortingEnabled(false);
	connect(tableDelegate, &QAbstractItemDelegate::closeEditor, this, &CWeconTable::CellCloseEdit);

	m_table->verticalHeader()->setDefaultSectionSize(30 * dHeightScale);  // 临时方案

	m_table->setSelectionBehavior(QTableView::SelectItems);
	m_table->setSelectionMode(QTableView::SingleSelection);

	m_table->horizontalHeader()->setStyleSheet("QHeaderView::section{background:" + IntToRGB(style.m_nHTableHeadColor, style.m_nTransparent) + ";color:" + Int16ToRgbStr(style.m_nHHeadTextColor) + "}");
	m_table->verticalHeader()->setStyleSheet("QHeaderView::section{background:" + IntToRGB(style.m_nVTableHeadColor, style.m_nTransparent) + ";color:" + Int16ToRgbStr(style.m_nVHeadTextColor) + "}");
	connect(m_table, &QTableView::doubleClicked, this, &CWeconTable::DoubleClickCell);
	m_table->setStyleSheet("background-color:rgb(255,255,255,0);gridline-color: " + IntToRGB(style.m_nLineColor, style.m_nTransparent) + ";selection-background-color:" + Int16ToRgbStr(style.m_nSelectColor) + ";");

	//set scrollbar style
	sStyle = "QScrollBar:vertical{width:" + QString::number(30 * dWidthScale) + "px;padding-top:" + QString::number(20 * dWidthScale) + "px;padding-bottom:" + QString::number(20 * dWidthScale) + "px;}";
	sStyle += "QScrollBar::sub-line:vertical{subcontrol-position:top;height:" + QString::number(20 * dWidthScale) + "px;}";
	sStyle += "QScrollBar::add-line:vertical{subcontrol-position:bottom;height:" + QString::number(20 * dWidthScale) + "px;}";
	sStyle += "QScrollBar::handle:vertical{ min-height: " + QString::number(40 * dWidthScale) + "px;}";
	m_table->verticalScrollBar()->setStyleSheet(sStyle);
	sStyle = "QScrollBar:horizontal{height:" + QString::number(30 * dWidthScale) + "px;padding-left:" + QString::number(20 * dWidthScale) + "px;padding-right:" + QString::number(20 * dWidthScale) + "px;}";
	sStyle += "QScrollBar::sub-line:horizontal{subcontrol-position:left;width:" + QString::number(20 * dWidthScale) + "px;}";
	sStyle += "QScrollBar::add-line:horizontal{subcontrol-position:right;width:" + QString::number(20 * dWidthScale) + "px;}";
	sStyle += "QScrollBar::handle:horizontal{ min-width: " + QString::number(40 * dWidthScale) + "px;}";
	m_table->horizontalScrollBar()->setStyleSheet(sStyle);

	//set let top cell can`t click and background
	m_table->setCornerButtonEnabled(false);
	QAbstractButton * leftTopBtn = m_table->findChild<QAbstractButton *>();
	if (nullptr != leftTopBtn) {
		leftTopBtn->setStyleSheet("QAbstractButton::section{background-color:" + IntToRGB(style.m_nVTableHeadColor, style.m_nTransparent) + ";}");
	}

	//init lock part
	m_lockImg = new QLabel(this);
	m_lockImg->setScaledContents(true);
	m_lockImg->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_lockImg->hide();

	//init loack rect
	auto nVHeadHeight = m_table->horizontalHeader()->rect().height();
	m_lockImg->setGeometry(0, 0, 10, nVHeadHeight);

	connect(m_table->verticalHeader(), &QHeaderView::sectionClicked, this, &CWeconTable::OnClickVHead);

	m_model->setEditFlag(false);
	m_model->setCellBgColor1(Int16ToRgbStr(style.m_nRowBgColor1), style.m_nTransparent);
	m_model->setCellBgColor2(Int16ToRgbStr(style.m_nRowBgColor2), style.m_nTransparent);
	m_model->setCellTextColor(Int16ToRgbStr(style.m_nTableTextColor));

	if (style.m_nCellHeight != 0) {
		m_table->verticalHeader()->setDefaultSectionSize(style.m_nCellHeight);
	}

	if (style.m_nCellWidth != 0) {
		m_table->horizontalHeader()->setDefaultSectionSize(style.m_nCellWidth);
	}
	m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	m_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	//init layout
	m_mainLayout = new QVBoxLayout(this);
	m_headLayout = new QHBoxLayout;
	m_bottomLayout = new QHBoxLayout;

	m_mainLayout->addLayout(m_headLayout);
	m_mainLayout->addWidget(m_table);
	m_mainLayout->addLayout(m_bottomLayout);

	//new head and function button
	m_tableHeadName = new QLabel(this);
	m_tableHeadName->setText(QString::fromStdString(style.m_sName));
	m_tableHeadName->setToolTip(m_tableHeadName->text());
	m_tableHeadName->setStyleSheet("color:" + Int16ToRgbStr(style.m_nNameColor));
	m_headLayout->addWidget(m_tableHeadName);
	tempSpaceItem = new QSpacerItem(10, 10, QSizePolicy::Expanding);
	m_headLayout->addSpacerItem(tempSpaceItem);
	//search button
	sSvgName = m_svgPath + getSgvNameByFuncType(TableSpace::Table_Search);
	m_searchBtn = new CSvgButton(sSvgName, this);
	m_searchBtn->setFixedHeight(30 * dHeightScale);
	m_searchBtn->setFixedWidth(30 * dHeightScale);
	m_headLayout->addWidget(m_searchBtn);
	connect(this->m_searchBtn, &QPushButton::clicked, this, &CWeconTable::OnClickSearchBtn);

	QHBoxLayout * btnLayout = new QHBoxLayout;
	btnLayout->setSpacing(0);
	for (size_t i = 0; i < style.m_funcs.size(); ++i) {

		TableSpace::funcType nFuncCode = (TableSpace::funcType)style.m_funcs[i].m_nCode;
		sSvgName = m_svgPath + getSgvNameByFuncType(nFuncCode);
		tempBtn = new CSvgButton(sSvgName, this);
		tempBtn->setFixedHeight(30 * dHeightScale);
		tempBtn->setFixedWidth(30 * dHeightScale);
		btnLayout->addWidget(tempBtn);
		initFuncBtnConnect(tempBtn, nFuncCode);
		m_functionBtn.insert(nFuncCode, tempBtn);
	}
	m_headLayout->addLayout(btnLayout);
	sSvgName = m_svgPath + getSgvNameByFuncType(TableSpace::Table_Constriction);
	m_expandBtn = new CSvgButton(sSvgName, this);
	m_expandBtn->setFixedHeight(30 * dHeightScale);
	m_expandBtn->setFixedWidth(30 * dHeightScale);
	m_headLayout->addWidget(m_expandBtn);
	connect(m_expandBtn, &CSvgButton::clicked, this, &CWeconTable::ClickExpandButton);

	m_onePageShowCount = new QComboBox(this);
	if (80 * dWidthScale < 60) {
		m_onePageShowCount->setFixedSize(60, 30 * dHeightScale);
	}
	else {
		m_onePageShowCount->setFixedSize(80 * dWidthScale, 30 * dHeightScale);
	}
	m_onePageShowCount->setFocusPolicy(Qt::NoFocus);
	m_onePageShowCount->setStyleSheet("border: 1px solid #888888;border-radius: 3px;");
	updateOnePageShowCount();

	m_bIsSendChangePageShowCount = true;
	connect(m_onePageShowCount, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeShowPageNumber(int)));

	m_pageNum = new CChangePageEdit(style.m_nTotalPage, style.m_nCurrentPage, this);
	m_pageNum->setInputMethodHints(Qt::ImhDigitsOnly);
	m_pageNum->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_pageNum->setAlignment(Qt::AlignHCenter);
	sStyle = "font-size:" + QString::number(nFontSize) + "px;";
	m_pageNum->setStyleSheet(sStyle);
	m_pageNum->setFixedSize(100 * dWidthScale, 30 * dHeightScale);
	connect(m_pageNum, &CChangePageEdit::editingFinished, this, &CWeconTable::OnTurnToPageUp);

	sSvgName = m_svgPath + getSgvNameByFuncType(TableSpace::Table_UpPage);
	m_upPage = new CSvgButton(sSvgName, this);
	m_upPage->setFixedHeight(30 * dHeightScale);
	m_upPage->setFixedWidth(30 * dHeightScale);
	sSvgName = m_svgPath + getSgvNameByFuncType(TableSpace::Table_DownPage);
	m_downPage = new CSvgButton(sSvgName, this);
	m_downPage->setFixedHeight(30 * dHeightScale);
	m_downPage->setFixedWidth(30 * dHeightScale);
	connect(m_upPage, &CSvgButton::clicked, this, &CWeconTable::ClickPageUpButton);
	connect(m_downPage, &CSvgButton::clicked, this, &CWeconTable::ClickPageDownButton);

	tempSpaceItem = new QSpacerItem(10, 10, QSizePolicy::Expanding);
	m_bottomLayout->addSpacerItem(tempSpaceItem);
	m_bottomLayout->addWidget(m_onePageShowCount);
	m_bottomLayout->addWidget(m_upPage);
	m_bottomLayout->addWidget(m_pageNum);
	m_bottomLayout->addWidget(m_downPage);

	std::thread([this] {
		QPixmap(QString::fromStdString(CWeconTableModel::g_sFormulaImagePath + "/filePathIcon.png"));
		QPixmap(QString::fromStdString(CWeconTableModel::g_sFormulaImagePath + "/UDisk.png"));
		QPixmap(QString::fromStdString(CWeconTableModel::g_sFormulaImagePath + "/SDCard.png"));
		QPixmap(QString::fromStdString(CWeconTableModel::g_sFormulaImagePath + "/file.png"));
		QPixmap(QString::fromStdString(CWeconTableModel::g_sFormulaImagePath + "/folder.png"));
		QPixmap(QString::fromStdString(CWeconTableModel::g_sFormulaImagePath + "/standBack.png"));
		QPixmap(QString::fromStdString(CWeconTableModel::g_sFormulaImagePath + "/forward.png"));
	}
	).detach();

	return 0;
}

int CWeconTable::initPosition()
{
	auto index = m_model->getIndexByUid(m_nPositionId);
	if (!index.isValid()) {

		return -1;
	}

	m_table->setSelectionBehavior(QTableView::SelectRows);
	m_table->setCurrentIndex(index);

	return 0;
}

int CWeconTable::updateLockImage(const QString & imagePath)
{
	updateLockRect();
	auto pixmap = QPixmap(imagePath);
	pixmap = pixmap.scaled(m_lockImg->size(), Qt::KeepAspectRatio);
	m_lockImg->setPixmap(pixmap);
	m_lockImg->show();
	return 0;
}

int CWeconTable::updateLockRect()
{
	auto tableRect = m_table->geometry();
	auto nVHeadHeight = m_table->horizontalHeader()->rect().height();
	auto nVHeadWidth = m_table->verticalHeader()->width();;
	int nLeft = tableRect.left() + nVHeadWidth / 4;
	m_lockImg->setGeometry(nLeft, tableRect.top(), nVHeadWidth / 2, nVHeadHeight);
	return 0;
}

int CWeconTable::hideLockImage()
{
	m_lockImg->hide();
	return 0;
}

int CWeconTable::setFormulaEnable(bool bIsEnable) {
	m_searchBtn->setEnabled(bIsEnable);
	for (auto iter = m_functionBtn.begin(); iter != m_functionBtn.end(); ++iter) {
		iter.value()->setEnabled(bIsEnable);
	}
	if (bIsEnable) {
		m_table->setEditTriggers(QTableView::DoubleClicked);
	}
	else {
		m_table->setEditTriggers(QTableView::NoEditTriggers);
	}
	return 0;
}

int CWeconTable::setHeadData(QList<QString>& headDatas)
{
	return m_model->setHeadData(headDatas);
}

QVariant CWeconTable::data(const QModelIndex & index, int role) const
{
	return m_model->data(index, role);
}

bool CWeconTable::setData(const QModelIndex & index, const QVariant & value, int role)
{
	return m_model->setData(index, value, role);
}

bool CWeconTable::setData(const QStringList & datas, int role)
{
	int nIndex = CWeconTableModel::rowId - Qt::UserRole;
	if ((int)datas.size() < nIndex) {

		return false;
	}
	return m_model->setData(datas[nIndex - 1], datas, role);
}

int CWeconTable::getLanguageType()
{
	return m_languageType;
}

void CWeconTable::setLanguageType(CWeconTable::LanguageType & type)
{
	if (m_languageType == type) {

		return;
	}
	m_languageType = type;

	auto nIndex = m_onePageShowCount->currentIndex();
	m_onePageShowCount->clear();
	updateOnePageShowCount();

	m_bIsSendChangePageShowCount = false;
	m_onePageShowCount->setCurrentIndex(nIndex);
}

bool CWeconTable::isNumberic(const QString & str)
{
	return REGEX_MATCH(str.toStdString(), REGEX("^[+\\-]?[0-9]+(\\.[0-9]+)?$"));
}

int CWeconTable::addHeadData(const QString & sText, Qt::Orientation orientation, int section)
{
	return m_model->addHeadData(sText, orientation, section);
}

int CWeconTable::initHHead(QList<QString>& heads)
{
	if (heads.size() == 0) {

		return -1;
	}
	double dWidthScale;
	if (m_nWidth > m_nHeight) {
		dWidthScale = (double)m_nWidth / 800;
	}
	else {
		dWidthScale = (double)m_nWidth / 480;
	}
	auto nCellWidth = 100 * dWidthScale;
	auto tableWidth = m_table->rect().width() + 30 * dWidthScale;
	if (tableWidth > nCellWidth * heads.size()) {

		nCellWidth = tableWidth / heads.size();
	}
	m_table->horizontalHeader()->setDefaultSectionSize(nCellWidth);
	return m_model->initHHead(heads);
}

int CWeconTable::initData(QList<QStringList>& allData)
{
	int nRet = m_model->initData(allData);

	//this->m_table->resizeRowsToContents();
	//this->m_table->resizeColumnsToContents();

	return nRet;
}

int CWeconTable::initData(const std::string & allData)
{
	return m_model->initData(allData);
}

int CWeconTable::addDatas(const QList<QStringList> & datas)
{
	foreach(auto data, datas) {

		addData(data);
	}
	//this->m_table->resizeRowsToContents();
	//this->m_table->resizeColumnsToContents();
	return 0;
}

int CWeconTable::addData(const QStringList & data, const bool isUpdate)
{
	int nRet = m_model->addData(data);

	if (isUpdate) {

		this->m_table->resizeRowsToContents();
		this->m_table->resizeColumnsToContents();
	}

	return nRet;
}

int CWeconTable::clearData()
{
	return m_model->clearData();
}

int CWeconTable::insertDataFormat(const int & nColumn, TableSpace::DATAFORMAT_T & dataFormat)
{
	return m_model->insertDataFormat(nColumn, dataFormat);
}

QString CWeconTable::Int16ToRgbStr(int colorValue)
{
	char cTemp[10] = { 0 };
	snprintf(cTemp, 8, "#%06x", colorValue);
	return cTemp;
}

QString CWeconTable::IntToRGB(int nColorValue, int nAlipha)
{
	QString sRgbStr = "rgb(";
	int nR = 0xFF & nColorValue;
	int nG = 0xFF00 & nColorValue;
	nG >>= 8;
	int nB = 0xFF0000 & nColorValue;
	nB >>= 16;
	sRgbStr += QString::number(nB) + "," + QString::number(nG) + "," + QString::number(nR) + "," + QString::number(nAlipha) + ")";
	return sRgbStr;
}

int CWeconTable::getPreview(const QString & sPath)
{
	QPixmap pixmap = this->grab();
	bool bRes = pixmap.save(sPath, "PNG");
	if (!bRes) {
		return -1;
	}
	return 0;
}

int CWeconTable::setSrcno(int nNum)
{
	m_nScrno = nNum;
	return 0;
}

int CWeconTable::setCurrentPageNum(const int & nNum)
{
	return m_pageNum->setCurrentPage(nNum);
}

int CWeconTable::setTotalPageNum(const int & nNum)
{
	return m_pageNum->setTotalPage(nNum);
}

int CWeconTable::setOnePageShowCount(const int & nNum)
{
	int nIndex = 0;
	if (20 == nNum) {

		nIndex = 1;
	}
	else if (30 == nNum) {

		nIndex = 2;
	}
	else if (50 == nNum) {

		nIndex = 3;
	}
	else if (100 == nNum) {

		nIndex = 4;
	}

	if (nIndex != m_onePageShowCount->currentIndex()) {

		m_bIsSendChangePageShowCount = false;
		m_onePageShowCount->setCurrentIndex(nIndex);
	}

	return 0;
}

int CWeconTable::setTableName(const QString & sName)
{
	if (nullptr != m_tableHeadName) {

		m_tableHeadName->setText(sName);
	}
	return 0;
}

int CWeconTable::setPosition(const int & nPosition)
{
	this->m_nPositionId = nPosition;
	return 0;
}

int CWeconTable::setExeWidth(const int & nWidth)
{
	m_nWidth = nWidth;
	return 0;
}

int CWeconTable::setExeHeight(const int & nHeight)
{
	m_nHeight = nHeight;
	return 0;
}

QModelIndex CWeconTable::getCurrentSelectIndex()
{
	return m_table->currentIndex();
}

int CWeconTable::getSrcno()
{
	return m_nScrno;
}

int CWeconTable::getCurrentPageNum()
{
	return m_pageNum->getCurrentPage();
}

int CWeconTable::getOnePageShowCount()
{
	int nIndex = m_onePageShowCount->currentIndex();
	int nNum = 10;
	if (1 == nIndex) {

		nNum = 20;
	}
	else if (2 == nIndex) {

		nNum = 30;
	}
	else if (3 == nIndex) {

		nNum = 50;
	}
	else if (4 == nIndex) {

		nNum = 100;
	}
	return nNum;
}

int CWeconTable::getExeWidth()
{
	return m_nWidth;
}

int CWeconTable::getExeHeight()
{
	return m_nHeight;
}

QString CWeconTable::getSvgPath()
{
	return m_svgPath;
}

int CWeconTable::getPosition()
{
	return m_nPositionId;
}

void CWeconTable::updateTable()
{
	m_table->update();
}

QPushButton * CWeconTable::AddLeftBottomBtn(const QString & sSvgName)
{
	if ("" == sSvgName) {

		return nullptr;
	}
	double dWidthScale, dHeightScale;
	if (m_nWidth > m_nHeight) {
		dWidthScale = (double)m_nWidth / 800;
		dHeightScale = (double)m_nHeight / 480;
	}
	else {
		dWidthScale = (double)m_nWidth / 480;
		dHeightScale = (double)m_nHeight / 800;
	}
	auto changeBtn = new CSvgButton(sSvgName, this);
	changeBtn->setFixedHeight(30 * dHeightScale);
	changeBtn->setFixedWidth(30 * dWidthScale);
	m_bottomLayout->insertWidget(0, changeBtn);
	return changeBtn;
}

int CWeconTable::AddItemToPageCountComBox(const QString & sText)
{
	if ("" == sText || nullptr == m_onePageShowCount) {

		return -1;
	}
	m_onePageShowCount->addItem(sText);
	return 0;
}


int CWeconTable::showMsgWnd(const QString & sTitleText, const QString & sMsgText, QMessageBox::StandardButton buttons, QWidget * parent)
{
	ToastDialog * toastdialog = new ToastDialog(m_nWidth, m_nHeight, parent);
	toastdialog->show(INFO, sMsgText);
	return 0;
}

QString CWeconTable::getSgvNameByFuncType(const int & type)
{
	switch (type) {
	case TableSpace::Table_UpPage:
		return "upPage.svg";
	case TableSpace::Table_DownPage:
		return "downPage.svg";
	case TableSpace::Table_Search:
		return "search.svg";
	case TableSpace::Table_Expand:
		return "expandButton.svg";
	case TableSpace::Table_Constriction:
		return "constrictionButton.svg";
	default:
		break;
	}
	return "";
}

int CWeconTable::setSeachBtnClickFunc(FUNC_ON_SEEARCH_BUTTON_CLICK func)
{
	if (nullptr == func) {

		return -1;
	}

	m_searchBtnClickFunc = func;
	return 0;
}

int CWeconTable::setTurnPageFunc(FUNC_ON_TURN_PAGE func)
{
	if (nullptr == func) {

		return -1;
	}
	m_turnPageFunc = func;
	return 0;
}

int CWeconTable::setCellEditFinishFunc(FUNC_ON_CELL_FINISHEDIT func)
{
	if (nullptr == func) {

		return -1;
	}
	m_cellFinishEditFunc = func;
	return 0;
}

int CWeconTable::setChangeShowCountFunc(FUNC_ON_CHANGE_SHOWCOUNT func)
{
	if (nullptr == func) {

		return -1;
	}
	m_changeShowCountFunc = func;
	return 0;
}

int CWeconTable::initLanguageInfo()
{
	return 0;
}

int CWeconTable::initFuncBtnConnect(QPushButton * btn, const int & nFuncCode)
{
	return 0;
}

bool CWeconTable::IsTableSelect()
{
	auto index = m_table->currentIndex();
	return index.isValid();
}

void CWeconTable::resizeEvent(QResizeEvent * event)
{
	QWidget::resizeEvent(event);
}

void CWeconTable::paintEvent(QPaintEvent * event)
{
	QWidget::paintEvent(event);
	if (m_nVHeadWidth != m_table->verticalHeader()->width()) {

		m_nVHeadWidth = m_table->verticalHeader()->width();
		if (m_model->columnCount() == 0) {

			return;
		}
		double dWidthScale;
		if (m_nWidth > m_nHeight) {
			dWidthScale = (double)m_nWidth / 800;
		}
		else {
			dWidthScale = (double)m_nWidth / 480;
		}
		auto nCellWidth = 100 * dWidthScale;
		auto tableWidth = m_table->rect().width();
		tableWidth = tableWidth - m_table->verticalHeader()->width() - (30 * dWidthScale) - 2;
		if (tableWidth > nCellWidth * m_model->columnCount()) {

			nCellWidth = tableWidth / m_model->columnCount();
			m_table->horizontalHeader()->setSectionResizeMode(m_model->columnCount() - 1, QHeaderView::Stretch);
		}
		m_table->horizontalHeader()->setDefaultSectionSize(nCellWidth);
	}
}

void CWeconTable::OnClickSearchBtn() {

	if (m_searchBtnClickFunc) {

		m_searchBtnClickFunc();
	}
}

void CWeconTable::OnTurnToPageUp()
{
	int nCurPage = m_pageNum->getCurrentPage();

	if (m_turnPageFunc) {

		m_turnPageFunc(nCurPage);
	}
}

void CWeconTable::ClickPageUpButton()
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
	int nCurPage = m_pageNum->getCurrentPage() - 1;
	int turnPage = nCurPage;
	if (nCurPage < 1) {

		turnPage = 1;
	}

	if (m_turnPageFunc) {

		m_turnPageFunc(turnPage);
	}
}

void CWeconTable::ClickPageDownButton()
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
	int nCurPage = m_pageNum->getCurrentPage() + 1;
	int nTotalPage = m_pageNum->getTotalPage();
	int turnPage = nCurPage;
	if (nCurPage > nTotalPage) {

		turnPage = nTotalPage;
	}

	if (m_turnPageFunc) {

		m_turnPageFunc(turnPage);
	}
}

void CWeconTable::ClickExpandButton()
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
	auto it = m_functionBtn.begin();
	if (m_bIsExpand) {

		m_bIsExpand = false;
		for (; it != m_functionBtn.end(); it++)
		{
			it.value()->hide();
		}
		QString sSvgName = m_svgPath + getSgvNameByFuncType(TableSpace::Table_Expand);
		m_expandBtn->setSvgPath(sSvgName);
	}
	else {
		m_bIsExpand = true;
		for (; it != m_functionBtn.end(); it++)
		{
			it.value()->show();
		}
		QString sSvgName = m_svgPath + getSgvNameByFuncType(TableSpace::Table_Constriction);
		m_expandBtn->setSvgPath(sSvgName);
	}
	m_expandBtn->update();
}

void CWeconTable::ChangeShowPageNumber(int nCurIndex)
{
	int nIndex = m_onePageShowCount->currentIndex();
	if (-1 == nIndex || !m_bIsSendChangePageShowCount || m_onePageShowCount->count() <= 1) {

		m_bIsSendChangePageShowCount = true;
		return;
	}

	int nNum = 10;
	switch (nIndex)
	{
	case 1:
		nNum = 20;
		break;
	case 2:
		nNum = 30;
		break;
	case 3:
		nNum = 50;
		break;
	case 4:
		nNum = 100;
		break;
	default:
		break;
	}
	m_changeShowCountFunc(nNum);
}

void CWeconTable::DoubleClickCell(const QModelIndex &index)
{
	if (!index.isValid()) {
		return;
	}

	m_table->setSelectionBehavior(QTableView::SelectItems);
	m_table->setCurrentIndex(index);
	if (m_bIsDisablesCellEditing) {
		return;
	}

	m_lastEditText = m_model->data(index);

	m_model->setCurrentEditIndex(index);
	m_model->setEditFlag(true);
	m_lastClickIndex = index;
	m_table->edit(index);
}

void CWeconTable::CellCloseEdit(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
	m_model->setEditFlag(false);

	auto index = m_model->getCurrentEditIndex();
	if (!index.isValid()) {

		m_model->setData(index, m_lastEditText);
		return;
	}

	QString sMsgText;
	auto cellData = ((QLineEdit*)editor)->text();
	auto dataFormat = m_model->getColumnDataFormat(index.column());
	/*if (dataFormat.m_nDataFormat == TableSpace::_DataString) {

		QString sText = cellData;
		if (sText.length() > dataFormat.m_nStringLength) {

			if (EnglishLanguage == m_languageType) {

				sMsgText = "String exceeds the length! " + QString::number(dataFormat.m_nStringLength);
			}
			else {

				sMsgText = "字符超出长度! " + QString::number(dataFormat.m_nStringLength);
			}
		}
	}
	else if ((dataFormat.m_nDataFormat == TableSpace::_Double || dataFormat.m_nDataFormat == TableSpace::_Float ||
		dataFormat.m_nDataFormat == TableSpace::_UInt || dataFormat.m_nDataFormat == TableSpace::_Bool_ || dataFormat.m_nDataFormat == TableSpace::_Int) && this->isNumberic(cellData)) {

		double dValue = cellData.toDouble();

		if ("" == cellData || dValue > dataFormat.m_dDoubleMax || dValue < dataFormat.m_dDoubleMin) {

			if (EnglishLanguage == m_languageType) {

				sMsgText = "Data exceeds the range! [" + QString::number(dataFormat.m_dDoubleMin, 'f', dataFormat.m_nDecimal) + "," + QString::number(dataFormat.m_dDoubleMax, 'f', dataFormat.m_nDecimal) + "]";
			}
			else {

				sMsgText = "数据超出范围! [" + QString::number(dataFormat.m_dDoubleMin, 'f', dataFormat.m_nDecimal) + "," + QString::number(dataFormat.m_dDoubleMax, 'f', dataFormat.m_nDecimal) + "]";
			}
		}
		auto nums = cellData.split('.');
		if (nums.size() == 2 && (int)nums[1].size() > dataFormat.m_nDecimal) {

			cellData = nums[0] + "." + nums[1].mid(0, dataFormat.m_nDecimal);
		}
	}
	else {

		if (EnglishLanguage == m_languageType) {

			sMsgText = "Invalid input, exceeds the limit!";
		}
		else {

			sMsgText = "超出范围，输入无效!";
		}
	}*/

	if (sMsgText != "") {

		OnShowMessageWnd(sMsgText);
		m_model->setData(index, m_lastEditText);
		return;
	}
	else {

		m_model->setData(index, cellData);
	}

	if (m_cellFinishEditFunc) {

		auto uid = m_model->data(index, CWeconTableModel::rowId);
		auto headname = m_model->headerData(index.column(), Qt::Horizontal);
		m_cellFinishEditFunc(uid.toInt(), headname.toString().toStdString(), cellData.toStdString());
	}
}

void CWeconTable::OnClickVHead(int nLogicalIndex)
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
	m_table->setSelectionBehavior(QTableView::SelectRows);
	m_table->selectRow(nLogicalIndex);
}

void CWeconTable::OnShowMessageWnd(QString & sMsgText)
{
	CWeconTable::showMsgWnd("warning", sMsgText, QMessageBox::NoButton, this);
}

void CWeconTable::getScaleWH() {
	if (this->parent() != nullptr) {

		auto rect = ((QWidget *)this->parent())->geometry();
		m_nWidth = rect.width();
		m_nHeight = rect.height();
	}
}