#ifndef _COMMON_H__20220829
#define _COMMON_H__20220829

typedef enum {
    LEFT_TO_RIGHT = 0,
    RIGHT_TO_LEFT = 1,
    TOP_TO_BOTTOM = 2,
    BOTTOM_TO_TOP = 3,
} DIRECT_T;

int genRandomNumber(int nMin, int nMax);

#endif
