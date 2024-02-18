#include "webserverCallback.h"
#include <cbl/cbl.h>
#include "webserver/webserver.h"
#include "macrosplatform/path.h"
#include "HmiBase/HmiBase.h"
#include "utility/utility.h"
#include "../EventDispatcher/EventDispatch.h"
#include "platform/platform.h"
#include "macrosplatform/registerAddr.h"
#include "RegisterOpt/RegisterOpt.h"
#include "../cloud/cloud.h"
#ifdef WIN32
#include "../Emulator/WECDecryptFile.h"
#endif
#include "CSlaveModule/CSlaveModule.h"

using namespace UTILITY_NAMESPACE;
using namespace PLATFORM_NAMESPACE;
static time_t	m_time;
#ifdef WIN32
static bool decryptImg(const std::string &sImgDir) {
	cbl::listDir64(sImgDir, true, [](const string& sFilePath, const struct __stat64& statFile) {
		if (_S_IFREG & statFile.st_mode) {
			std::string sSuffix = cbl::suffixName(sFilePath);
			if (sSuffix == ".svg") {
				char szTemp[MAX_PATH] = { 0 };
				char szTemp_mbs[MAX_PATH] = { 0 };
				char szFile_mbs[MAX_PATH] = { 0 };

				UTILITY_NAMESPACE::CState::GetTempPathCos(MAX_PATH, szTemp);
				strcat(szTemp, "_temp.svg");
				string sFile = cbl::dirName(sFilePath) + '/' + cbl::baseName(sFilePath);
				UTILITY_NAMESPACE::CUnicode::utf8_to_mbs(sFile.c_str(), szFile_mbs, MAX_PATH);
				UTILITY_NAMESPACE::CUnicode::utf8_to_mbs(szTemp, szTemp_mbs, MAX_PATH);
				DES_DecryptVector(szFile_mbs, "we_con_svg_key", szTemp_mbs);
				CopyFileA(szTemp_mbs, szFile_mbs, false);
			}
		}
		return 0;
	});
	return true;
}
#endif

static int doFileEvent(int nClientId, const std::string &sData) {
	int nRtn = 0;
	Json::Value root;
	if ((nRtn = cbl::loadJsonString(sData, root)) < 0) {
		return -1;
	}
	std::string sName = "";
	unsigned int nTotalChunks = 0, nCurrentChunk = 0;
	std::string sChunk = "", sBase64 = "";
	std::string sFilePath = "";
	int fd = -1;
	do 
	{
		if (root.isMember("name") &&
			root["name"].isString()) {
			sName = root["name"].asString();
		}
		if (root.isMember("chunk") &&
			root["chunk"].isString()) {
			sChunk = root["chunk"].asString();
		}
		if (root.isMember("totalChunk") &&
			root["totalChunk"].isUInt()) {
			nTotalChunks = root["totalChunk"].asUInt();
		}
		if (root.isMember("currentChunk") &&
			root["currentChunk"].isUInt()) {
			nCurrentChunk = root["currentChunk"].asUInt();
		}
#ifdef WIN32
		CState::GetModuleFilePath(sFilePath);
		sFilePath += std::string("web_root/formula/") + sName;
#else
		sFilePath = std::string("/tmp/web/formula/") + sName;
#endif
		if (nCurrentChunk == 1) {
			CFile::Delete(sFilePath);
		}
		cbl::CBase64 base64;
		if ((nRtn = base64.decode(sChunk, sBase64)) < 0) {
			printf("%s:base64 decode error:%d\r\n", __FUNCTION__, nRtn);
			nRtn = -2;
			break;
		}
		if ((nRtn = cbl::locateFile64(sFilePath, true, true, 0, SEEK_END, fd)) < 0) {
			printf("%s:locate file error:%d\r\n", __FUNCTION__, nRtn);
			nRtn = -3;
			break;
		}
		if ((nRtn = cbl::writeToFile(sBase64, fd)) < 0) {
			printf("%s:write to file error:%d\r\n", __FUNCTION__, nRtn);
			nRtn = -4;
			break;
		}
		if (nTotalChunks == nCurrentChunk) {
			std::string sbuffer = "";
			Json::Value jsonValue;
			jsonValue["name"] = sName;
			jsonValue["flag"] = true;
			if ((nRtn = saveJsonString(jsonValue, sbuffer)) < 0) {
				printf("%s:saveJsonString error:%d\r\n", __FUNCTION__, nRtn);
				nRtn = -5;
				break;
			}
			if ((nRtn = CWebServer::get_instance().sendText(nClientId, sbuffer)) < 0) {
				printf("%s:sendText error:%d\r\n", __FUNCTION__, nRtn);
				nRtn = -6;
				break;
			}
		}
	} while (0);
	if (fd >= 0) {
		close(fd);
		fd = 0;
	}
	return nRtn;
}

