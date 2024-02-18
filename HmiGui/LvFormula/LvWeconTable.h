#ifndef LVWECONTABLE_H
#define LVWECONTABLE_H

#include "cbl/cbl.h"
#include "../config/tableconfig.h"
#include "../config/tableconfig.h"
#include "../lvglParts/HMILvScreen.h"
#include <lvgl/lvgl/lvgl.h>


#include <map>
#include <string>
#include <vector>

using std::string;
using std::vector;

class LvWeconTable {

public:
	enum LvLanguageType {
		EnglishLanguage = 0,
		ChineseLanguage,
		LanguageTypeMax,
	};

	using FUNC_ON_SEEARCH_BUTTON_CLICK
		= function<int()>;
	using FUNC_ON_TURN_PAGE
		= function<void(const int & nPageNum)>;
	using FUNC_ON_CELL_FINISHEDIT
		= function<int(const int & nId, const std::string & sHead, const std::string & sData)>;
	using FUNC_ON_CHANGE_SHOWCOUNT
		= function<int(const int & nShowCount)>;
	using FUNC_ON_TIPS_MOVEMENT
		= function<int()>;

public:
	LvWeconTable(HMILvScreen *  parent = nullptr);
	LvWeconTable(TableSpace::TABLESTYLE & style, HMILvScreen *parent = nullptr);
	~LvWeconTable();

	int init(TableSpace::TABLESTYLE & style);
	int initPosition();

	int updateLockImage(const string & imagePath);
	int updateLockRect();
	int hideLockImage();



	int setFormulaEnable(bool bIsEnable);

	int setHeadData(vector<string> & headDatas);

	bool steDate(int index, const string &value);
	bool steDate(vector<string> & dates);

	int getLanguageType();
	void setLanguageType(LvWeconTable::LvLanguageType & type);
	virtual int initLanguageInfo();


protected:
	virtual void getScaleWH();

private:
	lv_obj_t * m_backGround;
	lv_obj_t * m_mainLayout;
	lv_obj_t * m_headLayout;
	lv_obj_t * m_bottomLayout;
	lv_obj_t * m_table;

	lv_obj_t * m_model;
	lv_obj_t * m_tableHeadName;

	map<int, lv_obj_t *> m_functionBtn;

	lv_obj_t * m_onePageShowCount;
	lv_obj_t * m_upPage;
	lv_obj_t * m_downPage;
	lv_obj_t * m_expandBtn;
	lv_obj_t * m_searchBtn;
	lv_obj_t * m_pageNum;


	string m_svgPath;

	lv_obj_t * m_lockImg;

	int m_nScrno;
	bool m_bIsExpand;
	int m_nPositionId;
	bool m_bIsSendChangePageShowCount;
	bool m_bIsDisablesCellEditing;
	static int m_nWidth;
	static int m_nHeight;
	int m_lastClickIndex;
	string m_lastEditText;
	int m_nVHeadWidth;
	LvLanguageType m_languageType;

private:

	FUNC_ON_TURN_PAGE m_turnPageFunc;
	FUNC_ON_CELL_FINISHEDIT m_cellFinishEditFunc;
	FUNC_ON_SEEARCH_BUTTON_CLICK m_searchBtnClickFunc;
	FUNC_ON_CHANGE_SHOWCOUNT m_changeShowCountFunc;
};
 

#endif