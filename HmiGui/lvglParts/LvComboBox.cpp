#include "LvComboBox.h"
#include <sstream>
#include <iostream>
#include <string>
#include <cbl/cbl.h>
static void event_handler(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED) {
		char buf[32];
		lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
		//LV_LOG_USER("Option: %s", buf);
	}
}

LvComboBox::LvComboBox(const hmiproto::hmidownlist &downlist, HMILvScreen * parent) {
	std::cout << "LvComboBox Construction" << std::endl;
	m_part.Clear();
	//m_comboxFontStyle = "";
	m_strPartType = "";
	m_nScrNo = -1;
	//m_comboxbtn = NULL;
	//m_QStyledItemDelegate = NULL;
	m_part.MergeFrom(downlist);

	/*m_ComboxPart = lv_dropdown_create(lv_scr_act());
	lv_dropdown_add_option(m_ComboxPart, "Apple", LV_DROPDOWN_POS_LAST);*/
}

LvComboBox::~LvComboBox()
{
}


void LvComboBox::Init(lv_obj_t * src)
{
	std::cout << "LvComboBox Init" << std::endl;

	RECT	rc;
	rc.left = m_part.basic().left();
	rc.right = m_part.basic().right();
	rc.top = m_part.basic().top();
	rc.bottom = m_part.basic().bottom();
	m_sPartName = m_part.basic().name();
	string strPartType(m_part.basic().type().c_str());
	m_strPartType = strPartType;

	m_nScrNo = m_part.basic().scrno();

	//λ�ü���С
	int nLeft = rc.left;
	int nTop = rc.top;
	int nWidth = rc.right - rc.left;
	int nHeight = rc.bottom - rc.top;

	m_ComboxPart = lv_dropdown_create(src);
	lv_obj_set_pos(m_ComboxPart, nLeft, nTop);
	lv_obj_set_size(m_ComboxPart, nWidth, nHeight);
	lv_obj_add_event_cb(m_ComboxPart, event_handler, LV_EVENT_ALL, NULL);
	//m_Rect = QRect(nLeft, nTop, nWidth, nHeight);


}

void LvComboBox::Update(const hmiproto::hmidownlist & downlist, HMILvScreen * parent)
{
	std::cout << "LvComboBox Update" << std::endl;

	if (downlist.pencolor_size() > 0) {
		m_part.clear_pencolor();
	}
	m_part.MergeFrom(downlist);

#if ((defined WIN32) && (defined WIN32SCALE))
	RECT	rc;

	rc.left = m_part.basic().left();
	rc.right = m_part.basic().right();
	rc.top = m_part.basic().top();
	rc.bottom = m_part.basic().bottom();
	memcpy(&m_OriginalRect, &rc, sizeof(RECT));
	ZoomPartSelf();
#endif
}

void LvComboBox::Draw(HMILvScreen * parent, int drawtype)
{
	std::cout << "LvComboBox Draw" << std::endl;
	DrawDownList(drawtype);
}

void LvComboBox::ZoomPartSelf()
{
}

void LvComboBox::OnPartShow()
{
}

void LvComboBox::OnPartHide()
{
}

void customSplit(string str, char separator, vector<string> &output)
{
	int startIndex = 0, endIndex = 0;
	for (int i = 0; i <= (int)str.size(); i++)
	{
		// If we reached the end of the word or the end of the input.
		if (str[i] == separator || i == (int)str.size())
		{
			endIndex = i;
			string temp;
			temp.append(str, startIndex, endIndex - startIndex);
			output.push_back(temp);
			startIndex = endIndex + 1;
		}
	}
}

void LvComboBox::DrawDownList(int drawtype)
{
	(void)drawtype;
	if (m_part.basic().hide()) {
	/*	this->hide();
		m_comboxbtn->hide();*/
		//lv_obj_set_hidden(m_ComboxPart, false);
		return;
	}
	else {
		//this->show();
	}

	char	szRowSeparation[2] = { 0x01 };
	string  strOptions(m_part.content().c_str());
	
	cbl::split(strOptions, szRowSeparation, &m_strList);
	// int nNum = m_strList.size();
	std::cout << "m_strList.size() : " << m_strList.size() << std::endl;
	lv_dropdown_clear_options(m_ComboxPart);
	for (auto & option : m_strList) {
		std::cout << "m_strList: " << option << std::endl;

		lv_dropdown_add_option(m_ComboxPart, option.c_str(), LV_DROPDOWN_POS_LAST);
	}

}
