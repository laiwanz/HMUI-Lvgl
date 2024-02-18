#ifndef HMIMGR_H
#define HMIMGR_H
#include <QFrame>
#include <QList>
#include <QThread>
#include <QMutex>
#include "hmidef.h"
#include "hmiscreen.h"
#include "./msgTip/msgTip/msgTip.h"
#include "./screenSaver/screenSaverFactory.h"
#include "SysSetGui/SysSetGui.h"

class HMIMgr : public QFrame {
	Q_OBJECT
public:
	explicit HMIMgr(QFrame *pFrame = 0);
	~HMIMgr();
protected:
#ifndef _LINUX_
	virtual void closeEvent(QCloseEvent *);
#endif
public:
	void ImgCache(const void *pListImg, QString strPath);//初始化图片信息
	void InitScrPart(const hmiproto::hmiact &act);
	HMIScreen* GetTopBaseScr();							//获取当前显示的主画面
	HMIScreen* GetTopScr();								//获取当前显示的置顶画面
	CScreenSaver* GetScreenSaver();
	void sendQuicklyDoEventSignal(hmiproto::hmiact& act);
	HMIScreen* GetRealTopScr();
	void quitScreenSaver();
	int parseCustomJson(hmiproto::hmiact& act);
private:
	bool    CheckInShow(int nScrNo);                // 判断是否已经在显示了
	void    ChangeScreen(hmiproto::hmiact& act);    // 切换画面
	bool    ScreenInCache(const int& nScrNo);       // 画面是否在缓存
	void    ShowScreenInCache(const int& nScrNo);   // 显示缓存中的画面
	void    PopChildScreen(hmiproto::hmiact& act);  // 弹出子画面
	void    CloseChildScr(hmiproto::hmiact& act);   // 关闭子画面
	void    CloseOpenedScreen();                    // 关闭打开的画面
	void    ShowIndirectcreen(hmiproto::hmiact& act);// 显示间接画面
	void    ShowCursor(hmiproto::hmiact& act);      // 显示光标
	void    Screenshot(hmiproto::hmiact& act);      // 截图
	void    RefreshScreen();                        // 刷新画面
	void    SendCursorPos(hmiproto::hmiact& act);	// 发送鼠标位置给Hmi
	void    SetCursorPos(hmiproto::hmiact& act);    // 设置鼠标位置
	void    FreeScreenCache(hmiproto::hmiact& act); // 释放画面缓存
	void	raiseMsgTip();							// 重新加载消息提示框
private:
	void	keyPressEvent(QKeyEvent *eventT);	// 按键按下事件
	void	keyReleaseEvent(QKeyEvent *eventT); // 按键抬起事件
	void	UpdatePart(const hmiproto::hmiact &act);
	void	clearLcd(hmiproto::hmiact& act);
    QFrame* getParent(const hmiproto::hmiact& act);
    void	showMsgTip(hmiproto::hmiact &act);
    bool    msgTipIsHide();
    int     updateMsgTip(bool bHide);
    int     showScreenSaver(CScreenSaver::TYPE_T type);
	int		showSysSetScreen(int nScreen);
    bool    screenSaverIsHide();
    int     updateScreenSaver(bool bHide);
	void	SendUpdateTopScr();
	int		runSyssetScreenFilter(unsigned int nEventType);
private:
	QList<HMIScreen *>			m_ScreenList;		//所有已打开的画面，包括缓存中
	unsigned int				m_dwMemSize;		//物理内存的大小
	QWidget*                    m_pClrLcdScr;       /* 清屏用的画面 */
	CMsgTip*                    m_msgTip;
    CScreenSaver*               m_pScreenSaver;      /* 屏保画面 */
	bool						m_bSyssetScreenRunning;
private slots:
	void Slot_QuicklyDoEvent(hmiproto::hmiact& act);
#if 0//后台切换指标，勿删除 add by xiangk
	void slot_showSys(int nType);
#endif

signals:
	void sgl_showSys(int nType);
	void Signal_QuicklyDoEvent(hmiproto::hmiact&);
#if ((defined WIN32) && (defined WIN32SCALE))
public:
	virtual void resizeEvent(QResizeEvent *eventT);
#endif
};
#endif // HMIMGR_H
