#ifndef _FORMULACONFIG_H_
#define _FORMULACONFIG_H_

#include <iostream>
#include "tableconfig.h"

namespace formulaSpace {

	enum FormulaFuncType {
		Formula_Add = 0,          //add group data
		Formula_Delete = 1,          //delete group data
		Formula_Change = 2,          //change formula
		Formula_Upload = 4,          //upload one group data to address
		Formula_Download = 5,          //download one group data to address
		Formula_LeadingIn = 6,          //leading in formula to hmi
		Formula_LeadingOUt = 7,          //leading out formula for dev
		Formula_Max
	};
}

#endif
