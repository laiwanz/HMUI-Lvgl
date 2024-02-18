#include "LvInputTextEdit.h"

std::string ta_event_data;
static void ta_event_cb_1(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * ta = lv_event_get_target(e);
	lv_obj_t * kb = (lv_obj_t *)lv_event_get_user_data(e);
	if (code == LV_EVENT_FOCUSED) {
		ta_event_data = lv_textarea_get_text(ta);
		lv_keyboard_set_textarea(kb, ta);
		lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
	}
	else if (code == LV_EVENT_DEFOCUSED) {
		lv_textarea_set_text(ta, ta_event_data.c_str());
		lv_keyboard_set_textarea(kb, NULL);
		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
		lv_indev_reset(NULL, ta);
	}
	else if (code == LV_EVENT_READY) {
		ta_event_data = lv_textarea_get_text(ta);
		//lv_obj_set_height(tv, LV_VER_RES);
		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_state(ta, LV_STATE_FOCUSED);
		lv_indev_reset(NULL, ta);
	}
	else if (code == LV_EVENT_CANCEL) {
		lv_textarea_set_text(ta, ta_event_data.c_str());
		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_state(ta, LV_STATE_FOCUSED);
		lv_indev_reset(NULL, ta);
	}
}

LvInputTextEdit::LvInputTextEdit(lv_obj_t * parent)
{
	m_sCurrentText.clear();
	m_sFrameText.clear();
	m_bFrameRepaint = false;
	m_bMouseButtonPress = m_bConfirmFlag = m_bConfirmStatRunning = false;
	m_nLanguage = m_nLength = m_nVerticalAlign = m_nConfirmTimeout = 0;
	
	m_pTextEdit = lv_textarea_create(parent);
	lv_obj_set_align(m_pTextEdit, LV_ALIGN_CENTER);
	lv_obj_set_style_size(m_pTextEdit, 50, 20);
	lv_obj_t * kb = lv_keyboard_create(lv_scr_act());
	lv_obj_move_foreground(kb);

	lv_obj_set_size(m_pTextEdit, 200, 50);
	lv_obj_add_event_cb(m_pTextEdit, ta_event_cb_1, LV_EVENT_ALL, kb);

	{
		static lv_style_t style;
		lv_style_init(&style);
		lv_style_set_border_opa(&style, LV_OPA_TRANSP);
		// lv_obj_add_style(m_pTextEdit, &style, NULL);
	}


}

LvInputTextEdit::~LvInputTextEdit()
{
}

int LvInputTextEdit::config(unsigned int nLength)
{
	return 0;
}

int LvInputTextEdit::setHVAlign(unsigned int nHorizontalAlign, unsigned int nVerticalAlign)
{
	return 0;
}

int LvInputTextEdit::setConfirmParam(bool bConfirmFlag, unsigned int nTimeout)
{
	return 0;
}

int LvInputTextEdit::setLanguage(unsigned int nLanguage)
{
	return 0;
}

int LvInputTextEdit::saveLastText(const string & sbuffer)
{
	return 0;
}

void LvInputTextEdit::setvalue(const string & sbuffer)
{
}

int LvInputTextEdit::setText(const std::string & input)
{
	return 0;
}


int LvInputTextEdit::selectNone()
{
	return 0;
}

int LvInputTextEdit::reset()
{
	return 0;
}

int LvInputTextEdit::save()
{
	return 0;
}

int LvInputTextEdit::confirmWidget()
{
	return 0;
}
