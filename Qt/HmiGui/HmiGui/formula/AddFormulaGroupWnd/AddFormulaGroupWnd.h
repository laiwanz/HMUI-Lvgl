#ifndef ADDGROUP_H_20230830_H
#define ADDGROUP_H_20230830_H
#include <QDialog>
#include <QLabel>
#include <QVariant>

class Ui_AddFormulaGroupWnd;

class CAddFormulaGroupWnd :public QDialog
{
	Q_OBJECT
public:
	explicit CAddFormulaGroupWnd(QWidget *parent = 0);
	~CAddFormulaGroupWnd();
	int getPosition();
	void changePosition(int &nPosition);
public:
	void clickAddButton();
	void clickPositionButton();
	void clickCloseButton();
protected:
	bool eventFilter(QObject *obj, QEvent *ev) override;
signals:
	void addGroup(int nPosition, const QString &sName);
private:
	int selectLabel(bool bSelected, QPushButton *pushButton);
private:
	Ui_AddFormulaGroupWnd *ui;
	QPushButton *m_currentPushButton;
	int m_nPosition;
	int m_nLanguage;
	int m_nFontSize;
	double	m_dWidthScale;
	double	m_dHeightScale;
	QString m_sOriginal;
};

#endif