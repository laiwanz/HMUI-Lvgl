#include "serialmanager.h"
#include "Register/Register.h"
#include "RegisterOpt/RegisterOpt.h"
#include "utility/utility.h"
#include "HmiBase/HmiBase.h"
#include "../cloud/cloud.h"
#include "macrosplatform/registerAddr.h"
#ifdef _LINUX_
#include "btl/btl.h"
using namespace btl;
#endif

#define  ETH_CONFIG_ADDROFFSET		7			//网络通讯，目标IP配置寄存器的偏移量

CSerialManager::CSerialManager() {

}

CSerialManager::~CSerialManager() {

}

CSerialManager& CSerialManager::get_instance() {
	static CSerialManager serialManager;
	return serialManager;
}

#ifdef _LINUX_
int onSerialInfo(const std::list<btl::SERIALINFO> &listSerialInfo, void *lpParameter) {
	return CSerialManager::get_instance().btltoaddr(listSerialInfo);
}
#endif

int CSerialManager::start() {
	SERIAL_LOAD_TYPE nType = SERIAL_FORM_XML;
	int nRtn = 0;
	if ((nRtn = this->loadSerialType(nType)) < 0) {
		printf("%s:load serial error:%d\r\n", __FUNCTION__, nRtn);
		return -1;
	}

#ifdef _LINUX_
	if ((nRtn = btl::registerSerialCallBack("HMIUI", onSerialInfo, this, false)) < 0){
		printf("%s:registerSerialCallBack error:%d\r\n", __FUNCTION__, nRtn);
		return -2;
	}
#endif

	// clear serial reg
	{
		RW_PARAM_T  rw = { RW_MODE_CACHE_DEV, 0, 0, -1, -1,  FUNC_NOT_CHECK_DATA | FUNC_CHANGE_NOT_NOTIFY };
		CRegister   reg(REG_TYPE_HS_, HSW_HMIPORTINFO, DATA_TYPE_WORD);
		uint16_t nBuffer[100] = { 0 };
		CRegisterRW::write(RW_FORMAT_INT, reg, nBuffer, 100, rw);
	}
	switch (nType)
	{
	case SERIAL_FORM_XML:
		{
			nRtn = loadSerialFromXML();
		}
		break;
	case SERIAL_FORM_SYSTEM:
		{
		nRtn = loadSerialFromSystem();
		}
		break;
	default:
		break;
	}

	return 0;
}

int CSerialManager::loadSerialType(SERIAL_LOAD_TYPE &nType) {
	RW_PARAM_T  rw = { RW_MODE_CACHE_DEV, 0, 0, -1, -1,  FUNC_NOT_CHECK_DATA | FUNC_CHANGE_NOT_NOTIFY };
	CRegister reg(REG_TYPE_HS_, 11000, DATA_TYPE_WORD);
	uint64_t ullTimestamp = 0;
	struct stat statbuf = { 0 };
	std::string sPath = getPrjDir() + "test.pi";
	int nRtn = 0;
#define	 SERIALUPDATEFLAG				"/mnt/data/prj/oldupdatenew"
	if ((nRtn = stat(sPath.c_str(), &statbuf)) < 0) {
		printf("%s:%s not exist\r\n", __FUNCTION__, sPath.c_str());
		return -1;
	}
	if ((nRtn = CRegisterRW::read(RW_FORMAT_INT, reg, &ullTimestamp, 4, rw)) < 0) {
		printf("%s:read addr error\r\n", __FUNCTION__);
		return -2;
	}

	if (ullTimestamp != (uint64_t)statbuf.st_mtime) {
		ullTimestamp = (uint64_t)statbuf.st_mtime;
		CRegisterRW::write(RW_FORMAT_INT, reg, &ullTimestamp, 4, rw);
		if (UTILITY_NAMESPACE::CFile::ExistFile(SERIALUPDATEFLAG)) {
			UTILITY_NAMESPACE::CFile::Delete(SERIALUPDATEFLAG);
			nType = SERIAL_FORM_SYSTEM;
		}
		else {
			nType = SERIAL_FORM_XML;
		}
	}
	else {
		nType = SERIAL_FORM_SYSTEM;
	}
	return 0;
}

std::string CSerialManager::getSerialName(const CONNECTNODE &connectNode) {
#ifdef WIN32
	return connectNode.Commset.szInitPortName;
#else
	return connectNode.Commset.szPortName;
#endif
}

