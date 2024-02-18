#ifndef LVSLIDER_H
#define LVSLIDER_H

#include "LvGeneralPart.h"
#include "LvCommonPart.h"
#include <lvgl/lvgl/lvgl.h>

#include <string>
using std::string;
#define SPACERLEN	20

class LvCSlider : public HMILvPart {
	enum {
		LeftToRight = 0,
		RightToLeft,
		TopToButtom,
		ButtomToTop,
	};

#define  MINCOTRLSIZE 40

public:
	explicit LvCSlider(const hmiproto::hmislider &slider, HMILvScreen * parent);
	~LvCSlider();

	virtual void Init(lv_obj_t* scr);
	
	virtual void Update(const hmiproto::hmislider &slider, HMILvScreen * parent);

	virtual void Draw(HMILvScreen * parent, int drawtype = DRAW_INIT);
#if ((defined WIN32) && (defined WIN32SCALE))
	virtual void ZoomPartSelf();	//����
#endif

public:
	void EndTheSlide();										//����̧���¼���������
	void SliderMove(int nValue);							//�����ƶ��¼���������
protected:
	bool CheckIsSlider(lv_event_t* eventT);
	int  GetHorMoveWidth(int nValue);
	int  GetHorMoveWidth();
	int  GetHorMoveHeight(int nValue);
	int  GetHorMoveHeight();
	void DrawImg(lv_obj_t* scr);
	void ChangeCurValue(int nValue);
	void IsShowUseBgColor();
	void InitValueDisplayLabel();							//����ֵ��ʾ��ǩ
	void InitSliderStyle();									//���û�������ʼ����ʽ
	void DoDIsplyCurValueMove(int nValue);					//��ǰֵ��ʾ�ƶ�
	void DoSliderAlarmColor(int nValue);						//��������ɫ
	void ParseLabelFontSize();								//����������ʽ
	void GetHandleAddress(int nValue);
	void InitLableStyle(lv_obj_t *label);						//��ʼ��������ʽ
	void DoScrollControl(lv_event_t *eventT);				//��������
	void mousePressEvent(lv_event_t *eventT);				//��갴�´�������
	void mouseMoveEvent(lv_event_t *eventT);				//����ƶ���������
	void mouseReleaseEvent(lv_event_t *eventT);				//���̧��������
	string	CutDataStrByDigitCount(int nValue);				//��intתstring��ʾ����

	void setGeometry(int x, int y, int width, int height);
	void setRange(int min, int max);
	void setValue(int Value);


private:
	bool				m_bWaring;						//�Ƿ����������þ���ɫ 0 ���� 1 ��
	bool				m_bNormal;						//�Ƿ���������������ɫ 0 ���� 1 ��
	bool				m_bMoveState;					//�Ƿ����ƶ�״̬ 0 ���� 1 ��
	bool				m_bMove;						//���Ƿ��ƶ� 0���� 1 ��
	int					m_nClickedDownStartHorPos;		//��ʼ��x����
	int					m_nClickedDownStartVerPos;		//��ʼ��y����
	int					m_nClickedDownstartValue;		//��ʼ���ƶ�ֵ
	int					m_nLableFontSize;				//��ʾ�����С
	int					m_nMarginSize;					//ˮƽ�������±߽�ֵ
	int					nLeft;							//����x����
	int					nTop;							//����y����
	int					nWidth;							//����
	int					nHeight;						//�߶�
	int                 m_nScrNo;						//�����
	unsigned int		m_dwSlideraddColor;				//����δ��������ɫ��ɫ
	unsigned int		m_dwSlidersubColor;				//���컬������ɫ
	unsigned int		m_blockcolor;
	long long			m_nSliderX;						//��������xֵ
	long long			m_nSliderY;						//��������Yֵ
	string				m_isBgColor;					//�Ƿ��б���ɫ ""�� "0" ��
	string				m_isSlideColor;					//�Ƿ��л���ɫ ""�� "0" ��
	string				m_isBorderColor;				//�Ƿ��б߿���ɫ ""�� "0" ��
	string				m_isShapColor;					//�Ƿ��б߿���ɫ ""�� "0" ��
	string				m_stylesheet;					//��ǰ������ʽ
	LvCommonPart		*m_commandpart;
	RECT				rectTemp;						//����
	lv_font_t			displayLabelFt;
	
	lv_obj_t				*m_displayLabel;				//���ٱ�ǩ
	lv_obj_t				*m_minLabel;					//��Сֵ��ǩ
	lv_obj_t				*m_maxLabel;					//���ֵ��ǩ
	hmiproto::hmislider m_part;							//������������
	lv_obj_t				*m_Slider;
};








#endif // !LVSLIDER_H
