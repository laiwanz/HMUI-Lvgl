#include "HmiGui.h"
#include <platform/platform.h>

#include "../lvglParts/HMILvMgr.h"
#include "../lvglParts/HMILvScreen.h"
#include <lvgl/lv_drivers/display/fbdev.h>
#include <lvgl/lv_drivers/indev/evdev.h>



using namespace ProtoXML;
void* HmiGui::m_pHmiMgr = NULL;

void HmiGui::SetSendFunc(FUNCTION_SENDEVENT func) {
	std::cout << "HMIUI HmiGui SetSendFunc " << std::endl;

	EventMgr::SetSendFunc(func);
}

#ifdef WIN32
#include <windows.h>
#include <commctrl.h>
#else
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#
//#include <iostream>
#endif

#include <iostream>
#include <lvgl/lvgl/lvgl.h>

#define DISP_BUF_SIZE (800 * 480)


/****************************************
作  者：xqh
时  间：20201022
功  能：立即处理hmi事件接口
参  数：
		nScrNo：画面号
		user：用户ID
		wDataType：数据类型
返回值：
		无
****************************************/
void HmiGui::doHmiEventQuickly(hmiproto::hmiact& act) {
	cout << endl;
	if (NULL == m_pHmiMgr) {
		return;
	}
	static_cast<HMILvMgr*>(m_pHmiMgr)->sendQuicklyDoEventSignal(act);

	return;
}

void HmiGui::setScale(int nWidth, int nHeight, int argc, char* argv[]) {
	if (NULL == m_pHmiMgr)  // hmi还未创建
	{
		return;
	}
	return;
}




string sPath;
const void * pList;



int HmiGui::runGui(int argc, char* argv[], const std::string& strImgPath, void* pPicList) {
//

#if 1
	std::cout << "RunGui ---------------------" << std::endl;
	sPath = strImgPath;
	pList = pPicList;
	// LvglInit();
	std::cout << "LvglInit --------------  " << std::endl;
 	lv_init();

    fbdev_init();

    static lv_color_t buf1[DISP_BUF_SIZE] __attribute__((section(".SDRAM1"))) ;
    
    static lv_color_t buf2[DISP_BUF_SIZE] __attribute__((section(".SDRAM1"))) ;


    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, DISP_BUF_SIZE );

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf   = &disp_buf;
    disp_drv.flush_cb   = fbdev_flush;
    disp_drv.hor_res    = 800;
    disp_drv.ver_res    = 480;
    disp_drv.full_refresh = 1;
    lv_disp_drv_register(&disp_drv);

    evdev_init();
    static lv_indev_drv_t indev_drv_1;
    lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
    indev_drv_1.type = LV_INDEV_TYPE_POINTER;
    
	/*This function will be called periodically (by the library) to get the mouse position and state*/
    indev_drv_1.read_cb = (void (*)(lv_indev_drv_t*, lv_indev_data_t*))evdev_read;
    lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv_1);
    if(mouse_indev){

	}
	HMILvMgr * hmilvmgr = new HMILvMgr();
	hmilvmgr->ImgCache(pList, sPath);
	m_pHmiMgr = hmilvmgr;
	// CommonPart::ReadQpf2File();
	EventMgr::SendInitScrEvent(-1);
	
	while(1) {
        lv_timer_handler();
        usleep(5000);
    }
	return 0;

#endif
}

int HmiGui::runMsg(int argc, char* argv[], const std::string& strMsgText) {

	return 0;
}

// int HmiGui::setCommuConfig(const COMMU_MGR_CONFIG_T &config) {
// 	return 0;
// }
