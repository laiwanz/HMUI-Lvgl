#ifndef _BALL_H__20220909
#define _BALL_H__20220909

#include <stdio.h>
#include <QPainter>
#include <QPoint>
#include <QList>
#include <QPixmap>

class CBall {
public:
    CBall(QList<QPixmap>& listPix);
	~CBall();
    
    void setCenter(const QPoint& pt);
    const QPoint& getCenter();
    void setRadius(int nRadius);
    int getRadius();
    void setSpeedX(int speed);
    int getSpeedX();
    void setSpeedY(int speed);
    int getSpeedY();
    void setDisp(bool bDisp);
    bool getDisp();
    void paint(QPainter& p);
    void resetOpa();

private:
    int init();

private:
    int     m_nRadius;      /* 半径 */
    QPoint  m_ptCenter;     /* 圆心 */
    int     m_nSpeedX;      /* x方向速度 */
    int     m_nSpeedY;      /* y方向速度 */
    bool    m_bDisp;        /* 是否显示 */
    float   m_fOpa[3];      /* 透明度 */
    float   m_fWeight[3];   /* b、g、r颜色权重 */
    QPixmap     m_pix;      /* 气泡当前样式 */
    QList<QPixmap>& m_listPix;  /* 气泡图片 */
};
#endif
