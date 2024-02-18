#ifndef LVTEXTVALIDATOR_H
#define LVTEXTVALIDATOR_H
#include <lvgl/lvgl/lvgl.h>

#include <string>

using std::string;

class LvTextValidator {

public:
	explicit LvTextValidator(lv_obj_t * parent = nullptr);
	virtual ~LvTextValidator();
	int config(unsigned int nDataFormat, const std::string &sMin, const std::string &sMax, unsigned int nLength, unsigned int nDecimal);
	int setMinRange(const std::string &sbuffer);
	int setMaxRange(const std::string &sbuffer);
	virtual void fixup(string &input) const;
	bool isValid(string &sbuffer);

private:
	unsigned int m_nLength;
	unsigned int m_nDataFormat;
	std::string m_sMin;
	std::string m_sMax;
	unsigned int m_nDecimal;
};

#endif // !LVTEXTVALIDATOR_H
