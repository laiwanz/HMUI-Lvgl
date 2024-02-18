#ifndef LVGLLABEL_H
#define LVGLLABEL_H
#include "../lvglParts/HMILvPart.h"
#include <lvgl/lvgl/lvgl.h>

#include <string>
class LvQRLabel
{
private:
	std::string m_Data;
public:
	explicit LvQRLabel(lv_obj_t *parent);
	void setLabelData(std::string data);
	~LvQRLabel();
	void setLvGeometry(int x, int y, int width, int height);
	void draw();
	
private:
	lv_obj_t* m_QRLabel;
protected:
	void paintEvent(lv_event_t* event);
private:
	int m_Size;
	int m_x;
	int m_y;
};
#endif
