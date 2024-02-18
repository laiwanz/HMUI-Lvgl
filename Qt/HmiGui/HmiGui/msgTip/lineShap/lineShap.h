#ifndef _LINESHAP_H__20220621
#define _LINESHAP_H__20220621

#include <QWidget>

class CLineShap : public QWidget {
public:
    explicit CLineShap(QWidget* parent = NULL);
	virtual ~CLineShap();
    
    void setColor(const QColor& color);
    void setWidth(int nWidth);

protected:
    void paintEvent(QPaintEvent* event);

private:
    QColor  m_color;
    int     m_nWidth;
};
#endif
