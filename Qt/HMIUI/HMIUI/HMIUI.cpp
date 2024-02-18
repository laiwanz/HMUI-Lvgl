#include <iostream>
#include "cbl/cbl.h"
#include "./task/task.h"
#include "./task/common.h"
#include "macrosplatform/calculateRunTime.h"
#include "utility/utility.h"
#include "HmiGui/HmiGui/HmiGui.h"
#include "GlobalModule/GlobalModule.h"
#include "EventDispatcher/EventDispatch.h"
#include "CSlaveModule/CSlaveModule.h"
#include "SlaveMode/SlaveMode.h"

#ifdef WIN32
CWSA wsa;
#endif

int g_agrc = 0;
char** g_agrv = { 0 };
unsigned long long g_ullRunTime[2] = {0, 0};
#ifdef WIN32
static void ParamToUtf8(int argc, wchar_t *argvw[], char *argv[10]) {
	char szUtf8[MAX_PATH] = { 0 };
	for (int nI = 0; nI < argc; nI++) {
		memset(szUtf8, 0, MAX_PATH);
		if (nI >= 10) {
			break;
		}
		UTILITY_NAMESPACE::CUnicode::utf16_to_utf8((char *) argvw[nI], szUtf8, MAX_PATH);
		argv[nI] = (char *) malloc(strlen(szUtf8) + 1);
		memset(argv[nI], 0, strlen(szUtf8) + 1);
		strncpy(argv[nI], szUtf8, strlen(szUtf8));
	}
}
#endif

#if ((defined WIN32) && (!defined _DEBUG))//软件模拟时通过wmain接收上位机传下来的宽字符参数，再转utf8，避免因代码页与电脑编码不一致时，导致软件模拟不起来的问题
int wmain(int argc, wchar_t *argvw[], wchar_t *env[])
#else
int main(int argc, char *argv[])
#endif
{
#if ((defined WIN32) && (!defined _DEBUG))
	char *argv[10] = { 0 };
	ParamToUtf8(argc, argvw, argv);//将宽字符参数转为utf8
#endif
	SET_TIME_MAIN;
	g_agrc = argc;
	g_agrv = argv;
	preloadProject();

#ifdef WIN32
	if (strcmp(argv[argc - 1], "slave") == 0) {

#else
	if (CSlaveModule::get_instance().isSlaveMode()) {
#endif // WIN32

		CSlaveModule::get_instance().setSlaveMode(true);
#ifdef WIN32
		g_agrc -= 1;
#endif // WIN32
		std::string sPath;
		getProjectPath(sPath);
		std::string		sProjectDir = getPrjDir();
		CSlaveModule::get_instance().start(redisSlaveRecvFunc, redisSlaveCloseFunc);
		UTILITY::CUtiThreadpool::get_instance().commit((int)(&runConnectToMasterHmi), runConnectToMasterHmi, nullptr);
		loadMasterResource();

		CGraphic::get_instance().load(sPath, IsProjSecure());
		HmiGui::SetSendFunc(std::bind((int(CEventDispatch:: *)(const EVENT_T&))&CEventDispatch::postEvent, &CEventDispatch::getInstance(), placeholders::_1));
		CEventDispatch::getInstance().start();
	} 
	else {

		CSlaveModule::get_instance().setSlaveMode(false);
		CSlaveModule::get_instance().start(redisMasterRecvFunc, redisMasterCloseFunc);
		UTILITY::CUtiThreadpool::get_instance().commit((int)(&runMasterCheckSlavePing), runMasterCheckSlavePing, nullptr);
	}
	CTask	task;
	return task.exec(g_agrc, g_agrv);
}