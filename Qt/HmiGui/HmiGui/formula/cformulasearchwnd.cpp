#include "cformulasearchwnd.h"

#include <QLabel>
#include <QFrame>
#include <QLayout>
#include <QKeyEvent>
#include <QListWidget>
#include <QSpacerItem>
#include <QPushButton>

#include "csvgbutton.h"
#include "cchangepageedit.h"
#include "cweconclearedit.h"
#include "cformulapart.h"
#include "cwecontablemodel.h"

#ifndef HMIPREVIEW
#ifdef _LINUX_
#include "btl/btl.h"
#endif // _LINUX_
#endif // !HMIPREVIEW

CFormulaSearchWnd::CFormulaSearchWnd(const int &nWidth, const int &nHeight, const QString & sPath, WndMode wndType, QWidget *parent, Qt::WindowFlags f)
	:QDialog(parent->parentWidget(), Qt::FramelessWindowHint)
{
	m_paretnptr = parent;
	auto parentptr = (CWeconTable *)m_paretnptr;
	if (nullptr != parentptr) {

		m_languageType = parentptr->getLanguageType();
	}
	else {

		m_languageType = 1;
	}
	QPoint ponint = parent->parentWidget()->mapToGlobal(QPoint(0, 0));
	if (nWidth > nHeight) {
		m_dWidthScale = (double)nWidth / 800;
		m_dHeightScale = (double)nHeight / 480;
		this->resize(450 * m_dWidthScale, 320 * m_dHeightScale);
		this->move(ponint.x() +  175 * m_dWidthScale, ponint.y() + 80 * m_dHeightScale);
	}
	else {
		m_dWidthScale = (double)nWidth / 480;
		m_dHeightScale = (double)nHeight / 800;
		this->resize(450 * m_dWidthScale, 320 * m_dHeightScale);
		this->move(ponint.x() + 15 * m_dWidthScale, ponint.y() + 240 * m_dHeightScale);
	}
	this->setFocus();
	this->setWindowTitle("search");
	int nFontSize = nHeight * 0.03;
	if (nWidth < nHeight) {
	
		nFontSize = nWidth * 0.03;
	}
	if (nFontSize < 8) {
		nFontSize = 8;
	}
	QString sStyle = "QDialog {background-color:rgb(255,255,255);border: 1px solid #888888;}";
	sStyle += "QLabel{font-size:" + QString::number(nFontSize) + "px;}";
	sStyle += "QPushButton{font-size:" + QString::number(nFontSize) + "px;}";
	sStyle += "QListWidget{font-size:" + QString::number(nFontSize) + "px;}";
	sStyle += "QLineEdit{font-size:" + QString::number(nFontSize) + "px;}";
	this->setStyleSheet(sStyle);
	m_nPageCount = 10;
	m_wndType = wndType;
    QSpacerItem * tempSpace = nullptr;
    m_sPath = sPath;

    //init layout
    m_mainLayout = new QVBoxLayout(this);
    m_closeLayout = new QHBoxLayout();
	m_searchLayout = new QHBoxLayout();
    m_labGroupLayout = new QHBoxLayout();
    m_pageLayout = new QHBoxLayout();

	tempSpace = new QSpacerItem(1, 1, QSizePolicy::Expanding);
	m_closeWnd = new CSvgButton(QString::fromStdString(CWeconTableModel::g_sFormulaImagePath + "/closeWnd.svg"), this);
	m_closeWnd->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_closeWnd->setMinimumSize(1,1);
	m_closeLayout->addSpacerItem(tempSpace);
	m_closeLayout->addWidget(m_closeWnd);
	connect(m_closeWnd, &CSvgButton::clicked, this, &CFormulaSearchWnd::OnClickClose);
	tempSpace = new QSpacerItem(1, 1, QSizePolicy::Expanding);
	m_closeLayout->addSpacerItem(tempSpace);
	m_closeLayout->setStretch(0, 352);
    m_closeLayout->setStretch(1, 30);
	m_closeLayout->setStretch(2, 1);
    m_mainLayout->addLayout(m_closeLayout);

    m_searchEdit = new CWeconClearEdit(sPath, "clearInput-click.svg",this);
	m_searchEdit->setStyleSheet("background-color:rgb(239,244,248);border: 0px solid #888888;border-radius: 5px;");
	m_searchEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sStyle = "font-size:" + QString::number(nFontSize) + "px;";
	m_searchEdit->setDownListStyle(sStyle);
	m_searchEdit->setMinimumSize(1,1);
	m_searchBtn = new CSvgButton(QString::fromStdString(CWeconTableModel::g_sFormulaImagePath + "/search.svg"), this);
	m_searchBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_searchBtn->setMinimumSize(1,1);
	tempSpace = new QSpacerItem(1, 1, QSizePolicy::Expanding);
	m_searchLayout->addSpacerItem(tempSpace);
	m_searchLayout->addWidget(m_searchEdit);
	tempSpace = new QSpacerItem(1, 1, QSizePolicy::Expanding);
	m_searchLayout->addSpacerItem(tempSpace);
	m_searchLayout->addWidget(m_searchBtn);
	tempSpace = new QSpacerItem(1, 1, QSizePolicy::Expanding);
	m_searchLayout->addSpacerItem(tempSpace);
	m_searchLayout->setStretch(0, 1);
    m_searchLayout->setStretch(1, 350);
	m_searchLayout->setStretch(2, 1);
	m_searchLayout->setStretch(3, 30);
    m_searchLayout->setStretch(4, 1);
    m_mainLayout->addLayout(m_searchLayout);
	
	connect(m_searchBtn, &QPushButton::clicked, this, &CFormulaSearchWnd::OnSearch);
	connect(m_searchEdit, &CWeconClearEdit::OnClearText, this, &CFormulaSearchWnd::OnSearch);

    m_mainLayout->addLayout(m_labGroupLayout);
    m_labAllGroup = new QLabel(this);
	m_labAllGroup->setStyleSheet("color:rgb(41,126,239)");
	m_labAllGroup->setText(CFormulaPart::m_languageInfo[(CWeconTable::LanguageType)m_languageType][CFormulaPart::allgroup]);
	m_labAllGroup->setMinimumSize(1, 1);
    m_labGroupLayout->addWidget(m_labAllGroup);

    QVBoxLayout * tempLayout = new QVBoxLayout();
    m_mainLayout->addLayout(tempLayout);
    tempSpace = new QSpacerItem(1,1,QSizePolicy::Expanding, QSizePolicy::Expanding);
    tempLayout->addSpacerItem(tempSpace);
	m_infoList = new QListWidget(this);
	sStyle = "QListWidget{border:0px}QListWidget::item{border-bottom: 1px solid rgb(239,244,248);height:" + QString::number(30 * m_dHeightScale) + "px;}QListWidget::item:selected{color:black;background-color:rbg(204,232,255);}";
	m_infoList->setStyleSheet(sStyle);
    tempLayout->addWidget(m_infoList);
	m_infoList->setMinimumSize(1, 1);
	tempLayout->setStretch(0, 2);
    tempLayout->setStretch(1, 200);
	connect(m_infoList, &QListWidget::clicked, this, &CFormulaSearchWnd::OnClickGroup);

    m_mainLayout->addLayout(m_pageLayout);
    m_pageDown = new CSvgButton(sPath + "downPage.svg", this);
	m_pageDown->setMinimumSize(1,1);
    m_pageUp = new CSvgButton(sPath + "upPage.svg", this);
	m_pageUp->setMinimumSize(1, 1);
    m_pageEdit = new CChangePageEdit(1, 1);
	m_pageEdit->setInputMethodHints(Qt::ImhDigitsOnly);
    m_pageEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_pageEdit->setAlignment(Qt::AlignCenter);
	m_pageEdit->setMinimumSize(30,1);
    tempSpace = new QSpacerItem(1,1,QSizePolicy::Expanding);
    m_pageLayout->addSpacerItem(tempSpace);
    m_pageLayout->addWidget(m_pageUp);
	tempSpace = new QSpacerItem(1,1,QSizePolicy::Expanding);
    m_pageLayout->addSpacerItem(tempSpace);
    m_pageLayout->addWidget(m_pageEdit);
	tempSpace = new QSpacerItem(1,1,QSizePolicy::Expanding);
    m_pageLayout->addSpacerItem(tempSpace);
    m_pageLayout->addWidget(m_pageDown);
	tempSpace = new QSpacerItem(1,1,QSizePolicy::Expanding);
    m_pageLayout->addSpacerItem(tempSpace);
	m_pageLayout->setStretch(0, 320);
    m_pageLayout->setStretch(1, 25);
	m_pageLayout->setStretch(2, 1);
	m_pageLayout->setStretch(3, 50);
    m_pageLayout->setStretch(4, 1);
	m_pageLayout->setStretch(5, 25);
	m_pageLayout->setStretch(6, 1);

	m_mainLayout->setStretch(0, 4);
    m_mainLayout->setStretch(1, 4);
	m_mainLayout->setStretch(2, 3);
	m_mainLayout->setStretch(3, 20);
    m_mainLayout->setStretch(4, 4);
	connect(m_pageDown, &CSvgButton::clicked, this, &CFormulaSearchWnd::OnClickPageDown);
	connect(m_pageUp, &CSvgButton::clicked, this, &CFormulaSearchWnd::OnClickPageUp);
	connect(m_pageEdit, &::CChangePageEdit::editingFinished, this, &CFormulaSearchWnd::OnTurnToPageNum);
}

