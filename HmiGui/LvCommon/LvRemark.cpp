#include "LvRemark.h"

LvRemark::LvRemark(lv_obj_t * parent)
{
	m_nOffset = 10;
	m_nTriangleWidth = 20;
	m_nTriangleHeight = 10;
	m_nRadius = 5;
	m_pLabel = lv_label_create(parent);
	lv_obj_set_align(m_pLabel, LV_ALIGN_CENTER);
	// int nWidth = 0, nHeight = 0;


	m_bWarning = false;

}

LvRemark::~LvRemark()
{
}

int LvRemark::setTriangeOffset(int nOffset)
{
	if (m_nOffset == nOffset) {
		return 0;
	}
	m_nOffset = nOffset;
	return 0;
}

int LvRemark::setTriangeSize(unsigned int nWidth, unsigned int nHeight)
{
	m_nTriangleHeight = nHeight;
	m_nTriangleWidth = nWidth;
	return 0;
}

int LvRemark::setText(const string & sText)
{
	lv_label_set_text(m_pLabel, sText.c_str());

	return 0;
}

int LvRemark::setDirection(LvWIDGETDIRECTION eDirection)
{
	return 0;
}



void LvRemark::Paint()
{

}
