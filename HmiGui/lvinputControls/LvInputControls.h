#ifndef LV_INPUT_CONTROLS_H
#define LV_INPUT_CONTROLS_H

#include "../HmiGui/hmidef.h"
#include "../lvglParts/HMILvPart.h"
#include "../lvglParts/HMILvScreen.h"
#include "./lvinputControls/LvInputLineEdit.h"
#include "./lvinputControls/LvInputTextEdit.h"
#include "ProtoXML/ProtoXML.h"

using std::string;

class LvInputControls : public HMILvPart {

public:
	explicit LvInputControls(const hmiproto::hmipartinput &partInput, HMILvScreen*parent = nullptr);
	virtual ~LvInputControls();
	virtual void Init(lv_obj_t* scr);

	virtual void Update(const hmiproto::hmipartinput &partInput, HMILvScreen * parent);

	virtual void Draw(HMILvScreen * parent, int drawtype = DRAW_INIT);
	virtual int parseJson(const std::string &sbuffer);

#if ((defined WIN32) && (defined WIN32SCALE))
	virtual void ZoomPartSelf();
#endif
private:

	int setInputText(const string &sText);
	int setInputHide(bool bHide);
	int setInputGeometry(int nX, int nY, int nWidth, int nHeight);
	void setGeometry(int nX, int nY, int nWidth, int nHeight);
	bool isInputReadOnly();
	int setInputReadOnly(bool enabled);
	int setInputFont(const string &sFont);
	int setInputAlignment(unsigned int nHorizontalAlign, unsigned int nVerticalAlign);
	int setInputTextColor(unsigned int nColor);

	int setInputFocus();
	int setLockImage(const std::string &sPath);
	int selectInputAll();
	int setConfirmFlag(bool bConfirmFlag, unsigned int nTimeout);
	int setLanguage(unsigned int nLanguage);
	int saveLastText(const string &sbuffer);
	bool hasInputFocus();

private:
	LvInputLineEdit *m_pLineEdit;
	LvInputTextEdit *m_pTextEdit;
	lv_obj_t		*m_BackgroundImg;

	unsigned int m_nScreenNo;
	bool m_bTextGray;
	unsigned int m_nTextColor;
	int m_nLockImgIndex, m_nBackgroundImg;
	unsigned int m_nLockWidth;
	bool m_bLoadLockImg;
	std::string m_sBackgroundImg;

};


#endif 
