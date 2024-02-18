#include "LvTextValidator.h"

LvTextValidator::LvTextValidator(lv_obj_t * parent)
{
}

LvTextValidator::~LvTextValidator()
{
	return;
}

int LvTextValidator::config(unsigned int nDataFormat, const std::string & sMin, const std::string & sMax, unsigned int nLength, unsigned int nDecimal)
{
	return 0;
}

int LvTextValidator::setMinRange(const std::string & sbuffer)
{
	return 0;
}

int LvTextValidator::setMaxRange(const std::string & sbuffer)
{
	return 0;
}

void LvTextValidator::fixup(string & input) const
{
}

bool LvTextValidator::isValid(string & sbuffer)
{
	return false;
}
