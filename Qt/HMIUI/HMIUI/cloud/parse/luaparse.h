#ifndef _LUAPARSE_20230424_
#define _LUAPARSE_20230424_
#include <iostream>
#include "utility/utility.h"
#include "Register/Register.h"
#include "LuaScript/LuaScript.h"
#include "../ccommon/ccommon.h"

typedef enum {
	LUAOPT_NONE = 0,
	LUAOPT_ADD = 1,		// 增加
	LUAOPT_UPDATE = 2,	// 修改
	LUAOPT_DELETE = 3,	// 删除
	LUAOPT_STOP = 4,	// 禁用
	LUAOPT_START = 5,	// 启用
	LUAOPT_OTHER,		// 其他
} LUAOPTENUM_T;

class CLuaPrase 
{
public :
	CLuaPrase();
	~CLuaPrase();
	int config(bool isDb, const std::string &sThirdDir = "", const std::list<std::string> &listPreloadPath = std::list<std::string>());
	int loadDb(std::list<LUASCRIPT_T> &scripts);
	int clearDb();
	int insert(const LUASCRIPT_T &script);
	int del(int nId);
	int start(const std::list<LUASCRIPT_T> &scripts, bool bInit = true);
	int stop();
	int restart(const std::list<LUASCRIPT_T> &scripts);
private:
	void openDb();
	int compile(const std::list<LUASCRIPT_T> &scripts);
	int parse(const std::list<LUASCRIPT_T> &scripts, bool bInit = true);
	int insertDb(const LUASCRIPT_T &script);
	int delDb(int nId);
private:
	CRegister m_addrNetFlag;
	UTILITY_NAMESPACE::CppSQLite3DB m_handle;
	bool m_bHasHandle;
	bool m_bIsDb;
	std::string m_sThirdDir;
	std::list<std::string> m_listPreloadPath;
};
#endif // _LUAPARSE_20230424_