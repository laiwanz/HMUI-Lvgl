#ifndef LVIMGCACHEMGR_H
#define LVIMGCACHEMGR_H
#include <string>
using std::string;
#include <vector>
#include "macrosplatform/macrosplatform.h"

typedef struct tagIMGNAME_ {

	std::string		sFileName;
	unsigned int	nZoomWidth;
	unsigned int	nZoomHeight;

} LvIMGNAME_T;

typedef struct _tagIMAGE_T_ {
	int                 nImgIndex;
	string              strImgPath;
	int					nImgType;
	int					nScaleWidth;	// ͼƬ���ſ���
	int					nScaleHeight;	// ͼƬ���Ÿ߶�
	unsigned char*		pImgDataInfo;
	int					nWidth;			// ͼƬ��
	int					nHeight;		// ͼƬ��
	int					nChannels;
} LvIMG_T;



class LvImgCacheMgr
{

public:
	LvImgCacheMgr();

	static int Init(const void *pListImg, const string& sPath);
	static void Destroy();
	static void Clear();
	
	static LvIMG_T*  GetImgInfo(int nImgIndex);

	static string GetImgPath(int nImgIndex);
private:
	static void inline LoadImg(LvIMG_T *pImgInfo);

private:
	static std::vector<LvIMG_T* > m_LvImgList;
public:
	enum IMGTYPE {
		NORMALIMG,
		GIFIMG,
		SVGIMG
	};
};

#endif