static int doHMIEventBase(int nClientId, hmiproto::hmievent & eventP) {

	int nEventType = eventP.type();
	int nScreenNo = eventP.scrno();
	// 初上线
	if (nEventType == EVENT_SCRINIT && -1 == nScreenNo) {
		// 访问限制
		unsigned int nLimit = CRegisterRW::readWord(REG_TYPE_HS_, HSW_CLIENTVISITLIMITTYPE);
		if (nLimit <= REMOTE_OFF) {
			std::list<int> nIdSet;
			nIdSet.push_back(nClientId);
			if (0 == disconnectWebClient(nIdSet, nLimit)) {
				return -2;
			}
		}
	}
	// 事件过滤
	if (nEventType < EVENT_CLICKDOWN ||
		nEventType > EVENT_JSON) {
		return -3;
	}
	EVENT_T hmiEvent;
	hmiEvent.nScreenID = nScreenNo;
	hmiEvent.nType = nEventType;
	hmiEvent.sPartName = eventP.partname();
	hmiEvent.sEventbuffer = eventP.eventbuffer();
	hmiEvent.nClientId = nClientId;
	return CEventDispatch::getInstance().postEvent(hmiEvent);
}

static int doHMIEvent(int nClientId, const std::string &sData) {
	hmiproto::hmievent eventP;
	// 序列化失败
	if (!eventP.ParseFromString(sData)) {
		return -1;
	}
	
	return doHMIEventBase(nClientId, eventP);
}

