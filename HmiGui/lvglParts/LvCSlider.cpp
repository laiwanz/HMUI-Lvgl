#include "LvCSlider.h"

LvCSlider::LvCSlider(const hmiproto::hmislider & slider, HMILvScreen * parent)
{
	m_part.Clear();
	m_isSlideColor = "";
	m_isBgColor = "";
	m_isBorderColor = "";
	m_isShapColor = "";
	m_stylesheet = "";
	m_minLabel = NULL;
	m_maxLabel = NULL;
	m_bWaring = true;
	m_bNormal = true;
	m_bMove = false;
	m_displayLabel = NULL;
	m_commandpart = NULL;
	m_bMoveState = false;
	m_nClickedDownStartHorPos = 0;
	m_nClickedDownStartVerPos = 0;
	m_nClickedDownstartValue = 0;
	m_nLableFontSize = 0;
	m_nMarginSize = 0;
	nLeft = 0;
	nTop = 0;
	nWidth = 0;
	nHeight = 0;
	m_nScrNo = 0;
	m_part.MergeFrom(slider);
}

LvCSlider::~LvCSlider()
{
	if (m_commandpart) {
		delete m_commandpart;
	}

	if (m_minLabel) {
		delete m_minLabel;
	}

	if (m_maxLabel) {
		delete m_maxLabel;
	}

	if (m_displayLabel) {
		delete m_displayLabel;
	}
}

void LvCSlider::Init(lv_obj_t * scr)
{
	RECT	rc;

	rc.left = m_part.basic().left();
	rc.right = m_part.basic().right();
	rc.top = m_part.basic().top();
	rc.bottom = m_part.basic().bottom();
	m_sPartName = m_part.basic().name();
	m_nScrNo = m_part.basic().scrno();

	//��ȡ����ֵ
	nLeft = rc.left;
	nTop = rc.top;
	nWidth = rc.right - rc.left;
	nHeight = rc.bottom - rc.top;

	//��ȡǰ��ɫ�ͱ���ɫ������޸�WEB��ɫ��	bug xk 20200324
	m_dwSlidersubColor = m_part.surpluscolor();
	m_dwSlideraddColor = m_part.slidercolor();
	m_blockcolor = m_part.frncolor();

	m_Slider = lv_slider_create(scr);
	this->DrawImg(scr);
	this->IsShowUseBgColor();
	this->ParseLabelFontSize();
	this->setGeometry(nLeft, nTop, nWidth, nHeight); //��������
	this->setRange(m_part.minvalue(), m_part.maxvalue());
	this->setValue(m_part.curvalue());
	this->InitSliderStyle();

	//lv_obj_t * slider1 = lv_slider_create(scr);
	//lv_obj_align(slider1, LV_ALIGN_TOP_LEFT, 50, 0);
	//lv_slider_set_range(slider1, -100, 100);
	//lv_slider_set_value(slider1, 100, LV_ANIM_OFF);
	//lv_obj_set_style_bg_color(slider1, lv_color_hex(0xff0000), LV_PART_MAIN);
	//lv_obj_set_style_bg_color(slider1, lv_color_hex(0xffffff), LV_PART_INDICATOR);
	//lv_obj_set_size(slider1, 10, 200);
	//lv_obj_add_event_cb(slider1, slider_cb, LV_EVENT_VALUE_CHANGED, NULL);

	//lv_obj_t * s1_label_start = lv_label_create(scr);
	//lv_label_set_text(s1_label_start, "-100");
	//lv_obj_align_to(s1_label_start, slider1, LV_ALIGN_OUT_LEFT_BOTTOM, -10, 0);

	//lv_obj_t * s1_label_send = lv_label_create(scr);
	//lv_label_set_text(s1_label_send, "100");
	//lv_obj_align_to(s1_label_send, slider1, LV_ALIGN_OUT_LEFT_TOP, -10, 0);


}

void LvCSlider::Update(const hmiproto::hmislider & slider, HMILvScreen * parent)
{
	m_part.MergeFrom(slider);

	if (m_part.has_minvalue()) //��Сֵ��ַ�б仯
	{
		
	}

	if (m_part.has_maxvalue()) //���ֵ��ַ�б仯
	{
		
	}


	if (m_displayLabel) {
		
	}

	if (m_minLabel && m_maxLabel) {
		
	}

	if (!m_bMove) //������ʱ����
	{
		
	}



#if ((defined WIN32) && (defined WIN32SCALE))
	RECT	rc;

	rc.left = m_part.basic().left();
	rc.right = m_part.basic().right();
	rc.top = m_part.basic().top();
	rc.bottom = m_part.basic().bottom();
	memcpy(&m_OriginalRect, &rc, sizeof(RECT));
	ZoomPartSelf();
#endif
}

void LvCSlider::Draw(HMILvScreen * parent, int drawtype)
{

}



void LvCSlider::setGeometry(int x, int y, int width, int height)
{
	if (m_Slider == NULL) {
		return;
	}
	lv_obj_set_pos(m_Slider, x, y);
	//lv_obj_set_size(m_Slider, width, height);
}

void LvCSlider::setRange(int min, int max)
{
	lv_slider_set_range(m_Slider, min, max);

}

void LvCSlider::setValue(int Value)
{
	lv_slider_set_value(m_Slider, Value, LV_ANIM_OFF);
}

void LvCSlider::DrawImg(lv_obj_t * scr)
{
}

void LvCSlider::IsShowUseBgColor()
{
}

void LvCSlider::InitSliderStyle()
{
	//lv_obj_remove_style_all(m_Slider);
	if (m_part.direction() == TopToButtom || m_part.direction() == ButtomToTop) {
		if (m_part.direction() == TopToButtom) {

		}
		else {

		}
	}
	/*lv_obj_remove_style_all(m_Slider);*/
	lv_obj_set_style_bg_color(m_Slider, lv_color_hex(m_dwSlideraddColor), LV_PART_MAIN);
	lv_obj_set_style_bg_color(m_Slider, lv_color_hex(m_dwSlidersubColor), LV_PART_INDICATOR);
	lv_obj_set_style_bg_color(m_Slider, lv_color_hex(m_blockcolor), LV_PART_KNOB);
	lv_obj_set_size(m_Slider, nWidth, m_part.height());
	//lv_obj_set_size(m_Slider, 200, 20);

	//lv_slider_set_value(m_Slider, 50, LV_ANIM_OFF);
	//
	//lv_obj_add_event_cb(m_Slider, slider_cb, LV_EVENT_VALUE_CHANGED, NULL);

	/*lv_style_t style;
	lv_style_init(&style);
	lv_style_set_radius(&style, 10);
	lv_obj_add_style(m_Slider, &style, 0);
*/
}

void LvCSlider::ParseLabelFontSize()
{
}
