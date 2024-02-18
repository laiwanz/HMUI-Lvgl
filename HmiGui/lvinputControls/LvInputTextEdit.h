#ifndef LV_INPUTTEXTEDIT_H_20240109_
#define LV_INPUTTEXTEDIT_H_20240109_

#include <lvgl/lvgl/lvgl.h>
#include <string>
using std::string;
class LvInputTextEdit {

public:
	explicit LvInputTextEdit(lv_obj_t *parent = nullptr);
	virtual ~LvInputTextEdit();

	int config(unsigned int nLength);
	int setHVAlign(unsigned int nHorizontalAlign, unsigned int nVerticalAlign);
	int setConfirmParam(bool bConfirmFlag, unsigned int nTimeout);
	int setLanguage(unsigned int nLanguage);
	int saveLastText(const string &sbuffer);

	void setvalue(const string &sbuffer);
	int setText(const std::string &input);



private:
	int selectNone();
	int reset();
	int save();
	int confirmWidget();

private:
	bool m_bMouseButtonPress, m_bConfirmFlag, m_bConfirmStatRunning;
	unsigned int m_nLength, m_nConfirmTimeout, m_nVerticalAlign, m_nLanguage;
	string m_sCurrentText;
	lv_obj_t *m_pTextEdit;
	string m_sFrameText;
	bool m_bFrameRepaint;
	int m_nWidgetWidth, m_nWidgetHeight;

};
#endif