int startWebServer() {
	if (false == ProjectCreatTime(m_time)) {
		return -1;
	}
	std::string sSiteRoot, sBinPath;
	std::string sImgPath, sFontPath, sFontDir, sSlaveImgPath;
#ifdef WIN32
	// 模拟非debug模式关闭
#ifndef _DEBUG
	return 0;
#endif
	char	szPath[128] = { 0 };
	CState::GetTempPathCos(MAX_PATH, szPath);
	sBinPath = cbl::formatPath(cbl::format("%s/HMIEmulation", szPath));
	CState::GetModuleFilePath(sSiteRoot);
	sSiteRoot += "web_root";
	sSiteRoot = cbl::formatPath(sSiteRoot);
	cbl::listDir64(sSiteRoot, false, [ ] (const string& sPath, const struct __stat64& statFile) {	
		if (!(_S_IFDIR & statFile.st_mode)) {
			return -1;
		}
		if (REGEX_SEARCH(sPath, REGEX("font", REGEX::icase)) ||
			REGEX_SEARCH(sPath, REGEX("img", REGEX::icase))) {
			cbl::removeDir(sPath);
		}
		return 0;
	});
	sImgPath = cbl::format("%s/img%ld", sSiteRoot.c_str(), m_time);
	sFontPath = cbl::format("%s/font%ld", sSiteRoot.c_str(), m_time);
	cbl::makeDir(sImgPath);
	cbl::makeDir(sFontPath);
	std::string sImage = sBinPath + "/img";
	std::string sFont = sBinPath + "/qpf2";
	CDir::CopyFolder(sImage, sImgPath, "*.*", false);
	CDir::CopyFolder(sFont, sFontPath, "*.*", false);
	decryptImg(sImgPath);
#else
	sSiteRoot = "/tmp/web";
	sSlaveImgPath = sSiteRoot + "/slaveImg";
	sBinPath = "/mnt/wecon/app/hmiapp/bin/web_root/*";
	sImgPath = cbl::format("%s/img%ld", sSiteRoot.c_str(), m_time);
	sFontPath = cbl::format("%s/font%ld", sSiteRoot.c_str(), m_time);
	cbl::makeDir(sSiteRoot);
	cbl::makeDir(sFontPath);

	std::string str = cbl::format("ln -s /mnt/wecon/wecon/run/project/img %s", sImgPath.c_str());
	CPlatShell::runCmd(str, NULL);
	str = cbl::format("ln -s /mnt/wecon/wecon/run/project/qpf2/* %s", sFontPath.c_str());
	CPlatShell::runCmd(str, NULL);
	str = cbl::format("ln -s %s %s", sBinPath.c_str(), sSiteRoot.c_str());
	CPlatShell::runCmd(str, NULL);
	str = cbl::format("ln -s /mnt/wecon/wecon/run/project/img %s", sSlaveImgPath.c_str());
	CPlatShell::runCmd(str, NULL);
#endif
	WEBSERVERFUNCSET_T	funSet;
	funSet.isUrlValid = [ ] (const std::string &sToken, const std::string &sSignature) {
		if (CCloudOperator::get_instance().verifyUrl(sToken, sSignature) == 0) {
			return true;
		}
		return false;
	};
	funSet.recv = [ ] (int nClientId, const std::string &sData, int nListenPort) {
		if (nListenPort == 9090) {
			return doFileEvent(nClientId, sData);
		}
		return doHMIEvent(nClientId, sData);
	};
	funSet.offline = [ ] (int nClientId) {
		EVENT_T Event;
		Event.nScreenID = -2;
		Event.nType = EVENT_WEBQUIT;
		Event.nClientId = nClientId;
		return CEventDispatch::getInstance().postEvent(Event);
	};
	bool bFluoriteCloud = false;
	string sYingshiJs = "";
	string sUseYingshiFile = format("%syingshi.ini", PROJECT_FILE_PATH);
	if (CFile::ExistFile(sUseYingshiFile)) {
		bFluoriteCloud = true;
	}
	return CWebServer::get_instance().start(sSiteRoot, cbl::format("font%ld", m_time), funSet, bFluoriteCloud);;
}

int sendtoWebClient(hmiproto::hmiact &act, bool bBroadcast) {
	std::string		sData;
	int nClientId = act.event().clientid();
	act.set_id(m_time);
	act.SerializeToString(&sData);
	return CWebServer::get_instance().sendBinary(nClientId, sData, bBroadcast);
}

int disconnectWebClient(const std::list<int> &sIdSet, unsigned int nLimit) {
	int			nretCode = -1;
	for (auto &iter : sIdSet) {
		int nId = iter;
		if (nId == QTCLIENT_T) {
			continue;
		}
		hmiproto::hmiact hmiact;
		hmiproto::hmievent* phmievent = hmiact.mutable_event();
		std::string sSerialize;
		phmievent->set_clientid(nId);
		phmievent->set_type(HMIEVENT_JUMPHTML);
		if (nLimit == REMOTE_OFF) {
			phmievent->set_html(CWebServer::get_instance().getHtml(REMOTE_OFF));
		}
		else {
			bool bLocalAreaNetwork = CWebServer::get_instance().isLocalAreaNetwork(nId);
			if (nLimit == REMOTE_ONLY_LAN &&
				!bLocalAreaNetwork) {
				// 关掉外网
				phmievent->set_html(CWebServer::get_instance().getHtml(REMOTE_ONLY_LAN));
			}
			else if (nLimit == REMOTE_ONLY_WLAN &&
				bLocalAreaNetwork) {
				// 关掉局域网
				phmievent->set_html(CWebServer::get_instance().getHtml(REMOTE_ONLY_WLAN));
			}
		}
		if (phmievent->has_html()) {
			sendtoWebClient(hmiact, false);
			nretCode = 0;
		}
	}
	return nretCode;
}

int doRemoteHmiEvent(hmiproto::hmievent &event)
{
	return doHMIEventBase(event.clientid(), event);
}