void CFormulaSearchWnd::init(QVector<SearchData> & datas)
{
	int nTotalPage = datas.size() / m_nPageCount;
	if (datas.size() % m_nPageCount != 0) {
	
		nTotalPage += 1;
	}
	m_pageEdit->setTotalPage(nTotalPage);

	m_totalDatas = datas;
	m_showTotalDatas = m_totalDatas;
	updateList();
}

void CFormulaSearchWnd::updateList()
{
	int i = 0;

	int nTotalPage = m_showTotalDatas.size() / m_nPageCount;
	if (m_showTotalDatas.size() % m_nPageCount != 0) {

		nTotalPage += 1;
	}
	m_pageEdit->setTotalPage(nTotalPage);

	int nCurPage = m_pageEdit->getCurrentPage();
	m_infoList->clear();

	auto it = m_showTotalDatas.begin();
	
	if (nCurPage > 1) {
	
		it += (nCurPage - 1) * m_nPageCount;
	}
	
	for (; it != m_showTotalDatas.end(); it++) {

		if (i > (m_nPageCount - 1)) {

			break;
		}
	
		QListWidgetItem * item = new QListWidgetItem(m_infoList);
		item->setText(it->m_sShowName);
		item->setWhatsThis(it->m_id.toString());
		m_infoList->addItem(item);

		i++;
	}
}

