#ifndef _INPUTTEXTEDIT_H_20231103_
#define _INPUTTEXTEDIT_H_20231103_
#include <QtWidgets/QTextEdit>
class CInputTextEdit : public QTextEdit
{
	Q_OBJECT
public:
	explicit CInputTextEdit(QWidget *parent = nullptr);
	~CInputTextEdit();
	int config(unsigned int nLength);
	int setHVAlign(unsigned int nHorizontalAlign, unsigned int nVerticalAlign);
	int setConfirmParam(bool bConfirmFlag, unsigned int nTimeout);
	int setLanguage(unsigned int nLanguage);
	int saveLastText(const QString &sbuffer);
signals:
	void setvalue(const QString &sbuffer);
protected:
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void paintEvent(QPaintEvent *event) override;
	bool eventFilter(QObject *obj, QEvent *ev) override;
	void mouseMoveEvent(QMouseEvent *event) override;
private:
	int selectNone();
	int reset();
	int save();
	int confirmWidget();
private:
	bool m_bMouseButtonPress, m_bConfirmFlag, m_bConfirmStatRunning;
	unsigned int m_nLength, m_nConfirmTimeout, m_nVerticalAlign, m_nLanguage;
	QString m_sCurrentText;
	QWidget *m_pWidget;
	QString m_sFrameText;
	bool m_bFrameRepaint;
	int m_nWidgetWidth, m_nWidgetHeight;
};
#endif // _INPUTTEXTEDIT_H_20231103_