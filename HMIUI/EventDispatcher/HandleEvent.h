#ifndef _HANDLEEVENT_H_20201020
#define _HANDLEEVENT_H_20201020
#include "macrosplatform/event.h"
#ifndef WITH_UI
#include <functional>
using EXIT_HMIUI = std::function<int()>;
int setEventConfig(EXIT_HMIUI funcExit);
#endif
bool HmiEvent_Do(EVENT_T& event); 
#endif