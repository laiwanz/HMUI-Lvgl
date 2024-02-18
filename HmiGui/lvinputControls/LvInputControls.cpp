#include "LvInputControls.h"
#include <lvgl/lvgl/src/extra/libs/stb_image/stb_image.h>
#include <iostream>



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


LvInputControls::LvInputControls(const hmiproto::hmipartinput & partInput, HMILvScreen * parent)
{
	m_pLineEdit = nullptr;
	m_pTextEdit = nullptr;

	m_bLoadLockImg = false;
	m_nLockImgIndex = -1;
	m_nScreenNo = partInput.basic().scrno();
	m_sPartName = partInput.basic().name();
	m_nTextColor = partInput.basic().textcolor();
	m_bTextGray = partInput.gray();
	
	m_nBackgroundImg = partInput.basic().imgindex();
	m_sBackgroundImg = partInput.basic().imgpath();
	if (m_sBackgroundImg.size() != 0) {
		int nWidth, nHeight, nChannel;
		string sPath = R"+*(/mnt/wecon/wecon/run/project/)+*";
		sPath += m_sBackgroundImg; 
		/*string sPath = R"+*(C:\Users\admin\AppData\Local\Temp\HMIEmulation/img/Bmp105_0.dat)+*";*/
		unsigned char *Data = stbi_load(sPath.c_str(), &nWidth, &nHeight, &nChannel, 4);
		m_BackgroundImg = lv_canvas_create(lv_scr_act());
		std::cout << "m_sBackgroundImg : " << sPath << std::endl;
		convert_color_depth(Data, nWidth * nHeight);
		lv_canvas_set_buffer(m_BackgroundImg, Data, nWidth, nHeight, LV_IMG_CF_TRUE_COLOR_ALPHA);
	}
	lv_obj_set_pos(m_BackgroundImg, partInput.basic().left(), partInput.basic().top());
	lv_obj_set_size(m_BackgroundImg, (partInput.basic().right() - partInput.basic().left()), (partInput.basic().bottom() - partInput.basic().top())); 


	if (!partInput.ismultiline()) {
		m_pLineEdit = new LvInputLineEdit(m_BackgroundImg);
		
		// set styleSheet
		if (partInput.star()) {

		}
		m_pLineEdit->config(partInput.dataformat(), partInput.min_(), partInput.max_(), partInput.length(), partInput.decimal());
		if (partInput.has_displayrange()) {
			m_pLineEdit->setDispalyRange(partInput.displayrange());
		}
	}
	else {
		m_pTextEdit = new LvInputTextEdit(m_BackgroundImg);
		// set pos and size 

		// connect
		m_pTextEdit->config(partInput.length());
	}

	this->setLanguage(partInput.basic().msgtiplanguage());
	this->setInputText((partInput.basic().text().c_str()));
	this->saveLastText((partInput.basic().text().c_str()));
	this->setInputHide(partInput.basic().hide());
	this->setInputFont(partInput.basic().font().c_str());
	this->setInputAlignment(partInput.basic().align(), partInput.basic().verticalalign());
	if (m_bTextGray) {
		// set the text color gray
	}
	else {
		this->setInputTextColor(m_nTextColor);
	}
	

	m_nLockWidth = partInput.lockimgwidth();
	if (partInput.has_lockimg()) {
		m_nLockImgIndex = partInput.lockimg();
		this->setLockImage(partInput.lockimgpath());
		std::cout << "lockimgpath : " << partInput.lockimgpath() << std::endl;

	}
	this->setConfirmFlag(partInput.operateconfirm(), partInput.operatewaittime());
}

LvInputControls::~LvInputControls()
{
	if (m_pLineEdit) {
		delete m_pLineEdit;
		m_pLineEdit = nullptr;
	}
	if (m_pTextEdit) {
		delete m_pTextEdit;
		m_pTextEdit = nullptr;
	}


}

void LvInputControls::Init(lv_obj_t * scr)
{
}

void LvInputControls::Update(const hmiproto::hmipartinput & partInput, HMILvScreen * parent)
{
}



void LvInputControls::Draw(HMILvScreen * parent, int drawtype)
{
}

int LvInputControls::parseJson(const std::string & sbuffer)
{
	return 0;
}

int LvInputControls::setInputText(const string & sText)
{
	if (m_pLineEdit) {
		//m_pLineEdit->setText(sText);
	}
	else if (m_pTextEdit) {
		//m_pTextEdit->setText(sText);
	}
	return 0;
}

int LvInputControls::setInputHide(bool bHide)
{
	return 0;
}

int LvInputControls::setInputGeometry(int nX, int nY, int nWidth, int nHeight)
{

	if (m_pLineEdit) {
	
	}
	else if (m_pTextEdit) {
	
	}
	return 0;
}

void LvInputControls::setGeometry(int nX, int nY, int nWidth, int nHeight)
{

}

bool LvInputControls::isInputReadOnly()
{
	return false;
}

int LvInputControls::setInputReadOnly(bool enabled)
{
	return 0;
}

int LvInputControls::setInputFont(const string & sFont)
{
	return 0;
}

int LvInputControls::setInputAlignment(unsigned int nHorizontalAlign, unsigned int nVerticalAlign)
{
	return 0;
}

int LvInputControls::setInputTextColor(unsigned int nColor)
{
	return 0;
}

int LvInputControls::setInputFocus()
{
	return 0;
}

int LvInputControls::setLockImage(const std::string & sPath)
{
	return 0;
}

int LvInputControls::selectInputAll()
{
	return 0;
}

int LvInputControls::setConfirmFlag(bool bConfirmFlag, unsigned int nTimeout)
{
	return 0;
}

int LvInputControls::setLanguage(unsigned int nLanguage)
{
	return 0;
}

int LvInputControls::saveLastText(const string & sbuffer)
{
	return 0;
}

bool LvInputControls::hasInputFocus()
{
	return false;
}
