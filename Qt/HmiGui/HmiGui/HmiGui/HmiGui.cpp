#include "HmiGui.h"
#include <QTextCodec>
#include <QDesktopWidget>
#include <QDir>
#include <QLabel>
#include "../hmimgr.h"
#include "../hmiutility.h"
#include "../EventMgr/EventMgr.h"
#include <platform/platform.h>
#include <QTimer>
#include "../commonpart.h"
#include "monitorScreen.h"

#if (QT_VERSION <= QT_VERSION_CHECK(5, 0, 0))
#include <QWSServer>
#endif
using namespace ProtoXML;
#include <QDebug>
void* HmiGui::m_pHmiMgr = NULL;

void HmiGui::SetSendFunc(FUNCTION_SENDEVENT func) {
	EventMgr::SetSendFunc(func);
}

/****************************************
作  者：xqh
时  间：20201022
功  能：立即处理hmi事件接口
参  数：
		nScrNo：画面号
		user：用户ID
		wDataType：数据类型
返回值：
		无
****************************************/
void HmiGui::doHmiEventQuickly(hmiproto::hmiact& act) {
	if (NULL == m_pHmiMgr) {
		return;
	}

	static_cast<HMIMgr*>( m_pHmiMgr )->sendQuicklyDoEventSignal(act);
	return;
}

void HmiGui::setScale(int nWidth, int nHeight, int argc, char* argv[]) {
	if (NULL == m_pHmiMgr)  // hmi还未创建
	{
		return;
	}

#if ((defined WIN32) && (defined WIN32SCALE))
	int                 nOperationType = 0;
	int                 nScaleWidth = 800;
	int                 nScaleHeight = 480;

	HMIUtility::m_nInitWidth = nWidth;
	HMIUtility::m_nInitHeight = nHeight;

	if (3 == argc) {
		QDesktopWidget*     desktopWidget = QApplication::desktop();
		QRect               screenRect = desktopWidget->availableGeometry();
		double              dRate = 1;
		int                 nCurrentScreenW = screenRect.width() - 20;
		int                 nCurrentScreenH = screenRect.height() - 40;
		int                 nWidthTemp = nWidth;
		int                 nHeightTemp = nHeight;

		while (nWidthTemp > nCurrentScreenW || nHeightTemp > nCurrentScreenH) {
			nWidthTemp = nWidth * dRate;
			nHeightTemp = nHeight * dRate;
			dRate -= 0.05;

			if (dRate <= 0) {
				break;
			}
		}

		nOperationType = Fixed_Tension;
		nScaleWidth = nWidthTemp;
		nScaleHeight = nHeightTemp;
	}
	else if (6 == argc) // 缩放
	{
		nOperationType = atoi(argv[3]);
		nScaleWidth = atoi(argv[4]);
		nScaleHeight = atoi(argv[5]);
	}

	HMIUtility::SetScaling(nScaleWidth, nScaleHeight);

	switch (nOperationType) {
	case Fixed_Tension:
		( (HMIMgr*) m_pHmiMgr )->setFixedSize(nScaleWidth, nScaleHeight);
		break;
	case Free_Stretching:
		( (HMIMgr*) m_pHmiMgr )->resize(QSize(nScaleWidth, nScaleHeight));
		break;
	default:
		break;
	}

	if (0 == nOperationType) {
		if (1 != HMIUtility::m_dWidthScale) {
			QString szTitle = "HMIUI(" + QString::number(HMIUtility::m_dWidthScale * 100) + "%)";
			( (HMIMgr*) ( (HMIMgr*) m_pHmiMgr ) )->setWindowTitle(szTitle);
		}
	}

#else
	static_cast<HMIMgr*>( m_pHmiMgr )->setFixedSize(nWidth, nHeight);
#endif

	return;
}

#ifndef _LINUX_
void HmiGui::GetScale(double &dWidthScale, double &dHeightScale) {
	dWidthScale = HMIUtility::m_dWidthScale;
	dHeightScale = HMIUtility::m_dHeightScale;
}

void HmiGui::GetScaleWH(int &nWidth, int &nHeight) {
	nWidth = (double) (HMIUtility::m_nInitWidth)*(double)HMIUtility::m_dWidthScale + 0.5;
	nHeight = (double)(HMIUtility::m_nInitHeight)*(double)HMIUtility::m_dHeightScale + 0.5;
}
#endif

void initQt() {
#ifndef WIN32
#if (QT_VERSION <= QT_VERSION_CHECK(5, 0, 0))
	QWSServer::setBackground(QBrush(Qt::NoBrush));
	QWSServer::setCursorVisible(false); // 程序启动时，先默认设为隐藏光标
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#else // use qt5
	QApplication::setOverrideCursor(Qt::BlankCursor);
#endif
#endif
	qRegisterMetaType<std::string>("std::string");
	return;
}

static char szQtParams[6][32] = { "HMIGUI", "-qws", "-display", "Transformed:Rot90",
								  "Transformed:Rot180", "Transformed:Rot270" };

