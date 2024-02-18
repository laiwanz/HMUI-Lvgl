#include "HSWAddrOperate.h"
#include "GlobalModule/GlobalModule.h"
#include <utility/utility.h>
#include "macrosplatform/path.h"


#define HAW_RegPageLen			1024				//2K	ÿ��ҳ�Ĵ�С2K  ��1024���ֵ�ַ
#define HAW_FileWord			19456				//1024*19 ÿ���ļ����� 19456���ֵ�ַ
#define SaveRegVersion			1					//�����Ʊ���ĵ��㱣�����ݵİ汾Ϊ1
#define SAVE_RegAllFile			HAW_MAX_SIZE_High/HAW_FileWord+1					//���籣���ļ�����
#define SAVE_RegAllPage			HAW_FileWord/HAW_RegPageLen+1						//���籣���ļ��е�ҳ����
#define SAVE_RPWRegAllFile		RPW_MAX_SIZE_High/HAW_FileWord+1					//RPW���籣���ļ�����

#define HMISaveRegFile_Tmp		"FileTmp"		//���籣�����ʱ�ļ���ÿ�δ洢���ȱ�����ʱ�ļ���
#define HMISaveRegFile_RPWTmp	"RpwFileTmp"	//RPW���籣�����ʱ�ļ���ÿ�δ洢���ȱ�����ʱ�ļ���

//���籣��ı����ļ��İ汾��Ϣ
typedef struct _tagHawRegVersionTmp {
	uint16_t	uVersion;		//�汾��
	unsigned long	ulVerify;		//У���
	uint16_t	uPage[SAVE_RegAllPage];		//�ı��ҳ��Ĭ��ֵΪ100��
	uint16_t	uFileID;		//��Ӧ���ļ� 
	uint16_t	uFileType;		//���ļ����� 1��HAW 2��HSW 4��RPW
	uint16_t	uValue[5];		//Ԥ��
} HawRegVersionTmp;

enum {
	ByteFile_HAW = 1,//HAW�ļ�
	ByteFile_HSW = 2,//HSW�ļ�
	ByteFile_RPW = 4,//RPW�ļ�
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
*��  �ߣ�LYQ	20170103
*��  �ܣ���ȡ���籣��ļĴ������ݣ�����HAW��HSW�ĵ��籣�沿�֣�
*��	 ����	FILE *fp		�ļ�ָ��
*			BYTE *pData		��ȡ�ļ��Ĵ��λ��
*			DWORD dwReadNum	��ȡ�ĳ���  word����
*����ֵ����
*************************************************/
static bool RegReadFile(FILE *fp, unsigned char *pData, unsigned long ulReadNum)//���ļ�
{
	unsigned long	ulReadSize = 0;
	unsigned long	ulNeedReadSize = ulReadNum * sizeof(uint16_t) / sizeof(unsigned char);
	unsigned long	ulFilelen = 0;	//�ļ�����

	if (!fp || NULL == pData)
	{
		return false;
	}

	fseek(fp, 0, SEEK_END);
	ulFilelen = ftell(fp);
	if (ulFilelen < ulNeedReadSize)//�ļ���Ҫ��ȡ������С
	{
		fclose(fp);
		return false;
	}

	fseek(fp, 0, SEEK_SET);//���ļ�ָ�������Ƶ��ļ�ͷ
	while (ulReadSize < ulNeedReadSize)
	{
		long	lRead = 0;
		lRead = fread(pData, 1, ulNeedReadSize - ulReadSize, fp);//��У��������  �ļ�����ֱ��������
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
*��  �ߣ�LYQ	20170103
*��  �ܣ���ȡ���籣��ļĴ������ݣ�����HAW��HSW�ĵ��籣�沿�֣�
*��	 ������
*����ֵ����
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

	for (wIndex = 0; wIndex <= (ulSaveSize / HAW_FileWord); wIndex++)//��ѯ�ļ����
	{
		uint16_t *pDataTmp = nullptr;
		pDataTmp = pData + wIndex * HAW_FileWord;

		dwReadNum = GetSaveFileLen(wIndex, ulSaveSize);//��wIndex���ļ��ļĴ�������
		sSaveFilename = format("%s%d", sSaveFilePath.c_str(), wIndex);

		if (CFile::ExistFile(sSaveFilename) == false)//�ļ�������
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

	if (CFile::ExistFile(sTempFilePath) == false)//�ļ�������
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
	unsigned long	ulVerify = 0;			//У��͵�ֵ
	long	lfilelen = 0;					//�ļ�����

	fseek(fp, 0, SEEK_END);
	lfilelen = ftell(fp);
	fseek(fp, lfilelen - sizeof(HawVersionTmp), SEEK_SET);				//ƫ�Ƶ�
	fread((unsigned char *)&HawVersionTmp, 1, sizeof(HawVersionTmp), fp);		//��ȡ����
	if (HawVersionTmp.uFileType != uFileType)//�ļ�����
	{
		free(pFileData);
		fclose(fp);
		return -5;
	}
	if (HawVersionTmp.uVersion != SaveRegVersion) 						//�汾��
	{
		free(pFileData);
		fclose(fp);
		CFile::Delete(sTempFilePath);
		return -6;
	}
	//����У��ֵ
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

	for (uint16_t wTmp = 0; wTmp <= HAW_FileWord / HAW_RegPageLen; wTmp++)//ҳ���
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

	for (uIndex = 0; uIndex <= (ulSaveSize / HAW_FileWord); uIndex++)//��ѯ�ļ����
	{
		sSaveFilename = format("%s%d", sSaveFilePath.c_str(), uIndex);

		if (CFile::ExistFile(sSaveFilename))//�ļ�����
		{
			CFile::Delete(sSaveFilename);
		}
	}

	string sTempFilePath = format("%s%s", SAVEREGFILEDIR, HMISaveRegFile_Tmp);

	if (CFile::ExistFile(sTempFilePath))//�ļ�����
	{
		CFile::Delete(sTempFilePath);
	}

	sTempFilePath = format("%s%s", SAVEREGFILEDIR, HMISaveRegFile_RPWTmp);
	if (CFile::ExistFile(sTempFilePath))//�ļ�����
	{
		CFile::Delete(sTempFilePath);
	}

	return 0;
}