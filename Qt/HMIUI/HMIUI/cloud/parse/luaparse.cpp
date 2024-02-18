#include "luaparse.h"
#include "HmiBase/HmiBase.h"
#include "macrosplatform/registerAddr.h"
#include "../../LuaCallback/LuaCallback.h"

#define LUA_SCRIPT_TABLE		"LuaScriptTable"	// LUA脚本表名称
#define LUA_SCRIPT_ID			"IndexID"			// LUA脚本序号
#define LUA_SCRIPT_TYPE			"Type"				// LUA脚本类型 0-初始化脚本 1-定时脚本 2-触发脚本 3-联网执行 4-断网执行
#define LUA_SCRIPT_OPTSTAT		"OptStat"			// LUA脚本状态 0-未执行 1-执行 2-暂停 3-出错
#define LUA_SCRIPT_CYCLE		"Cycle"				// 脚本周期
#define LUA_SCRIPT_TIME			"Time"				// LUA脚本更新时间
#define LUA_SCRIPT_MODULE		"Module"			// 脚本模块
#define LUA_SCRIPT_TEXT			"ScriptText"		// 脚本内容
enum {
	LUADB_ID = 0,			// ID
	LUADB_TYPE = 1,			// 类型
	LUADB_CYCLE = 2,		// 周期
	LUADB_OPTSTAT = 3,		// 状态
	LUADB_TIME = 4,			// 时间
	LUADB_NAME = 5,			// 脚本名称
	LUADB_CONTENT = 6		// 脚本内容
};
enum {
	LUATYPE_NONE = 0,
	LUATYPE_INIT = 1,		// 初始化脚本
	LUATYPE_CYCLE = 2,		// 周期脚本
	LUATYPE_TRIGGER = 3,	// 触发脚本
	LUATYPE_CONNECT = 4,	// 联网执行
	LUATYPE_DISCONECT = 5,	// 断网执行
	LUATYPE_BNS = 6			// 云商脚本
};
using namespace UTILITY_NAMESPACE;
CLuaPrase::CLuaPrase() {
	m_bHasHandle = m_bIsDb = false;
	m_addrNetFlag.setRegType(REG_TYPE_HS_);
	m_addrNetFlag.setDataType(DATA_TYPE_WORD);
	m_addrNetFlag.setAddr(HSW_MQTTSTATE);
	m_sThirdDir = "";
	m_listPreloadPath.clear();
}

CLuaPrase::~CLuaPrase() {
	if (m_bHasHandle) {
		m_handle.close();
	}
	m_bHasHandle = false;
	this->stop();
}

int CLuaPrase::config(bool isDb, const std::string &sThirdDir, const std::list<std::string> &listPreloadPath) {
	m_bIsDb = isDb;
	m_sThirdDir = sThirdDir;
	m_listPreloadPath = listPreloadPath;
	return 0;
}

void CLuaPrase::openDb() {
	try {
		std::string sPath = getPrjDir() + std::string("LuaScriptDb.db");
		m_handle.open(sPath.c_str());
		if (!m_handle.tableExists(LUA_SCRIPT_TABLE)) {
			std::string sSQL = cbl::format("create table '%s' ('%s' INTEGER PRIMARY KEY AUTOINCREMENT, '%s' INTEGER, '%s' INTEGER, '%s' INTEGER, '%s' DateTime, '%s' VARCHAR, '%s' VARCHAR);", LUA_SCRIPT_TABLE, LUA_SCRIPT_ID, LUA_SCRIPT_TYPE, LUA_SCRIPT_CYCLE, LUA_SCRIPT_OPTSTAT, LUA_SCRIPT_TIME, LUA_SCRIPT_MODULE, LUA_SCRIPT_TEXT);
			m_handle.execDML(sSQL.c_str());
			m_handle.execDML("PRAGMA auto_vacuum = 1");
		}
		m_bHasHandle = true;
	} catch (CppSQLite3Exception& e) {
		printf("%s:%s\r\n", __FUNCTION__, e.errorMessage());
	}
}

