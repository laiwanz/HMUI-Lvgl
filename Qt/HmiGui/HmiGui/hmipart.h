#ifndef HMIPART_H
#define HMIPART_H
#include "hmidef.h"
#include <QWidget>
#include "macrosplatform/macrosplatform.h"

class HMIPart {
public:
	HMIPart();
	virtual ~HMIPart();
public:
	virtual void Init(QWidget * pWidget) = 0;
	//绘图
	virtual void Draw(QWidget * pWidget, int drawtype = DRAW_INIT) = 0;
#if ((defined WIN32) && (defined WIN32SCALE))
	//比例缩放部件
	virtual void ZoomPartSelf() = 0;
#endif
	//从缓存中显示出来
	virtual void OnPartShow();
	//隐藏画面时触发
	virtual void OnPartHide();
	virtual int parseJson(const std::string &sbuffer);
#if ((defined WIN32) && (defined WIN32SCALE))
public:
	RECT	m_OriginalRect;
#endif
	std::string m_sPartName;
	int		m_nIndex;
};
#endif // HMIPART_H
