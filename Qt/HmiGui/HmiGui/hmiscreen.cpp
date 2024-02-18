#include "hmiscreen.h"
#include <QPalette>
#include <QMouseEvent>
#include <QPainter>
#include "hmiutility.h"
#include "generalpart.h"
#include "imgcachemgr.h"
#include "ComboBox.h"
#include "custompart.h"
#include "hmimgr.h"
#include "hmipart.h"
#include "EventMgr/EventMgr.h"
#include "slider.h"
#include <utility/utility.h>
#include "macrosplatform/registerAddr.h"
#include "RegisterOpt/RegisterOpt.h"
#include "HmiBase/HmiBase.h"
#include "macrosplatform/event.h"
#include "camera.h"
#include "weatherUi.h"
#include "formula/cformulapart.h"
#include "inputControls/inputControls.h"

using namespace UTILITY_NAMESPACE;
int HMIScreen::m_x = -1;
int HMIScreen::m_y = -1;
int HMIScreen::m_nClickScrNo = -2;
#ifdef _LINUX_
unsigned long long     HMIScreen::m_lastClick = 0;   //最后一次获取单击时间
#endif

HMIScreen::HMIScreen(QFrame *pFrame) :
	QFrame(pFrame) {
	m_nScrNo = -2;                  //画面号
	m_wScrType = 0;					//画面类型

	m_ScrInfo.Clear();
	m_parent = NULL;
	m_nImgIndex = -1;
#ifndef WIN32
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
#endif
	m_partlist.clear();
	m_bClick = false;
	m_bShow = false;
}

HMIScreen::~HMIScreen() {
	// 电脑模拟关闭画面时，抬起事件不会产生，所以在销毁画面时要判断是否需要补发一个抬起事件 xqh 2020815
	if (m_bClick) {// 有点击
		SendClickEvent(m_nClickScrNo, m_x, m_y, false);
	}
	foreach(HMIPart *part, m_partlist) {
		delete part;
		part = NULL;
	}
	m_partlist.clear();
}

void HMIScreen::mousePressEvent(QMouseEvent *eventT) {
	m_x = -1;
	m_y = -1;
	//SET_TIME_CLICK;
	qDebug() << "==========mousePressEvent time=" << CTime::get_instance().getTickCount();
	HMIMgr *hmiMgr = static_cast<HMIMgr *>(m_parent);
	if (hmiMgr == NULL) {
		qDebug() << "return hmiMgr";
		return;
	}
	hmiMgr->quitScreenSaver();
	HMIScreen *topScr = hmiMgr->GetTopScr();
	if (NULL == topScr) {
		qDebug() << "return topScr";
		return;
	}
	if (topScr->m_wScrType != SCREEN_INDIRECT &&
		this->m_nScrNo != topScr->GetScrNo())//且不允许点击到当前子画面外，默认不允许。
	{
		qDebug() << "return 1...";
		return;
	}
	if (eventT->button() != Qt::LeftButton) {
		qDebug() << "return 2...";
		return;
	}
#ifdef _LINUX_
	unsigned long long ullClickTimeLimit = CRegisterRW::readWord(REG_TYPE_HS_, HSW_CLICKINTERVALTIMELIMIT);
	if (ullClickTimeLimit > 500 || ullClickTimeLimit == 0) {
		ullClickTimeLimit = 100;
	}
	unsigned long long clickTime = CTime::get_instance().getUtc();
	if ((clickTime - m_lastClick) > (ullClickTimeLimit)) {
#endif
		m_x = eventT->x();
		m_y = eventT->y();
		m_nClickScrNo = m_nScrNo;
#if ((defined WIN32) && (defined WIN32SCALE))
		QPoint pointTemp(m_x, m_y);
		HMIUtility::CountMousePoint(&pointTemp);
		m_x = pointTemp.x();
		m_y = pointTemp.y();
#endif	
		SendClickEvent(m_nClickScrNo, m_x, m_y, true);
		if (SCREEN_INDIRECT == this->m_wScrType) {
			HMIScreen *pParentScr = hmiMgr->GetTopBaseScr();
			if (pParentScr) {
				EventMgr::SendClickIndirectScrEvent(pParentScr->GetScrNo(), m_ScrInfo.indirectpartname());
			}
		}
#ifdef _LINUX_
		m_lastClick = clickTime;
}
#endif
}

