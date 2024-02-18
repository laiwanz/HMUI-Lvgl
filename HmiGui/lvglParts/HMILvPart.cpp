#include "HMILvPart.h"

HMILvPart::HMILvPart()
{
#if ((defined WIN32) && (defined WIN32SCALE))
	memset(&m_OriginalRect, 0, sizeof(RECT));
#endif
	m_sPartName = "";
}

HMILvPart::~HMILvPart()
{
}

void HMILvPart::OnPartShow()
{

}

void HMILvPart::OnPartHide()
{
}

int HMILvPart::parseJson(const std::string & sbuffer)
{

	return 0;
}
