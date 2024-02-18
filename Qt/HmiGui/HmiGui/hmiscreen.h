#ifndef HMISCREEN_H
#define HMISCREEN_H
#include <QFrame>
#include <QList>
#include <QString>
#include <QDebug>
#include "hmidef.h"
#include "hmipart.h"
#include "ProtoXML/ProtoXML.h"

class HMIScreen : public QFrame {
	Q_OBJECT
public:
	explicit HMIScreen(QFrame *pFrame);
	~HMIScreen();
protected:
	void paintEvent(QPaintEvent *);
public:
	bool CheckInShowBaseScr(const hmiproto::hmiact &act, int nBaseScr);		//检查内置画面号
	bool Parse(const hmiproto::hmiact &act, int nDataType);					//解析数据
	int parseJson(const hmiproto::hmiact &act);
	void SetScrNo(int nScrNo);			// 设置画面号
	int GetScrNo();						// 获取画面号
	int GetScrType();					// 获取画面类型
	void setScrType(const int type);	// 设置画面类型
	bool Init();						// 初始化画面及部件
	bool IsInit();						// 是否初始化了
	void OnScrShow();					// 当画面从缓存中显示出来
	void OnScrHide();					// 当画面被置底了
	void setParent(QFrame *pFrame);		// 设置父窗口
	QFrame* getParent();				// 获取父窗口
	void SetShowStates(bool bStates);
	bool GetShowStates();
	void GetScrPopPoint(int &x, int &y);
#if ((defined WIN32) && (defined WIN32SCALE))
	void ScaleFrameSize();              // 设置缩放大小
	QList<HMIPart *>* GetPartlist();	// 获取部件链表
#endif
private:
	void mousePressEvent(QMouseEvent *eventT);
	void mouseReleaseEvent(QMouseEvent *eventT);
	template<class  PART, class  STRUCT>
	void ParsePart(STRUCT T, int nDataType);
	void SendClickEvent(int nScrNo, int nX, int nY, bool bDown);

public slots:
    void RecvOnclickedDown(QMouseEvent *eventT, int nX, int nY);
    void RecvOnclickedUp(QMouseEvent *eventT);
private:
	hmiproto::hmiscreen m_ScrInfo;		// 画面信息
	int                 m_nScrNo;		// 画面号
	unsigned short      m_wScrType;		// 画面类型
	QList<HMIPart *>    m_partlist;		// 所有部件
	QFrame*             m_parent;		// 父窗口
	bool                m_bClick;		// 是否有点击
	bool                m_bShow;        // 是否在显示
	// 使用静态变量记录点击的坐标、画面号，保证抬起和点击的坐标、画面一致 xqh 20200619
	static int          m_x;
	static int          m_y;
	static int          m_nClickScrNo;
#ifdef _LINUX_
	static unsigned long long     m_lastClick;   //最后一次获取单击时间
#endif
	int       m_nImgIndex;				// 图片序号
#if ((defined WIN32) && (defined WIN32SCALE))
	QRect     m_OriginalRect;
#endif
};
#endif // HMISCREEN_H
