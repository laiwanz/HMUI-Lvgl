#ifndef _LEAF_H__20220826
#define _LEAF_H__20220826

#include <QPainter>
#include "../common/common.h"

class CLeaf {
public:
    CLeaf();
	~CLeaf();

    void setDirect(DIRECT_T direct);
    void setGeometry(int x, int y, int width, int height);
	void paint(QPainter& p, int nHeight);

private:
    QRect       m_area;
    DIRECT_T    m_direct;
};
#endif
