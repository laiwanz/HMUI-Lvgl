#ifndef LVREMARK_H
#define LVREMARK_H
#include <lvgl/lvgl/lvgl.h>

#include <string>
using std::string;

typedef enum LvWIDGETDIRECTION {
	LvDirection_NONE = 0,
	LvDirection_top_right = 1,
	LvDirection_top_left = 2,
	LvDirection_bottom_right = 3,
	LvDirection_bottom_left = 4,
	LvDirection_left = 5,
	LvDirection_right = 6
} LvWIDGETDIRECTION_T;

class LvRemark {

public:
	explicit LvRemark(lv_obj_t *parent = nullptr);
	~LvRemark();

	int setTriangeOffset(int nOffset);
	int setTriangeSize(unsigned int nWidth, unsigned int nHeight);
	int setText(const string &sText);
	int setDirection(LvWIDGETDIRECTION eDirection);

private:
	void Paint();

private:
	int m_nOffset;
	unsigned int m_nTriangleWidth, m_nTriangleHeight, m_nRadius;
	LvWIDGETDIRECTION m_eDirection;
	lv_obj_t *m_pLabel;
	bool m_bWarning;
};
#endif // !LVREMARK_H
