#ifndef _CONTROLPREVIEW_H_
#define _CONTROLPREVIEW_H_

#include "../config/formulaconfig.h"

int CONTROLPREVIEW_OUT GetFormulaPreview(formulaSpace::FORMULASTYLE & style, const std::string & sSavePath, const formulaSpace::formulaLanguageType languageType = formulaSpace::Language_Chinese);

#endif
