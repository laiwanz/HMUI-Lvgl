#include "LvCustomPart.h"
#include <string>
using std::string;
LvCustomPart::LvCustomPart(const hmiproto::hmicustom & custom, HMILvScreen * parent)
{
	std::cout << "LvCustomPart Construction" << std::endl;
	m_svg = NULL;
	m_bUseShowHide = false;
	m_part.Clear();
	m_part.MergeFrom(custom);
}

LvCustomPart::~LvCustomPart()
{
}

void LvCustomPart::Init(lv_obj_t * pWidget)
{
	RECT	rc;

	rc.left = m_part.basic().left();
	rc.right = m_part.basic().right();
	rc.top = m_part.basic().top();
	rc.bottom = m_part.basic().bottom();
	string strPartType(m_part.basic().type().c_str());

	if (m_svg == NULL) {
		m_svg = new LvCommonPart(pWidget, strPartType);
		int nLeft = rc.left;
		int nTop = rc.top;
		int nWidth = rc.right - rc.left;
		int nHeight = rc.bottom - rc.top;
		memcpy(&m_rc, &rc, sizeof(RECT));
		m_svg->LvsetRoateRect(nLeft, nTop, nWidth, nHeight);
	}
}

void LvCustomPart::Update(const hmiproto::hmicustom & custom, HMILvScreen * pWidget)
{
	RECT	rc;
	(void)pWidget;
	m_part.MergeFrom(custom);

	rc.left = m_part.basic().left();
	rc.right = m_part.basic().right();
	rc.top = m_part.basic().top();
	rc.bottom = m_part.basic().bottom();
	if (memcmp(&m_rc, &rc, sizeof(RECT)) != 0) {
		memcpy(&m_rc, &rc, sizeof(RECT));
		if (m_svg) {
			m_svg->LvsetRoateRect(m_rc.left, m_rc.top, m_rc.right - m_rc.left,
				m_rc.bottom - m_rc.top);
		}
	}

	//֡�ʸ��´��� Linzhen 20200612
	unsigned int nFreq = m_svg->getFreq();
	if (nFreq != m_part.freq()) {
		m_svg->setFreq(m_part.freq());
	}


}

void LvCustomPart::Draw(HMILvScreen * pWidget, int drawtype)
{
	std::cout << "LvCustomPart Draw  " << std::endl;
	if (!m_part.has_content()) {
		return;
	}

	bool			bAll;
	string			strSvg;
	char*	contents;

	if (m_part.has_updatecontent()) {
		bAll = false;
		strSvg = m_part.updatecontent().c_str();
	}
	else {
		bAll = true;
		strSvg = m_part.content().c_str();
	}

	contents = (char *)malloc(strSvg.length() + 1);
	memcpy(contents, strSvg.c_str(), strSvg.length());
	contents[strSvg.length()] = '\0';

	if (m_part.basic().dispdirect() == 999)//��ͼƬ��ʾ
	{
		RECT	rc;

		rc.left = m_part.basic().left();
		rc.right = m_part.basic().right();
		rc.top = m_part.basic().top();
		rc.bottom = m_part.basic().bottom();
		m_svg->setImg(contents, rc);
	}
	else {
		m_svg->setSvg(contents, m_part.basic().type(), bAll);
	}

}