void HMIScreen::RecvOnclickedDown(QMouseEvent *eventT, int nX, int nY) {
	//SET_TIME_CLICK;
	qDebug() << "==========mousePressEvent time=" << CTime::get_instance().getTickCount();
	HMIMgr *hmiMgr = static_cast<HMIMgr *>(m_parent);
	if (hmiMgr == NULL) {
		qDebug() << "return hmiMgr";
		return;
	}
	hmiMgr->quitScreenSaver();
	HMIScreen *topScr = hmiMgr->GetTopScr();
	if (NULL == topScr) {
		qDebug() << "return topScr";
		return;
	}
	if (topScr->m_wScrType != SCREEN_INDIRECT &&
		this->m_nScrNo != topScr->GetScrNo())//且不允许点击到当前子画面外，默认不允许。
	{
		qDebug() << "return 1...";
		return;
	}

	if (eventT->button() != Qt::LeftButton) {
		qDebug() << "return 2...";
		return;
	}

#ifdef _LINUX_
	unsigned long long ullClickTimeLimit = CRegisterRW::readWord(REG_TYPE_HS_, HSW_CLICKINTERVALTIMELIMIT);
	if (ullClickTimeLimit > 500 || ullClickTimeLimit == 0) {
		ullClickTimeLimit = 100;
	}
	unsigned long long clickTime = CTime::get_instance().getUtc();
	if ((clickTime - m_lastClick) > (ullClickTimeLimit)) {
#endif
		m_x = nX;
		m_y = nY;
		m_nClickScrNo = m_nScrNo;
#if ((defined WIN32) && (defined WIN32SCALE))
		QPoint pointTemp(m_x, m_y);
		HMIUtility::CountMousePoint(&pointTemp);
		m_x = pointTemp.x();
		m_y = pointTemp.y();
#endif
		SendClickEvent(m_nClickScrNo, m_x, m_y, true);
		if (SCREEN_INDIRECT == this->m_wScrType) {
			HMIScreen *pParentScr = hmiMgr->GetTopBaseScr();
			if (pParentScr) {
				EventMgr::SendClickIndirectScrEvent(pParentScr->GetScrNo(), m_ScrInfo.indirectpartname());
			}
		}
#ifdef _LINUX_
		m_lastClick = clickTime;
	}
#endif
}

void HMIScreen::mouseReleaseEvent(QMouseEvent *eventT) {
	if (eventT->button() != Qt::LeftButton) {
		return;
	}
	if (m_x == -1 || m_y == -1) {
		return;
	}
	SendClickEvent(m_nClickScrNo, m_x, m_y, false);
}

void HMIScreen::RecvOnclickedUp(QMouseEvent *eventT) {
    if (eventT->button() != Qt::LeftButton) {
        return;
    }
    if (m_x == -1 || m_y == -1) {
        return;
    }
    SendClickEvent(m_nClickScrNo, m_x, m_y, false);
}

void HMIScreen::SetScrNo(int nScrNo) {
	m_nScrNo = nScrNo;
}

int HMIScreen::GetScrNo() {
	return m_nScrNo;
}

int HMIScreen::GetScrType() {
	return m_wScrType;
}

void HMIScreen::setScrType(const int type) {
	m_wScrType = type;
	return;
}

//排列判断
static int CompareIndex(const HMIPart* pPart1, const HMIPart* pPart2) {
	if (NULL == pPart1 || NULL == pPart2) {
		return -1;
	}
	return pPart1->m_nIndex < pPart2->m_nIndex;
}

