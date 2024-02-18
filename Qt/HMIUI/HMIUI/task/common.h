#ifndef _COMMON_20230228_
#define _COMMON_20230228_
#include "cbl/cbl.h"
#ifdef WIN32
std::string getIMGPath();
void removeRecordDir();
void *RunScript_OnLine(void *arg);
void MessageBoxW_T(int nType, int nLanguage);
void openConsole();
#endif
int getProjectPath(std::string &sPath);
void* loadAllIMGS();
int getLanguageId(const std::string &sPath, const std::string &sKey, unsigned short &nLanguage);
void makeRecordDir();
void runSystemTimer();
void setInitialScreenId();
void *runTriggerActionScript(void *arg);
void *runTimerActionScript(void *);
void *runBackgroundLuaScript(void *arg);
void *runDataRecordCollectionTask(void *arg);
int runAddrmap();
void *runDataRecordStorageTask(void *arg);
void *runRoutineTask(void *arg);
void *runOtherTask(void *arg);
void *runLuaBnsScriptTask(void *arg);
void exitProject();
int preloadProject();
#endif // _COMMON_20230228_