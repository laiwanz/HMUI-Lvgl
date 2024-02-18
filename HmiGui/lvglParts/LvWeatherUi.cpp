#include "LvWeatherUi.h"
#include <vector>
#include <iostream>
// #include "lvgl/src/font/lv_font.h"
#include <lvgl/lvgl/src/extra/libs/stb_image/stb_image.h>

LV_FONT_DECLARE(hanyu)
LV_FONT_DECLARE(lv_font_simsun_16_cjk)


lv_obj_t*  LvWeatherUi::m_listView = NULL;
static std::string toUtf8(const std::string &sStr) {

	cbl::CODE_FORMAT_T nSrcFormat = cbl::getCodeFormat(sStr);

	std::string sTagStr;
	int nRet = cbl::encode(nSrcFormat, cbl::UTF8, sStr, sTagStr);
	if (nRet < 0) {
		return sStr;
	}

	return sTagStr;
}

static void event_handler(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	// lv_obj_t * obj = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked: %s", lv_list_get_btn_text(list1, obj));
	}
}



LvWeatherUi::LvWeatherUi(const hmiproto::hmiweather & weather, HMILvScreen * parent)
{
	std::cout << "LvWeatherUi Construction" << std::endl;

	m_svg = NULL;
	m_container = lv_canvas_create(lv_scr_act());
	m_labHeadBg = lv_label_create(m_container);
	m_labProvince = lv_label_create(m_container);
	m_labCity = lv_label_create(m_container);

	m_listView = lv_list_create(m_container);
	m_btnCancel = lv_label_create(m_container);

	if (weather.has_designeddata()) {
		m_part.clear_designeddata();
	}
	m_part.MergeFrom(weather);


}

LvWeatherUi::~LvWeatherUi()
{
	try {
		if (m_svg) {
			delete m_svg;
			m_svg = nullptr;
		}
		if (m_labHeadBg) {
			delete m_labHeadBg;
			m_labHeadBg = nullptr;
		}
		if (m_labProvince) {
			delete m_labProvince;
			m_labProvince = nullptr;
		}
		if (m_labCity) {
			delete m_labCity;
			m_labCity = nullptr;
		}

		if (m_listView) {
			delete m_listView;
			m_listView = nullptr;
		}
		if (m_btnCancel) {
			delete m_btnCancel;
			m_btnCancel = nullptr;
		}
	}
	catch (...) {
	}
}

void LvWeatherUi::Init(lv_obj_t * scr)
{
	m_rc.left = m_part.basic().left();
	m_rc.right = m_part.basic().right();
	m_rc.top = m_part.basic().top();
	m_rc.bottom = m_part.basic().bottom();

	lv_obj_set_pos(m_container, m_rc.left, m_rc.top);
	lv_obj_set_size(m_container, m_rc.right - m_rc.left, m_rc.bottom - m_rc.top);

	string strPartType(m_part.basic().type().c_str());
	m_sPartName = m_part.basic().name();
	m_nScrNo = m_part.basic().scrno();

	/*lv_canvas_fill_bg(m_container, lv_palette_lighten(LV_PALETTE_GREY, 3), LV_OPA_COVER);*/

	getDesignedConfig(m_part);



#if ((defined WIN32) && (defined WIN32SCALE))
	memcpy(&m_OriginalRect, &m_rc, sizeof(RECT));
	ZoomPartSelf();
#endif
	updateBackground(m_part);
	updateSeltUi(m_part);

}

void LvWeatherUi::Update(const hmiproto::hmiweather &weather, HMILvScreen * parent)
{
	RECT	rc;
	if (weather.has_designeddata()) {
		m_part.clear_designeddata();
	}
	m_part.MergeFrom(weather);

	rc.left = m_part.basic().left();
	rc.right = m_part.basic().right();
	rc.top = m_part.basic().top();
	rc.bottom = m_part.basic().bottom();

	if (memcmp(&m_rc, &rc, sizeof(RECT)) != 0) {
		memcpy(&m_rc, &rc, sizeof(RECT));
	}

	//this->setGeometry(m_rc.left, m_rc.top, m_rc.right - m_rc.left, m_rc.bottom - m_rc.top);

	getDesignedConfig(m_part);

#if ((defined WIN32) && (defined WIN32SCALE))

	if (NULL != m_svg) {
		//m_svg->GetTextRect(m_part.basic(), &(m_svg->m_OriginalTextRect));
	}

	if (memcmp(&m_rc, &rc, sizeof(RECT)) != 0) {
		memcpy(&m_OriginalRect, &rc, sizeof(RECT));
		ZoomPartSelf();
	}
#endif

	updateBackground(weather);
	updateSeltUi(weather);
}

