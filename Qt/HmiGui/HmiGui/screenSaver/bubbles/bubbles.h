#ifndef _BUBBLES_H__20220909
#define _BUBBLES_H__20220909

#include <stdint.h>
#include <QWidget>
#include <QVector>
#include <QTimer>
#include <QList>

#include "../screenSaver/screenSaver.h"
#include "ball.h"

class CBubbles : public CScreenSaver {
    Q_OBJECT
public:
    CBubbles();
	virtual ~CBubbles();
    int start(QWidget* widget);
    bool isRunning();
    int stop();

protected:
    void paintEvent(QPaintEvent*);

private slots:
    int slotTimer();

private:
    int init(int w, int h);
    int loadConf(int w, int h);
    int createBubbl();
    int loadImg();
    void resetBallState();
    void popBubble();
    void updateBubblePos();
    void checkWalls(CBall& bubbl);
    void checkCollision(CBall& bub1, CBall& bub2);
    void checkStop();

private:
    QVector<CBall*>     m_vecBubbl;     /* 泡泡信息 */
    QList<QPixmap>      m_listPix;      /* 泡泡图片链表 */
    QTimer*             m_timer;        /* 定时器 */
    int                 m_nInterval;    /* 定时器周期时间(ms) */
    int                 m_nMaxSpeedX;   /* x方向最大速度 */
    int                 m_nMaxSpeedY;   /* y方向最大速度 */
    int                 m_nRadius;      /* 泡泡半径 */
    int                 m_nBubbleNum;   /* 泡泡数量 */
    bool                m_bAllShow;     /* 所有泡泡都弹出了 */
    bool                m_bInit;        /* 是否初始化 */
    uint64_t            m_ullPopTime;   /* 最后一次弹出泡泡的时间 */
};

#endif
