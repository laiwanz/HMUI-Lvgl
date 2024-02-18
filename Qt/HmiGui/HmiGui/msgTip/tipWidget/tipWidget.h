#ifndef _TIPWIDGET_H__20220621
#define _TIPWIDGET_H__20220621

#include <stdint.h>
#include <QWidget>
#include <QPainter>

class CTipWidget : public QWidget {
public:
    explicit CTipWidget(QWidget* parent = nullptr);
	virtual ~CTipWidget();
    
    virtual int  setTitle(const QString& sTitle);
    virtual void setDispTitle(bool enabled);
    virtual void setFont(const QFont& font);
    virtual int  doEvent(void* pEvent);
    virtual int  timer();
    virtual int  getMsgNum();
    virtual int  getNewMsg(std::string& sMsg, uint64_t& ullTime, int& nLevel);
    virtual void clearMsg();

protected:
    virtual void updateTitleArea();
    virtual void updateMsgArea();
    virtual void updateArea();
    virtual int  getMsg(QString& strMsg) = 0;
    virtual void drawTitle(QPainter& p);
    virtual void drawMsg(QPainter& p);
    virtual void paintEvent(QPaintEvent *event);
    
protected:
    QFont       m_font;
    QString     m_strTitle;
    QSize       m_sizeTitle;
    bool        m_bDispTitle;
    QSize       m_sizeMsg;
};
#endif
