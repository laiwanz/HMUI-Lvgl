#include "PluginDLL.h"
#include "../EventDispatcher/EventDispatch.h"
#include "Plugin/Plugin.h"
#include "hmiScreens/hmiScreens.h"

UTILITY_NAMESPACE::CDLL         CPluginDLL::m_dll;
PluginMailMessage   CPluginDLL::m_MailMsg = { 0 };
PluginAutoClick     CPluginDLL::m_AutoClick = { 0 };
PluginShortMessage  CPluginDLL::m_ShortMsg = { 0 };
PLUGINAUDIOMODULE_T   CPluginDLL::m_AudioModule;
PLUGINIPCAMERAMODULE_T CPluginDLL::m_IPCameraModule;

bool isSubScreen(int nScreenId) {

	return CHMIScreens::get_instance().isSubscreen(nScreenId);
}


int getRunningSunscreen(int nArray[], int nLen) {

	std::list<int> listScreens;
	CHMIScreens::get_instance().getAllRunningSubscreen(QTCLIENT_T, listScreens);
	int		nSize = mymin((int) listScreens.size(), nLen), nI = 0;

	for (auto &iter : listScreens) {

		if (nI < nSize) {

			nArray[nI] = iter;
		}
	}
	return 0;
}

bool CPluginDLL::Load(const std::string &sPath) {
	bool		bRet = false;
	if (m_dll.IsLoad()) {
		return true;
	}
	bRet = m_dll.LoadDllLibrary(sPath);
	return bRet;
}

bool CPluginDLL::IsLoad() {
	return m_dll.IsLoad();
}

UTILITY_NAMESPACE::CDLL	&CPluginDLL::GetDLL() {
	return m_dll;
}

PluginMailMessage &CPluginDLL::GetMailFunc() {
	return	m_MailMsg;
}

PluginAutoClick &CPluginDLL::GetClickFunc() {
	return m_AutoClick;
}

PluginShortMessage &CPluginDLL::GetMsgFunc() {
	return m_ShortMsg;
}

PLUGINAUDIOMODULE_T  &CPluginDLL::GetAudioModuleFunc() {

	return m_AudioModule;
}

PLUGINIPCAMERAMODULE_T & CPluginDLL::GetIPCameraModuleFunc() {

	return m_IPCameraModule;
}
