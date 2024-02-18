#ifndef LV_INPUT_LINE_EDIT_H
#define LV_INPUT_LINE_EDIT_H

#include "../LvCommon/LvTextValidator.h"
#include "../LvCommon/LvRemark.h"
#include <lvgl/lvgl/lvgl.h>

class LvInputLineEdit {

public:
	explicit LvInputLineEdit(lv_obj_t *parent = nullptr);
	virtual ~LvInputLineEdit();
	int config(unsigned int nDataFormat, const std::string &sMin, const std::string &sMax, unsigned int nLength, unsigned int nDecimal);
	int setDispalyRange(bool bDisPlayRange);
	int setLanguage(unsigned int nLanguage);
	int setRect(int nX, int nY, int nWidth, int nHeight);
	int setMinRange(const std::string &sbuffer);
	int setMaxRange(const std::string &sbuffer);
	int setConfirmParam(bool bConfirmFlag, unsigned int nTimeout);
	int saveLastText(const std::string &sbuffer);
	int setText(const std::string &input);

	void setvalue(const std::string &sbuffer);
protected:
	void mousePressEvent(lv_event_t *event);
	void mouseReleaseEvent(lv_event_t *event);
	void mouseMoveEvent(lv_event_t *event);
	bool eventFilter(lv_obj_t *obj, lv_event_t *ev);

private:
	int initRemarkWidget();
	int setRemarkWidgetRange();
	int calculate();
	int reset();
	int save();
	int confirmWidget();
private:
	bool m_bMouseButtonPress, m_bDisplayRange, m_bConfirmFlag, m_bConfirmStatRunning;
	std::string m_sCurrentText;
	lv_obj_t*   m_lineEdit;
	std::string m_sMin, m_sMax;
	unsigned int m_nLanguage, m_nConfirmTimeout;
	lv_obj_t*	m_parent;
	lv_obj_t*	m_kb;

	LvTextValidator	*m_pValidator;
	LvRemark *m_pRemark;

};
#endif // !LV_INPUT_LINE_EDIT_H
