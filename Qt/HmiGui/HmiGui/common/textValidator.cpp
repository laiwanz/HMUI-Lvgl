#include "textValidator.h"
#include "multiplatform/multiplatform.h"
#include <QDebug>

CTextValidator::CTextValidator(QObject * parent) : QValidator(parent) {
	m_nLength = m_nDataFormat = m_nDecimal = 0;
	m_sMin = m_sMax = "";
}

CTextValidator::~CTextValidator() {

}

int CTextValidator::config(unsigned int nDataFormat, const std::string &sMin, const std::string &sMax, unsigned int nLength, unsigned int nDecimal) {
	m_nDataFormat = nDataFormat;
	m_sMin = sMin;
	m_sMax = sMax;
	m_nLength = nLength;
	m_nDecimal = nDecimal;
	return 0;
}

QValidator::State CTextValidator::validate(QString &input, int &pos) const {
	QValidator::State state = QValidator::Invalid;
	switch (m_nDataFormat)
	{
	case _16Bit_Binary:
	case _32Bit_Binary:
	case _64Bit_Binary:
		{
			state = this->validateBinary(input, pos);
		}
		break;
	case _16Bit_Octal:
	case _32Bit_Octal:
	case _64Bit_Octal:
		{
			state = this->validateOctal(input, pos);
		}
		break;
	case _16Bit_Hex:
	case _32Bit_Hex:
	case _64Bit_Hex:
		{
			state = this->validateHex(input, pos);
		}
		break;
	case _16Bit_Bcd:
	case _32Bit_Bcd:
	case _64Bit_Bcd:
	case _16Bit_UnDecimal:
	case _32Bit_UnDecimal:
	case _64Bit_UnDecimal:
		{
			if (m_nDecimal == 0) {
				state = this->validateUnDecimal(input, pos);
			}
			else {
				state = this->validateDouble(input, pos, true);
			}
		}
		break;
	case _16Bit_Decimal:
	case _32Bit_Decimal:
	case _64Bit_Decimal:
		{
			if (m_nDecimal == 0) {
				state = this->validateDecimal(input, pos);
			}
			else {
				state = this->validateDouble(input, pos);
			}
		}
		break;
	case _32Bit_Float:
	case _64Bit_Double:
		{
			state = this->validateDouble(input, pos);
		}
		break;
	case _String:
		{
			state = this->validateString(input, pos);
		}
		break;
	default:
		break;
	}
	return state;
}

QValidator::State CTextValidator::validateString(QString &input, int &pos) const {
	std::string sbuffer = input.toStdString();
	if (sbuffer.length() > m_nLength) {
		return QValidator::Invalid;
	}
	return QValidator::Acceptable;
}

QValidator::State CTextValidator::validateOctal(QString &input, int &pos, bool bVerifyAll) const {
	bool bOk = false;
	if (input.isEmpty()) {
		if (bVerifyAll) {
			return QValidator::Invalid;
		}
		else {
			return QValidator::Intermediate;
		}
	}
	auto current = input.toULongLong(&bOk, 8);
	if (!bOk) {
		return QValidator::Invalid;
	}
	auto max = QString(m_sMax.c_str()).toULongLong(&bOk, 16);
	if (current > max) {
		return QValidator::Invalid;
	}
	auto min = QString(m_sMin.c_str()).toULongLong(&bOk, 16);
	if (current < min) {
		if (bVerifyAll) {
			return QValidator::Invalid;
		}
		else {
			return QValidator::Intermediate;
		}
	}
	return QValidator::Acceptable;
}

QValidator::State CTextValidator::validateHex(QString &input, int &pos, bool bVerifyAll) const {
	bool bOk = false;
	if (input.isEmpty()) {
		if (bVerifyAll) {
			return QValidator::Invalid;
		}
		else {
			return QValidator::Intermediate;
		}
	}
	auto current = input.toULongLong(&bOk, 16);
	if (!bOk) {
		return QValidator::Invalid;
	}
	auto max = QString(m_sMax.c_str()).toULongLong(&bOk, 16);
	if (current > max) {
		return QValidator::Invalid;
	}
	auto min = QString(m_sMin.c_str()).toULongLong(&bOk, 16);
	if (current < min) {
		if (bVerifyAll) {
			return QValidator::Invalid;
		}
		else {
			return QValidator::Intermediate;
		}
	}
	return QValidator::Acceptable;
}

QValidator::State CTextValidator::validateBinary(QString &input, int &pos, bool bVerifyAll) const {
	bool bOk = false;
	if (input.isEmpty()) {
		if (bVerifyAll) {
			return QValidator::Invalid;
		}
		else {
			return QValidator::Intermediate;
		}
	}
	auto current = input.toULongLong(&bOk, 2);
	if (!bOk) {
		return QValidator::Invalid;
	}
	auto max = QString(m_sMax.c_str()).toULongLong(&bOk, 16);
	if (current > max) {
		return QValidator::Invalid;
	}
	auto min = QString(m_sMin.c_str()).toULongLong(&bOk, 16);
	if (current < min) {
		if (bVerifyAll) {
			return QValidator::Invalid;
		}
		else {
			return QValidator::Intermediate;
		}
	}
	return QValidator::Acceptable;
}

