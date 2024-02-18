#ifndef LVGENERALPART_H
#define LVGENERALPART_H
#include <string>
#include <vector>
#include "ProtoXML/ProtoXML.h"
#include "../HmiGui/hmidef.h"
#include "HMILvPart.h"
#include "HMILvScreen.h"
#include "../lvglParts/LvCommonPart.h"
#include "LvglLabel.h"
using std::string;
using std::vector;

class LvCommonPart;

class LvGeneralPart : public HMILvPart {
	typedef void (LvGeneralPart ::*DRAW_FUN)(HMILvScreen *, int);
	enum {
		NORMALPART,
		QRPART,
		ROTATEPIC
	};
public:
	explicit LvGeneralPart(const hmiproto::hmipartcommon &common, HMILvScreen * parent);
	~LvGeneralPart();

	//��ʼ������
	virtual void Init(lv_obj_t * pWidget);
	//�յ���������
	void Update(const hmiproto::hmipartcommon &common, HMILvScreen * parent);
	//��ʼ���ػ�
	virtual void Draw(HMILvScreen * parent, int drawtype = DRAW_INIT);
	//���ػ���ʱ����
	virtual void OnPartHide();
	//�ӻ�������ʾ����
	virtual void OnPartShow();

#if ((defined WIN32) && (defined WIN32SCALE))
	//��������
	virtual void ZoomPartSelf();
#endif
private:
	void DrawImg(HMILvScreen * pWidget, int drawtype = DRAW_INIT);
	void DrawTxt(HMILvScreen * pWidget, int drawtype = DRAW_INIT);
	void DrawShowHide(HMILvScreen * pWidget, int drawtype = DRAW_INIT);
	void DrawArea(HMILvScreen * pWidget, int drawtype = DRAW_INIT);
	void DrawGlint(HMILvScreen * pWidget, int drawtype = DRAW_INIT);
	void DrawRotatePic(HMILvScreen * pWidget, int drawtype = DRAW_INIT);
	void DrawUpdate(HMILvScreen * pWidget, int drawtype = DRAW_INIT);
	void DoShowHide(bool bShow);
	void GetDrawFunc(int drawtype);
	void DrawSelect(HMILvScreen * pWidget, int drawtype);

private:
	void DoGlint();
private:
	hmiproto::hmipartcommon		m_part;
	LvCommonPart            *m_commandpart;
	LvQRLabel				*m_qrlabel;
	int                 m_parttype;
	unsigned long       m_TxtCrc32;             //�ı�У���
	RECT                m_rc;					//����
	bool                m_bUseShowHide;			//�Ƿ�����
	bool                m_bUseGlint;			//�Ƿ���˸
	int                 m_nImgIndex;            //ͼƬ���
	unsigned int		m_nLockImgIndex;		//��ͼƬ���
	int					m_nLockWidth;			//���Ĵ�С
	bool				m_bIsShowGrayScale;		//�ҽ�
	string              m_strPartType;          //��������
	string				m_strImgPath;			//λͼԤ��ͼƬ·��
	bool				m_bUpdateImg;
	bool				m_bUpdateText;
	bool				m_bUpdateHide;
	bool				m_bUpdateGlint;
	bool				m_bUpdateArea;
	vector<DRAW_FUN>	m_drawList;

};

#endif // !LVGENERALPART_H