int CFormulaSearchWnd::setListInfo(const QStringList & searchHistory) {

	m_searchList.clear();
	m_searchList = searchHistory;
	m_searchEdit->setListInfo(m_searchList);
	return 0;
}

QVariant CFormulaSearchWnd::getResult()
{
	return m_result;
}

void CFormulaSearchWnd::OnClickGroup()
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
	auto itemIndex = m_infoList->currentIndex();
	if (itemIndex.isValid() && m_lastClickItem == itemIndex) {
	
		auto item = m_infoList->currentItem();
		if (nullptr != item) {
		
			if (SearchWnd == m_wndType) {

				m_result = item->whatsThis();
			}
			else {

				m_result = item->text();
			}
			int nType = (int)m_wndType;
			OnSearchFunction(nType, m_result);
			this->close();
		}
	}
	else {
	
		this->m_lastClickItem = itemIndex;
	}
	
}

void CFormulaSearchWnd::OnClickPageDown()
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
	auto nCurPage = m_pageEdit->getCurrentPage() + 1;
	auto nTotalPage = m_pageEdit->getTotalPage();
	if (nCurPage > nTotalPage) {
		nCurPage = nTotalPage;
	}
	m_pageEdit->setCurrentPage(nCurPage);
	updateList();
}

void CFormulaSearchWnd::OnClickPageUp()
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
	auto nCurPage = m_pageEdit->getCurrentPage() - 1;
	if (nCurPage < 1) {
	
		nCurPage = 1;
	}
	m_pageEdit->setCurrentPage(nCurPage);
	updateList();
}

void CFormulaSearchWnd::OnTurnToPageNum()
{
	updateList();
}

void CFormulaSearchWnd::OnClickClose()
{
#ifndef HMIPREVIEW
#ifdef _LINUX_
	btl::beep(true);
#endif
#endif // !HMIPREVIEW
	this->close();
}

void CFormulaSearchWnd::OnSearch()
{
	auto sSearchStr = m_searchEdit->text();
	if (m_sLastSearchText == sSearchStr && "" != sSearchStr) {
	
		return;
	}

	m_sLastSearchText = sSearchStr;
	m_showTotalDatas.clear();

	auto it = m_totalDatas.begin();
	for (; it != m_totalDatas.end(); it++) {

		auto sText = it->m_sShowName;
		if (sText.contains(sSearchStr)) {

			m_showTotalDatas.push_back(*it);
		}
	}

	m_pageEdit->setCurrentPage(1);

	auto paretnptr = (CFormulaPart *)m_paretnptr;
	if ("" != sSearchStr && nullptr != paretnptr) {
		if (m_searchList.indexOf(sSearchStr) < 0) {
		
			if (m_searchList.size() > 3) {

				m_searchList.pop_front();
			}
			m_searchList.push_back(sSearchStr);
			m_searchEdit->setListInfo(m_searchList);
		}
	
		if (m_wndType == SearchWnd) {
		
			paretnptr->AddSearchGroupList(sSearchStr);
		}
		else {
		
			paretnptr->AddSearchFormulaList(sSearchStr);
		}
	}
	updateList();
}

void CFormulaSearchWnd::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {

		return;
	}
	QDialog::keyPressEvent(e);
}

int CFormulaSearchWnd::getIdByButton(QPushButton * btn)
{
	auto item = m_infoList->currentItem();
	if(nullptr != item) {
	
		m_result = item->whatsThis();
		return 0;
	}
	return -1;
}

QStringList CFormulaSearchWnd::getSearchHistory()
{
	return m_searchList;
}
