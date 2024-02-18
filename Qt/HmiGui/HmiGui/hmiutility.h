#ifndef HMIUTILITY_H
#define HMIUTILITY_H
#include <QChar>
#include "hmidef.h"
#include <QColor>
#include <QResizeEvent>
#include <QSize>
#include <QRect>
#include <QPoint>
#include "macrosplatform/macrosplatform.h"

QString HMISTR(const char *pStr);
void QStringToChar(const QString &strIn, char *psOut, const int &nMaxLen);
#ifdef _LINUX_
#include <stdio.h>
#define _snprintf   snprintf
#endif
#if ((defined WIN32) && (defined WIN32SCALE))
enum {
	Fixed_Tension = 0,  //窗口固定大小
	Free_Stretching = 1,//窗口可拉伸
};
#endif

class HMIUtility {
public:
	HMIUtility();

public:
#if ((defined WIN32) && (defined WIN32SCALE))
	static void     SetScaling(QResizeEvent *event);                        //计算缩放比例
	static void     SetScaling(int nScaleWidth, int nScaleHeight);          //计算缩放比例
	static void     CountScaleRect(RECT *rect);                             //计算缩放后的rect
	static void     CountScaleRect(QRect *rect);                            //计算缩放后的rect
	static void     CountScaleSize(QSize *size);                            //计算缩放后的size
	static void     CountScaleWH(int *pnWidth, int *pnHeight);              //计算缩放后的宽高
	static void     CountMousePoint(QPoint *point);                         //计算鼠标坐标
#endif

public:
#if ((defined WIN32) && (defined WIN32SCALE))
	static int      m_nInitWidth;                       //初始窗口宽度
	static int      m_nInitHeight;                      //初始窗口高度
	static double   m_dWidthScale;                      //窗口宽度缩放比例
	static double   m_dHeightScale;                     //窗口高度缩放比例
#endif

};

#endif // HMIUTILITY_H