#ifdef WIN32
#ifndef _HMIEMULATOR_CPP
#define _HMIEMULATOR_CPP
enum {
	LANGUANE_JTCHINESE	= 0,	// 简体中文
	LANGUANE_ENGLISH	= 1,	// 英文
	LANGUANE_FTCHINESE	= 2,	// 繁体中文
	LANGUANE_Italiano	= 3		// 意大利文
};

#define	EMULATORPATH		"C:/Emulator/"	
#define	EMULATORPATHFILE	"./EmulatorPath"

int UnzipProject(const std::string &sWmtPath, std::string &sPrjPath, std::string &sPrjDir);
int UnTgzProject(const std::string &sWmtPath, const std::string &sKey, std::string &sPrjPath, std::string &sPrjDir);
#endif //_HMIEMULATOR_CPP
#endif //WIN32