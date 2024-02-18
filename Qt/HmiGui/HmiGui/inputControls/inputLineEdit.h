#ifndef _INPUTLINEEDIT_H_20231103_
#define _INPUTLINEEDIT_H_20231103_
#include <QtWidgets/QLineEdit>
#include "../common/textValidator.h"
#include "../common/remarksWidget.h"
class CInputLineEdit : public QLineEdit
{
	Q_OBJECT
public:
	explicit CInputLineEdit(QWidget *parent = nullptr);
	~CInputLineEdit();
	int config(unsigned int nDataFormat, const std::string &sMin, const std::string &sMax, unsigned int nLength, unsigned int nDecimal);
	int setDispalyRange(bool bDisPlayRange);
	int setLanguage(unsigned int nLanguage);
	int setRect(const QRect &rc);
	int setMinRange(const std::string &sbuffer);
	int setMaxRange(const std::string &sbuffer);
	int setConfirmParam(bool bConfirmFlag, unsigned int nTimeout);
	int saveLastText(const QString &sbuffer);
signals:
	void setvalue(const QString &sbuffer);
protected:
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	bool eventFilter(QObject *obj, QEvent *ev) override;
private:
	int initRemarkWidget();
	int setRemarkWidgetRange();
	int calculate();
	int reset();
	int save();
	int confirmWidget();
private:
	bool m_bMouseButtonPress, m_bDisplayRange, m_bConfirmFlag, m_bConfirmStatRunning;
	QString m_sCurrentText;
	CTextValidator *m_pValidator;
	std::string m_sMin, m_sMax;
	unsigned int m_nLanguage, m_nConfirmTimeout;
	CRemarkWidget *m_pRemarkWidget;
};
#endif // _INPUTLINEEDIT_H_20231103_