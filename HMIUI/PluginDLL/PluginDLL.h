#ifndef _PLUSINDLL_H_20201207_
#define _PLUSINDLL_H_20201207_
#include <iostream>
#include <functional>
#include <utility/utility.h>

//自动点击
typedef int(*AC_Play)( unsigned short wScreenID );
typedef unsigned short(*AC_RWReg)( unsigned short wWrite );
typedef bool(*AC_Record)( const char *pBuf, int nScreenID, int nType );
typedef void(*AC_SetMac)( unsigned short wMac[6] );
typedef struct plugAutoClick {
	bool         bLoad;
	AC_Play      Play;
	AC_RWReg     RWReg;
	AC_Record    Record;
	AC_SetMac    SetMac;
}PluginAutoClick;

//邮件动态加载
typedef bool(*MM_Init)( const std::string &strFilePath );
typedef bool(*MM_DeInit)( );
typedef void(*MM_Collect)( );
typedef struct plugMailMessage {
	bool         bRun;
	MM_Init      Init;
	MM_DeInit    DeInit;
	MM_Collect   Collect;
}PluginMailMessage;

//ShortMessage模块
typedef bool(*SM_Init)( const std::string &strFilePath );
typedef bool(*SM_DeInit)( );
typedef void(*SM_Collect)( );
typedef struct plugShortMessage {
	bool         bRun;
	SM_Init      Init;
	SM_DeInit    DeInit;
	SM_Collect   Collect;
}PluginShortMessage;

//AudioModule模块
typedef bool(*AM_INIT)( const std::string &sXmlPath );
using AM_DESTROY =
std::function<bool()>;
using AM_COLLECT =
std::function<void()>;
typedef struct tagPLUGAUDIOMODULE {
	bool         bRun;
	AM_INIT      init;
	AM_DESTROY   destroy;
	AM_COLLECT   collect;

	tagPLUGAUDIOMODULE() {

		bRun = false;
		init = nullptr;
		destroy = nullptr;
		collect = nullptr;
	}
} PLUGINAUDIOMODULE_T;

// IPCameraModule模块
using IPCM_INIT =
std::function<bool()>;
using IPCM_DESTROY =
std::function<bool()>;
using IPCM_COLLECT =
std::function<void()>;
typedef struct tagPLUGIPCAMERAMODULE {
	bool			bRun;
	IPCM_INIT		init;
	IPCM_DESTROY	destroy;
	IPCM_COLLECT	collect;

	tagPLUGIPCAMERAMODULE() {

		bRun = false;
		init = nullptr;
		destroy = nullptr;
		collect = nullptr;
	}
} PLUGINIPCAMERAMODULE_T;

class CPluginDLL {
public:
	static bool	Load(const std::string &sPath);
	static bool	IsLoad();
	static UTILITY_NAMESPACE::CDLL	&GetDLL();
	static PluginMailMessage    &GetMailFunc();
	static PluginAutoClick	    &GetClickFunc();
	static PluginShortMessage   &GetMsgFunc();
	static PLUGINAUDIOMODULE_T    &GetAudioModuleFunc();
	static PLUGINIPCAMERAMODULE_T &GetIPCameraModuleFunc();
private:
	static UTILITY_NAMESPACE::CDLL		m_dll;
	static PluginMailMessage			m_MailMsg;
	static PluginAutoClick				m_AutoClick;
	static PluginShortMessage			m_ShortMsg;
	static PLUGINAUDIOMODULE_T			m_AudioModule;
	static PLUGINIPCAMERAMODULE_T		m_IPCameraModule;
};
#endif // _PLUSINDLL_H_20201207_