static int getQtParams(const std::string& sPrjPath, int& argc, char* argv[4]) {
	argc = 0;
	argv[argc++] = szQtParams[0];
#ifndef WIN32
#if (QT_VERSION <= QT_VERSION_CHECK(5, 0, 0))
	argv[argc++] = szQtParams[1];
#endif
	std::string sRotation;
	std::string sFile = sPrjPath + "/HMIUIRot";
	if (cbl::loadFromFile(sFile, sRotation) <= 0) {
		printf("%s:load %s error\n", __FUNCTION__, sFile.c_str());
		return -1;
	}
	unsigned long ulRot = strtoul(sRotation.c_str(), NULL, 10);
	argv[argc++] = szQtParams[2];
	switch (ulRot) {
	case 90: {
#if (QT_VERSION <= QT_VERSION_CHECK(5, 0, 0))
		argv[argc++] = szQtParams[3];
#else
		argv[argc++] = szQtParams[5];
#endif
		}
		break;
	case 180: {
			argv[argc++] = szQtParams[4];
		}
		break;
	case 270: {
#if (QT_VERSION <= QT_VERSION_CHECK(5, 0, 0))
		argv[argc++] = szQtParams[5];
#else
		argv[argc++] = szQtParams[3];
#endif
		}
		break;
	default: {
			argv[--argc] = NULL;
		}
		break;
	}
#endif
	return 0;
}

int HmiGui::runGui(int argc, char* argv[], const std::string& strImgPath, void* pPicList) {
	char*   qtArgv[4] = { 0 };
	int     qtArgc = 0;
	getQtParams(strImgPath, qtArgc, qtArgv);
#ifdef WIN32
#ifdef _DEBUG
	qputenv("QT_IM_MODULE", QByteArray("Qt5Input"));
	MyApplication a(qtArgc, qtArgv);
#else
	QApplication a(qtArgc, qtArgv);
#endif
#else
	qputenv("QT_IM_MODULE", QByteArray("Qt5Input"));
	MyApplication a(qtArgc, qtArgv);
#endif
	initQt();
	QString     strPrjPath = strImgPath.c_str();
#ifdef WIN32
	QDir::setCurrent(strPrjPath);
#endif
	HMIMgr              hmimgr;
	m_pHmiMgr = &hmimgr;
	//请求图片缓存信息
	hmimgr.ImgCache(pPicList, strPrjPath);
	//读取qpf2文件头信息
	CommonPart::ReadQpf2File();
	EventMgr::SendInitScrEvent(-1);
    int nWidth = 0, nHeight = 0;
#ifdef WIN32
	nWidth = atoi(argv[1]);
	nHeight = atoi(argv[2]);
	setScale(nWidth, nHeight, argc, argv);
	hmimgr.show();
#else
    int nTemp = 0;
	cbl::CLcd devLcd;
	devLcd.getResolution(nWidth, nHeight, nTemp);

    hmimgr.setFixedSize(nWidth, nHeight);
    hmimgr.setWindowFlags(Qt::FramelessWindowHint);
#endif
	return a.exec();
}

int HmiGui::runMsg(int argc, char* argv[], const std::string& strMsgText) {
#ifdef WIN32
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	UNREFERENCED_PARAMETER(strMsgText);
	return 0;
#else
#if (QT_VERSION <= QT_VERSION_CHECK(5, 0, 0))
	char    szParame[32] = { "-qws" };
	char*   parames[2] = { argv[0],  szParame };
	argc = 2;
	QApplication    a(argc, parames);
#else
	QApplication    a(argc, argv);
#endif
	int             nWidth = 0, nHeight = 0, nBitPerPixel = 0;
	initQt();
	cbl::CLcd devLcd;
	devLcd.getResolution(nWidth, nHeight, nBitPerPixel);

	QWidget w;
	w.setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
	w.setGeometry(0, 0, nWidth, nHeight);

	QFont   font;
	font.setFamily("simsun");

	if (320 == nWidth && 240 == nHeight) {  // 3.5寸
		font.setPixelSize(12);
	}
	else if (480 == nWidth && 272 == nHeight) { // 4.3寸
		font.setPixelSize(14);
	}
	else if (800 == nWidth && 480 == nHeight) { // 7寸
		font.setPixelSize(26);
	}
	else if (1024 == nWidth && 600 == nHeight) {    // 10.2寸
		font.setPixelSize(26);
	}
	else if (1920 == nWidth && 1080 == nHeight) {   // 15寸
		font.setPixelSize(40);
	}
	else {  // 其它
		font.setPixelSize(20);
	}
    QLabel  labTip(&w);
    labTip.setFont(font);
    labTip.setGeometry(0, 0, nWidth, nHeight);
    labTip.setAlignment(Qt::AlignCenter);
    labTip.setWordWrap(true);
    labTip.setStyleSheet("background-color: transparent;");
    labTip.setText(QString::fromStdString(strMsgText));
	QTimer::singleShot(3000, &a, SLOT(quit()));
    w.show();
	return a.exec();
#endif
}

int HmiGui::setCommuConfig(const COMMU_MGR_CONFIG_T &config) {
	return CSysSetBase::get_instance().configCommuMgr(config);
}