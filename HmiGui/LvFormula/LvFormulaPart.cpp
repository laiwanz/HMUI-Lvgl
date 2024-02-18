#include "./LvFormulaPart.h"                           
#include "../HmiGui/HmiGui.h"
#include "../EventMgr/EventMgr.h"

LvFormulaPart::LvFormulaPart(const hmiproto::cformula & protoData, HMILvScreen * parent)
	:LvWeconTable()
{
	m_part.Clear();
	m_part.MergeFrom(protoData);
	m_nInsertPositon = 2;
	std::cout << "LvFormulaPart Construction" << std::endl;


}

LvFormulaPart::~LvFormulaPart()
{
}

void LvFormulaPart::Init(lv_obj_t * parent)
{
	std::cout << "LvFormulaPart::Init " << std::endl;
	// bool bIsLock = false;
	std::string sBaseDir, sLockImage;
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
			//setSrcno(basic.scrno());
		}
		if (basic.has_name()) {
			m_sPartName = basic.name();
		}
		if (basic.has_msgtiplanguage()) {
			if (basic.msgtiplanguage() == 1) {
				//formulaLanguageType = CWeconTable::EnglishLanguage;
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
			// bIsLock = tableStyle.islock();
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
		addFormula((m_part.listname()[i]));
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
	// style.m_sImgPath = getPrjDir() + "img/formula/" + sBaseDir;
	//CWeconTableModel::g_sFormulaImagePath = style.m_sImgPath;
	init(style);
}

void LvFormulaPart::Update(const hmiproto::cformula & protoData, HMILvScreen * pWidget)
{
}

void LvFormulaPart::Draw(HMILvScreen * pWidget, int drawtype)
{
}

int LvFormulaPart::parseJson(const std::string & sbuffer)
{
	return 0;
}


int LvFormulaPart::initLanguageInfo()
{
	return 0;
}

int LvFormulaPart::AddSearchGroupList(const string & sSearchText)
{
	return 0;
}

int LvFormulaPart::AddSearchFormulaList(const string & sSearchText)
{
	return 0;
}

int LvFormulaPart::ClearSearchGroupList()
{
	return 0;
}

int LvFormulaPart::ClearSearchFormulaList()
{
	return 0;
}

int LvFormulaPart::addFormula(const string & sName)
{
	return 0;
}

int LvFormulaPart::clearFormula()
{
	return 0;
}

int LvFormulaPart::CellFinishEditFunc(const int & nId, const std::string & sHead, const std::string & sData)
{
	return 0;
}

void LvFormulaPart::OnUpdateOnePageShowCount()
{
}

void LvFormulaPart::DoAddGroupFunction(int nPosition, const string & sName)
{
}

void LvFormulaPart::ClickAddDataButton()
{
}

void LvFormulaPart::ClickDeleteButton()
{
}

void LvFormulaPart::ClickDownloadButton()
{
}

void LvFormulaPart::ClickUploadButton()
{
}

void LvFormulaPart::ClickLeadingInButton()
{
}

void LvFormulaPart::ClickLeadingOutButton()
{
}

int LvFormulaPart::ClickSearchButton()
{
	return 0;
}

void LvFormulaPart::ClickChangeFormulaButton()
{
}

void LvFormulaPart::DoLeadingInFunction(const string & sPath)
{
}

void LvFormulaPart::DoLeadingOutFunction(const string & sPath)
{
}

void LvFormulaPart::getScaleWH()
{
}

void LvFormulaPart::sendUidAndType(const int & nType)
{
}

string LvFormulaPart::getSgvNameByFuncType(const int & type)
{
	return string();
}


void LvFormulaPart::sendTurnPageEvent(const int & nNum)
{
}

int LvFormulaPart::sendChangeShowCountEvent(const int & nShowCount)
{
	return 0;
}
