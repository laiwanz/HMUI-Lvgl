#ifndef CSVGBUTTON_H
#define CSVGBUTTON_H

#include <QPushButton>

class CSvgButton: public QPushButton
{
public:
    CSvgButton(const QString & sPath, QWidget *parent = nullptr);
	CSvgButton(QWidget *parent = nullptr);
    int setSvgPath(const QString & sPath);
public:
    virtual void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *e) override;
	void mouseReleaseEvent(QMouseEvent *e) override;
private:
    QString m_sImagePath;
	QString m_sOriginalPath;
};

#endif // CSVGBUTTON_H
