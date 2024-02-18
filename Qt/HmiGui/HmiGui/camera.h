#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>
#include <QImage>
#include "hmidef.h"
#include "hmipart.h"
#include "commonpart.h"

class CCamera : public QObject, public HMIPart
{
	Q_OBJECT
public:
	explicit CCamera(const hmiproto::hmiipcamaro &camera, QWidget *parent = nullptr);
	~CCamera();
	//初始化数据
	virtual void Init(QWidget * pWidget);
	//收到更新数据
	void Update(const hmiproto::hmiipcamaro &camera, QWidget * pWidget);
	//绘图
	virtual void Draw(QWidget * pWidget, int drawtype = DRAW_INIT);
#if ((defined WIN32) && (defined WIN32SCALE))
	//比例缩放
	void ZoomPartSelf();
#endif

private:
	hmiproto::hmiipcamaro m_part;
	CommonPart      *m_Img;
	RECT            m_rc;
};

#endif // CAMARO_H