void LvWeatherUi::Draw(HMILvScreen * parent, int drawtype)
{
	std::cout << "LvWeatherUi Draw " << std::endl;
}


void LvWeatherUi::list_event_handler(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);

	if (code == LV_EVENT_CLICKED) {
		lv_obj_clear_flag(m_listView, LV_OBJ_FLAG_HIDDEN);
		
	}
	else if (code == LV_EVENT_VALUE_CHANGED) {
		LV_LOG_USER("Toggled");
	}

}


int LvWeatherUi::updateBackground(const hmiproto::hmiweather & weather)
{
	std::cout << "updateBackground--------------------------start" << std::endl;
	if (!weather.has_designeddata()) {
		return 0;
	}
	// for (auto &item : m_config.vecLine) {

	// 	static lv_style_t style_line;
	// 	lv_style_init(&style_line);
	// 	lv_style_set_line_width(&style_line, 8);
	// 	lv_style_set_line_color(&style_line, lv_palette_main(LV_PALETTE_BLUE));
	// 	lv_style_set_line_rounded(&style_line, true);

	// 	static lv_point_t line_points[] = { {item.nX1, item.nY1}, {item.nX2, item.nY2} };
	// 	lv_obj_t * line1;
	// 	line1 = lv_line_create(m_container);
	// 	lv_line_set_points(line1, line_points, 5);     /*Set the points*/
	// 	lv_obj_add_style(line1, &style_line, 0);
	// }
	std::cout << "updateBackground--------------------------1" << std::endl;

	for (auto &item : m_config.vecImage) {
		std::cout << "vecImage" << item.sFilePath << endl; 
		m_vecImg.push_back(lv_canvas_create(m_container));
	}
	std::cout << "updateBackground--------------------------2" << std::endl;

	int i = 0;
	lv_png_init();

	for (auto &item : m_config.vecImage) {
		std::cout << "Draw Image--------------------------start :"<< i << std::endl;
		std::cout << "Image: " << item.sFilePath.c_str() <<"     Index: "<< i <<std::endl;
		if (item.sFilePath.size() != 0 && strcmp(lv_fs_get_ext(item.sFilePath.c_str()), "png") == 0) {
			int width, height, channel;
			unsigned char* imgData = stbi_load(item.sFilePath.c_str(), &width, &height, &channel, 4);
			lv_canvas_set_buffer(m_vecImg[i], imgData, width, height, LV_IMG_CF_TRUE_COLOR_ALPHA);

			lv_obj_set_pos(m_vecImg[i], item.nX-80, item.nY-80);
			float scale = (1.0*item.nW / width);
			std::cout << "Image Zoom item.nW :"<< item.nW << "width: " << width << "scale :" << scale << std::endl;

			lv_img_set_zoom(m_vecImg[i], 256 * scale);

		}

		i++;
	}
	std::cout << "updateBackground--------------------------end" << std::endl;
	

	return 0;
}

