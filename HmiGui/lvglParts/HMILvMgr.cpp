#include "HMILvMgr.h"
#include "EventMgr/EventMgr.h"
#include "../hmidef.h"
#include "HmiGui/HmiGui.h"
#include <utility/utility.h>
#include "platform/platform.h"
#include <iostream>
#include "../lvglParts/LvImgCacheMgr.h"
using std::cout;
using std::endl;

HMILvMgr::HMILvMgr()
{
#ifdef WIN32
	//this->grabKeyboard(); //  ��׽�����¼�
#endif
	m_bSyssetScreenRunning = false;
	// m_msgTip = NULL;

	{	//��ʼ�������ڴ�ֵ
		unsigned int dwMemSize = 0;
		// CState::GetMemoryStatus(dwMemSize, dwFreeMemSize);
		m_dwMemSize = dwMemSize / (1024 * 1024);
		if (m_dwMemSize == 0) {
			m_dwMemSize = 128;
		}
	}

	m_pClrLcdScr = NULL;
	//m_pScreenSaver = nullptr;

}

HMILvMgr::~HMILvMgr()
{
	try {

		m_ScreenList.clear();



		if (NULL != m_pClrLcdScr) {
			delete m_pClrLcdScr;
		}
	}
	catch (...) {
	}
}

void HMILvMgr::Init(uint32_t width, uint32_t height)
{
}

void HMILvMgr::ImgCache(const void * pListImg, string strPath)
{

	std::cout << "ImgCache:" << strPath << std::endl;
	LvImgCacheMgr::Init(pListImg, strPath);
}

void HMILvMgr::InitScrPart(const hmiproto::hmiact & act, lv_obj_t* obj)
{
	std::cout << "InitScrPart" << std::endl;

	if (false == act.has_scr()) {
		return;
	}

	HMILvScreen* screen = new HMILvScreen(obj);
	
	HMILvScreen* pTopScr = GetTopBaseScr();

	if (NULL != pTopScr &&
		false == screen->CheckInShowBaseScr(act, pTopScr->GetScrNo())) {
		std::cout << "This is not Top Scr" << std::endl;

		delete screen;
		return;
	}
	//����
	if (false == screen->Parse(act, HMIEVENT_INITSCRPART) ||
		false == screen->Init()) {

		std::cout << "Could not parse" << std::endl;

		delete screen;
		return;
	}
	std::cout << "InitScrPart and show " << std::endl;

	m_ScreenList.push_back(screen);
	screen->Init();    
	//screen->show();
	screen->SetShowStates(true);

#ifdef WIN32
	screen->setFocus(); // ���������ý��㣬�����޷���������¼� xqh 20181217
#endif
}

HMILvScreen * HMILvMgr::GetTopBaseScr()
{
	HMILvScreen* pScr = NULL;
	for (int i = m_ScreenList.size() - 1; i >= 0; i--) {
		pScr = m_ScreenList[i];
		if (NULL != pScr &&
			SCREEN_MAIN == pScr->GetScrType() &&
			true == pScr->GetShowStates()) {
			return pScr;
		}
	}
	return NULL;
}

HMILvScreen * HMILvMgr::GetTopScr()
{
	if (m_ScreenList.size() == 0) {
		return NULL;
	}
	return m_ScreenList[m_ScreenList.size() - 1];
}

void HMILvMgr::sendQuicklyDoEventSignal(hmiproto::hmiact & act)
{
	Slot_QuicklyDoEvent(act);
}

HMILvScreen * HMILvMgr::GetRealTopScr()
{
	if (m_ScreenList.size() == 0) {
		return nullptr;
	}
	for (int i = m_ScreenList.size() - 1; i >= 0; i--) {
		HMILvScreen* pScr = nullptr;
		pScr = m_ScreenList[i];
		if (nullptr == pScr ||
			false == pScr->GetShowStates()) {
			continue;
		}
		if (SCREEN_INDIRECT == pScr->GetScrType()) {
			pScr = GetTopBaseScr();
		}
		return pScr;
	}
	return nullptr;
}

void HMILvMgr::quitScreenSaver()
{
}

int HMILvMgr::parseCustomJson(hmiproto::hmiact & act)
{
	if (false == act.has_event()) {
		return -1;
	}
	const hmiproto::hmievent &event = act.event();
	for (auto &iter : m_ScreenList) {
		if (event.scrno() != iter->GetScrNo()) {
			continue;
		}
		return iter->parseJson(act);
	}
	return -2;
}