unsigned short CSerialManager::getBaudRateAttr(int nBaudRate) {
	int nBaudRates[] = { 1200,2400,4800,9600,19200,38400,57600,115200,187500,
	230400,460800,500000,576000,921600,1000000,1152000,1500000,
	2000000,2500000,3000000,3500000,4000000 };
	for (unsigned short nI = 0; nI < sizeof(nBaudRates) / sizeof(nBaudRates[0]); ++nI) {
		if (nBaudRate == nBaudRates[nI]) {
			return nI;
		}
	}
	return 0;
}

int CSerialManager::getBaudRate(unsigned short nBaudRateId) {
	int nBaudRates[] = { 1200,2400,4800,9600,19200,38400,57600,115200,187500,
230400,460800,500000,576000,921600,1000000,1152000,1500000,
2000000,2500000,3000000,3500000,4000000 };
	if (nBaudRateId >= sizeof(nBaudRates) / sizeof(nBaudRates[0])) {
		return -1;
	}
	return nBaudRates[nBaudRateId];
}

unsigned short CSerialManager::getConnectMode(const std::string &sMode) {
	unsigned short nMode = 0;
	if (sMode == "RS232") {
		nMode = 0;
	}
	else if (sMode == "RS485_1") {
		nMode = 1;
	}
	else if (sMode == "RS422") {
		nMode = 2;
	}
	else if (sMode == "RS485_2") {
		nMode = 6;
	}
	else {
		nMode = 0;
	}
	return nMode;
}

std::string CSerialManager::getConnectModeName(unsigned short nMode) {
	std::string sModeName = "";
	switch (nMode)
	{
	case 0:
		{
			sModeName = "RS232";
		}
		break;
	case 1:
		{
			sModeName = "RS485_1";
		}
		break;
	case 2:
		{
			sModeName = "RS422";
		}
		break;
	case 6:
		{
			sModeName = "RS485_2";
		}
		break;
	default:
		{
			sModeName = "RS232";
		}
		break;
	}
	return sModeName;
}

unsigned short CSerialManager::getCheckBit(const std::string &sCheckBit) {
	unsigned short nCheckBit = 0;
	if ("NONE" == sCheckBit) {
		nCheckBit = 0;
	}
	else if ("ODD" == sCheckBit) {
		nCheckBit = 1;
	}
	else if ("EVEN" == sCheckBit) {
		nCheckBit = 2;
	}
	else if ("SPACE" == sCheckBit) {
		nCheckBit = 3;
	}
	else if ("MARK" == sCheckBit) {
		nCheckBit = 4;
	}
	return nCheckBit;
}

std::string CSerialManager::getCheckBitName(unsigned short nCheckBit) {
	std::string sCheckBitName = "";
	switch (nCheckBit) {
	case 0:
		{
			sCheckBitName = "NONE";
		}
		break;
	case 1:
		{
			sCheckBitName = "ODD";
		}
		break;
	case 2:
		{
			sCheckBitName = "EVEN";
		}
		break;
	case 3:
		{
			sCheckBitName = "SPACE";
		}
		break;
	case 4:
		{
			sCheckBitName = "MARK";
		}
		break;
	default:
		break;
	}
	return sCheckBitName;
}

