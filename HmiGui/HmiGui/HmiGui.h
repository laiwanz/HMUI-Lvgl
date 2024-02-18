#ifndef _UIEXPORT_H_20201022
#define _UIEXPORT_H_20201022
#include "ProtoXML/ProtoXML.h"

#include "../EventMgr/EventMgr.h"
#include <string>
#include "macrosplatform/macrosplatform.h"
// #include "SysSetGui/include/syssetgui_global.h"

class DLL_OUT HmiGui {
public:
	static void SetSendFunc(FUNCTION_SENDEVENT func);
	static void doHmiEventQuickly(hmiproto::hmiact& act);
	static int  runGui(int argc, char* argv[], const std::string& strImgPath, void* pPicList);
	static int  runMsg(int argc, char* argv[], const std::string& strMsgText);
	// static int  setCommuConfig(const COMMU_MGR_CONFIG_T &config);
#ifndef _LINUX_ 
	static void GetScale(double &dWidthScale, double &dHeightScale); //获取模拟上的缩放比例
	static void	GetScaleWH(int &nWidth, int &nHeight);				 //获取模拟的宽高
#endif	   
private:   
	static void setScale(int nWidth, int nHeight, int argc, char* argv[]);
private:
	static void	*m_pHmiMgr;
};
#endif