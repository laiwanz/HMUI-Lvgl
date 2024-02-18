#ifndef _KEYBOARD_H_xqh20190124
#define _KEYBOARD_H_xqh20190124
#include <QKeyEvent>
#include "hmidef.h"
#include "EventMgr/EventMgr.h"
#define KEY_NUM		19		// 按键数量
#define NUM_Count	11	

// qt中键盘键值与Linux中键值
typedef struct tagKeyValTable {
	int nQtVal;		// qt中的键值
	int	nDevVal;	// linux中的键值
}KEYVALTABLE, *LPKEYVALTABLE;

// 根据qt的键值获取linux中的键值
int GetKeyVal(int nQtKey);
//数字键盘组合键
void DigitalCompositeKey(int EventType, QKeyEvent *event);
#endif