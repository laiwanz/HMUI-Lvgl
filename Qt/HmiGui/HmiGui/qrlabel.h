#ifndef QRWIDGET_HPP
#define QRWIDGET_HPP
#include <QLabel>
#include <QString>

class QWidget;
class QRLabel : public QLabel {
	Q_OBJECT
private:
	QString data;
public:
	explicit QRLabel(QWidget *pWidget);
	void setQRData(QString data);
protected:
	void paintEvent(QPaintEvent *);
};
#endif // QRWIDGET_HPP