bool HMIScreen::Init() {
	//设置背景颜色/图片
	if (-1 != m_ScrInfo.imgindex()) {
		m_nImgIndex = m_ScrInfo.imgindex();
		/* 使用style sheet时指定对象的名字名字，防止子画面设置父画面后继承了父画面设置style sheet */
        QString strName = this->objectName();
		/*QString strStyle = QString::fromUtf8("#") + strName + QString::fromUtf8(" {background-color: transparent;}");
		this->setStyleSheet(strStyle);*/
	}
	else {
		QPalette    qPalette;
		QColor      qColor(m_ScrInfo.bgcolor());
		qPalette.setColor(QPalette::Background, qColor);
		this->setPalette(qPalette);
		this->setAutoFillBackground(true);
	}
	qSort(m_partlist.begin(), m_partlist.end(), CompareIndex);  // 将部件排序
#if ((defined WIN32) && (defined WIN32SCALE))
	QRect rectTemp = this->geometry();
	m_OriginalRect.setLeft(rectTemp.left());
	m_OriginalRect.setTop(rectTemp.top());
	m_OriginalRect.setWidth(rectTemp.width());
	m_OriginalRect.setHeight(rectTemp.height());
	ScaleFrameSize();
#endif
	//画所有部件
	foreach(HMIPart *part, m_partlist) {
		part->Init(this);
		part->Draw(this, DRAW_INIT);
	}
	return true;
}

template<class  PART, class  STRUCT>
void HMIScreen::ParsePart(STRUCT T, int nDataType) {
	hmiproto::hmipartbasic basic = T.basic();
	PART *part = NULL;
	if (nDataType == HMIEVENT_INITSCRPART) {
		part = new PART(T, this);
		part->m_nIndex = basic.index();
		m_partlist.append(part);
	}
	else if (nDataType == HMIEVENT_UPDATEPART) {
		int nPartIndex = basic.index();
		foreach(HMIPart *PartInfo, m_partlist) {
			if (PartInfo != NULL &&
				PartInfo->m_nIndex == nPartIndex) {
				part = static_cast<PART *>(PartInfo);
				part->Update(T, this);// 更新数据
				part->Draw(this, DRAW_UPDATE);// 绘图
			}
		}
	}
}

bool HMIScreen::CheckInShowBaseScr(const hmiproto::hmiact &act, int nBaseScr) {
	if (act.has_scr()) {
		m_ScrInfo.MergeFrom(act.scr());
		if (m_ScrInfo.type() != SCREEN_INDIRECT) {
			return true;
		}

		QString strScrName(m_ScrInfo.name().c_str());
		QStringList ScrNameList = strScrName.split("_");
		if (ScrNameList.length() == 2) {
			if (nBaseScr != ScrNameList.at(0).toInt()) {
				return false;
			}
		}
	}

	return true;
}

bool HMIScreen::Parse(const hmiproto::hmiact &act, int nDataType) {
	if (act.has_scr()) {
		m_ScrInfo.MergeFrom(act.scr());
		if (nDataType == HMIEVENT_INITSCRPART) {
			m_nScrNo = m_ScrInfo.scrno();
			QString objName = QString::fromUtf8("src_") + QString::number(m_nScrNo);
			this->setObjectName(objName);
			m_wScrType = m_ScrInfo.type();
			if (m_ScrInfo.type() == SCREEN_INDIRECT || SCREEN_SUB == m_ScrInfo.type()) {
				this->move(m_ScrInfo.left(), m_ScrInfo.top());
			}
			this->setFixedSize(m_ScrInfo.width(), m_ScrInfo.height());
		}
	}
	for (int nI = 0; nI < act.common_size(); nI++) {
		ParsePart<GeneralPart, hmiproto::hmipartcommon>(act.common(nI), nDataType);
	}
	for (int nI = 0; nI < act.custom_size(); nI++) {
		ParsePart<custompart, hmiproto::hmicustom>(act.custom(nI), nDataType);
	}
	for (int nI = 0; nI < act.downlist_size(); nI++) {
		ParsePart<CComboBox, hmiproto::hmidownlist>(act.downlist(nI), nDataType);
	}
	for (int nI = 0; nI < act.slider_size(); nI++) {
		ParsePart<CSlider, hmiproto::hmislider>(act.slider(nI), nDataType);
	}
	for (int nI = 0; nI < act.ipcamaro_size(); nI++) {
		ParsePart<CCamera, hmiproto::hmiipcamaro>(act.ipcamaro(nI), nDataType);
	}
	for (int nI = 0; nI < act.weather_size(); nI++) {
		ParsePart<CWeatherUi, hmiproto::hmiweather>(act.weather(nI), nDataType);
	}
	for (int nI = 0; nI < act.formula_size(); nI++) {
		ParsePart<CFormulaPart, hmiproto::cformula>(act.formula(nI), nDataType);
	}
	for (int nI = 0; nI < act.input_size(); nI++) {
		ParsePart<CInputControls, hmiproto::hmipartinput>(act.input(nI), nDataType);
	}
    return true;
}

