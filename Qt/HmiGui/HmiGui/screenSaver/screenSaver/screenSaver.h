#ifndef _SCREENSAVER_H__20220908
#define _SCREENSAVER_H__20220908

#include <QWidget>

class CScreenSaver : public QWidget {
public:
    CScreenSaver();
    ~CScreenSaver();

    virtual int start(QWidget* widget) = 0;
    virtual bool isRunning() = 0;
    virtual int stop() = 0;
    virtual void mousePressEvent(QMouseEvent* ev);
    virtual void mouseReleaseEvent(QMouseEvent* ev);

    typedef enum {
        
        TYPE_LATTICE    = 0,    /* 格子 */
		TYPE_BLINDS		= 1,    /* 百叶窗 */
		TYPE_BUBBLE		= 2,    /* 泡泡 */
        //TYPE_STRIPE     = 4,    /* 条纹 */
        TYPE_INVALID,
    } TYPE_T;

    TYPE_T type();

protected:
    TYPE_T  m_type;
    QString m_scrClick;     /* 点击的画面 */
};
#endif
