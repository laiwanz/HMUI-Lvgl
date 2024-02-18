// #define LV_USE_QRCODE
#include "LvglLabel.h"
#include "qrencode.h"
#include <lvgl/lvgl/src/extra/libs/qrcode/lv_qrcode.h>
#include <iostream>

void LvQRLabel::setLabelData(std::string data)
{
	std::cout << "Set QRLabel Data " << std::endl;
	std::cout << std::endl;

	m_Data = data;
	this->draw();
}

void LvQRLabel::paintEvent(lv_event_t * event)
{
	if (event -> code != LV_EVENT_REFR_EXT_DRAW_SIZE) {
		return;
	}

}

LvQRLabel::LvQRLabel(lv_obj_t * parent)
{
	// m_QRLabel = lv_label_create(parent);
}

LvQRLabel::~LvQRLabel()
{
}

void LvQRLabel::setLvGeometry(int x, int y, int width, int height)
{
	if (m_QRLabel == NULL) {
		return;
	}

	m_x = x;
	m_y = y;
	m_Size = width;
}

void LvQRLabel::draw()
{
	std::cout << " QRLabel Draw" << std::endl;
	m_QRLabel = lv_qrcode_create(lv_scr_act(), m_Size, lv_color_hex3(000), lv_color_hex3(0xfff));

	lv_qrcode_update(m_QRLabel, m_Data.c_str(), strlen(m_Data.c_str()));
	lv_obj_set_pos(m_QRLabel, m_x, m_y);
}
