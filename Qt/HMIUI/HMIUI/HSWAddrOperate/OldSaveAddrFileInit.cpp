#include "HSWAddrOperate.h"
#include "GlobalModule/GlobalModule.h"
#include <utility/utility.h>
#include "macrosplatform/path.h"


#define HAW_RegPageLen			1024				//2K	每个页的大小2K  既1024个字地址
#define HAW_FileWord			19456				//1024*19 每个文件保存 19456个字地址
#define SaveRegVersion			1					//二进制保存的钓点保存数据的版本为1
#define SAVE_RegAllFile			HAW_MAX_SIZE_High/HAW_FileWord+1					//掉电保存文件数量
#define SAVE_RegAllPage			HAW_FileWord/HAW_RegPageLen+1						//掉电保存文件中的页数量
#define SAVE_RPWRegAllFile		RPW_MAX_SIZE_High/HAW_FileWord+1					//RPW掉电保存文件数量

#define HMISaveRegFile_Tmp		"FileTmp"		//掉电保存的临时文件（每次存储都先保存临时文件）
#define HMISaveRegFile_RPWTmp	"RpwFileTmp"	//RPW掉电保存的临时文件（每次存储都先保存临时文件）

//掉电保存的备份文件的版本信息
typedef struct _tagHawRegVersionTmp {
	uint16_t	uVersion;		//版本号
	unsigned long	ulVerify;		//校验和
	uint16_t	uPage[SAVE_RegAllPage];		//改变的页（默认值为100）
	uint16_t	uFileID;		//对应的文件 
	uint16_t	uFileType;		//主文件类型 1是HAW 2是HSW 4是RPW
	uint16_t	uValue[5];		//预留
} HawRegVersionTmp;

enum {
	ByteFile_HAW = 1,//HAW文件
	ByteFile_HSW = 2,//HSW文件
	ByteFile_RPW = 4,//RPW文件
};

using namespace UTILITY_NAMESPACE;

static unsigned long	GetSavePageLen(uint16_t wIndex, uint32_t ulMaxSaveSize)
{
	unsigned long ulReadNum = 0;
	uint32_t ulCurPageLen = (wIndex + 1)*HAW_RegPageLen;

	if (ulCurPageLen > ulMaxSaveSize)
	{
		ulReadNum = ulMaxSaveSize % HAW_RegPageLen;
	}
	else
	{
		ulReadNum = HAW_RegPageLen;
	}
	
	return ulReadNum;
}

static unsigned long	GetSaveFileLen(uint16_t wIndex, uint32_t ulMaxSaveSize)
{
	unsigned long ulReadNum = 0;
	uint32_t ulCurFileLen = (wIndex + 1)*HAW_FileWord;

	if (ulCurFileLen > ulMaxSaveSize)
	{
		ulReadNum = ulMaxSaveSize % HAW_FileWord;
	}
	else
	{
		ulReadNum = HAW_FileWord;
	}

	return ulReadNum;
}

