#ifndef CTOASTDIALOG_H
#define CTOASTDIALOG_H
#include <QDialog>
#include <QLabel>
#include <QObject>
#include <QRect>

enum DIALOGLEVEL{
    INFO,
    WARN,
    ERRORDLG,
};

class ToastDialog: public QDialog
{
public:
    ToastDialog(const int &nWidth, const int &nHeight, QWidget *parent);
	~ToastDialog();
    void show(DIALOGLEVEL level, const QString& text);

private:
    void timerEvent(QTimerEvent *event) override;

private:
    QLabel* mLabel;
	double m_dWidthScale;
	double m_dHeightScale;
	int mTimerId{ 0 };
};

#endif // TOASTDIALOG_H
