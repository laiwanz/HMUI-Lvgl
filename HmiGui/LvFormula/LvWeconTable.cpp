#include "LvWeconTable.h"
#include <lvgl/lvgl/src/extra/libs/stb_image/stb_image.h>
#include <iostream>


int LvWeconTable::m_nWidth = 0;
int LvWeconTable::m_nHeight = 0;

LvWeconTable::LvWeconTable(HMILvScreen * parent)
{
	std::cout << "LvWeconTable Construction" << std::endl;

	getScaleWH();
}

LvWeconTable::LvWeconTable(TableSpace::TABLESTYLE & style, HMILvScreen * parent)
{
	std::cout << "LvWeconTable Init Construction" << std::endl;

	init(style);
	getScaleWH();
}

LvWeconTable::~LvWeconTable()
{
}

int LvWeconTable::init(TableSpace::TABLESTYLE & style)
{
	std::cout << "LvWeconTable::init " << std::endl;
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
	style.m_nTableFontSize = nFontSize; // ��ʱ����
	// double dWidthScale, dHeightScale;
	// if (m_nWidth > m_nHeight) {
	// 	dWidthScale = (double)m_nWidth / 800;
	// 	dHeightScale = (double)m_nHeight / 480;
	// }
	// else {
	// 	dWidthScale = (double)m_nWidth / 480;
	// 	dHeightScale = (double)m_nHeight / 800;
	// }
	if (!initFormulaStatic) {

		initLanguageInfo();
		initFormulaStatic = true;
	}

	m_bIsDisablesCellEditing = style.m_bIsDisablesCellEditing;
	m_nPositionId = -1;

	// lv_obj_t * tempBtn = nullptr;

	string sSvgName;
	string sStyle;
	m_bIsExpand = true;
	m_nVHeadWidth = 0;

	if (style.m_sBackgroundImage != "") {
		m_backGround = lv_canvas_create(lv_scr_act());
		int nWidth, nHeight, nChannel;
		unsigned char* Data = stbi_load(style.m_sBackgroundImage.c_str(), &nWidth, &nHeight, &nChannel, 4);
		lv_canvas_set_buffer(m_backGround, Data, nWidth, nHeight, LV_IMG_CF_TRUE_COLOR_ALPHA);
	}
	else {

	}

	if (style.m_sImgPath[style.m_sImgPath.size() - 1] != '\\') {
		m_svgPath = (style.m_sImgPath) + "/";
	}
	else if (style.m_sImgPath[style.m_sImgPath.size() - 1] != '/') {
		m_svgPath = (style.m_sImgPath.substr(0, style.m_sImgPath.size() - 1)) + "/";
	}
	else {
		m_svgPath = (style.m_sImgPath);
	}

	lv_obj_set_pos(m_backGround, style.m_nLeft, style.m_nTop);
	lv_obj_set_size(m_backGround, style.m_nWidth, style.m_nHeight);

	m_table = lv_table_create(m_backGround);
	lv_table_set_cell_value(m_table, 0, 0, "Name");
	lv_table_set_cell_value(m_table, 1, 0, "Apple");
	lv_table_set_cell_value(m_table, 2, 0, "Banana");
	lv_table_set_cell_value(m_table, 3, 0, "Lemon");
	lv_table_set_cell_value(m_table, 4, 0, "Grape");
	lv_table_set_cell_value(m_table, 5, 0, "Melon");
	lv_table_set_cell_value(m_table, 6, 0, "Peach");
	lv_table_set_cell_value(m_table, 7, 0, "Nuts");
	lv_table_set_cell_value(m_table, 0, 1, "Price");
	lv_table_set_cell_value(m_table, 1, 1, "$7");
	lv_table_set_cell_value(m_table, 2, 1, "$4");
	lv_table_set_cell_value(m_table, 3, 1, "$6");
	lv_table_set_cell_value(m_table, 4, 1, "$2");
	lv_table_set_cell_value(m_table, 5, 1, "$5");
	lv_table_set_cell_value(m_table, 6, 1, "$1");
	lv_table_set_cell_value(m_table, 7, 1, "$9");

	return 0;
}

int LvWeconTable::initPosition()
{
	return 0;
}

int LvWeconTable::updateLockImage(const string & imagePath)
{
	return 0;
}

int LvWeconTable::updateLockRect()
{
	return 0;
}

int LvWeconTable::hideLockImage()
{
	return 0;
}

int LvWeconTable::setFormulaEnable(bool bIsEnable)
{
	return 0;
}

int LvWeconTable::setHeadData(vector<string>& headDatas)
{
	return 0;
}

bool LvWeconTable::steDate(int index, const string & value)
{
	return false;
}

bool LvWeconTable::steDate(vector<string>& dates)
{
	return false;
}

int LvWeconTable::getLanguageType()
{
	return 0;
}

void LvWeconTable::setLanguageType(LvWeconTable::LvLanguageType & type)
{
}

int LvWeconTable::initLanguageInfo()
{
	return 0;
}

void LvWeconTable::getScaleWH()
{
}
