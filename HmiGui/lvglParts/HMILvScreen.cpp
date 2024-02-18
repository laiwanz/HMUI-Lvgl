#include "HMILvScreen.h"
#include <algorithm>
#include <ctime>
#include "EventMgr/EventMgr.h"
#include <utility/utility.h>
#include "macrosplatform/registerAddr.h"
#include "RegisterOpt/RegisterOpt.h"
#include "macrosplatform/event.h"
#include "../lvglParts/LvComboBox.h"
#include "../lvglParts/LvGeneralPart.h"
#include "../lvglParts/LvCSlider.h"
#include "../lvglParts/LvCustomPart.h"
#include "../lvglParts/LvWeatherUi.h"
#include "../LvFormula/LvFormulaPart.h"
#include "lvinputControls/LvInputControls.h"
#include "lvglParts/HMILvMgr.h"

int ScrNo = 0;

int HMILvScreen::m_x = -1;
int HMILvScreen::m_y = -1;
int HMILvScreen::m_nClickScrNo = -2;
int HMILvScreen::m_nScrNo = -2;
#ifdef _LINUX_
// unsigned long long     HMIScreen::m_lastClick = 0;   //���һ�λ�ȡ����ʱ��
#endif

HMILvScreen::HMILvScreen(lv_obj_t *src)
{
	m_Screen = src;
	m_nScrNo = -2;                  //�����
	m_wScrType = 0;					//��������

	m_ScrInfo.Clear();
	m_nImgIndex = -1;
	lv_obj_add_event_cb(m_Screen, Event, LV_EVENT_ALL, NULL);

	m_partlist.clear();
	m_bClick = false;
	m_bShow = false;
}

HMILvScreen::~HMILvScreen()
{
	if (m_bClick) {// �е��
		//SendClickEvent(m_nClickScrNo, m_x, m_y, false);
	}
	for(HMILvPart *part : m_partlist) {
		delete part;
		part = NULL;
	}
	m_partlist.clear();
}

void HMILvScreen::paintEvent()
{
}

bool HMILvScreen::CheckInShowBaseScr(const hmiproto::hmiact & act, int nBaseScr)
{
	return false;
}

template<class PART, class STRUCT>
inline void HMILvScreen::ParsePart(STRUCT T, int nDataType)
{
	std::cout << "Parse Part " << std::endl;
	hmiproto::hmipartbasic basic = T.basic();
	PART *part = NULL;
	if (nDataType == HMIEVENT_INITSCRPART) {
		part = new PART(T, this);
		part->m_nIndex = basic.index();
		m_partlist.push_back(part);
		std::cout << "Push Back Part ----------------------------- "<< basic.index() << std::endl;

	}
	else if (nDataType == HMIEVENT_UPDATEPART) {
		std::cout << "Update Part----------------------------- " << std::endl;

		int nPartIndex = basic.index();
		for(HMILvPart *PartInfo : m_partlist) {
			if (PartInfo != NULL &&
				PartInfo->m_nIndex == nPartIndex) {
				part = static_cast<PART *>(PartInfo);
				part->Update(T, this);// ��������
				part->Draw(this, DRAW_UPDATE);// ��ͼ
			}
		}
	}
}