/*************************************************
*作  者：LYQ	20170103
*功  能：读取掉电保存的寄存器数据（包括HAW、HSW的掉电保存部分）
*参	 数：	FILE *fp		文件指针
*			BYTE *pData		读取文件的存放位置
*			DWORD dwReadNum	读取的长度  word个数
*返回值：无
*************************************************/
static bool RegReadFile(FILE *fp, unsigned char *pData, unsigned long ulReadNum)//读文件
{
	unsigned long	ulReadSize = 0;
	unsigned long	ulNeedReadSize = ulReadNum * sizeof(uint16_t) / sizeof(unsigned char);
	unsigned long	ulFilelen = 0;	//文件长度

	if (!fp || NULL == pData)
	{
		return false;
	}

	fseek(fp, 0, SEEK_END);
	ulFilelen = ftell(fp);
	if (ulFilelen < ulNeedReadSize)//文件比要读取的数据小
	{
		fclose(fp);
		return false;
	}

	fseek(fp, 0, SEEK_SET);//将文件指针重新移到文件头
	while (ulReadSize < ulNeedReadSize)
	{
		long	lRead = 0;
		lRead = fread(pData, 1, ulNeedReadSize - ulReadSize, fp);//把校验正常的  文件数据直接拿来用
		if (lRead < 0)
		{
			fclose(fp);
			return false;
		}
		ulReadSize += lRead;
	}
	fclose(fp);
	fp = nullptr;

	return true;
}
/*************************************************
*作  者：LYQ	20170103
*功  能：读取掉电保存的寄存器数据（包括HAW、HSW的掉电保存部分）
*参	 数：无
*返回值：无
*************************************************/
static int  loadRegFile(string sSaveFilePath, uint32_t ulSaveSize, uint16_t *pData)
{
	string				sSaveFilename;
	unsigned short		wIndex = 0;
	unsigned long		dwReadNum = 0;
	int					nLoad = -1;

	if (sSaveFilePath.empty() || nullptr == pData) {
		return -1;
	}

	for (wIndex = 0; wIndex <= (ulSaveSize / HAW_FileWord); wIndex++)//轮询文件编号
	{
		uint16_t *pDataTmp = nullptr;
		pDataTmp = pData + wIndex * HAW_FileWord;

		dwReadNum = GetSaveFileLen(wIndex, ulSaveSize);//第wIndex个文件的寄存器数量
		sSaveFilename = format("%s%d", sSaveFilePath.c_str(), wIndex);

		if (CFile::ExistFile(sSaveFilename) == false)//文件不存在
		{
			continue;
		}

		FILE *fp = CFile::Open(sSaveFilename, "rb+");
		if (!fp)
		{
			continue;
		}
		else
		{
			if (RegReadFile(fp, (unsigned char *)pDataTmp, dwReadNum) == false)
			{
				fclose(fp);
				fp = nullptr;
				CFile::Delete(sSaveFilename);
				continue;
			}

			nLoad = 0;
		}
	}

	return nLoad;
}

static int loadRegTmpFile(string sTempFilePath, uint32_t ulSaveSize, uint16_t *pData, uint16_t uFileType)
{
	HawRegVersionTmp	HawVersionTmp = { 0 };
	uint16_t			*pFileData = nullptr;
	FILE *				fp = nullptr;
	unsigned long		ulReadNum = 0;
	int					nLoad = -1;

	if (sTempFilePath.empty() || nullptr == pData) {
		return -1;
	}

	pFileData = (uint16_t *)malloc(HAW_FileWord * sizeof(uint16_t));
	if (nullptr == pFileData)
	{
		return -2;
	}

	if (CFile::ExistFile(sTempFilePath) == false)//文件不存在
	{
		free(pFileData);
		return -3;
	}

	fp = CFile::Open(sTempFilePath, "rb+");
	if (!fp)
	{
		free(pFileData);
		CFile::Delete(sTempFilePath);
		return -4;
	}
	unsigned long	ulVerify = 0;			//校验和的值
	long	lfilelen = 0;					//文件长度

	fseek(fp, 0, SEEK_END);
	lfilelen = ftell(fp);
	fseek(fp, lfilelen - sizeof(HawVersionTmp), SEEK_SET);				//偏移到
	fread((unsigned char *)&HawVersionTmp, 1, sizeof(HawVersionTmp), fp);		//读取数据
	if (HawVersionTmp.uFileType != uFileType)//文件类型
	{
		free(pFileData);
		fclose(fp);
		return -5;
	}
	if (HawVersionTmp.uVersion != SaveRegVersion) 						//版本号
	{
		free(pFileData);
		fclose(fp);
		CFile::Delete(sTempFilePath);
		return -6;
	}
	//计算校验值
	unsigned long	dwRead = (lfilelen - sizeof(HawVersionTmp)) / sizeof(uint16_t);
	if (RegReadFile(fp, (unsigned char *)pFileData, dwRead) == false)
	{
		free(pFileData);
		fclose(fp);
		CFile::Delete(sTempFilePath);
		return -7;
	}
	fclose(fp);
	fp = NULL;

	for (unsigned long n = 0; n < dwRead; n++)
	{
		ulVerify += pFileData[n];
	}
	if (HawVersionTmp.ulVerify != ulVerify)
	{
		free(pFileData);
		CFile::Delete(sTempFilePath);
		return -8;
	}

	for (uint16_t wTmp = 0; wTmp <= HAW_FileWord / HAW_RegPageLen; wTmp++)//页编号
	{
		if (HawVersionTmp.uPage[wTmp] == 1000)
		{
			break;
		}
		ulReadNum = GetSavePageLen(HawVersionTmp.uFileID *(HAW_FileWord / HAW_RegPageLen) + HawVersionTmp.uPage[wTmp], ulSaveSize);

		uint16_t *pDataTmp = nullptr;
		pDataTmp = pData + HawVersionTmp.uFileID *HAW_FileWord + HawVersionTmp.uPage[wTmp] * HAW_RegPageLen * sizeof(uint16_t) / sizeof(unsigned char);

		if (nullptr != pDataTmp)
		{
			memcpy(pDataTmp, pFileData + wTmp * HAW_RegPageLen, ulReadNum * sizeof(uint16_t) / sizeof(unsigned char));
			nLoad = 0;
		}
	}

	free(pFileData);
	return nLoad;
}

