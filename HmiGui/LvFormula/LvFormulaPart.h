#ifndef LVFORMULAPART_H
#define LVFORMULAPART_H

#include "../lvglParts/HMILvScreen.h"
#include "../lvglParts/HMILvPart.h"
#include <string>
#include <vector>
#include "LvWeconTable.h"
#include "../lvglParts/LvGeneralPart.h"
#include "../config/formulaconfig.h"

using std::string;
using std::vector;

class LvFormulaPart : public HMILvPart, public LvWeconTable {

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

public:
	LvFormulaPart(const hmiproto::cformula & protoData, HMILvScreen *parent = nullptr);
	~LvFormulaPart();
	virtual void Init(lv_obj_t* scr);

	virtual void Update(const hmiproto::cformula &protoData, HMILvScreen * parent);

	virtual void Draw(HMILvScreen * parent, int drawtype = DRAW_INIT);
	virtual int parseJson(const std::string &sbuffer);
#if ((defined WIN32) && (defined WIN32SCALE))
	//�������Ų���
	virtual void ZoomPartSelf();
#endif

	int initLanguageInfo();


	int initFuncBtnConnect(lv_obj_t* btn, const int & nFuncCode);
	int initSearchBtnConnect(lv_obj_t* btn);

	int AddSearchGroupList(const string & sSearchText);
	int AddSearchFormulaList(const string & sSearchText);
	int ClearSearchGroupList();
	int ClearSearchFormulaList();


	int addFormula(const string & sName);
	int clearFormula();

	int CellFinishEditFunc(const int & nId, const std::string & sHead, const std::string & sData);


public:
	void OnUpdateOnePageShowCount();
	void DoAddGroupFunction(int nPosition, const string &sName);
	void ClickAddDataButton();
	void ClickDeleteButton();
	void ClickDownloadButton();
	void ClickUploadButton();
	void ClickLeadingInButton();
	void ClickLeadingOutButton();

	int ClickSearchButton();
	void ClickChangeFormulaButton();
	void DoLeadingInFunction(const string & sPath);
	void DoLeadingOutFunction(const string & sPath);

protected:
	virtual void getScaleWH();
private:
	void sendUidAndType(const int & nType);
	string getSgvNameByFuncType(const int & type);
	void paintEvent(lv_event_t *event);
public:
	void sendTurnPageEvent(const int & nNum);
	int sendChangeShowCountEvent(const int & nShowCount);
private:
	hmiproto::cformula m_part;
	lv_obj_t * m_changeBtn;
	int m_nInsertPositon;
	vector<string> m_searchGroupList;
	vector<string> m_searchFormulaList;

};


#endif