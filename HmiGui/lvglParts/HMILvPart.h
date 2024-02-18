#ifndef HMILVPART_H
#define HMILVPART_H
#include <lvgl/lvgl/lvgl.h>

#include "macrosplatform/macrosplatform.h"
#include "../lvglParts/HMILvScreen.h"
#include "../HmiGui/hmidef.h"
class HMILvScreen;

class HMILvPart
{
public:
	HMILvPart();
	virtual ~HMILvPart();
public:
	virtual void Init(lv_obj_t*) = 0;
	//��ͼ
	virtual void Draw( HMILvScreen * parent, int drawtype = DRAW_INIT) = 0;
#if ((defined WIN32) && (defined WIN32SCALE))
	//�������Ų���
	virtual void ZoomPartSelf() = 0;


#endif
	//�ӻ�������ʾ����
	virtual void OnPartShow();
	//���ػ���ʱ����
	virtual void OnPartHide();
	virtual int parseJson(const std::string &sbuffer);
#if ((defined WIN32) && (defined WIN32SCALE))
public:
	RECT	m_OriginalRect;
#endif
	std::string m_sPartName;
	int		m_nIndex;
};

#endif

