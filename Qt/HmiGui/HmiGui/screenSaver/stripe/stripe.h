#ifndef _STRIPE_H__20220907
#define _STRIPE_H__20220907

#include "../screenSaver/screenSaver.h"
#include "item.h"

class CStripe : public CScreenSaver {
    Q_OBJECT
public:
    CStripe();
    virtual ~CStripe();
    int start(QWidget* widget);
    bool isRunning();
    int stop();

    typedef enum  {
        MODE_IN     = 0,
        MODE_OUT    = 1,
        MODE_MOVE   = 2,
    } MODE_T;

protected:
    void paintEvent(QPaintEvent* ev);

private slots:
    void slotTimer();

private:
    int init();
    void resetItemArea();
    void updateItemLenth();
    void swapItemPosition();

private:
    QVector<CItem*> m_vecItem;
    QTimer*         m_pTimer;
    int             m_nFrameCnt;
    int             m_nItemLen;
    bool            m_bInit;
    QBrush          m_brushWhite;
    QBrush          m_brushBlack;
    int             m_nBrushType;
    MODE_T          m_mode;
};
#endif
