#include <iostream>
#include <list>
#include "LvImgCacheMgr.h"
#include "platform/platform.h"
#include <lvgl/lvgl/src/extra/libs/stb_image/stb_image.h>
#include <lvgl/lvgl/lvgl.h>

#define LV_COLOR_DEPTH 32

static void convert_color_depth(uint8_t * img, uint32_t px_cnt)
{
#if LV_COLOR_DEPTH == 32
	lv_color32_t * img_argb = (lv_color32_t *)img;
	lv_color_t c;
	lv_color_t * img_c = (lv_color_t *)img;
	uint32_t i;
	for (i = 0; i < px_cnt; i++) {
		c = lv_color_make(img_argb[i].ch.red, img_argb[i].ch.green, img_argb[i].ch.blue);
		img_c[i].ch.red = c.ch.blue;
		img_c[i].ch.blue = c.ch.red;
	}
#elif LV_COLOR_DEPTH == 16
	lv_color32_t * img_argb = (lv_color32_t *)img;
	lv_color_t c;
	uint32_t i;
	for (i = 0; i < px_cnt; i++) {
		c = lv_color_make(img_argb[i].ch.blue, img_argb[i].ch.green, img_argb[i].ch.red);
		img[i * 3 + 2] = img_argb[i].ch.alpha;
		img[i * 3 + 1] = c.full >> 8;
		img[i * 3 + 0] = c.full & 0xFF;
	}
#elif LV_COLOR_DEPTH == 8
	lv_color32_t * img_argb = (lv_color32_t *)img;
	lv_color_t c;
	uint32_t i;
	for (i = 0; i < px_cnt; i++) {
		c = lv_color_make(img_argb[i].ch.red, img_argb[i].ch.green, img_argb[i].ch.blue);
		img[i * 2 + 1] = img_argb[i].ch.alpha;
		img[i * 2 + 0] = c.full;
	}
#elif LV_COLOR_DEPTH == 1
	lv_color32_t * img_argb = (lv_color32_t *)img;
	uint8_t b;
	uint32_t i;
	for (i = 0; i < px_cnt; i++) {
		b = img_argb[i].ch.red | img_argb[i].ch.green | img_argb[i].ch.blue;
		img[i * 2 + 1] = img_argb[i].ch.alpha;
		img[i * 2 + 0] = b > 128 ? 1 : 0;
	}
#endif
}




std::vector<LvIMG_T* > LvImgCacheMgr::m_LvImgList;
LvImgCacheMgr::LvImgCacheMgr()
{
	
}

int LvImgCacheMgr::Init(const void * pListImg, const string & sPath)
{
	std::cout << "LvImgCacheMgr Init" << std::endl;
	std::list<LvIMGNAME_T> *plistImage = (std::list<LvIMGNAME_T> *)pListImg;

	if (!plistImage) {

		return -1;
	}
	int		nIndex = 0;

	for (auto &iter : *plistImage) {

		LvIMG_T *pImgInfo = new LvIMG_T;
		pImgInfo->pImgDataInfo = NULL;

		pImgInfo->nImgIndex = nIndex;
		pImgInfo->strImgPath = sPath;
		pImgInfo->strImgPath.append("img/");
		pImgInfo->strImgPath.append(iter.sFileName.c_str());
		std::cout << "strImgPath:" << pImgInfo->strImgPath << std::endl;

		pImgInfo->nImgType = NORMALIMG;
		pImgInfo->nScaleHeight = iter.nZoomHeight;
		pImgInfo->nScaleWidth = iter.nZoomWidth;
		
		m_LvImgList.push_back(pImgInfo);
		nIndex ++;
	}

	// svg type

	return 0;
}

LvIMG_T * LvImgCacheMgr::GetImgInfo(int nImgIndex)
{
	if (nImgIndex < 0 || nImgIndex > (int)m_LvImgList.size() || m_LvImgList.size() <= 0) {
		return NULL;
	}

	//if (m_LvImgList[nImgIndex].nImgType == GIFIMG) {
	//	return m_LvImgList[nImgIndex];
	//}

	LoadImg(m_LvImgList[nImgIndex]);

	return m_LvImgList[nImgIndex];
}

string LvImgCacheMgr::GetImgPath(int nImgIndex)
{
	if (nImgIndex < 0 || nImgIndex > (int)m_LvImgList.size() || m_LvImgList.size() <= 0) {
		std::cout << "can not get the Img Path" << std::endl;
		return string();
	}

	//if (m_LvImgList[nImgIndex].nImgType == GIFIMG) {
	//	return m_LvImgList[nImgIndex];
	//}
	
	return string(m_LvImgList[nImgIndex]->strImgPath);
	//return nullptr;
}

inline void LvImgCacheMgr::LoadImg(LvIMG_T * pImgInfo)
{
	//std::cout << "LoadImg Img Path: " << pImgInfo->strImgPath << std::endl;
	if (pImgInfo->nImgType == NORMALIMG && pImgInfo->pImgDataInfo == NULL) {

		pImgInfo->pImgDataInfo = stbi_load(pImgInfo->strImgPath.c_str(), &pImgInfo->nWidth, &pImgInfo->nHeight, &pImgInfo->nChannels, 4);
		convert_color_depth(pImgInfo->pImgDataInfo, pImgInfo->nWidth * pImgInfo->nHeight);

		if (pImgInfo->pImgDataInfo  == nullptr) {
			std::cerr << "Failed to load image." << std::endl;
			return ;
		}

		//// ��ӡͼ����Ϣ
		//std::cout << "Image width: " << pImgInfo->nWidth << std::endl;
		//std::cout << "Image height: " << pImgInfo->nHeight << std::endl;
		//std::cout << "Image channels: " << pImgInfo->nChannels << std::endl;
	}

}
