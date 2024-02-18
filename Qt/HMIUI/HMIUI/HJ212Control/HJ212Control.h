#ifndef _HJ212CONTROL_H_20240118_
#define _HJ212CONTROL_H_20240118_
#include <iostream>
#include "HJ212/Manager.h"

class CHJ212Control
{
public:
	CHJ212Control(const CHJ212Control&) = delete;
	CHJ212Control& operator=(const CHJ212Control&) = delete;
	/* get instance */
	static CHJ212Control& get_instance();
	/* start */
	int start();
	/* stop */
	int stop();
	int setLanguage(int nLanguage);
private:
	CHJ212Control();
	~CHJ212Control();
private:
	HJ212::CManager *m_pManager;
};

#endif //_HJ212CONTROL_H_20240118_