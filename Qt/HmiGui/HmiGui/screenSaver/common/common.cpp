#include "common.h"
#include <time.h>
#include <stdlib.h>

int genRandomNumber(int nMin, int nMax) {

    static bool bInit = false;

    if (false == bInit) {
        bInit = true;
        srand(time(NULL));
    }

    if (nMax < nMin) {
        int nTemp = nMax;
        nMax = nMin;
        nMin = nTemp;
    }

    return rand() % (nMax - nMin + 1) + nMin;
}
