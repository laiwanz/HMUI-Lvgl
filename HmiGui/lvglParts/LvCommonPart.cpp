#include "LvCommonPart.h"
#include "macrosplatform/path.h"
#include "GlobalModule/GlobalModule.h"
#include <lvgl/lvgl/src/extra/libs/svg/nanosvg.h>
#include <lvgl/lvgl/src/extra/libs/svg/nanosvgrast.h>
#include <lvgl/lvgl/src/extra/libs/stb_image/stb_image.h>

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

LvCommonPart::LvCommonPart(lv_obj_t *screen, string parttype)
{
	std::cout << "LvCommonPart construction" << std::endl;

	m_nImgIndex = BITMAPDISPLAY_INDEX;
	m_strImgPath = "";
	m_pImgInfo = NULL;
	m_pCustomImage = NULL;
	m_text = "";
	m_bTxtColor = false;
	m_bTxtBkColor = false;
	m_Screen = screen;
	m_Part = lv_canvas_create(screen);
	m_Text = lv_label_create(m_Part);
	lv_label_set_text(m_Text, "");
	m_movie = NULL;
	m_degrees = 0;
	m_bRoate = false;
	m_parttype = parttype;
	m_txtFlags = 0;
	m_strGifpath = "";
	m_pSvgImage = NULL;
	m_nLockImgIndex = -1;
	m_nLockWidth = 0;
	m_bIsShowGrayScale = false;
	m_nFreq = 0;
	memset(&m_Rect, 0, sizeof(RECT));
	m_nAlign = 0;
	m_bShowSelect = false;
	m_nSelectMode = CurSorMode_Default;


}

LvCommonPart::~LvCommonPart()
{
	try {
		//if (m_svg != NULL) {
		//	delete m_svg;
		//	m_svg = NULL;
		//}

		if (m_movie != NULL) {
			delete m_movie;
			m_movie = NULL;
		}

		if (m_pSvgImage) {
			delete m_pSvgImage;
			m_pSvgImage = NULL;
		}

		if (m_pCustomImage) {
			delete m_pCustomImage;
			m_pCustomImage = NULL;
		}
	}
	catch (...) {
	}
}

void LvCommonPart::LvsetRoateRect(int x, int y, int width, int height)
{
	//std::cout << "LvsetRoateRect" << std::endl;

	if (m_Part == NULL) {
		return;
	}
	lv_obj_set_size(m_Part, width, height);
	lv_obj_set_pos(m_Part, x, y);
}

void LvCommonPart::LvsetGeometry(int x, int y, int width, int height)
{
	//std::cout << "LvsetGeometry" << std::endl;


	if (m_Part == NULL) {
		return;
	}

	lv_obj_set_pos(m_Part, x, y);

}


void LvCommonPart::paintEvent()
{
	//std::cout << "LvCommonPart paintEvent" << std::endl;
	
	//��ת
	if (m_bRoate) {

	}

	drawImg();
	drawText();
}

void LvCommonPart::setRotate(int degress)
{
	std::cout << "setRotate" << std::endl;

	m_bRoate = true;
	m_degrees = degress;
}

int LvCommonPart::getRotate()
{
	return m_degrees;
	
	
}

void LvCommonPart::SetIMGIndex(const int & nIndex)
{
	m_strImgPath = "";
	m_nImgIndex = nIndex;
}

void LvCommonPart::SetIMGPath(const string & strImgPath, const RECT & rc)
{
	m_strImgPath = strImgPath;
	m_nImgIndex = BITMAPDISPLAY_INDEX;
	memcpy(&m_Rect, &rc, sizeof(RECT));

}


void LvCommonPart::setText(const hmiproto::hmipartcommon common, int drawtype)
{
	std::cout << "setText " << std::endl;

	string strText(common.basic().text().c_str());
	m_text = strText;
	// string strFont(common.basic().font().c_str());
	drawText();
}

void LvCommonPart::setSvg(char * contents, const std::string & strPartName, bool bAll)
{
	std::cout << "setSvg " << std::endl;
	NSVGimage* image = nsvgParse(contents, "px", 96);
	int nWidth, nHeight;
	nWidth = image->width;
	nHeight = image->height;
	unsigned char* svgImageData = (unsigned char*)malloc(nWidth * nHeight * 4);
	NSVGrasterizer* rasterizer = nsvgCreateRasterizer();

	nsvgRasterize(rasterizer, image, 0, 0, 1, svgImageData, nWidth, nHeight, nWidth * 4);
	convert_color_depth(svgImageData, nWidth * nHeight);

	//m_Part = lv_canvas_create(m_Screen);
	// lv_obj_t* svg = lv_canvas_create(m_Screen);
	//lv_obj_set_pos(svg, m_Rect.left, m_Rect.top);
	std::cout << "x:" << m_Rect.left << std::endl;
	std::cout << "y:" << m_Rect.top << std::endl;
	lv_canvas_set_buffer(m_Part, svgImageData, nWidth, nHeight, LV_IMG_CF_TRUE_COLOR_ALPHA);
	drawText();

	//paintEvent();
	//lv_obj_set_pos(m_Part, m_Rect.left, m_Rect.top);
	
}

