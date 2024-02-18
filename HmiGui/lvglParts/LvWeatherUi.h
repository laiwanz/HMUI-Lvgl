#ifndef LVWEATHER_H
#define LVWEATHER_H

#include "LvGeneralPart.h"
#include "LvImgCacheMgr.h"
#include "HMILvScreen.h"
#include <lvgl/lvgl/lvgl.h>

#include "LvCommonPart.h"
#include "SVGParts/SVGPartsDef.h"


class LvWeatherUi : public HMILvPart {
public:
	explicit LvWeatherUi(const hmiproto::hmiweather &weather, HMILvScreen * parent);
	~LvWeatherUi();

	virtual void Init(lv_obj_t* scr);

	virtual void Update(const hmiproto::hmiweather &weather, HMILvScreen * parent);

	virtual void Draw(HMILvScreen * parent, int drawtype = DRAW_INIT);

	static void list_event_handler(lv_event_t * e);
	
	static lv_obj_t			 *m_listView;


#if ((defined WIN32) && (defined WIN32SCALE))
	virtual void ZoomPartSelf();	//����
#endif
private:
	int updateBackground(const hmiproto::hmiweather &weather);
	int updateSeltUi(const hmiproto::hmiweather &weather);
	int getDesignedConfig(const hmiproto::hmiweather &weather);
private:
	void slotSelectItem(const int index);
	void slotBtnCancel();
	

private:
	RECT				 m_rc;
	int					 m_nScrNo;
	hmiproto::hmiweather m_part;	/* weather config info */
	lv_obj_t			 *m_container;
	lv_obj_t			 *m_labProvince;
	lv_obj_t			 *m_labCity;
	lv_obj_t			 *m_labHeadBg;
	lv_obj_t			 *m_btnCancel;
	vector<lv_obj_t*>	 m_vecImg;

	WEATHER_DESIGNED_CONFIG_T m_config;
	LvCommonPart      *m_svg;
};

#endif // !LVWEATHER_H
