#ifndef _MSGTIP_H__20220620
#define _MSGTIP_H__20220620

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QPainter>
#include <QMouseEvent>
#include "../tipWidget/tipWidget.h"
#include "../lineShap/lineShap.h"
#include "ProtoXML/ProtoXML.h"

class CMsgTip : public QWidget {
	Q_OBJECT
public:
    CMsgTip(QWidget* pParent = nullptr);
	~CMsgTip();
    
    int  handleEvent(const hmiproto::messages& msg);
    void timer();
    void setNewParent(QWidget* pParent);

private:
    int  init();
    void setLanguage(int nLanguage);
    void setDispDebugTip(bool enabled);
    void setDispCloseBtn(bool enabled);
    void setDisp(bool enabled);
    void setFold(bool enabled);
    void setDispFoldBtn(bool enabled);

    void setTopTipText(int nTextId);
    void setTopTipText(const QString& strText);
    int  msgNum();
    void getNewMsg(QString& strMsg, int& nType, int& nLeve);
    int  getText(int id, QString& strText);
	void paintEvent(QPaintEvent*);
    void drawTipAreaFram(QPainter& p);
    void drawBtnImg(QPainter& p);
    void updateHeight();
    
    void updateNewMsg();
    void updateTopTipArea();
    void updateBtnFold();
    void updateDispState();
    void mousePressEvent(QMouseEvent* pEvent);
    void mouseReleaseEvent(QMouseEvent* pEvent);
    //void showModalWidget();
    void dispAllMsg(bool beep);
    void dispDebugMsg(bool beep);

private slots:
    void slotBtnFoldClick();
    void slotBtnAllClick();
    void slotBtnCommonClick();
    void slotBtnTimeoutClick();
    void slotBtnDebugClick();
    void slotUpdate();
    void slotBtnCloseClick();
    void slotUpdateVerBar(int, int);

signals:
    void sigUpdate();

private:
    QPushButton*    m_btnFold;      /* 折叠btn(展开/折叠) */
    QPushButton*    m_btnClose;     /* 关闭btn */
    QLabel*         m_labTopTip;    /* 顶部提示 */
    QPushButton*    m_btnAll;       /* 所有消息切换btn */
    QPushButton*    m_btnTimeout;   /* 超时消息切换btn */
    QPushButton*    m_btnCommon;    /* 普通消息切换btn */
    QPushButton*    m_btnDebug;     /* 调试消息切换btn */
    QWidget*        m_widgetScroll; /* 消息显示区域 */
    QScrollBar*     m_verBar;       /* 垂直滚动条 */
    CTipWidget*     m_tipTimeout;   /* 超时消息显示widget */
    CTipWidget*     m_tipCommon;    /* 普通消息显示widget */
    CTipWidget*     m_tipDebug;     /* 调试消息显示widget */
    CLineShap*      m_line;         /* 线显示widget */
    //QWidget*        m_widgetModal;  /* 模态窗口 */
    int             m_nLanguage;    /* 语言 */
    int             m_nFontSize;    /* 字体大小 */
    int             m_nWidth;       /* 宽 */
    int             m_nHeight;      /* 高 */
    int             m_nMargin;      /* 边距 */
    int             m_nBorderWidth; /* 边宽 */
    bool            m_bFold;        /* 是否折叠 */
    bool            m_bTopMsgElide;  /* 顶部消息是否省略 */
    uint64_t        m_ullUpdateTime;/* 更新时间 */
    QString         m_scrClick;     /* 点击的画面 */
};
#endif
