#include "hmimgr.h"
#include <string.h>
#ifdef _LINUX_
#include <dlfcn.h>
#endif
#if (QT_VERSION <= QT_VERSION_CHECK(5, 0, 0))
#include <QWSServer>
#else
#include <QScreen>
#endif
#include <QString>
#include <QMutableListIterator>
#include <QDebug>
#include <QtCore/QCoreApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QCloseEvent>
#include <QApplication>
#include <QDir>
#include <QDesktopWidget>
#include <QBitmap>
#include <QKeyEvent>
#include "hmiscreen.h"
#include "EventMgr/EventMgr.h"
#include "imgcachemgr.h"
#include "hmiutility.h"
#include "hmipart.h"
#include "keyBoard.h"
#include "hmidef.h"
#include "commonpart.h"
#include "HmiGui/HmiGui.h"
#include <utility/utility.h>
#include "platform/platform.h"

using namespace UTILITY_NAMESPACE;

HMIMgr::HMIMgr(QFrame *pFrame) :
	QFrame(pFrame) {

#ifdef WIN32
	//this->grabKeyboard(); //  捕捉键盘事件
#endif
	m_bSyssetScreenRunning = false;
	m_msgTip = NULL;

	{	//初始化物理内存值
		unsigned int dwMemSize = 0, dwFreeMemSize = 0;
		CState::GetMemoryStatus(dwMemSize, dwFreeMemSize);
		m_dwMemSize = dwMemSize / ( 1024 * 1024 );
		if (m_dwMemSize == 0) {
			m_dwMemSize = 128;
		}
	}

	m_pClrLcdScr = NULL;
    m_pScreenSaver = nullptr;
	connect(this, SIGNAL(Signal_QuicklyDoEvent(hmiproto::hmiact&)), this, SLOT(Slot_QuicklyDoEvent(hmiproto::hmiact&)), Qt::BlockingQueuedConnection);
}

HMIMgr::~HMIMgr() {
	try {
        if (nullptr != m_pScreenSaver) {
            delete m_pScreenSaver;
        }

		foreach(HMIScreen *scr, m_ScreenList) {

            if (SCREEN_MAIN != scr->GetScrType()) {
                continue;
            }


            if (NULL != scr) {
			    delete scr;
		    }
		}

		m_ScreenList.clear();

		ImgCacheMgr::Destroy();
		if (CommonPart::m_Qpf2Info)//释放存放qpf2文件头信息的内存
		{
			free(CommonPart::m_Qpf2Info);
		}

		if (NULL != m_pClrLcdScr) {
			delete m_pClrLcdScr;
		}
	}
	catch (...) {
	}
	qDebug() << "UI Quit\r\n";
}

void HMIMgr::ImgCache(const void *pListImg, QString strPath) {
	ImgCacheMgr::Init(pListImg, strPath);
}


#ifndef _LINUX_
void HMIMgr::closeEvent(QCloseEvent *event) {
	EventMgr::SendExitEvent();
	event->ignore();    // 忽略退出型号
	return;
}

#endif