bool HMILvMgr::CheckInShow(int nScrNo)
{
	HMILvScreen * pScr = NULL;
	bool        bRet = false;
	for (int i = 0; i < (int)m_ScreenList.size(); i++) {
		pScr = m_ScreenList[i];
		if (NULL != pScr && nScrNo == pScr->GetScrNo() && true == pScr->GetShowStates()) {
			bRet = true;
			break;
		}
	}
	return bRet;
}

void HMILvMgr::ChangeScreen(hmiproto::hmiact & act)
{
	if (false == act.has_event()) {
		return;
	}
	if (m_dwMemSize <= 300) {//�ڴ����300M�Ļ��ͣ�ÿ���л����涼����һ��ͼƬ
	}
	const hmiproto::hmievent&   msgEvent = act.event();
	bool  bHideMsg = msgTipIsHide();
	// bool  bHideScrSaver = screenSaverIsHide();
	CloseOpenedScreen(); // ��ʾ�µ�������ǰ��֮ǰ�򿪵Ļ���ر�
	if (true == ScreenInCache(msgEvent.scrno())) {
		std::cout << "Screen In Cache ----------------- : " << msgEvent.scrno() << std::endl;

		
		ShowScreenInCache(msgEvent.scrno());
		UpdatePart(act);
	}
	else {
		std::cout << "Load New Screen----------------- : " << msgEvent.scrno() << std::endl;
		lv_obj_t* page = lv_obj_create(NULL);
		lv_coord_t screen_width = lv_obj_get_width(lv_scr_act());
		lv_coord_t screen_height = lv_obj_get_height(lv_scr_act());
		lv_scr_load(page);

		lv_obj_clean(page);
		lv_obj_set_size(page, screen_width, screen_height);

		InitScrPart(act, page);
	}
	updateMsgTip(bHideMsg);
	//updateScreenSaver(bHideScrSaver);
	return;
}

bool HMILvMgr::ScreenInCache(const int & nScrNo)
{
	
	HMILvScreen  *pScr = NULL;
	bool        bRtn = false;
	for (int nI = 0; nI < (int)m_ScreenList.size(); nI++) {
		pScr = m_ScreenList[nI];
		std::cout << "ScrNo :  " << nScrNo << "Screen ScrNo: " << pScr->GetScrNo() << std::endl;
		if (NULL != pScr && nScrNo == pScr->GetScrNo()) {
			bRtn = true;
			break;
		}
	}
	return bRtn;
}

void HMILvMgr::ShowScreenInCache(const int & nScrNo)
{
	HMILvScreen	 *pScr = NULL;
	/*lv_obj_clean(lv_scr_act());*/
	for (int i = 0; i < (int)m_ScreenList.size(); i++) {
		pScr = m_ScreenList[i];
		if (NULL == pScr || nScrNo != pScr->GetScrNo()) {
			continue;
		}
		pScr->SetShowStates(true);
		pScr->OnScrShow();
		pScr->show();
#		ifdef WIN32
			pScr->setFocus(); // ���������ý��㣬�����޷���������¼� xqh 20181217
#		endif

		//// ����ʾ�Ļ��������β��
		m_ScreenList.erase(m_ScreenList.begin() + i);
		m_ScreenList.push_back(pScr);
		break;
	}
	return;
}

void HMILvMgr::PopChildScreen(hmiproto::hmiact & act)
{
}

void HMILvMgr::CloseChildScr(hmiproto::hmiact & act)
{
}

void HMILvMgr::CloseOpenedScreen()
{

}

void HMILvMgr::ShowIndirectcreen(hmiproto::hmiact & act)
{
}

void HMILvMgr::ShowCursor(hmiproto::hmiact & act)
{
}

void HMILvMgr::Screenshot(hmiproto::hmiact & act)
{

}

void HMILvMgr::RefreshScreen()
{
}

void HMILvMgr::SendCursorPos(hmiproto::hmiact & act)
{
}

void HMILvMgr::SetCursorPos(hmiproto::hmiact & act)
{
}

void HMILvMgr::FreeScreenCache(hmiproto::hmiact & act)
{
}

void HMILvMgr::raiseMsgTip()
{
}

void HMILvMgr::keyPressEvent(lv_event_t * eventT)
{
}

void HMILvMgr::keyReleaseEvent(lv_event_t * eventT)
{
}

