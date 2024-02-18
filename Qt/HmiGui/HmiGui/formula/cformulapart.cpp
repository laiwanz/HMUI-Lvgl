#include "cformulapart.h"
#include <QFileDialog>
#include "HmiBase/HmiBase.h"
#include "../hmiutility.h"
#include "cdialogbox.h"
#include "../HmiGui/HmiGui.h"
#include "../EventMgr/EventMgr.h"
#include "SysSetGui/SysSetGui.h"
#ifndef HMIPREVIEW
#ifdef _LINUX_
#include "btl/btl.h"
#include "platform/platform.h"
#include "cweconselectwnd.h"
#endif // _LINUX_
#endif // HMIPREVIEW

QString CFormulaPart::m_languageInfo[CWeconTable::LanguageTypeMax][LanguageCodeMax];

CFormulaPart::CFormulaPart(const hmiproto::cformula & protoData, QWidget *parent)
	:CWeconTable()
{
	this->getScaleWH();
	m_part.Clear();
	m_part.MergeFrom(protoData);
	m_nInsertPositon = 2;
	m_pAddFormulaGroupWnd = nullptr;
	m_pWeconSelectWnd = nullptr;
	m_pFormulaSearchWnd = nullptr;

	this->setCellEditFinishFunc(std::bind(&CFormulaPart::CellFinishEditFunc, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	this->setTurnPageFunc(std::bind(&CFormulaPart::sendTurnPageEvent, this, std::placeholders::_1));
	this->setChangeShowCountFunc(std::bind(&CFormulaPart::sendChangeShowCountEvent, this, std::placeholders::_1));
	this->setSeachBtnClickFunc(std::bind(&CFormulaPart::ClickSearchButton, this));

	connect(this, &CFormulaPart::updateOnePageShowCount, this, &CFormulaPart::OnUpdateOnePageShowCount);
	connect(this, &CFormulaPart::showMessageWnd, this, &CFormulaPart::OnShowMessageWnd);

	qRegisterMetaType<QVector<CFormulaSearchWnd::SearchData>>("QVector<CFormulaSearchWnd::SearchData> &");
	connect(this, &CFormulaPart::showSearchWnd, this, &CFormulaPart::OnShowSearchWnd, Qt::QueuedConnection);
}

void CFormulaPart::Init(QWidget * pWidget) {
	bool bIsLock = false;
	CWeconTable::LanguageType formulaLanguageType = CWeconTable::ChineseLanguage;
	std::string sBaseDir, sLockImage;
	this->setParent(pWidget);
	TableSpace::TABLESTYLE style;
	if (m_part.has_basic()) {
		auto basic = m_part.basic();
		if (basic.has_left()) {
			style.m_nLeft = basic.left();
		}
		if (basic.has_top()) {
			style.m_nTop = basic.top();
		}
		if (basic.has_right()) {
			style.m_nWidth = basic.right() - basic.left();
		}
		if (basic.has_bottom()) {
			style.m_nHeight = basic.bottom() - basic.top();
		}
		if (basic.has_scrno()) {
			setSrcno(basic.scrno());
		}
		if (basic.has_name()) {
			m_sPartName = basic.name();
		}
		if (basic.has_msgtiplanguage()) {
			if (basic.msgtiplanguage() == 1) {
				formulaLanguageType = CWeconTable::EnglishLanguage;
			}
		}
	}
	if (m_part.has_style()) {
		auto tableStyle = m_part.style();
		if (tableStyle.has_hheadcolor()) {
			style.m_nHTableHeadColor = tableStyle.hheadcolor();
		}
		if (tableStyle.has_vheadcolor()) {
			style.m_nVTableHeadColor = tableStyle.vheadcolor();
		}
		if (tableStyle.has_selectcolor()) {
			style.m_nSelectColor = tableStyle.selectcolor();
		}
		if (tableStyle.has_linecolor()) {
			style.m_nLineColor = tableStyle.linecolor();
		}
		if (tableStyle.has_bordercolor()) {
			style.m_nBackgroundColor = tableStyle.bordercolor();
		}
		if (tableStyle.has_rowheight()) {
			style.m_nCellHeight = tableStyle.rowheight();
		}
		if (tableStyle.has_colwidth()) {
			style.m_nCellWidth = tableStyle.colwidth();
		}
		if (tableStyle.has_transparent()) {
			style.m_nTransparent = tableStyle.transparent() / 100.0 * 255;
		}
		if (tableStyle.has_alternatecolor1()) {
			style.m_nRowBgColor1 = tableStyle.alternatecolor1();
		}
		if (tableStyle.has_alternatecolor2()) {
			style.m_nRowBgColor2 = tableStyle.alternatecolor2();
		}
		if (tableStyle.has_hheadtextcolor()) {
			style.m_nHHeadTextColor = tableStyle.hheadtextcolor();
		}
		if (tableStyle.has_vheadtextcolor()) {
			style.m_nVHeadTextColor = tableStyle.vheadtextcolor();
		}
		if (tableStyle.has_datatextcolor()) {
			style.m_nTableTextColor = tableStyle.datatextcolor();
		}
		if (tableStyle.has_templatestyle()) {
			sBaseDir = std::to_string(tableStyle.templatestyle());
		}
		if (tableStyle.has_namecolor()) {
			style.m_nNameColor = tableStyle.namecolor();
		}
		if (tableStyle.has_lockimgpath()) {
			sLockImage = tableStyle.lockimgpath();
		}
		if (tableStyle.has_islock()) {
			bIsLock = tableStyle.islock();
		}
		if (tableStyle.has_disablescelledit()) {
			style.m_bIsDisablesCellEditing = tableStyle.disablescelledit();
		}
	}
	if (m_part.has_currentname()) {
		style.m_sName = m_part.currentname();
	}
	if (m_part.has_currentpage()) {
		style.m_nCurrentPage = m_part.currentpage();
	}
	if (m_part.has_totalpage()) {
		style.m_nTotalPage = m_part.totalpage();
	}
	for (int i = 0; i < m_part.listname_size(); i++) {
		addFormula(QString::fromStdString(m_part.listname()[i]));
	}

	for (int i = 0; i < m_part.functionbtn_size(); i++) {
		TableSpace::FUNCSTYLE funcBtn;
		auto funcBtnStyle = m_part.functionbtn()[i];
		if (funcBtnStyle.has_code()) {
			funcBtn.m_nCode = (TableSpace::funcType)funcBtnStyle.code();
		}
		if (funcBtnStyle.has_desc()) {
			funcBtn.m_sDesc = funcBtnStyle.desc();
		}
		style.m_funcs.push_back(funcBtn);
	}
	style.m_sImgPath = getPrjDir() + "img/formula/" + sBaseDir;
	CWeconTableModel::g_sFormulaImagePath = style.m_sImgPath;
	init(style);
	//chang formula butto
	QString sSvgName = QString::fromStdString(style.m_sImgPath) + "/" + getSgvNameByFuncType(formulaSpace::Formula_Change);
	m_changeBtn = static_cast<CSvgButton *>(AddLeftBottomBtn(sSvgName));
	connect(m_changeBtn, &CSvgButton::clicked, this, &CFormulaPart::ClickChangeFormulaButton);
	setLanguageType(formulaLanguageType);
	for (int i = 0; i < m_part.dataformat_size(); i++)
	{
		auto dataproto = m_part.dataformat()[i];
		if (dataproto.has_col()) {
			int nCol = dataproto.col();
			if (dataproto.has_dataformat()) {
				TableSpace::DATAFORMAT_T dataFormat;
				dataFormat.nDataFormat = dataproto.dataformat();
				dataFormat.nLength = dataproto.length();
				dataFormat.sMax = dataproto.max_();
				dataFormat.sMin = dataproto.min_();
				dataFormat.nDecimal = dataproto.decimal();
				insertDataFormat(nCol - 1, dataFormat);
			} 
		}
	}
	QList<QString> hheads;
	if (m_part.has_horizontalhead()) {
		auto hHeadDatas = m_part.horizontalhead();
		for (int i = 0; i < hHeadDatas.value_size(); i++)
		{
			hheads.push_back((hHeadDatas.value()[i].c_str()));
		}
	}
	initHHead(hheads);
	initData(m_part.datas());
	if (bIsLock) {
		updateLockImage(QString::fromStdString(getPrjDir() + sLockImage));
	}
	else {
		hideLockImage();
	}
	setFormulaEnable(!bIsLock);
#if ((defined WIN32) && (defined WIN32SCALE))
	RECT	rc;
	rc.left = style.m_nLeft;
	rc.top = style.m_nTop;
	rc.right = style.m_nWidth + style.m_nLeft;
	rc.bottom = style.m_nHeight + style.m_nTop;

	memcpy(&m_OriginalRect, &rc, sizeof(RECT));
	this->ZoomPartSelf();
#endif
	m_part.Clear();
}

void CFormulaPart::Update(const hmiproto::cformula & protoData, QWidget * pWidget)
{
	int nCurPageNum = -1;
	std::string sLockImage;
	bool bIsLock = false;

	if (protoData.tablegroup_size() > 0) {
	
		QVector<CFormulaSearchWnd::SearchData> datas;
		for (int i = 0; i < protoData.tablegroup_size(); i++)
		{
			auto groupData = protoData.tablegroup()[i];
			if (groupData.has_id() && groupData.has_name()) {
			
				CFormulaSearchWnd::SearchData data;
				data.m_sShowName = QString::fromStdString(groupData.name());
				data.m_id = groupData.id();
				datas.push_back(data);
			}
		}
		showSearchWnd(datas);
	}

	if (protoData.has_basic()) {
	
		auto basic = protoData.basic();
		if (basic.has_msgtiplanguage()) {
		
			auto formulaLanguageType = CWeconTable::ChineseLanguage;
			if (basic.msgtiplanguage() == 1) {

				formulaLanguageType = CWeconTable::EnglishLanguage;
			}
			setLanguageType(formulaLanguageType);
		}
	}

	if (protoData.has_style()) {
	
		auto tableStyle = protoData.style();

		if (tableStyle.has_lockimgpath()) {

			sLockImage = tableStyle.lockimgpath();
		}
		if (tableStyle.has_islock()) {

			bIsLock = tableStyle.islock();
		}
	}

	if (protoData.has_horizontalhead()) {

		QList<QString> hheads;
		auto hHeadDatas = protoData.horizontalhead();
		for (int i = 0; i < hHeadDatas.value_size(); i++)
		{
			hheads.push_back((hHeadDatas.value()[i].c_str()));
		}
		setHeadData(hheads);
	}

	if (protoData.listname_size() > 0) {
	
		clearFormula();
		for (int i = 0; i < protoData.listname_size(); i++) {

			addFormula(QString::fromStdString(protoData.listname()[i]));
		}
	}

	if (protoData.has_currentpage()) {

		nCurPageNum = protoData.currentpage();
	}

	if (protoData.has_currentname()) {
		setTableName(QString::fromStdString(protoData.currentname()));
	}

	if (protoData.has_totalpage()) {

		this->setTotalPageNum(protoData.totalpage());
	}
	if (protoData.has_prepagenum()) {
		
		this->setOnePageShowCount(protoData.prepagenum());
	}
	this->setCurrentPageNum(nCurPageNum);
	if (protoData.has_datas()) {
	
		this->clearData();
		this->initData(protoData.datas());
		this->initPosition();
		this->updateTable();
	}

	if (bIsLock) {

		updateLockImage(QString::fromStdString(getPrjDir() + sLockImage));
	}
	else {

		hideLockImage();
	}
	setFormulaEnable(!bIsLock);
}

void CFormulaPart::Draw(QWidget * pWidget, int drawtype) {

}

int CFormulaPart::parseJson(const std::string & sbuffer) {
	int nRtn = 0;
	Json::Value root;
	if ((nRtn = cbl::loadJsonString(sbuffer, root)) < 0) {
		printf("%s:load json string error:%d\r\n", __FUNCTION__, nRtn);
		return -1;
	}
	if (!root.isMember("formula") ||
		!root["formula"].isMember("errno") ||
		!root["formula"].isMember("msg") ||
		!root["formula"].isMember("type")) {
		return -2;
	}
	QString sText = "";
	int nType = root["formula"]["type"].asInt();
	int nErrno = root["formula"]["errno"].asInt();
	if (nErrno != 0 || 
		(nErrno == 0 && (4 == nType || 5 == nType || 7 == nType || 6 == nType))) {
		sText = QString::fromStdString(root["formula"]["msg"].asString());
		showMessageWnd(sText);
	}
	return 0;
}

#if ((defined WIN32) && (defined WIN32SCALE))
void CFormulaPart::ZoomPartSelf()
{
	RECT rectTemp = m_OriginalRect;

	HMIUtility::CountScaleRect(&rectTemp);

	int nWidth = rectTemp.right - rectTemp.left;
	int nHeight = rectTemp.bottom - rectTemp.top;
	this->setGeometry(rectTemp.left, rectTemp.top, nWidth, nHeight);
}
#endif

int CFormulaPart::initLanguageInfo()
{
	m_languageInfo[EnglishLanguage][exceedPage] = "Invalid input, exceeds the limit!";
	m_languageInfo[EnglishLanguage][selcetRow] = "Please select a row!";
	m_languageInfo[EnglishLanguage][dataExceed] = "Data exceeds the range!";
	m_languageInfo[EnglishLanguage][stringExceed] = "String exceeds the length!";
	m_languageInfo[EnglishLanguage][search] = "Search";
	m_languageInfo[EnglishLanguage][add] = "Add";
	m_languageInfo[EnglishLanguage][Delete] = "Delete";
	m_languageInfo[EnglishLanguage][upload] = "Upload";
	m_languageInfo[EnglishLanguage][download] = "Download";
	m_languageInfo[EnglishLanguage][Import] = "Import";
	m_languageInfo[EnglishLanguage][Export] = "Export";
	m_languageInfo[EnglishLanguage][formula] = "Formula";
	m_languageInfo[EnglishLanguage][prepage] = "Previous page";
	m_languageInfo[EnglishLanguage][nextpage] = "Next page";
	m_languageInfo[EnglishLanguage][preline] = "Previous line";
	m_languageInfo[EnglishLanguage][nextline] = "Next line";
	m_languageInfo[EnglishLanguage][intop] = "Move to top";
	m_languageInfo[EnglishLanguage][inbottom] = "Move to bottom";
	m_languageInfo[EnglishLanguage][tens] = "10/page";
	m_languageInfo[EnglishLanguage][twenty] = "20/page";
	m_languageInfo[EnglishLanguage][thirty] = "30/page";
	m_languageInfo[EnglishLanguage][fifty] = "50/page";
	m_languageInfo[EnglishLanguage][hundred] = "100/page";
	m_languageInfo[EnglishLanguage][allgroup] = "All groups";
	m_languageInfo[EnglishLanguage][clear] = "Clear records";
	m_languageInfo[EnglishLanguage][pleaseInput] = "Please enter";
	m_languageInfo[EnglishLanguage][pleaseSelect] = "Select a group position";
	m_languageInfo[EnglishLanguage][insertDev] = "please insert sd card/USB flash disk";
	m_languageInfo[EnglishLanguage][groupIsEmpty] = "group name can`t empty";
	m_languageInfo[EnglishLanguage][Cancel] = "Cancel";
	m_languageInfo[EnglishLanguage][Determine] = "Determine";
	m_languageInfo[EnglishLanguage][Folder] = "Folder";
	m_languageInfo[EnglishLanguage][CreateFolder] = "Create new folder";
	m_languageInfo[EnglishLanguage][Tips] = "Tips";
	m_languageInfo[EnglishLanguage][DeleteLine] = "Whether or not to delete that line!";
	m_languageInfo[EnglishLanguage][CreateFailed] = "Failed to create folder!";

	m_languageInfo[ChineseLanguage][exceedPage] = "超出范围，输入无效!";
	m_languageInfo[ChineseLanguage][selcetRow] = "请先选择行!";
	m_languageInfo[ChineseLanguage][dataExceed] = "数据超出范围!";
	m_languageInfo[ChineseLanguage][stringExceed] = "字符超出长度!";
	m_languageInfo[ChineseLanguage][search] = "搜索";
	m_languageInfo[ChineseLanguage][add] = "添加";
	m_languageInfo[ChineseLanguage][Delete] = "删除";
	m_languageInfo[ChineseLanguage][upload] = "上传";
	m_languageInfo[ChineseLanguage][download] = "下载";
	m_languageInfo[ChineseLanguage][Import] = "导入";
	m_languageInfo[ChineseLanguage][Export] = "导出";
	m_languageInfo[ChineseLanguage][formula] = "配方";
	m_languageInfo[ChineseLanguage][prepage] = "上一页";
	m_languageInfo[ChineseLanguage][nextpage] = "下一页";
	m_languageInfo[ChineseLanguage][preline] = "上一行";
	m_languageInfo[ChineseLanguage][nextline] = "下一行";
	m_languageInfo[ChineseLanguage][intop] = "置顶";
	m_languageInfo[ChineseLanguage][inbottom] = "置底";
#ifdef HMIPREVIEW
	m_languageInfo[ChineseLanguage][tens] = QString::fromLocal8Bit("10条/页");
	m_languageInfo[ChineseLanguage][twenty] = QString::fromLocal8Bit("20条/页");
	m_languageInfo[ChineseLanguage][thirty] = QString::fromLocal8Bit("30条/页");
	m_languageInfo[ChineseLanguage][fifty] = QString::fromLocal8Bit("50条/页");
	m_languageInfo[ChineseLanguage][hundred] = QString::fromLocal8Bit("100条/页");
#else
	m_languageInfo[ChineseLanguage][tens] = "10条/页";
	m_languageInfo[ChineseLanguage][twenty] = "20条/页";
	m_languageInfo[ChineseLanguage][thirty] = "30条/页";
	m_languageInfo[ChineseLanguage][fifty] = "50条/页";
	m_languageInfo[ChineseLanguage][hundred] = "100条/页";
#endif
	m_languageInfo[ChineseLanguage][allgroup] = "全部分组";
	m_languageInfo[ChineseLanguage][clear] = "清空记录";
	m_languageInfo[ChineseLanguage][pleaseInput] = "请输入";
	m_languageInfo[ChineseLanguage][pleaseSelect] = "选择组位置";
	m_languageInfo[ChineseLanguage][insertDev] = "请插入U盘/SD卡";
	m_languageInfo[ChineseLanguage][groupIsEmpty] = "组名不能为空!";
	m_languageInfo[ChineseLanguage][Cancel] = "取消";
	m_languageInfo[ChineseLanguage][Determine] = "确定";
	m_languageInfo[ChineseLanguage][Folder] = "文件夹";
	m_languageInfo[ChineseLanguage][CreateFolder] = "新建文件夹";
	m_languageInfo[ChineseLanguage][Tips] = "提示";
	m_languageInfo[ChineseLanguage][DeleteLine] = "是否删除该行!";
	m_languageInfo[ChineseLanguage][CreateFailed] = "创建文件夹失败!";

	return 0;
}

int CFormulaPart::initFuncBtnConnect(QPushButton * btn, const int & nFuncCode)
{
	if (formulaSpace::FormulaFuncType::Formula_Add == nFuncCode) {
		connect(btn, &CSvgButton::clicked, this, &CFormulaPart::ClickAddDataButton);
	}
	else if (formulaSpace::FormulaFuncType::Formula_Delete == nFuncCode) {
		connect(btn, &CSvgButton::clicked, this, &CFormulaPart::ClickDeleteButton);
	}
	else if (formulaSpace::FormulaFuncType::Formula_Download == nFuncCode) {
		connect(btn, &CSvgButton::clicked, this, &CFormulaPart::ClickDownloadButton);
	}
	else if (formulaSpace::FormulaFuncType::Formula_Upload == nFuncCode) {
		connect(btn, &CSvgButton::clicked, this, &CFormulaPart::ClickUploadButton);
	}
	else if (formulaSpace::FormulaFuncType::Formula_LeadingIn == nFuncCode) {
		connect(btn, &CSvgButton::clicked, this, &CFormulaPart::ClickLeadingInButton);
	}
	else if (formulaSpace::FormulaFuncType::Formula_LeadingOUt == nFuncCode) {
		connect(btn, &CSvgButton::clicked, this, &CFormulaPart::ClickLeadingOutButton);
	}
	return 0;
}

int CFormulaPart::initSearchBtnConnect(QPushButton * btn)
{
	connect(btn, &CSvgButton::clicked, this, &CFormulaPart::ClickSearchButton);
	return 0;
}

int CFormulaPart::AddSearchGroupList(const QString & sSearchText)
{
	if (m_searchGroupList.size() >= 3) {
		m_searchGroupList.pop_back();
	}
	m_searchGroupList.push_front(sSearchText);
	return 0;
}

int CFormulaPart::AddSearchFormulaList(const QString & sSearchText)
{
	if (m_searchFormulaList.size() >= 3) {

		m_searchFormulaList.pop_back();

	}
	m_searchFormulaList.push_front(sSearchText);
	return 0;
}

int CFormulaPart::ClearSearchGroupList()
{
	m_searchGroupList.clear();
	return 0;
}

int CFormulaPart::ClearSearchFormulaList()
{
	m_searchFormulaList.clear();
	return 0;
}

int CFormulaPart::addFormula(const QString & sName)
{
	CFormulaSearchWnd::SearchData data;
	data.m_sShowName = sName;
	data.m_id = 0;
	m_formulaList.push_back(data);
	return 0;
}

int CFormulaPart::clearFormula()
{
	m_formulaList.clear();
	return 0;
}

int CFormulaPart::CellFinishEditFunc(const int & nId, const std::string & sHead, const std::string & sData)
{
#ifndef HMIPREVIEW
	if (sHead == "") {
		return -1;
	}
	std::string sJsonStr;
	Json::Value data;
	data["formula"]["ids"][0] = nId;
	data["formula"]["type"] = 8;
	data["formula"]["elementname"] = sHead;
	data["formula"]["data"] = sData;
	sJsonStr = data.toStyledString();
	EventMgr::SendDataTransEvent(this->getSrcno(), m_sPartName, sJsonStr);
#endif // !HMIPREVIEW
	return 0;
}

void CFormulaPart::OnUpdateOnePageShowCount()
{
	AddItemToPageCountComBox(m_languageInfo[getLanguageType()][tens]);
	AddItemToPageCountComBox(m_languageInfo[getLanguageType()][twenty]);
	AddItemToPageCountComBox(m_languageInfo[getLanguageType()][thirty]);
	AddItemToPageCountComBox(m_languageInfo[getLanguageType()][fifty]);
	AddItemToPageCountComBox(m_languageInfo[getLanguageType()][hundred]);
}

void CFormulaPart::DoAddGroupFunction(int nPosition, const QString &sName)
{
#ifndef HMIPREVIEW
	if ("" != sName && getSrcno() >= 0 && m_sPartName != "") {

		m_nInsertPositon = nPosition;
		int nUid = 1;
		QModelIndex nIndex = getCurrentSelectIndex();
		if (nIndex.isValid()) {

			nUid = data(nIndex, CWeconTableModel::rowId).toInt();
		}
		else if (0 == nPosition || 1 == nPosition) {

			OnShowMessageWnd(m_languageInfo[getLanguageType()][selcetRow]);
			return;
		}

		std::string sJsonStr;
		Json::Value data;
		data["formula"]["ids"][0] = nUid;
		data["formula"]["type"] = 0;
		data["formula"]["groupname"] = sName.toStdString();
		data["formula"]["position"] = nPosition;
		sJsonStr = data.toStyledString();

		EventMgr::SendDataTransEvent(getSrcno(), m_sPartName, sJsonStr);
	}
#endif // !HMIPREVIEW
}

void CFormulaPart::OnShowSearchWnd(QVector<CFormulaSearchWnd::SearchData>& datas)
{
	QWidget maskLayer(this->parentWidget());
	maskLayer.setFixedSize(this->window()->width(), this->window()->height());
	maskLayer.setStyleSheet("background-color: rgba(0, 0, 0, 0.5);"); // 设置蒙版层的背景色和透明度
	maskLayer.show();
	if (nullptr == m_pFormulaSearchWnd) {
		m_pFormulaSearchWnd = new CFormulaSearchWnd(this->getExeWidth(), this->getExeHeight(), this->getSvgPath(), CFormulaSearchWnd::SearchWnd, this);
	}
	m_pFormulaSearchWnd->setListInfo(m_searchGroupList);
	m_pFormulaSearchWnd->init(datas);
	m_pFormulaSearchWnd->setWindowModality(Qt::WindowModal);
	connect(m_pFormulaSearchWnd, &CFormulaSearchWnd::OnSearchFunction, this, &CFormulaPart::DoSearchFunction);
	m_pFormulaSearchWnd->exec();
}

void CFormulaPart::DoSearchFunction(int & nWndType, QVariant & id)
{
	if (nWndType == CFormulaSearchWnd::ChangeWnd) {
#ifndef HMIPREVIEW
		if (!id.isValid()) {
			return;
		}
		Json::Value data;
		data["formula"] = Json::Value();
		data["formula"]["name"] = id.toString().toStdString();
		data["formula"]["type"] = 2;
		EventMgr::SendDataTransEvent(this->getSrcno(), m_sPartName, data.toStyledString());
#endif // !HMIPREVIEW
	}
	else {
#ifndef HMIPREVIEW
		if (!id.isValid()) {
			return;
		}
		this->setPosition(id.toString().toInt());
		initPosition();
		Json::Value data;
		data["formula"]["ids"][0] = this->getPosition();
		data["formula"]["type"] = 10;
		EventMgr::SendDataTransEvent(this->getSrcno(), m_sPartName, data.toStyledString());
#endif // !HMIPREVIEW
	}
}

void CFormulaPart::ClickAddDataButton()
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
	{
		QWidget maskLayer(this->parentWidget());
		maskLayer.setFixedSize(this->window()->width(), this->window()->height());
		maskLayer.setStyleSheet("background-color: rgba(0, 0, 0, 0.5);"); // 设置蒙版层的背景色和透明度
		maskLayer.show();
		if (nullptr == m_pAddFormulaGroupWnd){
			m_pAddFormulaGroupWnd = new CAddFormulaGroupWnd(this);
		}
		m_pAddFormulaGroupWnd->changePosition(m_nInsertPositon);
		m_pAddFormulaGroupWnd->setWindowModality(Qt::WindowModal);
		connect(m_pAddFormulaGroupWnd, &CAddFormulaGroupWnd::addGroup, this, &CFormulaPart::DoAddGroupFunction);
		m_pAddFormulaGroupWnd->exec();
		delete m_pAddFormulaGroupWnd;
		m_pAddFormulaGroupWnd = nullptr;
	}
}

void CFormulaPart::ClickDeleteButton()
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
	if (!IsTableSelect()) {

		CWeconTable::showMsgWnd("warning", m_languageInfo[getLanguageType()][selcetRow], QMessageBox::NoButton, this);
		return;
	}
	{
		QWidget maskLayer(this->parentWidget());
		maskLayer.setFixedSize(this->window()->width(), this->window()->height());
		maskLayer.setStyleSheet("background-color: rgba(0, 0, 0, 0.5);"); // 设置蒙版层的背景色和透明度
		maskLayer.show();
		CDialogBox dialogBox(getExeWidth(), getExeHeight(), this);
		dialogBox.config(m_languageInfo[getLanguageType()][Tips].toStdString(), m_languageInfo[getLanguageType()][DeleteLine].toStdString());
		int res = dialogBox.exec();
		if (res == QDialog::Accepted) {
			sendUidAndType(1);
		}
	}
}

