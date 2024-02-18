#ifndef LVCOMBOBOX_H
#define LVCOMBOBOX_H
#include "ProtoXML/ProtoXML.h"
#include "HMILvPart.h"
#include "HMILvScreen.h"

class LvComboBox : public HMILvPart
{
public:
	explicit LvComboBox(const hmiproto::hmidownlist &downlist, HMILvScreen * parent);
	~LvComboBox();

	void Init(lv_obj_t * pWidget);
	//�յ���������
	void Update(const hmiproto::hmidownlist &downlist, HMILvScreen * parent);
	//��ͼ
	void Draw(HMILvScreen * parent, int drawtype = DRAW_INIT);

	void ZoomPartSelf();
	//�ӻ�������ʾ����
	void OnPartShow();
	//���ػ���ʱ����
	void OnPartHide();
private:
	void DrawDownList(int drawtype);

private:
	hmiproto::hmidownlist m_part;
	string				  m_strPartType;
	int                   m_nScrNo;
	lv_obj_t*			  m_ComboxPart;
	vector<string>		  m_strList;
	//QRect                 m_Rect;

};

#endif // LVCOMBOBOX_H
