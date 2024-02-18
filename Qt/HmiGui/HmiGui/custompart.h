#ifndef CUSTOMPART_H
#define CUSTOMPART_H
#include <QObject>
#include <QImage>
#include "hmidef.h"
#include "hmipart.h"
#include "commonpart.h"
class custompart : public QObject, public HMIPart {
	Q_OBJECT
public:
	explicit custompart(const hmiproto::hmicustom &custom, QWidget *parent = nullptr);
	~custompart();
	//初始化数据
	virtual void Init(QWidget * pWidget);
	//收到更新数据
	void Update(const hmiproto::hmicustom &custom, QWidget * pWidget);
	//绘图
	virtual void Draw(QWidget * pWidget, int drawtype = DRAW_INIT);
#if ((defined WIN32) && (defined WIN32SCALE))
	//比例缩放
	void ZoomPartSelf();
#endif
private:
	hmiproto::hmicustom m_part;
	CommonPart      *m_svg;
	RECT            m_rc;
	bool            m_bUseShowHide;			//是否隐藏
};
#endif // CUSTOMPART_H
