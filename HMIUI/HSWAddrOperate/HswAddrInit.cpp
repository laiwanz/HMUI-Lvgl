#include "HSWAddrOperate.h"
#include "HmiBase/HmiBase.h"
#include <string.h>
#include "platform/platform.h"
#include "GlobalModule/GlobalModule.h"
#include "RegisterOpt/RegisterOpt.h"
#include "macrosplatform/registerAddr.h"
#include "serialmanager.h"
#ifndef WIN32
#include "btl/btl.h"
#endif

using namespace PLATFORM_NAMESPACE;

#define Beep_Default_Freq	2000
#define Beep_Default_Volume	80
#define Beep_Default_Time	100
#define Backlight_Default_Brightness 100

static void LanguageAddrInit() {
	PROJSETUP*      pps = GetProjInfo();
	unsigned short  nLangNum = 0;
	RW_PARAM_T      rw;

	nLangNum = CRegisterRW::readWord(REG_TYPE_HS_, HSW_LANGSAVE);
	CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_LANGNUM, 0, &nLangNum, 1, &rw); //刚开机时将上一次保存的语言(HSW10609)赋给多国语言寄存器HSW0 Lizh 20181207
	CRegisterRW::writeWord(REG_TYPE_HS_, HSW_INTERNALLANGTYPE, (unsigned short)pps->BaseSet.nOsLanguage);//20150907 chenzl 工程语言设置
}
#ifdef _LINUX_
int beepCallBack(const btl::BEEPINFO &stBeepInfo,void *lpParameter) {
	CRegisterRW::writeBit(REG_TYPE_HS_, HSW_USEBEEPTIMESAVE, 0, !stBeepInfo.bEnable);
	CRegisterRW::writeWord(REG_TYPE_HS_, HSW_USEBEEPTIME, !stBeepInfo.bEnable);
	CRegisterRW::writeWord(REG_TYPE_HS_, HSW_BEEPTIME, stBeepInfo.nDurationMs);
	CRegisterRW::writeWord(REG_TYPE_HS_, HSW_BEEPTYPE, stBeepInfo.nBuzzerType);
	return 0;
}

int backLightCallBack(const btl::BACKLIGHTINFO stBacklightInfo,void * lpParameter) {
	CRegisterRW::writeWord(REG_TYPE_HS_, HSW_SETBACKLIGHTTIME, stBacklightInfo.nBacklightTime);
	CRegisterRW::writeWord(REG_TYPE_HS_, HSW_SETBACKLIGHTNESS, stBacklightInfo.nBacklightBrightness);
	return 0;
}
#endif

