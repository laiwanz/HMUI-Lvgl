#ifndef _TEXTVALIDATOR_H_20231103_
#define _TEXTVALIDATOR_H_20231103_
#include <QValidator>
class CTextValidator : public QValidator
{
	Q_OBJECT
public:
	explicit CTextValidator(QObject * parent = nullptr);
	~CTextValidator();
	int config(unsigned int nDataFormat, const std::string &sMin, const std::string &sMax, unsigned int nLength, unsigned int nDecimal);
	int setMinRange(const std::string &sbuffer);
	int setMaxRange(const std::string &sbuffer);
	virtual QValidator::State validate(QString &input, int &pos) const;
	virtual void fixup(QString &input) const;
	bool isValid(QString &sbuffer);
private:
	QValidator::State validateString(QString &input, int &pos) const;
	QValidator::State validateBinary(QString &input, int &pos, bool bVerifyAll = false) const;
	QValidator::State validateOctal(QString &input, int &pos, bool bVerifyAll = false) const;
	QValidator::State validateHex(QString &input, int &pos, bool bVerifyAll = false) const;
	QValidator::State validateUnDecimal(QString &input, int &pos, bool bVerifyAll = false) const;
	QValidator::State validateDecimal(QString &input, int &pos, bool bVerifyAll = false) const;
	QValidator::State validateDouble(QString &input, int &pos, bool isOnlyPositive = false, bool bVerifyAll = false) const;
private:
	unsigned int m_nLength;
	unsigned int m_nDataFormat;
	std::string m_sMin;
	std::string m_sMax;
	unsigned int m_nDecimal;
};
#endif // _TEXTVALIDATOR_H_20231103_