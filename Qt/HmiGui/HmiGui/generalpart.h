#ifndef GENERALPART_H
#define GENERALPART_H

#include <QObject>
#include <QList>
#include "hmipart.h"
#include "hmidef.h"
#include "commonpart.h"
#include "ProtoXML/ProtoXML.h"
#include "qrlabel.h"

class GeneralPart : public QObject, public HMIPart {
	typedef void ( GeneralPart::*DRAW_FUN )( QWidget *, int );
	Q_OBJECT
		enum {
		NORMALPART,
		QRPART,
		ROTATEPIC
	};
public:
	explicit GeneralPart(const hmiproto::hmipartcommon &common, QWidget *parent = nullptr);
	~GeneralPart();
	//初始化数据
	virtual void Init(QWidget * pWidget);
	//收到更新数据
	void Update(const hmiproto::hmipartcommon &common, QWidget * pWidget);
	//初始化重绘
	virtual void Draw(QWidget * pWidget, int drawtype = DRAW_INIT);
	//隐藏画面时触发
	virtual void OnPartHide();
	//从缓存中显示出来
	virtual void OnPartShow();
#if ((defined WIN32) && (defined WIN32SCALE))
	//比例缩放
	virtual void ZoomPartSelf();
#endif
private:
	void DrawImg(QWidget * pWidget, int drawtype = DRAW_INIT);
	void DrawTxt(QWidget * pWidget, int drawtype = DRAW_INIT);
	void DrawShowHide(QWidget * pWidget, int drawtype = DRAW_INIT);
	void DrawArea(QWidget * pWidget, int drawtype = DRAW_INIT);
	void DrawGlint(QWidget * pWidget, int drawtype = DRAW_INIT);
	void DrawRotatePic(QWidget * pWidget, int drawtype = DRAW_INIT);
	void DrawUpdate(QWidget * pWidget, int drawtype = DRAW_INIT);
	void DoShowHide(bool bShow);
	void GetDrawFunc(int drawtype);
	void DrawSelect(QWidget * pWidget, int drawtype);
private slots:
	void DoGlint();
private:
	hmiproto::hmipartcommon		m_part;
	CommonPart          *m_commandpart;
	QTimer              *m_glintTimer;
	QRLabel             *m_qrlabel;
	int                 m_parttype;
	unsigned long       m_TxtCrc32;             //文本校验和
	RECT                m_rc;					//轮廓
	bool                m_bUseShowHide;			//是否隐藏
	bool                m_bUseGlint;			//是否闪烁
	int                 m_nImgIndex;            //图片序号
	unsigned int		m_nLockImgIndex;		//锁图片序号
	int					m_nLockWidth;			//锁的大小
	bool				m_bIsShowGrayScale;		//灰阶
	QString             m_strPartType;          //部件类型
	QString				m_strImgPath;			//位图预览图片路径
	bool				m_bUpdateImg;
	bool				m_bUpdateText;
	bool				m_bUpdateHide;
	bool				m_bUpdateGlint;
	bool				m_bUpdateArea;
	QList<DRAW_FUN>		m_drawList;
};


#endif // GENERALPART_H
