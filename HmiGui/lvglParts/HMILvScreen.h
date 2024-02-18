#ifndef HMILVSCREEN_H
#define HMILVSCREEN_H
#include <lvgl/lvgl/lvgl.h>

#include <memory>
#include <functional>
#include <vector>
#include "ProtoXML/ProtoXML.h"
#include "../lvglParts/LvImgCacheMgr.h"
#include "HMILvPart.h"
#include <string>

using std::vector;
class HMILvPart;

class HMILvScreen 
{
public:
	explicit HMILvScreen(lv_obj_t *src);
	~HMILvScreen();

protected:
	void paintEvent();

public:
	bool CheckInShowBaseScr(const hmiproto::hmiact &act, int nBaseScr);		//������û����
	bool Parse(const hmiproto::hmiact &act, int nDataType);					//��������
	int parseJson(const hmiproto::hmiact &act);
	void SetScrNo(int nScrNo);			// ���û����
	int GetScrNo();						// ��ȡ�����
	int GetScrType();					// ��ȡ��������
	void setScrType(const int type);	// ���û�������
	bool Init();						// ��ʼ�����漰����
	bool IsInit();						// �Ƿ��ʼ����
	void OnScrShow();					// ������ӻ�������ʾ����
	void OnScrHide();					// �����汻�õ���
	void setParent(lv_obj_t *);		// ���ø�����
	void show();
	void move(int x, int y);
	void setFixedSize(int width, int height);
	void setFocus();
	lv_obj_t * getParent();

	void SetShowStates(bool bStates);
	bool GetShowStates();
	void GetScrPopPoint(int &x, int &y);

	void ScaleFrameSize();              // �������Ŵ�С
	static void Event(lv_event_t* event);
	static void SendClickEvent(int nScrNo, int nX, int nY, bool bDown);
private:
	void mousePressEvent(lv_obj_t* obj, lv_event_t* event);
	void mouseReleaseEvent(lv_event_t *eventT);

private:
	template<class  PART, class  STRUCT>
	void ParsePart(STRUCT T, int nDataType);

public:
private:
	hmiproto::hmiscreen m_ScrInfo;		// ������Ϣ
	static int                 m_nScrNo;		// �����
	lv_obj_t*			m_Screen;
	int					m_ScrNo; 
	unsigned short      m_wScrType;		// ��������
	vector< HMILvPart* >	m_partlist;

	bool                m_bClick;		// �Ƿ��е��
	bool                m_bShow;        // �Ƿ�����ʾ
	// ʹ�þ�̬������¼��������ꡢ����ţ���̧֤��͵�������ꡢ����һ�� xqh 20200619
	static int          m_x;
	static int          m_y;
	static int          m_nClickScrNo;
#ifdef _LINUX_
	static unsigned long long     m_lastClick;   //���һ�λ�ȡ����ʱ��
#endif
	int       m_nImgIndex;				// ͼƬ���
};
#endif

