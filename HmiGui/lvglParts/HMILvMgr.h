#ifndef HMILVMGR_H
#define HMILVMGR_H
#include <lvgl/lvgl/lvgl.h>
#include <string>
#include <vector>
#include "HMILvScreen.h"

using std::string;
// 


class HMILvMgr
{
public:
	HMILvMgr();
	~HMILvMgr();
protected:

public:
	void Init(uint32_t width, uint32_t height);
	void ImgCache(const void *pListImg, string strPath);//��ʼ��ͼƬ��Ϣ
	void InitScrPart(const hmiproto::hmiact &act, lv_obj_t* obj);
	HMILvScreen* GetTopBaseScr();
	HMILvScreen* GetTopScr();

	void sendQuicklyDoEventSignal(hmiproto::hmiact& act);
	HMILvScreen* GetRealTopScr();
	void quitScreenSaver();
	int parseCustomJson(hmiproto::hmiact& act);
private:
	bool    CheckInShow(int nScrNo);                // �ж��Ƿ��Ѿ�����ʾ��
	void    ChangeScreen(hmiproto::hmiact& act);    // �л�����
	bool    ScreenInCache(const int& nScrNo);       // �����Ƿ��ڻ���
	void    ShowScreenInCache(const int& nScrNo);   // ��ʾ�����еĻ���
	void    PopChildScreen(hmiproto::hmiact& act);  // �����ӻ���
	void    CloseChildScr(hmiproto::hmiact& act);   // �ر��ӻ���
	void    CloseOpenedScreen();                    // �رմ򿪵Ļ���
	void    ShowIndirectcreen(hmiproto::hmiact& act);// ��ʾ��ӻ���
	void    ShowCursor(hmiproto::hmiact& act);      // ��ʾ���
	void    Screenshot(hmiproto::hmiact& act);      // ��ͼ
	void    RefreshScreen();                        // ˢ�»���
	void    SendCursorPos(hmiproto::hmiact& act);	// �������λ�ø�Hmi
	void    SetCursorPos(hmiproto::hmiact& act);    // �������λ��
	void    FreeScreenCache(hmiproto::hmiact& act); // �ͷŻ��滺��
	void	raiseMsgTip();							// ���¼�����Ϣ��ʾ��

private:
	void	keyPressEvent(lv_event_t* eventT);	// ���������¼�
	void	keyReleaseEvent(lv_event_t *eventT); // ����̧���¼�
	void	UpdatePart(const hmiproto::hmiact &act);
	void	clearLcd(hmiproto::hmiact& act);
	lv_obj_t* getParent(const hmiproto::hmiact& act);
	void	showMsgTip(hmiproto::hmiact &act);
	bool    msgTipIsHide();
	int     updateMsgTip(bool bHide);
	//int     showScreenSaver(CScreenSaver::TYPE_T type);
	int		showSysSetScreen(int nScreen);
	bool    screenSaverIsHide();
	int     updateScreenSaver(bool bHide);
	void	SendUpdateTopScr();
	int		runSyssetScreenFilter(unsigned int nEventType);

private:
	void Slot_QuicklyDoEvent(hmiproto::hmiact& act);


private:
	vector<HMILvScreen *>			m_ScreenList;
	unsigned int				m_dwMemSize;
	lv_obj_t*					m_pClrLcdScr;
	// CMsgTip*					m_msgTip;
	//CScreenSaver*               m_pScreenSaver;      /* �������� */
	bool						m_bSyssetScreenRunning;

};

void test();

#endif