void CFormulaPart::ClickDownloadButton()
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
	if (!IsTableSelect()) {


		CWeconTable::showMsgWnd("warning", m_languageInfo[getLanguageType()][selcetRow], QMessageBox::NoButton, this);
		return;
	}
	sendUidAndType(5);
}

void CFormulaPart::ClickUploadButton()
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
	if (!IsTableSelect()) {

		QString sMsgTitle = "warning";
		QString sMsgText = m_languageInfo[getLanguageType()][selcetRow];

		showMsgWnd(sMsgTitle, sMsgText, QMessageBox::NoButton, this);
		return;
	}
	sendUidAndType(4);
}

void CFormulaPart::ClickLeadingInButton()
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
#ifndef WIN32
	QString sInitPath, sTurnPath, sInitName, sTurnName;
	if (CHmiMedia::isUSBExsit()) {

		std::string sTempStr;
		CHmiMedia::getUdiskPath(sTempStr);
		sInitPath = QString::fromStdString(sTempStr);
		sInitName = "USB FLASH";
	}
	if (CHmiMedia::isSDExsit()) {

		std::string sTempStr;
		CHmiMedia::getSDPath(sTempStr);
		sTurnPath = QString::fromStdString(sTempStr);
		sTurnName = "SD CARD";
	}
	if ("" == sInitPath) {

		sInitPath = sTurnPath;
		sInitName = sTurnName;
		sTurnName = "";
		sTurnPath = "";
	}

	if ("" == sInitPath) {

		CWeconTable::showMsgWnd("warning", m_languageInfo[getLanguageType()][insertDev], QMessageBox::NoButton, this);
		return;
	}
	{
		QWidget maskLayer(this->parentWidget());
		maskLayer.setFixedSize(this->window()->width(), this->window()->height());
		maskLayer.setStyleSheet("background-color: rgba(0, 0, 0, 0.5);"); // 设置蒙版层的背景色和透明度
		maskLayer.show();
		if (nullptr == m_pWeconSelectWnd){
			m_pWeconSelectWnd = new CWeconSelectWnd(this->getExeWidth(), this->getExeHeight(), CWeconSelectWnd::ImportWnd, sInitPath, CWeconSelectWnd::TypeFile, sInitName, sTurnName, sTurnPath, this);
		}
		m_pWeconSelectWnd->setWindowModality(Qt::WindowModal);
		connect(m_pWeconSelectWnd, &CWeconSelectWnd::OnSelectResult, this, &CFormulaPart::DoLeadingInFunction);
		m_pWeconSelectWnd->exec();
		delete m_pWeconSelectWnd;
		m_pWeconSelectWnd = nullptr;
	}