int CSerialManager::loadSerialFromXML() {
	unsigned short nMaxCommu = 0;
	static unsigned short nCurrentCan0 = 0, nCurrentCan1 = 0, nCurrentCom1 = 0, \
		nCurrentCom2 = 0, nCurrentCom3 = 0;
	RW_PARAM_T  rw = { RW_MODE_CACHE_DEV, 0, 0, -1, -1,  FUNC_NOT_CHECK_DATA | FUNC_CHANGE_NOT_NOTIFY };
	CRegister   reg(REG_TYPE_HS_, 0, DATA_TYPE_WORD);
	unsigned int nIndex = 0;
#ifdef _LINUX_
	std::list<SERIALINFO> listSerials;
#endif
	std::list<CONNECTNODE> listCommu = CCloudParse::get_instance().getCommu();
	for (auto &iter : listCommu) {
		uint16_t nBuffer[12] = { 0 };
		uint16_t nBaudRate = 0;
#ifdef _LINUX_
		SERIALINFO serial;
		std::string sPortName = "";
#endif
		std::string sName = this->getSerialName(iter);
		if (Comm_Ethernet == iter.Commset.nCommMode) {
			char szDstIPV4[32] = { 0 };
			strncpy(szDstIPV4, iter.Commset.szDstIPV4, 32);
			unsigned long dwDestIP = inet_addr(szDstIPV4);
			int nPortId = iter.nPortId;
			nBuffer[0] = dwDestIP & 0xFF;
			nBuffer[1] = (dwDestIP & 0xFF00) >> 8;
			nBuffer[2] = (dwDestIP & 0xFF0000) >> 16;
			nBuffer[3] = (dwDestIP & 0xFF000000) >> 24;
			nBuffer[4] = iter.Commset.iDstPort;
			nBuffer[5] = iter.Commset.nEthernet_IsUDP;
			reg.setAddr(11004 + nPortId * ETH_CONFIG_ADDROFFSET);
			CRegisterRW::write(RW_FORMAT_INT, reg, nBuffer, ETH_CONFIG_ADDROFFSET, rw);
			continue;
		}
		else if ("CAN0" == sName) {
			nMaxCommu = 1;
			if (nCurrentCan0 >= nMaxCommu) {
				continue;
			}
			nCurrentCan0++;
			nBaudRate = iter.Commset.nBaudRate / 1000;
			reg.setAddr(10116);
			CRegisterRW::write(RW_FORMAT_INT, reg, &nBaudRate, 1, rw);
			continue;
		}
		else if ("CAN1" == sName) {
			nMaxCommu = 1;
			if (nCurrentCan1 >= nMaxCommu) {
				continue;
			}
			nCurrentCan1++;
			nBaudRate = iter.Commset.nBaudRate / 1000;
			reg.setAddr(10117);
			CRegisterRW::write(RW_FORMAT_INT, reg, &nBaudRate, 1, rw);
			continue;
		}
		else {
			if ("COM1" == sName) {
				nMaxCommu = 3;
				if (nCurrentCom1 >= nMaxCommu) {
					continue;
				}
				nCurrentCom1++;
#ifdef _LINUX_
				sPortName = std::string("COM1-") + std::to_string(nCurrentCom1);
#endif
				if (nCurrentCom1 == 1) {
					nIndex = 10061;
				}
				else if (nCurrentCom1 == 2) {
					nIndex = 10094;
				}
				else {
					nIndex = 10120;
				}
			}
			else if ("COM2" == sName) {
				nMaxCommu = 2;
				if (nCurrentCom2 >= nMaxCommu) {
					continue;
				}
				nCurrentCom2++;
#ifdef _LINUX_
				sPortName = std::string("COM2-") + std::to_string(nCurrentCom2);
#endif
				if (nCurrentCom2 == 1) {
					nIndex = 10072;
				}
				else {
					nIndex = 10105;
				}
			}
			else if ("COM3" == sName) {
				nMaxCommu = 1;
				if (nCurrentCom3 >= nMaxCommu) {
					continue;
				}
				nCurrentCom3++;
#ifdef _LINUX_
				sPortName = "COM3";
#endif
				nIndex = 10083;
			}
			else {
				continue;
			}
			nBuffer[0] = iter.Commset.nCommMode;
			nBuffer[1] = this->getBaudRateAttr(iter.Commset.nBaudRate);
			nBuffer[2] = iter.Commset.nDataLength;
			nBuffer[3] = iter.Commset.nStopBits;
			nBuffer[4] = this->getCheckBit(iter.Commset.szCheckBit);
			nBuffer[5] = iter.Commset.nWaitTimeout;
			nBuffer[6] = iter.Commset.nRecvTimeout;
			nBuffer[7] = iter.Commset.nRetryTimes;
			nBuffer[8] = iter.Commset.nRetryTimeOut;
			nBuffer[9] = iter.PlcSet.nHmiMachineNo;
			nBuffer[10] = iter.PlcSet.nPlcMachineNo;
#ifdef _LINUX_
			serial.sSerialportNum = sPortName;
			serial.sConnectMode = this->getConnectModeName(nBuffer[0]);
			serial.nBaudrate = iter.Commset.nBaudRate;
			serial.nDataBit = nBuffer[2];
			serial.nStopBit = nBuffer[3];
			serial.nVerifyBit = nBuffer[4];
			serial.nWaitTimeout = nBuffer[5];
			serial.nReadTimeout = nBuffer[6];
			serial.nRetryTimes = nBuffer[7];
			serial.nRetryTimeout = nBuffer[8];
			serial.nHmiStationNum = nBuffer[9];
			serial.nDeviceStationNum = nBuffer[10];
#endif
			reg.setAddr(nIndex);
			CRegisterRW::write(RW_FORMAT_INT, reg, nBuffer, 11, rw);
			reg.setAddr(10060);
			nBuffer[0] = 1;
			CRegisterRW::write(RW_FORMAT_INT, reg, nBuffer, 1, rw);
		}
#ifdef _LINUX_
		listSerials.push_back(serial);
#endif
	}
	return 0;
}

