#ifndef LVCUSTOMPART_H
#define LVCUSTOMPART_H
#include "HMILvPart.h"
#include "LvCommonPart.h"


class LvCustomPart : public HMILvPart {

public:
	explicit LvCustomPart(const hmiproto::hmicustom &custom, HMILvScreen * parent);
	~LvCustomPart();

	//��ʼ������
	virtual void Init(lv_obj_t * pWidget);
	//�յ���������
	virtual void Update(const hmiproto::hmicustom &custom, HMILvScreen * pWidget);
	//��ͼ
	virtual void Draw(HMILvScreen * pWidget, int drawtype = DRAW_INIT);
#if ((defined WIN32) && (defined WIN32SCALE))
	//��������
	void ZoomPartSelf();
#endif
private:
	hmiproto::hmicustom m_part;
	LvCommonPart      *m_svg;
	RECT            m_rc;
	bool            m_bUseShowHide;			//�Ƿ�����

};


#endif // !LVCUSTOMPART_H