#else
	QString sFilePath = QFileDialog::getOpenFileName(this, m_languageInfo[getLanguageType()][Import], "./ ", "*.csv");
	DoLeadingInFunction(sFilePath);
#endif // WIN32
}

void CFormulaPart::ClickLeadingOutButton()
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
#ifndef WIN32
	QString sInitPath, sTurnPath, sInitName, sTurnName;
	if (CHmiMedia::isUSBExsit()) {
		std::string sTempStr;
		CHmiMedia::getUdiskPath(sTempStr);
		sInitPath = QString::fromStdString(sTempStr);
		sInitName = "USB FLASH";
	}
	if (CHmiMedia::isSDExsit()) {
		std::string sTempStr;
		CHmiMedia::getSDPath(sTempStr);
		sTurnPath = QString::fromStdString(sTempStr);
		sTurnName = "SD CARD";
	}
	if ("" == sInitPath) {
		sInitPath = sTurnPath;
		sInitName = sTurnName;
		sTurnName = "";
		sTurnPath = "";
	}
	if ("" == sInitPath) {
		CWeconTable::showMsgWnd("warning", m_languageInfo[getLanguageType()][insertDev], QMessageBox::NoButton, this);
		return;
	}
	{
		QWidget maskLayer(this->parentWidget());
		maskLayer.setFixedSize(this->window()->width(), this->window()->height());
		maskLayer.setStyleSheet("background-color: rgba(0, 0, 0, 0.5);"); // 设置蒙版层的背景色和透明度
		maskLayer.show();
		if (nullptr == m_pWeconSelectWnd){
			m_pWeconSelectWnd = new CWeconSelectWnd(this->getExeWidth(), this->getExeHeight(), CWeconSelectWnd::ExportWnd, sInitPath, CWeconSelectWnd::TypeDocument, sInitName, sTurnName, sTurnPath, this);
		}
		m_pWeconSelectWnd->setWindowModality(Qt::WindowModal);
		connect(m_pWeconSelectWnd, &CWeconSelectWnd::OnSelectResult, this, &CFormulaPart::DoLeadingOutFunction);
		m_pWeconSelectWnd->exec();
	}