bool HMILvScreen::Parse(const hmiproto::hmiact & act, int nDataType)
{
	std::cout << "Parse" << std::endl;

	if (act.has_scr()) {
		std::cout << "sct has scr " << std::endl;
		m_ScrInfo.MergeFrom(act.scr());
		if (nDataType == HMIEVENT_INITSCRPART) {
			m_nScrNo = m_ScrInfo.scrno();
			m_ScrNo = m_ScrInfo.scrno();
			std::string objName = std::string("src_") + std::to_string(m_nScrNo);
			//this->setObjectName(objName);
			m_wScrType = m_ScrInfo.type();
			if (m_ScrInfo.type() == SCREEN_INDIRECT || SCREEN_SUB == m_ScrInfo.type()) {
				this->move(m_ScrInfo.left(), m_ScrInfo.top());
			}
			this->setFixedSize(m_ScrInfo.width(), m_ScrInfo.height());
		}
		std::cout << "Common Part Size " << act.common_size() << std::endl;
		for (int nI = 0; nI < act.common_size(); nI++) {
			ParsePart<LvGeneralPart, hmiproto::hmipartcommon>(act.common(nI), nDataType);
		}

		for (int nI = 0; nI < act.custom_size(); nI++) {
			ParsePart<LvCustomPart, hmiproto::hmicustom>(act.custom(nI), nDataType);
		}

		for (int nI = 0; nI < act.downlist_size(); nI++) {
			ParsePart<LvComboBox, hmiproto::hmidownlist>(act.downlist(nI), nDataType);
		}

		for (int nI = 0; nI < act.slider_size(); nI++) {
			ParsePart<LvCSlider, hmiproto::hmislider>(act.slider(nI), nDataType);
		}

		for (int nI = 0; nI < act.weather_size(); nI++) {
			ParsePart<LvWeatherUi, hmiproto::hmiweather>(act.weather(nI), nDataType);
		}

		for (int nI = 0; nI < act.formula_size(); nI++) {
			ParsePart<LvFormulaPart, hmiproto::cformula>(act.formula(nI), nDataType);
		}

		for (int nI = 0; nI < act.input_size(); nI++) {
			ParsePart<LvInputControls, hmiproto::hmipartinput>(act.input(nI), nDataType);
		}

	}


	return true;
}

int HMILvScreen::parseJson(const hmiproto::hmiact & act)
{
	return 0;
}

void HMILvScreen::SetScrNo(int nScrNo)
{
}

int HMILvScreen::GetScrNo()
{
	return m_ScrNo;
}

int HMILvScreen::GetScrType()
{
	return m_wScrType;
}

void HMILvScreen::setScrType(const int type)
{
}

bool HMILvScreen::Init()
{
	std::cout << "HMILvScreen Init m_partlist.size() : "<< m_partlist.size() << std::endl;

	if (-1 != m_ScrInfo.imgindex()) {
		m_nImgIndex = m_ScrInfo.imgindex();
	}
	else {
		lv_obj_set_style_bg_color(m_Screen, lv_color_hex3(0x1af), 0);
	}
	
	for(HMILvPart *part : m_partlist) {
		part->Init(m_Screen);
		part->Draw(this, DRAW_INIT);
	}
	return true;
}

bool HMILvScreen::IsInit()
{
	return false;
}

void HMILvScreen::OnScrShow()
{
	m_nScrNo = m_ScrNo;
}

void HMILvScreen::OnScrHide()
{
}

void HMILvScreen::setParent(lv_obj_t *)
{
}

void HMILvScreen::show()
{
	std::cout << "Screen ScrNo: " << m_ScrNo << " is Showing " << std::endl;
	lv_scr_load(m_Screen);
}

void HMILvScreen::move(int x, int y)
{
}

void HMILvScreen::setFixedSize(int width, int height)
{
}

void HMILvScreen::setFocus()
{
}

lv_obj_t * HMILvScreen::getParent()
{
	return lv_scr_act();
}

void HMILvScreen::SetShowStates(bool bStates)
{
}

bool HMILvScreen::GetShowStates()
{
	return m_bShow;
}

void HMILvScreen::GetScrPopPoint(int & x, int & y)
{
}

void HMILvScreen::ScaleFrameSize()
{
}

void HMILvScreen::Event(lv_event_t * event)
{
	lv_event_code_t code = lv_event_get_code(event);

	if (code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
		lv_point_t pos;
		lv_indev_get_point(lv_indev_get_act(), &pos);
		m_x = pos.x;
		m_y = pos.y;
		SendClickEvent(m_nScrNo, m_x, m_y, true);
	}
	else if (code == LV_EVENT_VALUE_CHANGED) {
		LV_LOG_USER("Toggled");
	}

}

void HMILvScreen::SendClickEvent(int nScrNo, int nX, int nY, bool bDown)
{
	std::cout << "SendClickEvent x = " << (800 - nX) << "y= " << nY <<  std::endl;


	EventMgr::SendMouseEvent(nScrNo,800 - nX, nY, bDown);
	return;

}



void HMILvScreen::mousePressEvent(lv_obj_t* obj, lv_event_t * event)
{
	m_x = -1;
	m_y = -1;
	// std::cout << "==========mousePressEvent time=" << CTime::get_instance().getTickCount();
}

void HMILvScreen::mouseReleaseEvent(lv_event_t * eventT)
{
}
