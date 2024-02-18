#include "keyBoard.h"
#include <QKeyEvent>

enum KeyValueTable {
	WEKEY_PAUSE = 69,	//PauseBreak
	WEKEY_KPENTER = 284,	//Enter 回车（小数字键盘）
	WEKEY_RIGHTCTRL = 285,	//右 Ctrl
	WEKEY_KPSLASH = 309,	// /（小数字键盘）
	WEKEY_RIGHTALT = 312,	//右 Atl
	WEKEY_NUMLOCK = 325,	//NumLock
	WEKEY_HOME = 327,	//Hmoe
	WEKEY_UP = 328,	//方向上
	WEKEY_PAGEUP = 329,	//PageUp
	WEKEY_LEFT = 331,	//方向左
	WEKEY_RIGHT = 333,	//方向右
	WEKEY_DOWN = 336,	//方向下
	WEKEY_PAGEDOWN = 337,	//PageDown
	WEKEY_INSERT = 338,	//Insert
	WEKEY_DELETE = 339,	//Delete
	WEKEY_END = 335,	//End
	WEKEY_LEFTMETA = 347,	//左 Windows
	WEKEY_RIGHTMETA = 348,	//右 Windows
	WEKEY_COMPOSE = 349,	//菜单
	WEKEY_LEFTSHIFT = 42,	//左 Shift
	WEKEY_RIGHTSHIFT = 54,	//右 Shift
	WEKEY_KP7 = 71,	//7（小数字键盘）
	WEKEY_KP8 = 72,	//8（小数字键盘）
	WEKEY_KP9 = 73,	//9（小数字键盘）
	WEKEY_KPMINUS = 74,	//-（小数字键盘）
	WEKEY_KP4 = 75,	//4（小数字键盘）
	WEKEY_KP5 = 76,	//5（小数字键盘）
	WEKEY_KP6 = 77,	//6（小数字键盘）
	WEKEY_KPPLUS = 78,	//+（小数字键盘）
	WEKEY_KP1 = 79,	//1（小数字键盘）
	WEKEY_KP2 = 80,	//2（小数字键盘）
	WEKEY_KP3 = 81,	//3（小数字键盘）
	WEKEY_KP0 = 82,	//0（小数字键盘）
	WEKEY_KPDOT = 83,	//.（小数字键盘）
};

// 键值表
static const KEYVALTABLE keyValTable[KEY_NUM] = {
	{WEKEY_NUMLOCK, 69},		//NumLock
	{WEKEY_KPENTER, 96},		//Enter 回车（小数字键盘）
	{WEKEY_RIGHTCTRL, 97},		//右 Ctrl
	{WEKEY_KPSLASH, 98},		// /（小数字键盘）
	{WEKEY_RIGHTALT, 100},		//右 Atl
	{WEKEY_HOME, 102},			//Hmoe
	{WEKEY_UP, 103},			//方向上
	{WEKEY_PAGEUP, 104},		//PageUp
	{WEKEY_LEFT, 105},			//方向左
	{WEKEY_RIGHT, 106},			//方向右
	{WEKEY_END, 107},			//End
	{WEKEY_DOWN, 108},			//方向下
	{WEKEY_PAGEDOWN, 109},		//PageDown
	{WEKEY_INSERT, 110},		//Insert
	{WEKEY_DELETE, 111},		//Delete
	{WEKEY_PAUSE, 119},			//PauseBreak
	{WEKEY_LEFTMETA, 125},		//左 Windows
	{WEKEY_RIGHTMETA, 126},		//右 Windows
	{WEKEY_COMPOSE, 127},		//菜单
};

int GetKeyVal(int nQtKey) {
	for (int i = 0; i < KEY_NUM; i++) {
		if (keyValTable[i].nQtVal == nQtKey) {
			return keyValTable[i].nDevVal;
		}
	}

	return nQtKey;
}

void DigitalCompositeKey(int nEventType, QKeyEvent *event) {
	int nIndex = 0;
	int nVal = 0;
	static int nPreVal = 0;
	int nNumber[NUM_Count][2] = { {WEKEY_KP7,Qt::Key_Home},{WEKEY_KP8,Qt::Key_Up},{WEKEY_KP9,Qt::Key_PageUp},{WEKEY_KP4,Qt::Key_Left},
	{WEKEY_KP5,Qt::Key_Clear},{WEKEY_KP6,Qt::Key_Right},{WEKEY_KP1,Qt::Key_End},{WEKEY_KP2,Qt::Key_Down},
	{WEKEY_KP3,Qt::Key_PageDown},{WEKEY_KP0,Qt::Key_Insert},{WEKEY_KPDOT,Qt::Key_Delete} };
	if (event == NULL) {
		return;
	}
	if (nEventType == EVENT_KEYDOWN) {
		nVal = GetKeyVal(event->nativeScanCode());
		for (nIndex = 0; nIndex < NUM_Count; nIndex++) {	//按下shift键的同时按下小键盘数字键时UI会发送shift释放消息给HMI，导致shift+小键盘数字键失效，因此手动发送shift按下/释放消息给HMI
			if (nVal == nNumber[nIndex][0] &&
				event->key() == nNumber[nIndex][1] &&
				( nPreVal == WEKEY_LEFTSHIFT ||
				nPreVal == WEKEY_RIGHTSHIFT )) {
				EventMgr::SendKeyBoardEvent(-1, WEKEY_LEFTSHIFT, true);
				EventMgr::SendKeyBoardEvent(-1, nVal, true);
				EventMgr::SendKeyBoardEvent(-1, WEKEY_LEFTSHIFT, false);
				break;
			}
		}
		if (nIndex == NUM_Count) {
			EventMgr::SendKeyBoardEvent(-1, nVal, true);
		}
		nPreVal = nVal;
	}
	else {
		nVal = GetKeyVal(event->nativeScanCode());
		EventMgr::SendKeyBoardEvent(-1, nVal, false);
	}
}