static void BuzzerAndBacklightAddrInit() {
	LPPROJSETUP	    pps = GetProjInfo();
	bool		    bInit = false;
	bool			bBtlInit = false;
	
	unsigned short  wBeepPWM = 1;
	unsigned short  wBeepClose = 0;
	unsigned short  wBeepTime = Beep_Default_Time;
	unsigned short  wBrightness = Backlight_Default_Brightness;
	unsigned short	wBgOffTime = pps->BaseSet.nBgOffTime;

	bInit = (bool)CRegisterRW::readBit(REG_TYPE_HS_, HSW_USEBEEPTIMESAVE, 1);
	bBtlInit = (bool)CRegisterRW::readBit(REG_TYPE_HS_, HSW_USEBEEPTIMESAVE, 2);
	if (bInit && !bBtlInit) {
#ifdef _LINUX_
		CRegisterRW::writeBit(REG_TYPE_HS_, HSW_USEBEEPTIMESAVE, 2, true);
		wBeepClose = (BYTE)CRegisterRW::readBit(REG_TYPE_HS_, HSW_USEBEEPTIMESAVE, 0);
		wBeepPWM = CRegisterRW::readWord(REG_TYPE_HS_, HSW_BEEPTYPESAVE);
		wBeepTime = CRegisterRW::readWord(REG_TYPE_HS_, HSW_BEEPTIMESAVE);

		btl::beepEnable(!wBeepClose);
		btl::beepTouchEnable(!wBeepClose);
		btl::beepDurationMs(wBeepTime);
		btl::beepBuzzerType(wBeepPWM);

		wBgOffTime = CRegisterRW::readWord(REG_TYPE_HS_, HSW_SETBACKLIGHTTIME);
		wBrightness = CRegisterRW::readWord(REG_TYPE_HS_, HSW_SETBACKLIGHTNESS);

		if (0 == wBgOffTime)
		{
			btl::backlightEnable(false);
	}
		else {
			btl::backlightEnable(true);
			btl::backlightTime(wBgOffTime);
		}
		btl::backlightBrightness(wBrightness);
#endif
	}
	else if (bBtlInit) {
#ifdef _LINUX_
		btl::BEEPINFO stBeepInfo;
		btl::getBeepInfo(stBeepInfo);
		CRegisterRW::writeWord(REG_TYPE_HS_, HSW_BEEPTYPE, stBeepInfo.nBuzzerType);
		CRegisterRW::writeWord(REG_TYPE_HS_, HSW_BEEPTIME, stBeepInfo.nDurationMs);

		btl::BACKLIGHTINFO stBacklightInfo;
		btl::getBacklightInfo(stBacklightInfo);
		CRegisterRW::writeWord(REG_TYPE_HS_, HSW_SETBACKLIGHTTIME, stBacklightInfo.nBacklightTime);
		CRegisterRW::writeWord(REG_TYPE_HS_, HSW_SETBACKLIGHTNESS, stBacklightInfo.nBacklightBrightness);
#endif // _LINUX_
	}
	else {
		bInit = true;
		CRegisterRW::writeBit(REG_TYPE_HS_, HSW_USEBEEPTIMESAVE, 1, (unsigned short)bInit);    // 写入已初始化标志
		CRegisterRW::writeBit(REG_TYPE_HS_, HSW_USEBEEPTIMESAVE, 2, (unsigned short)bInit);

		CRegisterRW::writeBit(REG_TYPE_HS_, HSW_USEBEEPTIMESAVE, 0, !wBeepClose);  // 开机首次默认使用蜂鸣器
		CRegisterRW::writeWord(REG_TYPE_HS_, HSW_BEEPTYPE, wBeepPWM);
		CRegisterRW::writeWord(REG_TYPE_HS_, HSW_BEEPTIME, wBeepTime);

		CRegisterRW::writeWord(REG_TYPE_HS_, HSW_SETBACKLIGHTTIME, wBgOffTime);
		CRegisterRW::writeWord(REG_TYPE_HS_, HSW_SETBACKLIGHTNESS, wBrightness);

#ifndef WIN32
		btl::beepEnable(!wBeepClose);
		btl::beepTouchEnable(!wBeepClose);
		btl::beepDurationMs(wBeepTime);
		btl::beepBuzzerType(wBeepPWM);

		if (0 == wBgOffTime){
			btl::backlightEnable(false);
		}
		else {
			btl::backlightEnable(true);
		}
		btl::backlightTime(wBgOffTime);
		btl::backlightBrightness(wBrightness);
#endif
	}
#ifndef WIN32
	btl::registerBeepCallBack("beepCallBack", beepCallBack, NULL, false);
	btl::registerBacklightCallBack("backLightCallBack", backLightCallBack, NULL, false);
	#endif
}

static void ComCtrlAddrInit() {
	unsigned short      wSrc[256] = { 0 };
	int                 i = 0;
	// 从掉电地址中读取通讯控制位信息
	CRegisterRW::read(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_COMCTRLSAVEADDR, 0, &wSrc, 256);
	// 将掉电地址的值写入原来的通讯控制位地址中
	for (i = 0; i < 16; i++) {
		CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_COMMUCONTRL + i * 16, 0, &wSrc[i * 8], 8);
	}
	for (i = 0; i < 16; i++) {
		CRegisterRW::write(RW_FORMAT_INT, REG_TYPE_HS_, DATA_TYPE_WORD, HSW_COMMUCONTRL2 + i * 16, 0, &wSrc[( i + 16 ) * 8], 8);
	}
}

void hswAddrInit() {
	if (VBOX_MODE != CPlatBoard::getDeviceMode()) {
		LanguageAddrInit();
		BuzzerAndBacklightAddrInit();   // 蜂鸣器、背光灯
		ComCtrlAddrInit();              // 通讯控制位
	}
}