int CLuaPrase::loadDb(std::list<LUASCRIPT_T> &scripts) {
	CppSQLite3Query qRet;
	if (!m_bIsDb) {
		return -1;
	}
	this->openDb();
	if (!m_bHasHandle) {
		return -2;
	}
	try {
		std::string sSQL = cbl::format("select * from %s;", LUA_SCRIPT_TABLE);
		qRet = m_handle.execQuery(sSQL.c_str());
		while (!qRet.eof()) {
			LUASCRIPT_T	script;
			script.nId = qRet.getIntField(LUADB_ID);
			script.nType = qRet.getIntField(LUADB_TYPE);
			script.nCycle = qRet.getIntField(LUADB_CYCLE);
			script.nOptStat = qRet.getIntField(LUADB_OPTSTAT);
			script.sName = qRet.getStringField(LUADB_NAME);
			script.sContent = qRet.getStringField(LUADB_CONTENT);
			if (!script.sName.empty() &&
				!script.sContent.empty()) {
				scripts.push_back(script);
			}
			qRet.nextRow();
		}
		qRet.finalize();
	} catch (CppSQLite3Exception& e) {
		printf("%s:%s\r\n", __FUNCTION__, e.errorMessage());
		return -3;
	}
	return 0;
}

int CLuaPrase::compile(const std::list<LUASCRIPT_T> &scripts) {
	if (scripts.empty()) {
		return 0;
	}
	for (auto &iter : scripts) {
		if (iter.sContent.empty()) {
			continue;
		}
		if (0 > lua_compile(iter)) {
			return -1;
		}
	}
	return 0;
}

int CLuaPrase::parse(const std::list<LUASCRIPT_T> &scripts, bool bInit) {
	int nRtn = 0;
	if (scripts.empty()) {
		return 0;
	}
	RW_PARAM_T	rw = { RW_MODE_CACHE_DEV, 0, 0, -1, REG_SRC_SCRIPT };
	int nValue = 0;
	for (auto &iter : scripts) {
		if (iter.sName.empty() ||
			iter.sContent.empty()) {
			continue;
		}
		switch (iter.nType)
		{
		case LUATYPE_INIT:
			{
				nRtn = creatInitBgScript((LUASCRIPT_T &)iter);
			}
			break;
		case LUATYPE_CYCLE:
			{
				nRtn = createTimerFunc(-1, iter.nId, iter.nCycle, iter.sName);
			}
			break;
		case LUATYPE_CONNECT:
			{
				if (!bInit) {
					CRegisterRW::read(RW_FORMAT_INT, m_addrNetFlag, &nValue, 1, rw);
				}
				nRtn = createTriggerFunc(-1, iter.nId, LUA_TRIGGER_TYPE_EQUAL, m_addrNetFlag, iter.sName, 1, nValue);
			}
			break;
		case LUATYPE_DISCONECT:
			{
				if (!bInit) {
					CRegisterRW::read(RW_FORMAT_INT, m_addrNetFlag, &nValue, 1, rw);
				}
				nRtn = createTriggerFunc(-1, iter.nId, LUA_TRIGGER_TYPE_EQUAL, m_addrNetFlag, iter.sName, 2, nValue);
			}
			break;
		case LUATYPE_BNS:
			{
				nRtn = creatBnsScript((LUASCRIPT_T &)iter);
			}
			break;
		case LUATYPE_NONE:
		case LUATYPE_TRIGGER:
		default:
			break;
		}
		if (nRtn != 0) {
			break;
		}
	}
	return nRtn;
}

int CLuaPrase::start(const std::list<LUASCRIPT_T> &scripts, bool bInit) {
	int nRtn = 0;
	this->compile(scripts);
	if ((nRtn = this->parse(scripts, bInit)) < 0) {
		printf("%s:parse error:%d\r\n", __FUNCTION__, nRtn);
		return -1;
	}
	if ((nRtn = registerLuaFunction() < 0)) {
		printf("%s:registerLuaFunction error:%d\r\n", __FUNCTION__, nRtn);
		return -2;
	}
	if ((nRtn = lua_start(m_sThirdDir, m_listPreloadPath, scripts)) < 0) {
		printf("%s:lua_start error:%d\r\n", __FUNCTION__, nRtn);
		return -3;
	}
	if (bInit) {
		if ((nRtn = runInitBgScript()) < 0) {
			printf("%s:runInitBgScript error:%d\r\n", __FUNCTION__, nRtn);
			return -4;
		}
	}
	return 0;
}

