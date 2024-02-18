#include "cbl/cbl.h"
#include "HJ212Control.h"
#include "HmiBase/HmiBase.h"

/* construct
**/
CHJ212Control::CHJ212Control() {
	m_pManager = nullptr;
}

/* destruct
**/
CHJ212Control::~CHJ212Control() {
	this->stop();
}

/* get instance
**/
CHJ212Control& CHJ212Control::get_instance() {
	static CHJ212Control HJ212;
	return HJ212;
}

/* start 
**/
int CHJ212Control::start() {
	if (m_pManager == nullptr) {
		m_pManager = new HJ212::CManager();
	}
	int nRtn = 0;
	if ((nRtn = m_pManager->start(getPrjDir())) < 0) {
		printf("%s:HJ212 start error:%d\r\n", __FUNCTION__, nRtn);
		delete m_pManager;
		m_pManager = nullptr;
		return -1;
	}
	m_pManager->setMessageFunc([this](const string &sText) {
		return CMessageCenter::get_instance().insert(hmiproto::message_type_insert, sText);
	});
	return 0;
}

/* stop 
**/
int CHJ212Control::stop() {
	if (m_pManager) {
		m_pManager->stop();
		delete m_pManager;
		m_pManager = nullptr;
	}
	return 0;
}

int CHJ212Control::setLanguage(int nLanguage) {
	if (m_pManager) {
		return m_pManager->setLanguage(nLanguage);
	}
	return -1;
}