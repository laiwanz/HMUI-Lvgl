#ifndef _CFORMULAPART_H_
#define _CFORMULAPART_H_

#include "cwecontable.h"
#include "../generalpart.h"
#include "../config/formulaconfig.h"
#include "AddFormulaGroupWnd/AddFormulaGroupWnd.h"
#include "cweconselectwnd.h"
#include "cformulasearchwnd.h"

class CFormulaPart :public CWeconTable, public HMIPart{

public:
	enum LanguageCode {
		exceedPage = 0,
		selcetRow,
		dataExceed,
		stringExceed,
		search,
		add,
		Delete,
		upload,
		download,
		Import,
		Export,
		formula,
		prepage,
		nextpage,
		preline,
		nextline,
		intop,
		inbottom,
		tens,
		twenty,
		thirty,
		fifty,
		hundred,
		allgroup,
		clear,
		pleaseInput,
		pleaseSelect,
		insertDev,
		groupIsEmpty,
		Cancel,
		Determine,
		Folder,
		CreateFolder,
		Tips,
		DeleteLine,
		CreateFailed,
		LanguageCodeMax,
	};

	Q_OBJECT
public :
	CFormulaPart(const hmiproto::cformula & protoData, QWidget *parent = nullptr);
	//初始化数据
	virtual void Init(QWidget * pWidget);
	//收到更新数据
	virtual void Update(const hmiproto::cformula &protoData, QWidget * pWidget);
	//绘图
	virtual void Draw(QWidget * pWidget, int drawtype = DRAW_INIT);
	virtual int parseJson(const std::string &sbuffer);
#if ((defined WIN32) && (defined WIN32SCALE))
	//比例缩放部件
	virtual void ZoomPartSelf();
#endif

	int initLanguageInfo();

	int initFuncBtnConnect(QPushButton * btn, const int & nFuncCode);
	int initSearchBtnConnect(QPushButton * btn);

	int AddSearchGroupList(const QString & sSearchText);
	int AddSearchFormulaList(const QString & sSearchText);
	int ClearSearchGroupList();
	int ClearSearchFormulaList();

	int addFormula(const QString & sName);
	int clearFormula();

	int CellFinishEditFunc(const int & nId, const std::string & sHead, const std::string & sData);

public:
	void OnUpdateOnePageShowCount();
	void DoAddGroupFunction(int nPosition, const QString &sName);
	void OnShowSearchWnd(QVector<CFormulaSearchWnd::SearchData> & datas);
	void DoSearchFunction(int & nWndType, QVariant & id);

	void ClickAddDataButton();
	void ClickDeleteButton();
	void ClickDownloadButton();
	void ClickUploadButton();
	void ClickLeadingInButton();
	void ClickLeadingOutButton();

	int ClickSearchButton();
	void ClickChangeFormulaButton();

	void DoLeadingInFunction(const QString & sPath);
	void DoLeadingOutFunction(const QString & sPath);
signals:
	void showSearchWnd(QVector<CFormulaSearchWnd::SearchData> & datas);
protected:
	virtual void getScaleWH();
private:
	void sendUidAndType(const int & nType);
	QString getSgvNameByFuncType(const int & type);
	void paintEvent(QPaintEvent *event);
public:
	void sendTurnPageEvent(const int & nNum);
	int sendChangeShowCountEvent(const int & nShowCount);
private:
	hmiproto::cformula m_part;
	CSvgButton * m_changeBtn;
	int m_nInsertPositon;
	QStringList m_searchGroupList;
	QStringList m_searchFormulaList;
	CAddFormulaGroupWnd * m_pAddFormulaGroupWnd;
	CWeconSelectWnd * m_pWeconSelectWnd;
	CFormulaSearchWnd * m_pFormulaSearchWnd;
	QVector<CFormulaSearchWnd::SearchData> m_formulaList;
public:
	static QString m_languageInfo[LanguageTypeMax][LanguageCodeMax];
};

#endif