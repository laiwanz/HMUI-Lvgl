#include "hmipart.h"
#include <QWidget>

HMIPart::HMIPart() {
#if ((defined WIN32) && (defined WIN32SCALE))
	memset(&m_OriginalRect, 0, sizeof(RECT));
#endif
	m_sPartName = "";
}

HMIPart::~HMIPart() {
}

void HMIPart::OnPartShow() {
}

void HMIPart::OnPartHide() {
}

int HMIPart::parseJson(const std::string &sbuffer) {
	return 0;
}