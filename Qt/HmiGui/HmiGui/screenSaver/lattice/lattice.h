#ifndef _LATTICE_H__20220921
#define _LATTICE_H__20220921

#include "../screenSaver/screenSaver.h"

class CLattice : public CScreenSaver {
    Q_OBJECT
public:
    CLattice();
    virtual ~CLattice();
    int start(QWidget* widget);
    bool isRunning();
    int stop();

protected:
    void paintEvent(QPaintEvent* ev);

private slots:
    void slotTimer();

private:
    int init();

private:
    QTimer* m_pTimer;       /* 定时器 */
    bool    m_bInit;        /* 是否初始化 */
    int     m_nBrushType;   /* 画刷类型(0:刷白,1:刷黑) */
    bool    m_bDispOddLine; /* 从奇数行开始显示 */
};
#endif