int LvWeatherUi::updateSeltUi(const hmiproto::hmiweather & weather)
{
	std::cout << "updateSeltUi--------------------------" << std::endl;

	if (weather.has_isshowselectui() ? weather.isshowselectui() : false) {

	}
	else {
		lv_obj_add_flag(m_listView, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(m_labProvince, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(m_labCity, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(m_btnCancel, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(m_labHeadBg, LV_OBJ_FLAG_HIDDEN);

	}
	string sProvince = weather.has_selectprovince() && !weather.selectprovince().empty() ? weather.selectprovince().c_str() : toUtf8("ʡ��").c_str();
	string sCity = toUtf8("����").c_str();
	vector<string> vecData;


	lv_obj_t* btn;
	
	lv_list_add_text(m_listView, "City");
	for (int i = 0; i < weather.listcitys().size(); ++i) {
		//vecData.push_back(weather.listcitys(i).c_str());
		btn = lv_list_add_btn(m_listView, NULL, weather.listcitys(i).c_str());
		lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
	}
	
	lv_label_set_text(m_labProvince, sProvince.c_str());
	lv_label_set_text(m_labCity, sCity.c_str());


	// layout position
	{

		lv_obj_t* city_button = lv_btn_create(m_container);
		lv_obj_t* city_label = lv_label_create(city_button);
		lv_label_set_text(city_label, "City");
		lv_obj_add_event_cb(city_button, list_event_handler, LV_EVENT_ALL, NULL);
		lv_obj_set_align(city_button, LV_ALIGN_TOP_LEFT);

		static lv_style_t style;
		lv_style_init(&style);
		//lv_style_set_border_opa(&style, LV_OPA_TRANSP);
		lv_style_set_bg_opa(&style, LV_OPA_TRANSP);
		// lv_obj_add_style(city_button, &style, NULL);
	}


	return 0;
}

int LvWeatherUi::getDesignedConfig(const hmiproto::hmiweather & weather)
{
	WEATHER_DESIGNED_CONFIG_T designedConfig;
	if (weather.has_designeddata()) {
		designedConfig.nViewHeight = weather.designeddata().height();
		designedConfig.nViewWidth = weather.designeddata().width();
		for (int nIndex = 0; nIndex < weather.designeddata().listimage_size(); ++nIndex) {
			WEATHER_DESIGNED_CONFIG_T::IMAGE_ELEMENT_T image;
			image.nX = weather.designeddata().listimage(nIndex).x();
			image.nY = weather.designeddata().listimage(nIndex).y();
			image.nW = weather.designeddata().listimage(nIndex).width();
			image.nH = weather.designeddata().listimage(nIndex).height();
			image.fOpacity = weather.designeddata().listimage(nIndex).opacity();
			image.sFilePath = weather.designeddata().listimage(nIndex).srcpath();
			designedConfig.vecImage.push_back(image);
		}
		for (int nIndex = 0; nIndex < weather.designeddata().listline_size(); ++nIndex) {
			WEATHER_DESIGNED_CONFIG_T::LINE_ELEMENT_T line;
			line.nX1 = weather.designeddata().listline(nIndex).x1();
			line.nY1 = weather.designeddata().listline(nIndex).y1();
			line.nX2 = weather.designeddata().listline(nIndex).x2();
			line.nY2 = weather.designeddata().listline(nIndex).y2();
			line.nStrokeWidth = weather.designeddata().listline(nIndex).strokewidth();
			line.sColor = weather.designeddata().listline(nIndex).color();
			designedConfig.vecLine.push_back(line);
		}
		for (int nIndex = 0; nIndex < weather.designeddata().listrect_size(); ++nIndex) {
			WEATHER_DESIGNED_CONFIG_T::RECT_ELEMENT_T rect;
			rect.nX = weather.designeddata().listrect(nIndex).x();
			rect.nY = weather.designeddata().listrect(nIndex).y();
			rect.nW = weather.designeddata().listrect(nIndex).width();
			rect.nH = weather.designeddata().listrect(nIndex).height();
			rect.fOpacity = weather.designeddata().listrect(nIndex).opacity();
			rect.sFillColor = weather.designeddata().listrect(nIndex).fillcolor();
			designedConfig.vecRect.push_back(rect);
		}
		for (int nIndex = 0; nIndex < weather.designeddata().listtext_size(); ++nIndex) {
			WEATHER_DESIGNED_CONFIG_T::TEXT_ELEMENT_T text;
			text.nX = weather.designeddata().listtext(nIndex).x();
			text.nY = weather.designeddata().listtext(nIndex).y();
			text.nW = weather.designeddata().listtext(nIndex).width();
			text.nH = weather.designeddata().listtext(nIndex).height();
			text.nAlignType = weather.designeddata().listtext(nIndex).align();
			text.sFont = weather.designeddata().listtext(nIndex).font();
			text.sText = weather.designeddata().listtext(nIndex).text();
			text.sColor = weather.designeddata().listtext(nIndex).color();
			designedConfig.vecText.push_back(text);
		}
	}
	m_config = designedConfig;

	return 0;
}