void HMIScreen::OnScrShow() {
	foreach(HMIPart *part, m_partlist) {
		part->OnPartShow();
	}
}

void HMIScreen::OnScrHide() {
	foreach(HMIPart *part, m_partlist) {
		part->OnPartHide();
	}
	if (m_bClick) {
		SendClickEvent(m_nClickScrNo, m_x, m_y, false);
	}
	hide();
}

void HMIScreen::setParent(QFrame *pFrame) {
	this->m_parent = pFrame;
}

QFrame * HMIScreen::getParent() {
	return this->m_parent;
}

void HMIScreen::SetShowStates(bool bStates) {
	m_bShow = bStates;
}

bool HMIScreen::GetShowStates() {
	return m_bShow;
}

#if ((defined WIN32) && (defined WIN32SCALE))
void HMIScreen::ScaleFrameSize() {
	QRect rectTemp(m_OriginalRect.left(), m_OriginalRect.top(), m_OriginalRect.width(), m_OriginalRect.height());
	HMIUtility::CountScaleRect(&rectTemp);
	this->setFixedSize(rectTemp.width(), rectTemp.height());
	switch (m_wScrType) {
	case SCREEN_MAIN:
		break;
	case SCREEN_SUB:
		this->move(rectTemp.left(), rectTemp.top());
		break;
	case SCREEN_INDIRECT:
		this->move(rectTemp.left(), rectTemp.top());
		break;
	default:
		break;
	}
}

QList<HMIPart *>* HMIScreen::GetPartlist() {
	return &m_partlist;
}
#endif

void HMIScreen::paintEvent(QPaintEvent *) {
	QPainter p(this);
	//背景图
	if (m_nImgIndex != -1) {
		IMG_T *img = ImgCacheMgr::GetImgInfo(m_nImgIndex);
		if (img != NULL && img->pix != NULL) {
			img->mutex.lock();
			p.drawPixmap(this->rect(), *img->pix);
			img->mutex.unlock();
		}
	}
}

void HMIScreen::SendClickEvent(int nScrNo, int nX, int nY, bool bDown) {
	HMIMgr *hmiMgr = static_cast<HMIMgr *>(m_parent);
	if (hmiMgr == NULL) {
		return;
	}
	if (( bDown && m_bClick ) ||
		( !bDown && !m_bClick )) {//过滤不匹配的事件
		return;
	}
	m_bClick = bDown;
	EventMgr::SendMouseEvent(nScrNo, nX, nY, bDown);
	return;
}

void HMIScreen::GetScrPopPoint(int &x, int &y) {
	x = m_ScrInfo.left();
	y = m_ScrInfo.top();
}

int HMIScreen::parseJson(const hmiproto::hmiact &act) {
	const hmiproto::hmievent &event = act.event();
	std::string sName = event.partname();
	std::string sEventbuffer = event.eventbuffer();
	for (auto &iter : m_partlist) {
		if (iter == nullptr ||
			iter->m_sPartName != sName) {
			continue;
		}
		return iter->parseJson(sEventbuffer);
	}
	return -1;
}