#ifndef _ITEM_H__20220907
#define _ITEM_H__20220907

#include <QPainter>
#include "../common/common.h"

class CItem {
public:
    CItem();
	~CItem();

    void setDirect(DIRECT_T direct);
    void setGeometry(int x, int y, int width, int height);
    void move(int x, int y);
    void paint(QPainter& p, int nLen);

private:
    DIRECT_T    m_direct;
    QRect       m_area;
};
#endif
