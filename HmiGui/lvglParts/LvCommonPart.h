#ifndef LVCOMMONPART_H
#define LVCOMMONPART_H
#include "ProtoXML/ProtoXML.h"
#include "macrosplatform/macrosplatform.h"
#include "HMILvPart.h"
#include "../lvglParts/LvImgCacheMgr.h"
#include <string>
using std::string;

typedef struct LvFONTINFO {
	char	szFontFamily[MAX_FontTypeStrLen];	//������
	int		nFontSize;							//�ֺ�
	int		nWeight;							//75-�Ӵ� 50-����
	int		nStyle;								//1-б�� 0-����
}LvFONTINFO;

class LvCommonPart {
public:
	explicit LvCommonPart(lv_obj_t * scr, string parttype = "");
	virtual ~LvCommonPart();
public:
	void LvsetRoateRect(int x, int y, int width, int height);
	void LvsetGeometry(int x, int y, int width, int height);

	void paintEvent(); 
	void setRotate(int degress);										//��ת�Ƕ�
	int	 getRotate();													//��ȡ�Ƕ�
	void SetIMGIndex(const int &nIndex);		
	void SetIMGPath(const string &strImgPath, const RECT &rc);			//�����ļ�·��
	void setText(const hmiproto::hmipartcommon common, int drawtype);						//�����ı���Ϣ
	void setSvg(char *contents, const std::string &strPartName, bool bAll);
	void setImg(const string ImgPath, RECT rc);
	void OnPartHide();
	void OnPartShow();
	void setFreq(int nFreq);
	int  getFreq();
	bool FindFontInfo(int nWeight, int nStyle, string &strFontFamily, string strFontSize); //���������������Ƿ���ڸ�������Ϣ
	static void	ReadQpf2File();															  //��ȡqpf2�ļ�ͷ��Ϣ
	void setLockImgInfo(int nLockImgIndex, int nLockWidth, bool bIsShowGrayScale);		  //������ͼƬ��Ϣ
	void CalculateTextArea();
	void setPos(RECT rect);
	void setSelect(bool bShowSelect, int nSelectMode);
	bool getSelect();
	int getSelectMode();

	virtual void setGeometry(int x, int y, int width, int height);
public:
	void drawImg();
	void drawText();
	void drawImg_NORMAL();
	


private:
	int				m_nImgIndex;
	string			m_strImgPath;
	RECT			m_Rect;
	lv_obj_t*		m_Screen;
	lv_obj_t*		m_Part;
	lv_obj_t*		m_Text;
	lv_obj_t		*m_pCustomImage;
	LvIMG_T*		m_pImgInfo;
	lv_obj_t		*m_svg;					//svg
	lv_obj_t		*m_movie;				//gif
	lv_obj_t		*m_pSvgImage;
	RECT			m_SvgRect;	
	RECT			m_roateRect;
	string			m_strGifpath;			//gif·�����������·���ǲ�һ���Ĳ���������
	int				m_nLockImgIndex;		//����ͼƬ���
	int				m_nLockWidth;			//���Ĵ�С
	bool			m_bIsShowGrayScale;		//�Ƿ�ҽ�

public:
	static unsigned short		m_wQpf2fFileVersion;	//qpf2�ļ��汾
	static int		m_nQpf2fFileCount;		//qpf2�ļ�����
	static LvFONTINFO	*m_Qpf2Info;			//qpf2�ļ���Ϣ
	//�ı���Ϣ
private:
	int				m_nFreq;
	int				m_degrees;
	bool			m_bRoate;
	int				m_txtFlags;
	lv_color_t			m_txtColor;
	lv_color_t			m_txtBkColor;
	bool			m_bTxtColor;
	bool			m_bTxtBkColor;
	string			m_parttype;
	//QMutex			m_svgmutex;
	int             m_nAlign;
	bool			m_bShowSelect;
	int				m_nSelectMode;
public:
	string			m_text;
	//lv_font_t		m_txtFont;
	RECT			m_txtRect;
	RECT			m_txtBgRect;

#if ((defined WIN32) && (defined WIN32SCALE))
	RECT			m_OriginalTextRect; //ԭʼ������ʾ����
	//QFont			m_OriginalTxtFont;  //ԭʼ����

#endif
};
#endif // !LVCOMMONPART_H