#else
	QString sFilePath = QFileDialog::getExistingDirectory(this, m_languageInfo[getLanguageType()][Export], "./");
	DoLeadingOutFunction(sFilePath);
#endif
}

int CFormulaPart::ClickSearchButton()
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
	Json::Value data;
	data["formula"]["type"] = 9;
	EventMgr::SendDataTransEvent(this->getSrcno(), m_sPartName, data.toStyledString());
#endif // !HMIPREVIEW
	return 0;
}

void CFormulaPart::ClickChangeFormulaButton()
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
	QWidget maskLayer(this->parentWidget());
	maskLayer.setFixedSize(this->window()->width(), this->window()->height());
	maskLayer.setStyleSheet("background-color: rgba(0, 0, 0, 0.5);"); // 设置蒙版层的背景色和透明度
	maskLayer.show();
	if (nullptr == m_pFormulaSearchWnd) {
		m_pFormulaSearchWnd = new CFormulaSearchWnd(this->getExeWidth(), this->getExeHeight(), this->getSvgPath(), CFormulaSearchWnd::ChangeWnd, this);
	}
	m_pFormulaSearchWnd->setListInfo(m_searchFormulaList);
	m_pFormulaSearchWnd->init(m_formulaList);
	m_pFormulaSearchWnd->setWindowModality(Qt::WindowModal);
	connect(m_pFormulaSearchWnd, &CFormulaSearchWnd::OnSearchFunction, this, &CFormulaPart::DoSearchFunction);
	m_pFormulaSearchWnd->exec();
	if (m_pFormulaSearchWnd){
		delete m_pFormulaSearchWnd;
		m_pFormulaSearchWnd = nullptr;
	}
}

