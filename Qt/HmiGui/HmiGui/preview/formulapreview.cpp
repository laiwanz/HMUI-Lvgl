#include "formulapreview.h"
#include "HmiGui/formula/cwecontable.h"

int GetFormulaPreview(formulaSpace::FORMULASTYLE &style, const std::string & sSavePath, const formulaSpace::formulaLanguageType languageType)
{
    if(sSavePath == "") {

        return -1;
    }

	style.m_nTransparent = style.m_nTransparent / 100.0 * 255;

    CWeconTable table(style);

	QString sTempStr;

	CWeconTable::LanguageType tableLanguageType = CWeconTable::ChineseLanguage;

	if (formulaSpace::Language_Chinese == languageType) {
	
		
		sTempStr = QString::fromLocal8Bit("成分");
	} else { 
	
		tableLanguageType = CWeconTable::EnglishLanguage;
		sTempStr = "Elem";
	}
	table.setLanguageType(tableLanguageType);
	for (size_t i = 0; i < 30; i++)
	{
		table.addHeadData(sTempStr + QString::number(i), Qt::Horizontal);
	}

	if (formulaSpace::Language_Chinese == languageType) {

		sTempStr = QString::fromLocal8Bit("组");
	}
	else {

		sTempStr = "Group";
	}
	for (size_t i = 0; i < 30; i++)
	{
		table.addHeadData(sTempStr + QString::number(i), Qt::Vertical);
	}

    table.getPreview(QString::fromStdString(sSavePath.c_str()));
    return 0;
}
