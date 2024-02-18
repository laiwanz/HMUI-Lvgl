#ifdef WIN32
#include "WsfiFile/UnPackFile.h"
#include "HmiEmulator.h"
#include "WECDecryptFile.h"
#include "utility/utility.h"
#include "macrosplatform/path.h"
#include "HmiBase/HmiBase.h"
#include "cbl/cbl.h"

#define PROJECT_ZIP_WECON_PW  64    //工程通用解密 长度

bool isProjectValid(const std::string &sPrjPath) {
	if (sPrjPath.empty()) {
		return false;
	}
	bool bPrjFileExist = false;
	bool bDevMergBlock = false;
	std::vector<std::string> lFileName;

	if (UTILITY_NAMESPACE::CDir::listDir(sPrjPath, false, "*.*", lFileName) < 0 ||
		lFileName.empty()) {
		return false;
	}

	for (auto &iter : lFileName) {

		if (iter == "test.pi") {
			bPrjFileExist = true;
		}
		else if (iter == "DevMergBlock.xml") {
			bDevMergBlock = true;
		}

		if (bPrjFileExist && bDevMergBlock) {
			return true;
		}
	}

	return false;
}

static bool encryptImg(const std::string &sImgDir) {
	cbl::listDir64(sImgDir, true, [ ] (const string& sFilePath, const struct __stat64& statFile) {
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
				DES_EncryptVector(szFile_mbs, "we_con_svg_key", szTemp_mbs);
				CopyFileA(szTemp_mbs, szFile_mbs, false);
			}
		}
		return 0;
	});

	return true;
}

bool RunExec(const char *cmd, char *para, unsigned int dwMilliseconds) {
	SHELLEXECUTEINFOA ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = cmd;
	ShExecInfo.lpParameters = para;
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL;
	bool suc = UTILITY_NAMESPACE::CState::ShellExecuteEx(&ShExecInfo);
	WaitForSingleObject(ShExecInfo.hProcess, dwMilliseconds);
	return suc;
}

int UnzipProject(const std::string &sWmtPath, std::string &sPrjPath, std::string &sPrjDir) {
	std::string		sPwd = "";

	if (UTILITY_NAMESPACE::CFile::ExistFile(EMULATORPATHFILE)) {
		sPrjDir = EMULATORPATH;
	}
	else {
		char	szPath[MAX_PATH] = { 0 };

		UTILITY_NAMESPACE::CState::GetTempPathCos(MAX_PATH, szPath);
		sPrjDir = szPath;
	}

	if (sPrjDir.empty()) {
		return -4;
	}
	sPrjDir = cbl::formatPath(sPrjDir);
	sPrjDir = sPrjDir + std::string("/") + TMPDIR + std::string("/");
	cbl::removeDir(sPrjDir);
	cbl::makeDir(sPrjDir);
	getProjectPwd(sWmtPath, sPwd);

	std::string sUnzipParam = "";
	if (sPwd.empty()) {
		sUnzipParam = cbl::format("x -tzip \"%s\" -o%s -y", sWmtPath.c_str(), sPrjDir.c_str());
	}
	else {
		sUnzipParam = cbl::format("x -tzip -p%s \"%s\" -o%s -y", sPwd.c_str(), sWmtPath.c_str(), sPrjDir.c_str());
	}

	std::string sExePath = UTILITY_NAMESPACE::CState::GetEnvironmentV("ParentPath");
	if (sExePath.empty()) {
		UTILITY_NAMESPACE::CState::GetModuleFilePath(sExePath);
	}
	sExePath.append("\\7z.exe");
	RunExec(sExePath.c_str(), (char *) sUnzipParam.c_str(), 15000);
	encryptImg(sPrjDir + std::string("img"));
	//检验
	if (!isProjectValid(sPrjDir)) {
		return -5;
	}
	sPrjPath = sPrjDir + std::string("/test.pi");
	return 0;
}

int UnTgzProject(const std::string &sWmtPath, const std::string &sKey, std::string &sPrjPath, std::string &sPrjDir) {
	if (UTILITY_NAMESPACE::CFile::ExistFile(EMULATORPATHFILE)) {
		sPrjDir = EMULATORPATH;
	}
	else {
		char	szPath[MAX_PATH] = { 0 };

		UTILITY_NAMESPACE::CState::GetTempPathCos(MAX_PATH, szPath);
		sPrjDir = szPath;
	}

	if (sPrjDir.empty()) {
		return -4;
	}
	sPrjDir = cbl::formatPath(sPrjDir);
	sPrjDir = sPrjDir + std::string("/") + TMPDIR + std::string("/");
	cbl::removeDir(sPrjDir);
	cbl::makeDir(sPrjDir);
	{
		bool		bTgzRet = false;
		CUnPackFile UnPackFile;
		wstring		wstrOutDir;
		wstring		wstrFile;
		
		wchar_t		wsTmpPath[MAX_PATH] = { 0 };
		wchar_t		szWsfiFile[MAX_PATH] = { 0 };
		int			nRet = 0;
		strconv_t	strconv;

		wcscpy(wsTmpPath, strconv.utf8_to_wchar(sPrjDir.c_str()));
		wcscpy(szWsfiFile, strconv.utf8_to_wchar(sWmtPath.c_str()));
		wstrOutDir.append(wsTmpPath);
		wstrFile.append(szWsfiFile);
		
		std::string strkey = sKey;
#if ((defined WIN32) && (!defined _DEBUG))
		wstring		wstr;
		wstr.append(strconv.utf8_to_wchar(sKey.c_str()));
		strkey = cbl::wstring2string(wstr);
#endif
		nRet = UnPackFile.loadTgzFile(wstrFile);	//加载文件
		if (nRet < 0) {
			bTgzRet = false;
		}
		if (UnPackFile.checkInputFile() &&
			UnPackFile.checkUserKey(sKey) == 0) {
			UnPackFile.setOutDir(wstrOutDir);
			if (UnPackFile.wsfiToFile(wstrFile) < 0) {
				bTgzRet = false;
			}
			else {
				bTgzRet = true;
			}
		}
		else {
			bTgzRet = false;
		}

		if (bTgzRet == false) {
			return -5;
		}
	}
	encryptImg(sPrjDir + std::string("img"));
	//检验
	if (!isProjectValid(sPrjDir)) {
		return -5;
	}
	sPrjPath = sPrjDir + std::string("/test.pi");
	return 0;
}
#endif //WIN32