void CFormulaPart::DoLeadingInFunction(const QString & sPath)
{
	if ("" != sPath) {
#ifndef HMIPREVIEW
		Json::Value data;
		data["formula"]["path"] = sPath.toStdString();
		data["formula"]["type"] = 7;
		EventMgr::SendDataTransEvent(this->getSrcno(), m_sPartName, data.toStyledString());
#endif // !HMIPREVIEW
	}
}

void CFormulaPart::DoLeadingOutFunction(const QString & sPath)
{
	if ("" != sPath) {
#ifndef HMIPREVIEW
		Json::Value data;
		data["formula"] = Json::Value();
		data["formula"]["path"] = sPath.toStdString() + "/";
		data["formula"]["type"] = 6;
		EventMgr::SendDataTransEvent(this->getSrcno(), m_sPartName, data.toStyledString());
#endif // !HMIPREVIEW
	}
}

void CFormulaPart::sendUidAndType(const int & nType)
{
#ifndef HMIPREVIEW
	QModelIndex index = this->getCurrentSelectIndex();
	if (!index.isValid()) {
		return;
	}
	Json::Value data;
	data["formula"]["ids"][0] = this->data(index, CWeconTableModel::rowId).toInt();
	data["formula"]["type"] = nType;
	EventMgr::SendDataTransEvent(this->getSrcno(), m_sPartName, data.toStyledString());
#endif // !HMIPREVIEW
}