int CLuaPrase::stop() {
	int nRtn = 0;
	if ((nRtn = lua_unload()) < 0) {
		printf("%s:lua_unload error:%d\r\n", __FUNCTION__, nRtn);
		return -1;
	}
	return 0;
}

int CLuaPrase::restart(const std::list<LUASCRIPT_T> &scripts) {
	if (0 > this->stop()) {
		printf("%s, stop error\r\n", __FUNCTION__);
		return -1;
	}
	if (0 > this->start(scripts, false)) {
		printf("%s: start error\r\n", __FUNCTION__);
		return -2;
	}
	return 0;
}

int CLuaPrase::insert(const LUASCRIPT_T &script) {
	int		nRtn = 0;
	std::list<LUASCRIPT_T> scripts;
	if (!m_bIsDb) {
		return -1;
	}
	if ((nRtn = this->insertDb(script)) < 0) {
		printf("%s:insertDb error:%d\r\n", __FUNCTION__, nRtn);
		return -2;
	}
	if ((nRtn = this->loadDb(scripts)) < 0) {
		printf("%s:load db error:%d\r\n", __FUNCTION__, nRtn);
		return -3;
	}
	if ((nRtn = this->restart(scripts)) < 0) {
		printf("%s:restart error:%d\r\n", __FUNCTION__, nRtn);
		return -4;
	}
	return 0;
}

int CLuaPrase::insertDb(const LUASCRIPT_T &script) {
	if (!m_bHasHandle) {
		return -1;
	}
	if (0 > this->delDb(script.nId)) {
		return -2;
	}
	try {
		if (!m_handle.tableExists(LUA_SCRIPT_TABLE)) {
			return -3;
		}
		std::string sContent = cbl::replaceall(script.sContent, "'", "''");
		std::string sSQL = cbl::format("insert into %s values(%d, %d, %d, %d, '', '%s', '%s');", LUA_SCRIPT_TABLE, script.nId, script.nType, \
									   script.nCycle, script.nOptStat, script.sName.c_str(), sContent.c_str());
		m_handle.execDML(sSQL.c_str());
	} catch (CppSQLite3Exception& e) {
		printf("%s:%s\r\n", __FUNCTION__, e.errorMessage());
		return -4;
	}
	return 0;
}

int CLuaPrase::delDb(int nId) {
	CppSQLite3Query query;
	if (!m_bHasHandle) {
		return -1;
	}
	try {
		if (!m_handle.tableExists(LUA_SCRIPT_TABLE)) {
			return -2;
		}
		std::string sSQL = cbl::format("select IndexID from %s where %s = %d;", LUA_SCRIPT_TABLE, LUA_SCRIPT_ID, nId);
		query = m_handle.execQuery(sSQL.c_str());
		if (!query.eof()) {
			sSQL = cbl::format("delete from %s where %s = %d;", LUA_SCRIPT_TABLE, LUA_SCRIPT_ID, nId);
			m_handle.execDML(sSQL.c_str());
		}
		query.finalize();
	} catch (CppSQLite3Exception& e) {
		printf("%s:%s\r\n", __FUNCTION__, e.errorMessage());
		return -3;
	}
	return 0;
}

int CLuaPrase::del(int nId) {
	int nRtn = 0;
	std::list<LUASCRIPT_T> scripts;
	if (!m_bIsDb) {
		return -1;
	}
	if ((nRtn = this->delDb(nId)) < 0) {
		printf("%s:delDb error:%d\r\n", __FUNCTION__, nRtn);
		return -2;
	}
	if ((nRtn = this->loadDb(scripts)) < 0) {
		printf("%s:load db error:%d\r\n", __FUNCTION__, nRtn);
		return -3;
	}
	if ((nRtn = restart(scripts)) < 0) {
		printf("%s:restart error:%d\r\n", __FUNCTION__, nRtn);
		return -4;
	}
	return 0;
}

int CLuaPrase::clearDb() {
	if (0 > this->stop()) {
		printf("%s, stop error\r\n", __FUNCTION__);
		return -1;
	}
	if (m_bIsDb) {
		if (m_bHasHandle) {
			m_handle.close();
			m_bHasHandle = false;
		}
		std::string sPath = getPrjDir() + std::string("LuaScriptDb.db");
		if (cbl::getFileSize(sPath) >= 0) {
			UTILITY_NAMESPACE::CFile::Delete(sPath);
		}
		this->openDb();
	}
	return 0;
}