void HMILvMgr::UpdatePart(const hmiproto::hmiact & act)
{

	if (false == act.has_scr()) {
		return;
	}
	for(HMILvScreen *scr : m_ScreenList) {
		if (act.scr().scrno() != scr->GetScrNo()) {
			continue;
		}
		std::cout << "Screen Part Update ScrNo : " << act.scr().scrno() << std::endl;
		scr->Parse(act, HMIEVENT_UPDATEPART);
		break;
	}
	return;
}

void HMILvMgr::clearLcd(hmiproto::hmiact & act)
{
}

lv_obj_t * HMILvMgr::getParent(const hmiproto::hmiact & act)
{
	return nullptr;
}

void HMILvMgr::showMsgTip(hmiproto::hmiact & act)
{
	if (false == act.has_message()) {
		return;
	}

	return;
}

bool HMILvMgr::msgTipIsHide()
{
	return false;
}

int HMILvMgr::updateMsgTip(bool bHide)
{
	return 0;
}

int HMILvMgr::showSysSetScreen(int nScreen)
{
	return 0;
}

bool HMILvMgr::screenSaverIsHide()
{
	return false;
}

int HMILvMgr::updateScreenSaver(bool bHide)
{
	return 0;
}

void HMILvMgr::SendUpdateTopScr()
{
}

int HMILvMgr::runSyssetScreenFilter(unsigned int nEventType)
{
	return 0;
}

void HMILvMgr::Slot_QuicklyDoEvent(hmiproto::hmiact & act)
{
	int nRtn = 0;
	if (false == act.has_event()) {
		return;
	}
	const hmiproto::hmievent	&msgEvent = act.event();
	unsigned int nType = msgEvent.type();
	if ((nRtn = this->runSyssetScreenFilter(nType)) < 0) {
		return;
	}
	std::cout << "NType : " << nType << std::endl;
	switch (nType) {

	case HMIEVENT_INITSCRPART: // ��ʼ������
	{
		std::cout << "Init" << std::endl;
		InitScrPart(act, lv_scr_act());
		SendUpdateTopScr();
	}
	break;
	case HMIEVENT_UPDATEPART: // ���²���
	{
		
		UpdatePart(act);
	}
	break;
	case HMIEVENT_CHANGESCR: //������ת
	{
		std::cout << "Change" << std::endl;

		ChangeScreen(act);
		raiseMsgTip();
		SendUpdateTopScr();
	}
	break;
	case HMIEVENT_POPSCR: // �����ӻ���
	{
		std::cout << "Pop" << std::endl;

		PopChildScreen(act);
		raiseMsgTip();
		SendUpdateTopScr();
	}
	break;
	case HMIEVENT_CLOSECHILDSCR: //�ر��ӻ���
	{
		std::cout << "Close" << std::endl;

		CloseChildScr(act);
		SendUpdateTopScr();
	}
	break;
	case HMIEVENT_UPDATESCREEN: // ˢ�»���
	{
		std::cout << "Update Screen" << std::endl;

		RefreshScreen();
	}
	break;
	case HMIEVENT_INDIRECTSCRINIT: // ��ӻ����ʼ��
	{

		ShowIndirectcreen(act);
	}
	break;
	case HMIEVENT_SHOWCURSOR:       //��ʾ/�������
	{
		ShowCursor(act);
	}
	break;
	case HMIEVENT_PRINTPART: // ��ͼ
	{
		Screenshot(act);
	}
	break;
	case HMIEVENT_SETCURSOR: // �������λ��
	{
		SetCursorPos(act);
	}
	break;
	case HMIEVENT_GETCURSOR: // ��ȡ���λ��
	{
		SendCursorPos(act);
	}
	break;
	case HMIEVENT_FREESCRCACHE: // �Ƴ����滺��
	{
		FreeScreenCache(act);
	}
	break;
	case HMIEVENT_EXITUI:
	{
		//qApp->exit(0);
	}
	break;
	case HMIEVENT_DISPLAYMSG:
	{
		std::cout << "Show Msg Tip" << std::endl;
		showMsgTip(act);
	}
	break;
	case HMIEVENT_CLEAR_LCD:
	{
		clearLcd(act);
	}
	break;
	case HMIEVENT_SHOW_SCREENSAVER:
	{
		//showScreenSaver((CScreenSaver::TYPE_T)msgEvent.scrno());
	}
	break;
	case HMIEVENT_GO_SYSSET:
	{
		showSysSetScreen(msgEvent.scrno());
	}
	break;
	case HMIEVENT_CLOSE_SCREENSAVER:
	{
		quitScreenSaver();
	}
	break;
	case HMIEVENT_JSON:
	{
		this->parseCustomJson(act);
	}
	break;
	default:
		break;
	}
	return;
}