int loadHswRegFile(string sSaveFilePath, uint32_t ulSaveSize, uint16_t *pData)
{
	int nLoad = -1;
	int nLoadTmp = -1;

	if (sSaveFilePath.empty() == false &&
		nullptr != pData){
		nLoad = loadRegFile(sSaveFilePath, ulSaveSize, pData);
	}

	string sTempFilePath = format("%s%s", SAVEREGFILEDIR, HMISaveRegFile_Tmp);

	if (CFile::ExistFile(sTempFilePath))
	{
		nLoadTmp = loadRegTmpFile(sTempFilePath, ulSaveSize, pData, ByteFile_HSW);
	}

	if (0 == nLoad ||
		0 == nLoadTmp) {
		return 0;
	}

	return -1;
}

int loadHawRegFile(string sSaveFilePath, uint32_t ulSaveSize, uint16_t *pData)
{
	int nLoad = -1;
	int nLoadTmp = -1;

	if (sSaveFilePath.empty() == false &&
		nullptr != pData) {
		nLoad = loadRegFile(sSaveFilePath, ulSaveSize, pData);
	}

	string sTempFilePath = format("%s%s", SAVEREGFILEDIR, HMISaveRegFile_Tmp);

	if (CFile::ExistFile(sTempFilePath))
	{
		nLoadTmp = loadRegTmpFile(sTempFilePath, ulSaveSize, pData, ByteFile_HAW);
	}

	if (0 == nLoad ||
		0 == nLoadTmp) {
		return 0;
	}

	return -1;
}

int loadRpwRegFile(string sSaveFilePath, uint32_t ulSaveSize, uint16_t *pData)
{
	int nLoad = -1;
	int nLoadTmp = -1;

	if (sSaveFilePath.empty() == false &&
		nullptr != pData) {
		nLoad = loadRegFile(sSaveFilePath, ulSaveSize, pData);
	}

	string sTempFilePath = format("%s%s", RECIPEFILEDIR, HMISaveRegFile_RPWTmp);

	if (CFile::ExistFile(sTempFilePath)){
		nLoadTmp = loadRegTmpFile(sTempFilePath, ulSaveSize, pData, ByteFile_RPW);
	}

	if (0 == nLoad ||
		0 == nLoadTmp){
		return 0;
	}

	return -1;
}

int reMoveOldFile(string sSaveFilePath, uint32_t ulSaveSize)
{
	string				sSaveFilename;
	unsigned short		uIndex = 0;

	for (uIndex = 0; uIndex <= (ulSaveSize / HAW_FileWord); uIndex++)//轮询文件编号
	{
		sSaveFilename = format("%s%d", sSaveFilePath.c_str(), uIndex);

		if (CFile::ExistFile(sSaveFilename))//文件存在
		{
			CFile::Delete(sSaveFilename);
		}
	}

	string sTempFilePath = format("%s%s", SAVEREGFILEDIR, HMISaveRegFile_Tmp);

	if (CFile::ExistFile(sTempFilePath))//文件存在
	{
		CFile::Delete(sTempFilePath);
	}

	sTempFilePath = format("%s%s", SAVEREGFILEDIR, HMISaveRegFile_RPWTmp);
	if (CFile::ExistFile(sTempFilePath))//文件存在
	{
		CFile::Delete(sTempFilePath);
	}

	return 0;
}