HMIScreen* HMIMgr::GetTopBaseScr() {
	HMIScreen*  pScr = NULL;
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

HMIScreen* HMIMgr::GetTopScr() {
	if (m_ScreenList.size() == 0) {
		return NULL;
	}
	return m_ScreenList[m_ScreenList.size() - 1];
}

CScreenSaver * HMIMgr::GetScreenSaver()
{
	return m_pScreenSaver;
}

#if ((defined WIN32) && (defined WIN32SCALE))
void HMIMgr::resizeEvent(QResizeEvent *eventT) {
	HMIUtility::SetScaling(eventT);
	foreach(HMIScreen *scr, m_ScreenList) {
		scr->ScaleFrameSize();
		QList<HMIPart *>* pPartlist = scr->GetPartlist();
		if (NULL != pPartlist) {
			foreach(HMIPart *part, *pPartlist) {
				part->ZoomPartSelf();
			}
		}
	}
}
#endif

void HMIMgr::keyPressEvent(QKeyEvent *eventT) {
	if (eventT == NULL) {
		return;
	}
	DigitalCompositeKey(EVENT_KEYDOWN, eventT);
}

void HMIMgr::keyReleaseEvent(QKeyEvent *eventT) {
	if (eventT == NULL) {
		return;
	}
	DigitalCompositeKey(EVENT_KEYUP, eventT);
}

void HMIMgr::sendQuicklyDoEventSignal(hmiproto::hmiact& act) {
	emit Signal_QuicklyDoEvent(act);
}

QFrame* HMIMgr::getParent(const hmiproto::hmiact& act) {
    if (false == act.has_scr()) {
        return NULL;
    }
    if (SCREEN_MAIN != act.scr().type()) {
        return GetTopBaseScr();
    }
#ifdef WIN32
    return this;
#else
    return  NULL;
#endif
}

void HMIMgr::InitScrPart(const hmiproto::hmiact &act) {
	if (false == act.has_scr()) {
		return;
	}
	HMIScreen*  screen = new HMIScreen(getParent(act));
	screen->setParent(this);
	//检查是否允许显示间接画面
	HMIScreen*	pTopScr = GetTopBaseScr();
	if (NULL != pTopScr &&
		false == screen->CheckInShowBaseScr(act, pTopScr->GetScrNo())) {
		delete screen;
		return;
	}
	//解析
	if (false == screen->Parse(act, HMIEVENT_INITSCRPART) ||
		false == screen->Init()) {
		delete screen;
		return;
	}
	m_ScreenList.append(screen);
	screen->raise();    // 显示
	screen->show();
	screen->SetShowStates(true);
#ifdef WIN32
	screen->setFocus(); // 给画面设置焦点，否则无法捕获键盘事件 xqh 20181217
#endif
}

//更新画面数据
void HMIMgr::UpdatePart(const hmiproto::hmiact &act) {
	if (false == act.has_scr()) {
		return;
	}
	foreach(HMIScreen *scr, m_ScreenList) {
		if (act.scr().scrno() != scr->GetScrNo()) {
			continue;
		}
		scr->Parse(act, HMIEVENT_UPDATEPART);
		break;
	}
	return;
}

void HMIMgr::ChangeScreen(hmiproto::hmiact& act) {
	if (false == act.has_event()) {
		return;
	}
	if (m_dwMemSize <= 300) {//内存低于300M的机型，每次切换画面都清理一次图片
		ImgCacheMgr::Clear();
	}
	const hmiproto::hmievent&   msgEvent = act.event();
    bool  bHideMsg = msgTipIsHide();
    bool  bHideScrSaver = screenSaverIsHide();
	CloseOpenedScreen(); // 显示新的主画面前将之前打开的画面关闭
	if (true == ScreenInCache(msgEvent.scrno())) {
		ShowScreenInCache(msgEvent.scrno());
		UpdatePart(act);
	}
	else {
		InitScrPart(act);
	}
    updateMsgTip(bHideMsg);
    updateScreenSaver(bHideScrSaver);
	return;
}

void HMIMgr::PopChildScreen(hmiproto::hmiact& act) {
	if (false == act.has_event() ||
		true == CheckInShow(act.event().scrno())) {  // 已经在显示了
		return;
	}
	InitScrPart(act);
	return;
}

void HMIMgr::CloseChildScr(hmiproto::hmiact& act) {
	int         nScrNo = -1;
	HMIScreen   *pScr = NULL;
	if (false == act.has_event()) {
		return;
	}
	nScrNo = act.event().scrno();
	for (int nI = 0; nI < m_ScreenList.size(); nI++) {
		pScr = m_ScreenList[nI];
		if (NULL == pScr || nScrNo != pScr->GetScrNo()) {
			continue;
		}
		m_ScreenList.removeAt(nI);
		delete pScr;
		break;
	}
	return;
}

void HMIMgr::RefreshScreen() {
	if (NULL != m_pClrLcdScr) {
		m_pClrLcdScr->hide();
	}
	HMIScreen*  pScr = GetTopBaseScr();
	if (NULL != pScr) {
		pScr->update();
	}
	return;
}

void HMIMgr::SendCursorPos(hmiproto::hmiact& act) {
	if (false == act.has_event() || false == act.event().has_eventbuffer()) {
		return;
	}
	const hmiproto::hmievent&   msgEvent = act.event();
	const string&               strData = msgEvent.eventbuffer();
	QPoint                      posGlobal = QCursor::pos(); // 获取相对显示器坐标
	QPoint                      qPos = this->mapFromGlobal(posGlobal); // 获取相对主画面的坐标
	QString                     szPoint = QString("%1,%2,%3").arg(qPos.x()).arg(qPos.y()).arg(strData.c_str());
	EventMgr::SendCursorPosEvent(szPoint);
	return;
}

void HMIMgr::SetCursorPos(hmiproto::hmiact& act) {
	if (false == act.has_event()) {
		return;
	}

	const hmiproto::hmievent&   msgEvent = act.event();
	int                         nX = 0;
	int                         nY = 0;
	QPoint		                posGlobal;

	nX = msgEvent.x();
	nY = msgEvent.y();
	posGlobal = this->mapToGlobal(QPoint(nX, nY));
	QCursor::setPos(posGlobal);
	return;
}

void HMIMgr::FreeScreenCache(hmiproto::hmiact& act) {
	if (false == act.has_event()) {
		return;
	}
	int         nScrNo = -1;
	HMIScreen   *pScr = NULL;
	nScrNo = act.event().scrno();
	for (int i = 0; i < m_ScreenList.size(); i++) {
		pScr = m_ScreenList[i];
		if (NULL == pScr || nScrNo != pScr->GetScrNo()) {
			continue;
		}
		if (true == pScr->GetShowStates()) {// 画面正在显示
			break;
		}
		m_ScreenList.removeAt(i);
		delete pScr;
		break;
	}
	return;
}

void HMIMgr::raiseMsgTip() {
    if (NULL == m_msgTip) {
        return;
    }
    m_msgTip->raise();
    return;
}

void HMIMgr::ShowIndirectcreen(hmiproto::hmiact& act) {
	if (false == act.has_event() ||
		true == CheckInShow(act.event().scrno())) {  // 已经在显示了
		return;
	}
	InitScrPart(act);
	return;
}

void HMIMgr::ShowCursor(hmiproto::hmiact& act) {
#ifdef WIN32
	return;
#endif // WIN32
	if (false == act.has_event()) {
		return;
	}
	const hmiproto::hmievent&   msgEvent = act.event();
#if (QT_VERSION <= QT_VERSION_CHECK(5, 0, 0))
	QWSServer::setCursorVisible(msgEvent.cursor());
#else
	if (true == msgEvent.cursor()) {
		QApplication::setOverrideCursor(Qt::ArrowCursor);
	}
	else {
		QApplication::setOverrideCursor(Qt::BlankCursor);
	}
#endif
	return;
}

#if (QT_VERSION <= QT_VERSION_CHECK(5, 0, 0))
void HMIMgr::Screenshot(hmiproto::hmiact& act) {
	if (false == act.has_event() || false == act.event().has_imgpath()) {
		return;
	}
	const hmiproto::hmievent&   msgEvent = act.event();
	int                         nX = 0;
	int                         nY = 0;
	for (int i = 0; i < m_ScreenList.size(); i++) {  // 获取画面位置信息
		HMIScreen* pScreen = m_ScreenList[i];
		if (NULL == pScreen || pScreen->GetScrNo() != msgEvent.scrno()) {
			continue;
		}
		nX = pScreen->x() + msgEvent.x();
		nY = pScreen->y() + msgEvent.y();
		break;
	}
	//文件路径
	QString     strFileName = msgEvent.imgpath().c_str();
	QString     strResult;
	QImage      image;
	if (0 == msgEvent.imgtype()) {
		QBitmap pixmap;
#ifdef _LINUX_
		pixmap = QBitmap::grabWindow(QApplication::desktop()->winId(), nX, nY, msgEvent.width(), msgEvent.height());
#else
#if ((defined WIN32) && (defined WIN32SCALE))
		pixmap = QBitmap::grabWindow(this->winId(), nX, nY, msgEvent.width()*HMIUtility::m_dWidthScale, msgEvent.height()*HMIUtility::m_dHeightScale);
#else
		pixmap = QBitmap::grabWindow(this->winId(), nX, nY, msgEvent.width(), msgEvent.height());
#endif
#endif
		if (0 != msgEvent.degrees()) {//旋转
			QMatrix leftmatrix;
			leftmatrix.rotate(msgEvent.degrees());
			pixmap = pixmap.transformed(leftmatrix);
		}
		image = pixmap.toImage(); // 转成qimage
	}
	else {
		QPixmap pixmap;
#ifdef _LINUX_
		pixmap = QBitmap::grabWindow(QApplication::desktop()->winId(), nX, nY, msgEvent.width(), msgEvent.height());
#else
#if ((defined WIN32) && (defined WIN32SCALE))
		pixmap = QBitmap::grabWindow(this->winId(), nX, nY, msgEvent.width()*HMIUtility::m_dWidthScale, msgEvent.height()*HMIUtility::m_dHeightScale);
#else
		pixmap = QBitmap::grabWindow(this->winId(), nX, nY, msgEvent.width(), msgEvent.height());
#endif
#endif
		if (0 != msgEvent.degrees()) {//旋转
			QMatrix leftmatrix;
			leftmatrix.rotate(msgEvent.degrees());
			pixmap = pixmap.transformed(leftmatrix);
		}
		image = pixmap.toImage(); // 转成qimage
	}
#if ((defined WIN32) && (defined WIN32SCALE))
	image = image.scaled(msgEvent.paperwidth()*HMIUtility::m_dWidthScale, msgEvent.paperheight()*HMIUtility::m_dHeightScale);
#else
	image = image.scaled(msgEvent.paperwidth(), msgEvent.paperheight());
#endif
	strResult = QString::number(image.save(strFileName)) + " " + strFileName + " " + QString::number(msgEvent.imgtype());
	EventMgr::SendScreenhotEvent(strResult);
	return;
}

#else
void HMIMgr::Screenshot(hmiproto::hmiact& act) {
	if (false == act.has_event() || false == act.event().has_imgpath()) {
		return;
	}
	const hmiproto::hmievent& msgEvent = act.event();
	HMIScreen* pScr = NULL;
	for (int nI = 0; nI < m_ScreenList.size(); nI++) {
		pScr = m_ScreenList[nI];
		if (NULL != pScr && msgEvent.scrno() == pScr->GetScrNo()) {
			break;
		}
	}
	if (NULL == pScr) {
		return;
	}
	int         nX = msgEvent.x();
	int         nY = msgEvent.y();
#ifndef _LINUX_
	QScreen*    qtScreen = QGuiApplication::primaryScreen();
#endif	
	QImage      image;
	if (0 == msgEvent.imgtype()) {	
		QBitmap pixmap;
#ifdef _LINUX_
		pixmap = pScr->grab(QRect(nX, nY, msgEvent.width(), msgEvent.height()));
#elif ((defined WIN32) && (defined WIN32SCALE))
		pixmap = qtScreen->grabWindow(this->winId(), nX, nY, msgEvent.width()*HMIUtility::m_dWidthScale, msgEvent.height()*HMIUtility::m_dHeightScale);
#else
		pixmap = qtScreen->grabWindow(this->winId(), nX, nY, msgEvent.width(), msgEvent.height());
#endif
		if (0 != msgEvent.degrees()) {
			QMatrix leftmatrix;
			leftmatrix.rotate(msgEvent.degrees());
			pixmap = pixmap.transformed(leftmatrix);
		}
		image = pixmap.toImage(); // 转成qimage
	}
	else {
		QPixmap pixmap;
#ifdef _LINUX_
		pixmap = pScr->grab(QRect(nX, nY, msgEvent.width(), msgEvent.height()));
#elif ((defined WIN32) && (defined WIN32SCALE))
		pixmap = qtScreen->grabWindow(pScr->winId(), nX, nY, msgEvent.width()*HMIUtility::m_dWidthScale, msgEvent.height()*HMIUtility::m_dHeightScale);
#else
		pixmap = qtScreen->grabWindow(pScr->winId(), nX, nY, msgEvent.width(), msgEvent.height());
#endif
		if (0 != msgEvent.degrees()) {
			QMatrix leftmatrix;
			leftmatrix.rotate(msgEvent.degrees());
			pixmap = pixmap.transformed(leftmatrix);
		}
		image = pixmap.toImage(); // 转成qimage
	}
#if ((defined WIN32) && (defined WIN32SCALE))
	image = image.scaled(msgEvent.paperwidth()*HMIUtility::m_dWidthScale, msgEvent.paperheight()*HMIUtility::m_dHeightScale);
#else
	image = image.scaled(msgEvent.paperwidth(), msgEvent.paperheight());
#endif
	/* 文件路径不存在，则要创建 */
	QString sFilePath = msgEvent.imgpath().c_str();
	int nIndex = sFilePath.lastIndexOf("/");
	if (nIndex != -1) {
		sFilePath = sFilePath.left(nIndex);
		if (!QDir().exists(sFilePath)) {
			QDir().mkpath(sFilePath);
		}
	}
	QString strResult = QString::number(image.save(msgEvent.imgpath().c_str()));
	strResult += QString(" ") + msgEvent.imgpath().c_str() + " " + QString::number(msgEvent.imgtype());
	EventMgr::SendScreenhotEvent(strResult);
	return;
}
#endif

int	HMIMgr::runSyssetScreenFilter(unsigned int nEventType) {
	if (!m_bSyssetScreenRunning) {
		return 0;
	}
	if (nEventType == HMIEVENT_INITSCRPART ||
		nEventType == HMIEVENT_UPDATEPART ||
		nEventType == HMIEVENT_CHANGESCR ||
		nEventType == HMIEVENT_POPSCR ||
		nEventType == HMIEVENT_UPDATESCREEN ||
		nEventType == HMIEVENT_INDIRECTSCRINIT ||
		nEventType == HMIEVENT_PRINTPART ||
		nEventType == HMIEVENT_DISPLAYMSG ||
		nEventType == HMIEVENT_CLEAR_LCD ||
		nEventType == HMIEVENT_SHOW_SCREENSAVER ||
		nEventType == HMIEVENT_CLOSE_SCREENSAVER) {
		return -1;
	}
	return 0;
}

void HMIMgr::Slot_QuicklyDoEvent(hmiproto::hmiact& act) {
	int nRtn = 0;
	if (false == act.has_event()) {
		return;
	}
	const hmiproto::hmievent	&msgEvent = act.event();
	unsigned int nType = msgEvent.type();
	if ((nRtn = this->runSyssetScreenFilter(nType)) < 0) {
		return;
	}
	switch (nType) {
	case HMIEVENT_INITSCRPART: // 初始化画面
		{
			InitScrPart(act);
			SendUpdateTopScr();
		}
		break;
	case HMIEVENT_UPDATEPART: // 更新部件
		{
			UpdatePart(act);
		}
		break;
	case HMIEVENT_CHANGESCR: //画面跳转
		{
			ChangeScreen(act);
			raiseMsgTip();
			SendUpdateTopScr();
		}
		break;
	case HMIEVENT_POPSCR: // 弹出子画面
		{
			PopChildScreen(act);
			raiseMsgTip();
			SendUpdateTopScr();
		}
		break;
	case HMIEVENT_CLOSECHILDSCR: //关闭子画面
		{
			CloseChildScr(act);
			SendUpdateTopScr();
		}
		break;
	case HMIEVENT_UPDATESCREEN: // 刷新画面
		{
			RefreshScreen();
		}
		break;
	case HMIEVENT_INDIRECTSCRINIT: // 间接画面初始化
		{
			ShowIndirectcreen(act);
		}
		break;
	case HMIEVENT_SHOWCURSOR:       //显示/隐藏鼠标
		{
			ShowCursor(act);
		}
		break;
	case HMIEVENT_PRINTPART: // 截图
		{
			Screenshot(act);
		}
		break;
	case HMIEVENT_SETCURSOR: // 设置鼠标位置
		{
			SetCursorPos(act);
		}
		break;
	case HMIEVENT_GETCURSOR: // 获取鼠标位置
		{
			SendCursorPos(act);
		}
		break;
	case HMIEVENT_FREESCRCACHE: // 移除画面缓存
		{
			FreeScreenCache(act);
		}
		break;
	case HMIEVENT_EXITUI:
		{
			qApp->exit(0);
		}
		break;
	case HMIEVENT_DISPLAYMSG:
		{
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
            showScreenSaver((CScreenSaver::TYPE_T)msgEvent.scrno());
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

#if 0 //后台切换指标，勿删除 add by xiangk
void HMIMgr::slot_showSys(int nType) {
	printf("slot_showSys --- [%d]\n", nType);
	if (1 == nType){
		CSysSetUi::get_instance().showUi(0);
	}
	if (2 == nType){
		CSysSetUi::get_instance().showch(0);
	}
	if (3 == nType) {
		CSysSetUi::get_instance().showch(1);
	}
	if (4 == nType) {
		CSysSetUi::get_instance().showch(2);
	}
	if (5 == nType) {
		CSysSetUi::get_instance().showch(3);
	}
	if (6 == nType) {
		CSysSetUi::get_instance().slot_Return();
	}
}
#endif

bool HMIMgr::ScreenInCache(const int& nScrNo) {
	HMIScreen   *pScr = NULL;
	bool        bRtn = false;
	for (int nI = 0; nI < m_ScreenList.size(); nI++) {
		pScr = m_ScreenList[nI];
		if (NULL != pScr && nScrNo == pScr->GetScrNo()) {
			bRtn = true;
			break;
		}
	}
	return bRtn;
}

void HMIMgr::CloseOpenedScreen() {
	HMIScreen   *pScr = NULL;
	for (int i = 0; i < m_ScreenList.size(); i++) {
		pScr = m_ScreenList[i];
		if (NULL == pScr || false == pScr->GetShowStates()) {
			continue;
		}
		if (SCREEN_MAIN == pScr->GetScrType()) {
			pScr->OnScrHide();
			pScr->SetShowStates(false);
		}
		else {    // 间接画面和子画面不缓存直接删除
			m_ScreenList.removeAt(i);
			delete pScr;
			i--;
		}
	}
	return;
}

void HMIMgr::ShowScreenInCache(const int& nScrNo) {
	HMIScreen   *pScr = NULL;
	for (int i = 0; i < m_ScreenList.size(); i++) {
		pScr = m_ScreenList[i];
		if (NULL == pScr || nScrNo != pScr->GetScrNo()) {
			continue;
		}
		pScr->SetShowStates(true);
		pScr->OnScrShow();
		pScr->raise();    // 显示
		pScr->show();
#		ifdef WIN32
		pScr->setFocus(); // 给画面设置焦点，否则无法捕获键盘事件 xqh 20181217
#		endif
		// 将显示的画面放在最尾巴
		m_ScreenList.removeAt(i);
		m_ScreenList.append(pScr);
		break;
	}
	return;
}

bool HMIMgr::CheckInShow(int nScrNo) {
	HMIScreen   *pScr = NULL;
	bool        bRet = false;
	for (int i = 0; i < m_ScreenList.size(); i++) {
		pScr = m_ScreenList[i];
		if (NULL != pScr && nScrNo == pScr->GetScrNo() && true == pScr->GetShowStates()) {
			bRet = true;
			break;
		}
	}
	return bRet;
}

void HMIMgr::clearLcd(hmiproto::hmiact& act) {
	if (false == act.has_event()) {
		return;
	}
	if (NULL == m_pClrLcdScr) {
		m_pClrLcdScr = new QWidget();
		if (NULL == m_pClrLcdScr) {
			return;
		}
		int nWidth = 0;
		int nHeight = 0;
		int nBitPerPixel = 0;
#ifdef WIN32
		nWidth = 1920;
		nHeight = 1080;
		nBitPerPixel = 32;
#else
		cbl::CLcd devLcd;
		devLcd.getResolution(nWidth, nHeight, nBitPerPixel);
#endif 
		m_pClrLcdScr->setFixedSize(nWidth, nHeight);
		m_pClrLcdScr->setAttribute(Qt::WA_TransparentForMouseEvents);
		m_pClrLcdScr->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
	}
#ifdef WIN32
	m_pClrLcdScr->setParent(this);
#endif // WIN32
	unsigned int    nBgColor = act.event().color();
	QColor          qColor;
	if (0 == nBgColor) {  /* 刷黑 */
		qColor.setRgb(qRgb(0, 0, 0));
	}
	else if (1 == nBgColor) { /* 刷白 */
		qColor.setRgb(qRgb(255, 255, 255));
	}
	QPalette	qPalette;
	qPalette.setColor(QPalette::Background, qColor);
	m_pClrLcdScr->setPalette(qPalette);
	m_pClrLcdScr->setAutoFillBackground(true);
	m_pClrLcdScr->raise();
	m_pClrLcdScr->show();
	return;
}

void HMIMgr::showMsgTip(hmiproto::hmiact& act) {
    if (false == act.has_message()) {
        return;
    }
    if (NULL == m_msgTip) {
        m_msgTip = new CMsgTip(GetTopBaseScr());
    }
    m_msgTip->handleEvent(act.message());
    return;
}

bool HMIMgr::msgTipIsHide() {
    if (NULL == m_msgTip) {
        return true;
    }
    return m_msgTip->isHidden();
}

int HMIMgr::updateMsgTip(bool bHide) {
    if (NULL == m_msgTip) {
        return -1;
    }
    m_msgTip->setNewParent(GetTopBaseScr());
    if (false == bHide) {
        m_msgTip->show();
    }
    return 0;
}

int HMIMgr::showScreenSaver(CScreenSaver::TYPE_T type) {
    if ((nullptr != m_pScreenSaver) && (type == m_pScreenSaver->type())) {
        m_pScreenSaver->start(GetTopBaseScr());
        return 0;
    }
    if (nullptr != m_pScreenSaver) {
        delete m_pScreenSaver;
    }
    m_pScreenSaver = createScreenSaver(type);
    if (nullptr == m_pScreenSaver) {
        return -1;
    }
    m_pScreenSaver->start(GetTopBaseScr());
    return 0;
}

#if 0 //后台切换指标测试线程，勿删除 add by xiangk
#include <QDateTime>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

void *runSyStem(void *arg) {
	HMIMgr *p = (HMIMgr *)arg;
	static int i = 0, j = 0;
	emit p->sgl_showSys(i);
	i++;
	if (i == 7){
		// 获取当前时间
		QDateTime currentTime = QDateTime::currentDateTime();
		QString formattedTime = currentTime.toString("yyyy-MM-dd HH:mm:ss");
		// 打开文件
#ifdef _LINUX_
		QString fileName = "/mnt/data/output.txt";
#else
		QString fileName = "D:\\output.txt";
#endif
		QFile file(fileName);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			//qDebug() << "无法打开文件：" << file.errorString();
			return nullptr;
		}
		// 写入时间到文件
		QTextStream stream(&file);
		stream << "time:" << formattedTime << "num:" << ++j <<"\n";
		// 关闭文件
		file.close();
		i = 0;
	}
	return nullptr;
}
#endif

int HMIMgr::showSysSetScreen(int nScreen) {
#if 0 //后台切换指标测试入口，勿删除 add by xiangk
	connect(this, &HMIMgr::sgl_showSys, this, &HMIMgr::slot_showSys, Qt::BlockingQueuedConnection);
	CRateTimer::addTimer(runSyStem, this, (int)(&runSyStem), 500);
	return 0;
#endif

	m_bSyssetScreenRunning = true;
	CSysSetUi::get_instance().registerGoback([ this ] (SYSSET_EVENT_TYPE_T nEvenType, void *lpParameter) {
		lpParameter = lpParameter;
		if (nEvenType == SYSSET_EVENT_TYPE_T::EVENT_ExitSysSet) {
			m_bSyssetScreenRunning = false;
			EventMgr::SendExitSysSet();
		}
		return 0;
	}, this);
	CSysSetUi::get_instance().showUi(nScreen, this->GetTopScr());

	return 0;
}

void HMIMgr::quitScreenSaver() {
	if (nullptr == m_pScreenSaver) {
		return;
	}
	m_pScreenSaver->stop();
}

bool HMIMgr::screenSaverIsHide() {
    if (nullptr == m_pScreenSaver) {
        return true;
    }
    if (false == m_pScreenSaver->isRunning()) {
        return true;
    }
    return false;
}

int HMIMgr::updateScreenSaver(bool bHide) {
    if (true == bHide) {
        return -1;
    }
    if (nullptr == m_pScreenSaver) {
        return -2;
    }
    if (false == m_pScreenSaver->isRunning()) {
        return -3;
    }
	m_pScreenSaver->setParent(GetTopScr());
    return 0;
}

void HMIMgr::SendUpdateTopScr() {
	HMIScreen *pTopScr = GetRealTopScr();
	int x = -1;
	int y = -1;
	if (nullptr != pTopScr){
		pTopScr->GetScrPopPoint(x, y);
		EventMgr::SendUpdateTopScrEvent(pTopScr->GetScrNo() , x, y);
	}
	return;
}
HMIScreen* HMIMgr::GetRealTopScr() {
	if (m_ScreenList.size() == 0) {
		return nullptr;
	}
	for (int i = m_ScreenList.size() - 1; i >= 0; i--) {
		HMIScreen* pScr = nullptr;
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

int HMIMgr::parseCustomJson(hmiproto::hmiact& act) {
	if (false == act.has_event()) {
		return - 1;
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