void LvCommonPart::setImg(string ImgPath, RECT rc)
{
	std::cout << "setImg " << std::endl;
	//m_svgmutex.lock();

	m_Part = lv_img_create(m_Screen);
	lv_img_set_src(m_Part, ImgPath.c_str());
	lv_obj_set_size(m_Part, rc.right - rc.left, rc.bottom - rc.top);

}

void LvCommonPart::OnPartHide()
{
}

void LvCommonPart::OnPartShow()
{
}

void LvCommonPart::setGeometry(int x, int y, int width, int height)
{
	// if (m_Part = nullptr) {
	// 	return;
	// }
	lv_obj_set_pos(m_Part, x, y);
}

void LvCommonPart::drawImg()
{
	std::cout << "LvCommonPart drawImg" << std::endl;
	
	if (m_nImgIndex != BITMAPDISPLAY_INDEX) {
		//std::cout << "LvCommonPart drawImg In the Cache: index:"<< m_nImgIndex << std::endl;
		m_pImgInfo = LvImgCacheMgr::GetImgInfo(m_nImgIndex);
		if (m_pImgInfo == NULL) {
			//std::cout << "LvCommonPart ImgInfo is NULL" << std::endl;
			return;
		}
		if (m_pImgInfo->nImgType == LvImgCacheMgr::NORMALIMG) {
			drawImg_NORMAL();
		}
	}
	else {
		std::cout << "LvCommonPart drawImg Not In the Cache" << std::endl;
		if (m_strImgPath.size() <= 0) {
			return;
		}
	}
}

void LvCommonPart::drawText()
{

	
	if (m_text.size() == 0) {
		return;
	}
	std::cout << "Draw Text =============================: " << m_text << std::endl;
	if(m_Part == NULL){
		std::cout << "Part Not Init  =============================: " << std::endl;
	}
	if(m_Text == NULL){
		std::cout << "Text Not Init  =============================: " << std::endl;

		m_Text = lv_label_create(m_Part);
	}
	lv_label_set_text(m_Text, "");
	std::cout << "Draw Text =============================: 1" << std::endl;

	lv_label_set_text(m_Text, m_text.c_str());
	std::cout << "Draw Text =============================: 2" << std::endl;

	lv_obj_set_align(m_Text, LV_ALIGN_CENTER);
	std::cout << "Draw Text =============================: 3" << std::endl;

}

void LvCommonPart::drawImg_NORMAL()
{
	//std::cout << "LvCommonPart drawImg_Normal" << std::endl;
	if (m_pImgInfo == NULL ||
		m_pImgInfo->nImgType != LvImgCacheMgr::NORMALIMG ||
		m_pImgInfo->pImgDataInfo == NULL) {
		return;
	}

	float scale = (float)(m_Rect.right - m_Rect.left) / m_pImgInfo->nWidth;

	lv_canvas_set_buffer(m_Part, m_pImgInfo->pImgDataInfo, m_pImgInfo->nWidth, m_pImgInfo->nHeight, LV_IMG_CF_TRUE_COLOR_ALPHA);

	lv_img_set_pivot(m_Part, 0, 0);
	//lv_img_set_auto_size(m_Part, true);
	lv_img_set_zoom(m_Part, 256 * scale);

	LvsetGeometry(m_Rect.left, m_Rect.top, m_Rect.right - m_Rect.left, m_Rect.bottom - m_Rect.top);

}

void LvCommonPart::setPos(RECT rect)
{
	m_Rect = rect;
}

void LvCommonPart::setSelect(bool bShowSelect, int nSelectMode)
{
	m_bShowSelect = bShowSelect;
	m_nSelectMode = nSelectMode;
}

bool LvCommonPart::getSelect()
{
	return m_bShowSelect;

}

int LvCommonPart::getSelectMode()
{
	return m_nSelectMode;

}

void LvCommonPart::setFreq(int nFreq)
{
}

int LvCommonPart::getFreq()
{
	return 0;
}

bool LvCommonPart::FindFontInfo(int nWeight, int nStyle, string & strFontFamily, string strFontSize)
{
	return false;
}

void LvCommonPart::ReadQpf2File()
{
}

void LvCommonPart::setLockImgInfo(int nLockImgIndex, int nLockWidth, bool bIsShowGrayScale)
{
}

void LvCommonPart::CalculateTextArea()
{
}