int CSerialManager::loadSerialFromSystem() {
	if (btltoaddr() < 0){
		return -1;
	}
	RW_PARAM_T  rw = { RW_MODE_CACHE_DEV, 0, 0, -1, -1,  FUNC_NOT_CHECK_DATA | FUNC_CHANGE_NOT_NOTIFY };
	CRegister   reg(REG_TYPE_HS_, 0, DATA_TYPE_WORD);
	reg.setAddr(10060);
	uint16_t nBuffer[2] = {1};
	CRegisterRW::write(RW_FORMAT_INT, reg, nBuffer, 1, rw);
	return 0;
}

bool CSerialManager::isFreeLabel(const std::string &sPLCType) {
	std::string sFreeLabel[] = { "OMRON NX Ethernet/IP",
									"Allen-Bradley FreeTag",
									"Allen-Bradley FreeTag Ethernet/IP(CompactLogix)",
									"Allen-Bradley FreeTag Ethernet/IP",
									"OMRON NJ Ethernet/IP",
									"Allen-Bradley EtherNet/IP",
									"OMRON EtherNet/IP (NX/NJ Series)" };
	for (unsigned short nI = 0; nI < sizeof(sFreeLabel) / sizeof(sFreeLabel[0]); nI++) {
		if (sPLCType == sFreeLabel[nI]) {
			return true;
		}
	}
	return false;
}

