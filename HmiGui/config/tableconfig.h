#ifndef _TABLECONFIG_H_
#define _TABLECONFIG_H_

#include <map>
#include <vector>
#include <iostream>

#ifdef WIN32
#		define CONTROLPREVIEW_OUT __declspec(dllexport)
#else
#		define CONTROLPREVIEW_OUT
#endif

namespace TableSpace {
	enum LanguageType {
		Language_Chinese = 0,
		Language_English = 1,
		Language_Max,
	};

	enum funcType {
		Table_UpPage = 100,				//turn page up
		Table_DownPage = 101,			//turn page down
		Table_Search = 102,				//search data by group name
		Table_Expand = 103,				//expand other button
		Table_Constriction = 104,       //Constriction other button
		Table_Max
	};

	typedef struct tagDataformat {
		unsigned int nDataFormat;
		unsigned int nLength;
		std::string sMax;
		std::string sMin;
		unsigned int nDecimal;
	} DATAFORMAT_T;

	typedef struct tagFunctionStyle {
		int m_nCode;                       //code of function
		std::string m_sDesc;                    //describe of function
	} FUNCSTYLE;

	typedef struct tagTableStyle {

		std::string m_sName;                    //table name
		int m_nNameColor;                       //table name color
		std::string m_sImgPath;                 //table button image path(not include image name)
		int m_nHTableHeadColor;                 //horizontal head background color
		int m_nVTableHeadColor;                 //vertical head background color
		int m_nRowBgColor1;                     //table row alternate background color1
		int m_nRowBgColor2;                     //table row alternate background color2
		int m_nCellWidth;                       //table cell width(0：self-adaption)
		int m_nCellHeight;                      //table cell height(0：self-adaption)
		int m_nLeft;                            //table rect position of left
		int m_nTop;                             //table rect position of top
		int m_nWidth;                           //table rect position of width
		int m_nHeight;                          //table rect position of height
		int m_nTransparent;                     //table background transparent
		int m_nLineColor;                       //table grid line color
		int m_nLineWidth;                       //table grid line width
		int m_nSelectColor;                     //table select color
		int m_nHHeadTextColor;                  //horizontal head text color
		int m_nVHeadTextColor;                  //vertical head text color
		int m_nTableTextColor;                  //table data text color
		int m_nBackgroundColor;                 //table background color
		std::string m_sBackgroundImage;			//table background image (background must use image, if m_sBackgroundImage not`t empty. use image transparent isn`t use)
		int m_nTotalPage;                       //total page
		int m_nCurrentPage;                     //current page
		int m_nTableFontSize;
		int m_nPrjWidth;
		int m_nPrjHeight;
		bool m_bIsDisablesCellEditing;			// Disables cell editing
		std::vector<FUNCSTYLE> m_funcs;

		tagTableStyle() {
			TableSpace::FUNCSTYLE funcStyle;

			this->m_nTableFontSize = 12;
			this->m_nLeft = 0;
			this->m_nTop = 0;
			this->m_nWidth = 400;
			this->m_nHeight = 400;
			this->m_sImgPath = "";
			this->m_sName = "";
			this->m_nNameColor = 0x000000;
			this->m_nHHeadTextColor = 0xF00000;
			this->m_nVHeadTextColor = 0xFFFFFF;
			this->m_nHTableHeadColor = 0xF000FF;
			this->m_nVTableHeadColor = 0xF00FF0;
			this->m_nLineColor = 0;
			this->m_nLineWidth = 2;
			this->m_nCellHeight = 50;
			this->m_nCellWidth = 100;
			this->m_nCurrentPage = 10;
			this->m_nTotalPage = 100;
			this->m_nRowBgColor1 = 0x78A140;
			this->m_nRowBgColor1 = 0x10A104;
			this->m_nSelectColor = 0x00FFFF;

			this->m_nBackgroundColor = 0xF0FFF0;
			this->m_nPrjWidth = 0;
			this->m_nPrjHeight = 0;
		}
	} TABLESTYLE;

}

#endif