QString CFormulaPart::getSgvNameByFuncType(const int & type)
{
	switch (type) {
	case formulaSpace::Formula_Add:
		return "add.svg";
	case formulaSpace::Formula_Delete:
		return "delete.svg";
	case formulaSpace::Formula_Upload:
		return "upload.svg";
	case formulaSpace::Formula_Download:
		return "download.svg";
	case formulaSpace::Formula_LeadingIn:
		return "leadingIn.svg";
	case formulaSpace::Formula_LeadingOUt:
		return "leadingOut.svg";
	case formulaSpace::Formula_Change:
		return "change.svg";
	case formulaSpace::Formula_Max:
		break;
	default:
		return CWeconTable::getSgvNameByFuncType(type);
		break;
	}
	return "";
}

void CFormulaPart::paintEvent(QPaintEvent * event) {
	if (false ==  CSysSetUi::get_instance().isSysSetShow()){
		if (m_pAddFormulaGroupWnd) {
			m_pAddFormulaGroupWnd->raise();
		}

		if (m_pWeconSelectWnd) {
			m_pWeconSelectWnd->raise();
		}

		if (m_pFormulaSearchWnd) {
			m_pFormulaSearchWnd->raise();
		}
	}
	return QWidget::paintEvent(event);
}

void CFormulaPart::getScaleWH()
{
	int nWidth = 0, nHeight = 0;
#ifndef _LINUX_
#ifndef HMIPREVIEW
	HmiGui::GetScaleWH(nWidth, nHeight);
#endif
#else
	int nBitPerPixel = 0;
	UTILITY::CState::getLcdResolutionByHMIUIRot(nWidth, nHeight, nBitPerPixel);
#endif
	this->setExeWidth(nWidth);
	this->setExeHeight(nHeight);
}

void CFormulaPart::sendTurnPageEvent(const int & nNum)
{
#ifndef HMIPREVIEW
	std::string sJson;
	Json::Value data;
	data["formula"] = Json::Value();
	data["formula"]["pagenum"] = nNum;
	data["formula"]["type"] = 3;
	sJson = data.toStyledString();
	EventMgr::SendDataTransEvent(this->getSrcno(), m_sPartName, sJson);
#endif // !HMIPREVIEW
}

int CFormulaPart::sendChangeShowCountEvent(const int & nShowCount)
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
	std::string sJson;
	Json::Value data;
	data["formula"]["pagenum"] = nShowCount;
	data["formula"]["type"] = 11;
	sJson = data.toStyledString();
	EventMgr::SendDataTransEvent(this->getSrcno(), m_sPartName, sJson);
#endif // !HMIPREVIEW
	return 0;
}
