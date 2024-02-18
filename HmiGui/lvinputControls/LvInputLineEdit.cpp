#include "LvInputLineEdit.h"
#include "multiplatform/multiplatform.h"


std::string ta_event_data_buff;

static void ta_event_cb_1(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * ta = lv_event_get_target(e);
	lv_obj_t * kb = (lv_obj_t *)lv_event_get_user_data(e);
	if (code == LV_EVENT_FOCUSED) {
		ta_event_data_buff = lv_textarea_get_text(ta);
		lv_keyboard_set_textarea(kb, ta);
		lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
	}
	else if (code == LV_EVENT_DEFOCUSED) {
		lv_textarea_set_text(ta, ta_event_data_buff.c_str());
		lv_keyboard_set_textarea(kb, NULL);
		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
		lv_indev_reset(NULL, ta);
	}
	else if (code == LV_EVENT_READY) {
		ta_event_data_buff = lv_textarea_get_text(ta);
		//lv_obj_set_height(tv, LV_VER_RES);
		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_state(ta, LV_STATE_FOCUSED);
		lv_indev_reset(NULL, ta);
	}
	else if (code == LV_EVENT_CANCEL) {
		lv_textarea_set_text(ta, ta_event_data_buff.c_str());
		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_state(ta, LV_STATE_FOCUSED);
		lv_indev_reset(NULL, ta);
	}
}

static void ta_event_cb(lv_event_t *e) {
	lv_event_code_t code = lv_event_get_code(e);
#if LV_USE_KEYBOARD || LV_USE_ZH_KEYBOARD
	lv_obj_t *ta = lv_event_get_target(e);
#endif
	lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);
	if (code == LV_EVENT_FOCUSED || code == LV_EVENT_CLICKED)
	{
#if LV_USE_ZH_KEYBOARD != 0
		lv_zh_keyboard_set_textarea(kb, ta);
#endif
#if LV_USE_KEYBOARD != 0
		lv_keyboard_set_textarea(kb, ta);
#endif
		lv_obj_move_foreground(kb);
		lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
	}
	if (code == LV_EVENT_CANCEL || code == LV_EVENT_DEFOCUSED)
	{

#if LV_USE_ZH_KEYBOARD != 0
		lv_zh_keyboard_set_textarea(kb, ta);
#endif
#if LV_USE_KEYBOARD != 0
		lv_keyboard_set_textarea(kb, ta);
#endif
		lv_obj_move_background(kb);
		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
	}
}


LvInputLineEdit::LvInputLineEdit(lv_obj_t * parent)
{
	m_lineEdit = lv_textarea_create(parent);
	m_bMouseButtonPress = m_bConfirmFlag = m_bConfirmStatRunning = false;
	m_sCurrentText.clear();
	m_sMin = m_sMax = "";
	m_nLanguage = m_nConfirmTimeout = 0;
	m_pValidator = nullptr;
	m_pRemark = nullptr;
	m_parent = parent;
	lv_obj_set_align(m_lineEdit, LV_ALIGN_CENTER);
	lv_obj_set_style_size(m_lineEdit, 50, 20);
	m_kb = lv_keyboard_create(lv_scr_act());
	lv_obj_move_foreground(m_kb);
	lv_keyboard_set_textarea(m_kb, m_lineEdit);
	lv_obj_add_flag(m_kb, LV_OBJ_FLAG_HIDDEN);
	

	
	static lv_style_t style;
	lv_style_init(&style);
	lv_style_set_border_opa(&style, LV_OPA_TRANSP);
	lv_style_set_bg_opa(&style, LV_OPA_TRANSP);
	lv_obj_add_style(m_lineEdit, &style, 0);
}

LvInputLineEdit::~LvInputLineEdit()
{
	if (m_pValidator) {
		delete m_pValidator;
		m_pValidator = nullptr;
	}
	if (m_pRemark) {
		delete m_pRemark;
		m_pRemark = nullptr;
	}
}


int LvInputLineEdit::config(unsigned int nDataFormat, const std::string & sMin, const std::string & sMax, unsigned int nLength, unsigned int nDecimal)
{

	m_sMin = sMin;
	m_sMax = sMax;
	lv_textarea_set_max_length(m_lineEdit, nLength);

	if (_String != nDataFormat) {
		lv_obj_add_event_cb(m_lineEdit, ta_event_cb, LV_EVENT_ALL, m_kb);
	}
	else {
		lv_keyboard_set_mode(m_kb, LV_KEYBOARD_MODE_NUMBER);

		lv_obj_add_event_cb(m_lineEdit, ta_event_cb_1, LV_EVENT_ALL, m_kb);
	}
	lv_textarea_set_align(m_lineEdit, LV_TEXT_ALIGN_CENTER);// ����

	lv_obj_set_size(m_lineEdit, 100, 100);
	lv_obj_set_pos(m_kb, 0, 20);
	lv_textarea_set_one_line(m_lineEdit, true);
	
	return 0;
}

int LvInputLineEdit::setDispalyRange(bool bDisPlayRange)
{
	m_bDisplayRange = bDisPlayRange;
	if (m_bDisplayRange) {
		initRemarkWidget();
	}
	return 0;
}

int LvInputLineEdit::setLanguage(unsigned int nLanguage)
{
	m_nLanguage = nLanguage;

	return 0;
}

int LvInputLineEdit::setRect(int nX, int nY, int nWidth, int nHeight)
{
	lv_obj_set_pos(m_lineEdit, nX, nY);
	lv_obj_set_size(m_lineEdit, nWidth, nHeight);

	return 0;
}

int LvInputLineEdit::setMinRange(const std::string & sbuffer)
{
	if (m_sMin == sbuffer) {
		return 0;
	}
	return 0;
}

int LvInputLineEdit::setMaxRange(const std::string & sbuffer)
{
	if (m_sMax == sbuffer) {
		return 0;
	}

	return 0;
}

int LvInputLineEdit::setConfirmParam(bool bConfirmFlag, unsigned int nTimeout)
{
	return 0;
}

int LvInputLineEdit::saveLastText(const std::string & sbuffer)
{
	return 0;
}

int LvInputLineEdit::setText(const std::string & input)
{
	return 0;
}

void LvInputLineEdit::setvalue(const std::string & sbuffer)
{
}



int LvInputLineEdit::initRemarkWidget()
{
	return 0;
}

int LvInputLineEdit::setRemarkWidgetRange()
{
	return 0;
}

int LvInputLineEdit::calculate()
{
	return 0;
}

int LvInputLineEdit::reset()
{
	return 0;
}

int LvInputLineEdit::save()
{
	return 0;
}

int LvInputLineEdit::confirmWidget()
{
	return 0;
}
