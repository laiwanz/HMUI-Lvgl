#ifndef _BLINDS_H__20220826
#define _BLINDS_H__20220826


#include <QVector>
#include "../screenSaver/screenSaver.h"
#include "leaf.h"

class CBlinds : public CScreenSaver {
    Q_OBJECT
public:
    CBlinds();
	virtual ~CBlinds();
    int start(QWidget* widget);
    bool isRunning();
    int stop();

protected:
    void paintEvent(QPaintEvent* ev);

private:
    int init();
    void resetArea();

private slots:
    void slotTimer();
    void resetDirect(DIRECT_T direct);
    void ascentLeaf();
    void descentLeaf();

private:
    QVector<CLeaf*> m_vecLeaf;      /* 叶片 */
    QTimer*         m_pTimer;       /* 定时器 */
    int             m_nFrameCnt;    /* 帧数 */
    int             m_nLeafHeight;  /* 叶片高 */
    bool            m_bInit;        /* 是否初始化 */
    bool            m_bFinishFlag;  /* 结束标记 */
    int             m_nBrushType;   /* 画刷类型 */
    bool            m_bFill;        /* 是否填充 */
};
#endif