int CSerialManager::addrtobtl() {
	int nRtn = 0;
	RW_PARAM_T  rw = { RW_MODE_CACHE_DEV, 0, 0, -1, -1,  FUNC_NOT_CHECK_DATA | FUNC_CHANGE_NOT_NOTIFY };
	CRegister   reg(REG_TYPE_HS_, 0, DATA_TYPE_WORD);
	std::list<CONNECTNODE> listCommu = CCloudParse::get_instance().getCommu();
#ifdef _LINUX_
	std::list<SERIALINFO> listSerials;
	if ((nRtn = btl::getAllServialInfo(listSerials)) < 0) {
		printf("%s:get serial error:%d\r\n", __FUNCTION__, nRtn);
		return -1;
	}
	unsigned short nCurrentCom1 = 0, nCurrentCom2 = 0;
	if (!listSerials.empty() &&
		!listCommu.empty()) {
		bool bUpdate = false;
		for (auto &iter : listSerials) {
			for (auto &iterCommu : listCommu) {
				unsigned short nMaxCommu = 0;
				unsigned int nIndex = 0;
				std::string sPortName = getSerialName(iterCommu);
				std::transform(sPortName.begin(), sPortName.end(), sPortName.begin(), ::tolower);
				if (Comm_Ethernet == iterCommu.Commset.nCommMode ||
					"can1" == sPortName ||
					"can0" == sPortName) {
					continue;
				}
				if ("com1" == sPortName) {
					nMaxCommu = 3;
					if (nCurrentCom1 >= nMaxCommu ||
						iter.sSerialportNum != sPortName + std::string("-") + std::to_string(nCurrentCom1 + 1)) {
						continue;
					}
					nCurrentCom1++;
					if (nCurrentCom1 == 1) {
						nIndex = 10061;
					}
					else if (nCurrentCom1 == 2) {
						nIndex = 10094;
					}
					else {
						nIndex = 10120;
					}
				}
				else if ("com2" == sPortName) {
					nMaxCommu = 2;
					if (nCurrentCom2 >= nMaxCommu ||
						iter.sSerialportNum != sPortName + std::string("-") + std::to_string(nCurrentCom2 + 1)) {
						continue;
					}
					nCurrentCom2++;
					if (nCurrentCom2 == 1) {
						nIndex = 10072;
					}
					else {
						nIndex = 10105;
					}
				}
				else if ("com3" == sPortName) {
					if (iter.sSerialportNum != sPortName) {
						continue;
					}
					nIndex = 10083;
				}

				reg.setAddr(nIndex);
				uint16_t nBuffer[12] = { 0 };
				CRegisterRW::read(RW_FORMAT_INT, reg, nBuffer, 11, rw);

				if (iterCommu.Commset.nCommMode != nBuffer[0] ||
					iterCommu.Commset.nBaudRate != this->getBaudRate(nBuffer[1]) ||
					iterCommu.Commset.nDataLength != nBuffer[2] ||
					iterCommu.Commset.nStopBits != nBuffer[3] ||
					strcmp(iterCommu.Commset.szCheckBit, this->getCheckBitName(nBuffer[4]).c_str()) != 0 ||
					iterCommu.Commset.nWaitTimeout != nBuffer[5] ||
					iterCommu.Commset.nRecvTimeout != nBuffer[6] ||
					iterCommu.Commset.nRetryTimes != nBuffer[7] ||
					iterCommu.Commset.nRetryTimeOut != nBuffer[8] ||
					iterCommu.PlcSet.nHmiMachineNo != nBuffer[9] ||
					iterCommu.PlcSet.nPlcMachineNo != nBuffer[10]) {

					bUpdate = true;
					iter.sConnectMode = this->getConnectModeName(nBuffer[0]);
					iterCommu.Commset.nCommMode = nBuffer[0];
					iterCommu.Commset.nBaudRate = iter.nBaudrate = this->getBaudRate(nBuffer[1]);
					iterCommu.Commset.nDataLength = iter.nDataBit = nBuffer[2];
					iterCommu.Commset.nStopBits = iter.nStopBit = nBuffer[3];
					iter.nVerifyBit = nBuffer[4];
					strcpy(iterCommu.Commset.szCheckBit, this->getCheckBitName(nBuffer[4]).c_str());
					iterCommu.Commset.nWaitTimeout = iter.nWaitTimeout = nBuffer[5];
					iterCommu.Commset.nRecvTimeout = iter.nReadTimeout = nBuffer[6];
					iterCommu.Commset.nRetryTimes = iter.nRetryTimes = nBuffer[7];
					iterCommu.Commset.nRetryTimeOut = iter.nRetryTimeout = nBuffer[8];
					iterCommu.PlcSet.nHmiMachineNo = iter.nHmiStationNum = nBuffer[9];
					iterCommu.PlcSet.nPlcMachineNo = iter.nDeviceStationNum = nBuffer[10];
					if ((nRtn = btl::setSerialInfo(iter)) < 0) {
						printf("%s:setSerialInfo error:%d\r\n", __FUNCTION__, nRtn);
						return -2;
					}
				}
			}
		}

		if (bUpdate){
			for (auto &iter : listCommu) {
				if ((nRtn = CEngineMgr::getInstance().reLoadPlcDriVer(iter)) < 0) {
					printf("%s: reload plc driver error:%d\r\n", __FUNCTION__, nRtn);
					return -4;
				}
			}

			if ((nRtn = CCloudParse::get_instance().setCommu(listCommu)) < 0) {
				printf("%s:set commu error:%d\r\n", __FUNCTION__, nRtn);
				return -5;
			}
		}
	}
	else {
		return -2;
	}
#endif

	return 0;
}

int CSerialManager::btltoaddr() {
#ifdef _LINUX_
	int nRtn = 0;
	std::list<SERIALINFO> listSerials;
	if ((nRtn = btl::getAllServialInfo(listSerials)) < 0) {
		printf("%s:get serial error:%d\r\n", __FUNCTION__, nRtn);
		return -1;
	}

	if ((nRtn = btltoaddr(listSerials)) < 0){
		printf("%s:btltoaddr error:%d\r\n", __FUNCTION__, nRtn);
		return -2;
	}
#endif
	return 0;
}