QValidator::State CTextValidator::validateUnDecimal(QString &input, int &pos, bool bVerifyAll) const {
	bool bOk = false;
	if (input.isEmpty()) {
		if (bVerifyAll) {
			return QValidator::Invalid;
		}
		else {
			return QValidator::Intermediate;
		}
	}
	auto current = input.toULongLong(&bOk, 10);
	if (!bOk) {
		return QValidator::Invalid;
	}
	auto max = QString(m_sMax.c_str()).toULongLong(&bOk, 10);
	if (current > max) {
		return QValidator::Invalid;
	}
	auto min = QString(m_sMin.c_str()).toULongLong(&bOk, 10);
	if (current < min) {
		if (bVerifyAll) {
			return QValidator::Invalid;
		}
		else {
			return QValidator::Intermediate;
		}
	}
	return QValidator::Acceptable;
}

QValidator::State CTextValidator::validateDecimal(QString &input, int &pos, bool bVerifyAll) const {
	bool bOk = false;
	auto max = QString(m_sMax.c_str()).toLongLong(&bOk, 10);
	auto min = QString(m_sMin.c_str()).toLongLong(&bOk, 10);
	if (input.isEmpty()) {
		if (bVerifyAll) {
			return QValidator::Invalid;
		}
		else {
			return QValidator::Intermediate;
		}
	}
	const bool startsWithMinus(input[0] == '-');
	if (min >= 0 && startsWithMinus) {
		return QValidator::Invalid;
	}
	const bool startsWithPlus(input[0] == '+');
	if (max < 0 && startsWithPlus) {
		return QValidator::Invalid;
	}
	if (input.size() == 1 &&
		(startsWithPlus || startsWithMinus)) {
		if (bVerifyAll) {
			return QValidator::Invalid;
		}
		else {
			return QValidator::Intermediate;
		}
	}
	auto current = input.toLongLong(&bOk, 10);
	if (!bOk) {
		return QValidator::Invalid;
	}
	if (current > max) {
		return QValidator::Invalid;
	}
	if (startsWithMinus && current < min) {
		return QValidator::Invalid;
	}
	if (bVerifyAll &&
		current < min) {
		return QValidator::Invalid;
	}
	return QValidator::Acceptable;
}

QValidator::State CTextValidator::validateDouble(QString &input, int &pos, bool isOnlyPositive, bool bVerifyAll) const {
	bool bOk = false;
	auto max = QString(m_sMax.c_str()).toDouble(&bOk);
	auto min = QString(m_sMin.c_str()).toDouble(&bOk);
	if (input.isEmpty()) {
		if (bVerifyAll) {
			return QValidator::Invalid;
		}
		else {
			return QValidator::Intermediate;
		}
	}
	const bool startsWithMinus(input[0] == '-');
	if ((min >= 0 && startsWithMinus) ||
		(isOnlyPositive && startsWithMinus)) {
		return QValidator::Invalid;
	}
	const bool startsWithPlus(input[0] == '+');
	if (max < 0 && startsWithPlus) {
		return QValidator::Invalid;
	}
	if (input.size() == 1 &&
		(startsWithPlus || startsWithMinus)) {
		if (bVerifyAll) {
			return QValidator::Invalid;
		}
		else {
			return QValidator::Intermediate;
		}
	}
	auto current = input.toDouble(&bOk);
	if (!bOk) {
		return QValidator::Invalid;
	}
	if (current > max) {
		return QValidator::Invalid;
	}
	if (startsWithMinus && current < min) {
		return QValidator::Invalid;
	}
	if (bVerifyAll &&
		current < min) {
		return QValidator::Invalid;
	}
	return QValidator::Acceptable;
}

bool CTextValidator::isValid(QString &sbuffer) {
	QValidator::State state = QValidator::Invalid;
	int pos = 0;
	switch (m_nDataFormat)
	{
	case _16Bit_Binary:
	case _32Bit_Binary:
	case _64Bit_Binary:
		{
			state = this->validateBinary(sbuffer, pos, true);
		}
		break;
	case _16Bit_Octal:
	case _32Bit_Octal:
	case _64Bit_Octal:
		{
			state = this->validateOctal(sbuffer, pos, true);
		}
		break;
	case _16Bit_Hex:
	case _32Bit_Hex:
	case _64Bit_Hex:
		{
			state = this->validateHex(sbuffer, pos, true);
		}
		break;
	case _16Bit_Bcd:
	case _32Bit_Bcd:
	case _64Bit_Bcd:
	case _16Bit_UnDecimal:
	case _32Bit_UnDecimal:
	case _64Bit_UnDecimal:
		{
			if (m_nDecimal == 0) {
				state = this->validateUnDecimal(sbuffer, pos, true);
			}
			else {
				state = this->validateDouble(sbuffer, pos, true, true);
			}
		}
		break;
	case _16Bit_Decimal:
	case _32Bit_Decimal:
	case _64Bit_Decimal:
		{
			if (m_nDecimal == 0) {
				state = this->validateDecimal(sbuffer, pos, true);
			}
			else {
				state = this->validateDouble(sbuffer, pos, false, true);
			}
		}
		break;
	case _32Bit_Float:
	case _64Bit_Double:
		{
			state = this->validateDouble(sbuffer, pos, false, true);
		}
		break;
	case _String:
		{
			state = this->validateString(sbuffer, pos);
		}
		break;
	default:
		break;
	}
	if (state == QValidator::Invalid) {
		return false;
	}
	return true;
}

void CTextValidator::fixup(QString &input) const {
	
}

int CTextValidator::setMinRange(const std::string &sbuffer) {
	m_sMin = sbuffer;
	return 0;
}

int CTextValidator::setMaxRange(const std::string &sbuffer) {
	m_sMax = sbuffer;
	return 0;
}