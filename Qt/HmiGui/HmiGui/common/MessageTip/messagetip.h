#ifndef MESSAGETIP_H
#define MESSAGETIP_H
#include <QDialog>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MessageTip; }
QT_END_NAMESPACE

class MessageTip : public QDialog
{
    Q_OBJECT
public:
    MessageTip(QWidget *parent = nullptr);
    ~MessageTip();
	int setText(const QString &sbuffer);
	int confirmParam(bool bFlag, unsigned int nTimeout);
	int setOKText(const QString &sbuffer);
	int setCancelText(const QString &sbuffer);
protected:
	void paintEvent(QPaintEvent *event) override;
	bool eventFilter(QObject *obj, QEvent *ev) override;
private slots:
	void on_OK_pressed();
	void on_cancel_pressed();
private:
    Ui::MessageTip *ui;
	unsigned int m_nRadius;
	QTimer *m_pTimer;
	int m_nWidth, m_nHeight;
};
#endif // MESSAGETIP_H
