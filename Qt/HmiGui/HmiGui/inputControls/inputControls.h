#ifndef _INPUTCONTROLS_H_20231103_
#define _INPUTCONTROLS_H_20231103_
#include "../hmipart.h"
#include "./inputLineEdit.h"
#include "./inputTextEdit.h"
#include "../common/backgroundWidget.h"
#include <QtWidgets/QTextEdit>
#include "ProtoXML/ProtoXML.h"
class CInputControls : public QWidget, public HMIPart
{
	Q_OBJECT
public:
	explicit CInputControls(const hmiproto::hmipartinput &partInput, QWidget *parent = nullptr);
	~CInputControls();
	virtual void Init(QWidget *pWidget);
	virtual void Update(const hmiproto::hmipartinput &partInput, QWidget *pWidget);
	virtual void Draw(QWidget *pWidget, int drawtype = DRAW_INIT);
	virtual int parseJson(const std::string &sbuffer);
#if ((defined WIN32) && (defined WIN32SCALE))
	virtual void ZoomPartSelf();
#endif
private:
	int setInputText(const QString &sText);
	int setInputHide(bool bHide);
	int setInputGeometry(const QRect &rc);
	QRect getInputGeometry();
	bool isInputReadOnly();
	int setInputReadOnly(bool enabled);
	int setInputFont(const QString &sFont);
	int setInputAlignment(unsigned int nHorizontalAlign, unsigned int nVerticalAlign);
	int setInputTextColor(unsigned int nColor);
	int setInputTextColor(const QColor &color);
	int setInputFocus();
	int setLockImage(const std::string &sPath);
	int selectInputAll();
	int setConfirmFlag(bool bConfirmFlag, unsigned int nTimeout);
	int setLanguage(unsigned int nLanguage);
	int saveLastText(const QString &sbuffer);
	bool hasInputFocus();
private:
	CInputLineEdit *m_pLineEdit;
	CInputTextEdit *m_pTextEdit;
	QWidget	*m_pLockWidget;
	CBackgroundWidget *m_pBackGroundWidget;
	unsigned int m_nScreenNo;
	bool m_bTextGray;
	unsigned int m_nTextColor;
	int m_nLockImgIndex, m_nBackgroundImg;
	unsigned int m_nLockWidth;
	bool m_bLoadLockImg;
	std::string m_sBackgroundImg;
#if ((defined WIN32) && (defined WIN32SCALE))
	QRect m_rect;
#endif
};
#endif