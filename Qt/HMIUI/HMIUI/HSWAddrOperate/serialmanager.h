#ifndef _SERIALMANAGER_H_20231020_
#define _SERIALMANAGER_H_20231020_
#include <iostream>
#include "DevCacheMgr/DriverConfig.h"

#ifdef _LINUX_
#include "btl/btl.h"
#endif

typedef enum {
	SERIAL_FORM_XML = 0,
	SERIAL_FORM_SYSTEM = 1,
} SERIAL_LOAD_TYPE;

class CSerialManager
{
public:
	CSerialManager& operator=(const CSerialManager&) = delete;
	static CSerialManager& get_instance();
	~CSerialManager();
	int start();
	bool isFreeLabel(const std::string &sPLCType);
	int addrtobtl();
	int btltoaddr();
#ifdef _LINUX_
	int btltoaddr(const std::list<btl::SERIALINFO> &listSerialInfo);
#endif
private:
	CSerialManager();
	int loadSerialType(SERIAL_LOAD_TYPE &nType);
	int loadSerialFromXML();
	int loadSerialFromSystem();
	std::string getSerialName(const CONNECTNODE &connectNode);
	unsigned short getBaudRateAttr(int nBaudRate);
	int getBaudRate(unsigned short);
	unsigned short getConnectMode(const std::string &sMode);
	std::string getConnectModeName(unsigned short nMode);
	unsigned short getCheckBit(const std::string &sCheckBit);
	std::string getCheckBitName(unsigned short nCheckBit);
#ifdef _LINUX_
	friend int onSerialInfo(const std::list<btl::SERIALINFO> &listSerialInfo, void *lpParameter);
#endif
};
#endif