#ifdef _LINUX_
int CSerialManager::btltoaddr(const std::list<btl::SERIALINFO> &listSerials) {

	int nRtn = 0;
	uint16_t nBuffer[12] = { 0 };
	RW_PARAM_T  rw = { RW_MODE_CACHE_DEV, 0, 0, -1, -1,  FUNC_NOT_CHECK_DATA | FUNC_CHANGE_NOT_NOTIFY };
	CRegister   reg(REG_TYPE_HS_, 0, DATA_TYPE_WORD);
	std::list<CONNECTNODE> listCommu = CCloudParse::get_instance().getCommu();
	unsigned short nCurrentCom1 = 0, nCurrentCom2 = 0;
	if (!listSerials.empty() &&
		!listCommu.empty()) {
		bool bUpdate = false;
		for (auto &iter : listSerials) {
			for (auto &iterCommu : listCommu) {
				unsigned short nMaxCommu = 0;
				unsigned int nIndex = 0;
				std::string sPortName = getSerialName(iterCommu);
				std::transform(sPortName.begin(), sPortName.end(), sPortName.begin(), ::tolower);
				if (Comm_Ethernet == iterCommu.Commset.nCommMode ||
					"can1" == sPortName ||
					"can0" == sPortName) {
					continue;
				}
				if ("com1" == sPortName) {
					nMaxCommu = 3;
					if (nCurrentCom1 >= nMaxCommu ||
						iter.sSerialportNum != sPortName + std::string("-") + std::to_string(nCurrentCom1 + 1)) {
						continue;
					}
					nCurrentCom1++;
					if (nCurrentCom1 == 1) {
						nIndex = 10061;
					}
					else if (nCurrentCom1 == 2) {
						nIndex = 10094;
					}
					else {
						nIndex = 10120;
					}
				}
				else if ("com2" == sPortName) {
					nMaxCommu = 2;
					if (nCurrentCom2 >= nMaxCommu ||
						iter.sSerialportNum != sPortName + std::string("-") + std::to_string(nCurrentCom2 + 1)) {
						continue;
					}
					nCurrentCom2++;
					if (nCurrentCom2 == 1) {
						nIndex = 10072;
					}
					else {
						nIndex = 10105;
					}
				}
				else if ("com3" == sPortName) {
					if (iter.sSerialportNum != sPortName) {
						continue;
					}
					nIndex = 10083;
				}

				if (iterCommu.Commset.nCommMode != this->getConnectMode(iter.sConnectMode) ||
					iterCommu.Commset.nBaudRate != iter.nBaudrate ||
					iterCommu.Commset.nDataLength != iter.nDataBit ||
					iterCommu.Commset.nStopBits != iter.nStopBit ||
					0 != strcmp(iterCommu.Commset.szCheckBit, this->getCheckBitName(iter.nVerifyBit).c_str()) ||
					iterCommu.Commset.nWaitTimeout != iter.nWaitTimeout ||
					iterCommu.Commset.nRecvTimeout != iter.nReadTimeout ||
					iterCommu.Commset.nRetryTimes != iter.nRetryTimes ||
					iterCommu.Commset.nRetryTimeOut != iter.nRetryTimeout ||
					iterCommu.PlcSet.nHmiMachineNo != iter.nHmiStationNum ||
					iterCommu.PlcSet.nPlcMachineNo != iter.nDeviceStationNum){

					bUpdate = true;
					nBuffer[0] = iterCommu.Commset.nCommMode = this->getConnectMode(iter.sConnectMode);
					nBuffer[1] = this->getBaudRateAttr(iter.nBaudrate);
					iterCommu.Commset.nBaudRate = iter.nBaudrate;
					nBuffer[2] = iterCommu.Commset.nDataLength = iter.nDataBit;
					nBuffer[3] = iterCommu.Commset.nStopBits = iter.nStopBit;
					strcpy(iterCommu.Commset.szCheckBit, this->getCheckBitName(iter.nVerifyBit).c_str());
					nBuffer[4] = iter.nVerifyBit;
					nBuffer[5] = iterCommu.Commset.nWaitTimeout = iter.nWaitTimeout;
					nBuffer[6] = iterCommu.Commset.nRecvTimeout = iter.nReadTimeout;
					nBuffer[7] = iterCommu.Commset.nRetryTimes = iter.nRetryTimes;
					nBuffer[8] = iterCommu.Commset.nRetryTimeOut = iter.nRetryTimeout;
					nBuffer[9] = iterCommu.PlcSet.nHmiMachineNo = iter.nHmiStationNum;
					nBuffer[10] = iterCommu.PlcSet.nPlcMachineNo = iter.nDeviceStationNum;
					reg.setAddr(nIndex);
					CRegisterRW::write(RW_FORMAT_INT, reg, nBuffer, 11, rw);
				}

				
			}
		}

		if (bUpdate) {
			for (auto &iter : listCommu) {
				if ((nRtn = CEngineMgr::getInstance().reLoadPlcDriVer(iter)) < 0) {
					printf("%s: reload plc driver error:%d\r\n", __FUNCTION__, nRtn);
					return -4;
				}
			}

			if ((nRtn = CCloudParse::get_instance().setCommu(listCommu)) < 0) {
				printf("%s:set commu error:%d\r\n", __FUNCTION__, nRtn);
				return -5;
			}

		}
	}
	else {
		return -2;
	}
	return 0;
}
#endif