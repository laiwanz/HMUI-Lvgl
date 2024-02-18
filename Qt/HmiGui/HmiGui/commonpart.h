#ifndef COMMONPART_H
#define COMMONPART_H
#include <QWidget>
#include <QMutex>
#include "imgcachemgr.h"
#include "ProtoXML/ProtoXML.h"
#include "macrosplatform/macrosplatform.h"

typedef struct FONTINFO {
	char	szFontFamily[MAX_FontTypeStrLen];	//字体名
	int		nFontSize;							//字号
	int		nWeight;							//75-加粗 50-常规
	int		nStyle;								//1-斜体 0-常规
}FONTINFO;

class CommonPart : public QWidget {
	Q_OBJECT
public:
	explicit CommonPart(QWidget *, QString parttype = "");
	~CommonPart();
	virtual void paintEvent(QPaintEvent *event);
public:
	void setRoateRect(QRect);
	void setRotate(int degress);										//旋转角度
	int	getRotate();													//获取角度
	void SetIMGIndex(const int &nIndex);								//设置图片编号
	void SetIMGPath(const QString &strImgPath, const RECT &rc);			//设置文件路径
	void setText(const hmiproto::hmipartcommon common, int drawtype);						//设置文本信息
	void setSvg(const QByteArray &contents, const std::string &strPartName, bool bAll);
	void setImg(const QByteArray &contents, RECT rc);
	void OnPartHide();
	void OnPartShow();
	void setFreq(int nFreq);
	int  getFreq();						//获取帧率
	bool FindFontInfo(int nWeight, int nStyle, QString &strFontFamily, QString strFontSize); //查找链表中数据是否存在该字体信息
	static void	ReadQpf2File();															  //读取qpf2文件头信息
	void setLockImgInfo(int nLockImgIndex, int nLockWidth, bool bIsShowGrayScale);		  //设置锁图片信息
	void CalculateTextArea();
#if ((defined WIN32) && (defined WIN32SCALE))
	void GetTextRect(const hmiproto::hmipartbasic &basic, QRect* rectTemp);//获取文本显示区域
#endif
	void setSelect(bool bShowSelect, int nSelectMode);
	bool getSelect();
	int getSelectMode();
private:
	void drawImg(QPainter& p);
	void drawText(QPainter& p);
	void drawImg_SVG(QPainter& p);
	void drawImg_GIF(QPainter& p);
	void drawImg_NORMAL(QPainter& p);
	void drawImg_LOCK(QPainter& p);
	void calculateStaticTextArea();
	void calculateCustomTextArea();
	void setAlign(int nAlign);
private slots:
	void svgRepaintNeeded();
	void movieUpdated(const QRect& rc);
private:
	int				m_nImgIndex;
	QString			m_strImgPath;
	IMG_T			*m_pImgInfo;
	QImage			*m_pCustomImage;
	RECT			m_Rect;
	QSvgRenderer	*m_svg;					//svg
	QMovie			*m_movie;				//gif
	QImage			*m_pSvgImage;
	QRect			m_SvgRect;
	QRect			m_roateRect;
	QString			m_strGifpath;			//gif路径，如果两次路径是不一样的才重新设置
	int				m_nLockImgIndex;		//锁的图片序号
	int				m_nLockWidth;			//锁的大小
	bool			m_bIsShowGrayScale;		//是否灰阶
public:
	static unsigned short		m_wQpf2fFileVersion;	//qpf2文件版本
	static int		m_nQpf2fFileCount;		//qpf2文件个数
	static FONTINFO	*m_Qpf2Info;			//qpf2文件信息
//文本信息
private:
	int				m_nFreq;
	int				m_degrees;
	bool			m_bRoate;
	int				m_txtFlags;
	QColor			m_txtColor;
	QColor			m_txtBkColor;
	bool			m_bTxtColor;
	bool			m_bTxtBkColor;
	QString			m_parttype;
	QMutex			m_svgmutex;
	int             m_nAlign;
	bool			m_bShowSelect;
	int				m_nSelectMode;
public:
	QString			m_text;
	QFont			m_txtFont;
	QRect			m_txtRect;
	QRect			m_txtBgRect;
#if ((defined WIN32) && (defined WIN32SCALE))
	QRect			m_OriginalTextRect; //原始字体显示区域
	QFont			m_OriginalTxtFont;  //原始字体
#endif
};
#endif // COMMONPART_H