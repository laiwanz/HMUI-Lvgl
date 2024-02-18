#ifndef _BACKGROUNDWIDGET_H_20231103_
#define _BACKGROUNDWIDGET_H_20231103_
#include <QWidget>
#include <QMovie>
class CBackgroundWidget : public QWidget {
	Q_OBJECT
public:
	explicit CBackgroundWidget(QWidget *parent = nullptr);
	~CBackgroundWidget();
	int setBackgroudImage(int nIndex, const QString &sPath);
protected:
	void paintEvent(QPaintEvent *event) override;
private:
	QMovie	*m_pMovie;
	int m_nIndex;
	QString m_sPath;
	QString m_sGifpath;
};
#endif // _BACKGROUNDWIDGET_H_20231103_