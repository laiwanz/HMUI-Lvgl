#ifndef _COMBOBOX_H_20211130_
#define _COMBOBOX_H_20211130_
#include <QObject>
#include "hmipart.h"
#include <QRect>
#include <QComboBox>
#include <QPushButton>
#include <QStyledItemDelegate>
#include "ProtoXML/ProtoXML.h"
#include "hmimgr.h"

class CComboBox : public QComboBox, public HMIPart {
	Q_OBJECT
public:
	explicit CComboBox(const hmiproto::hmidownlist &downlist, QWidget *parent = nullptr);
	~CComboBox();
	//初始化数据
	virtual void Init(QWidget * pWidget);
	//收到更新数据
	virtual void Update(const hmiproto::hmidownlist &downlist, QWidget * pWidget);
	//绘图
	virtual void Draw(QWidget * pWidget, int drawtype = DRAW_INIT);
	//从缓存中显示出来
	virtual void OnPartShow();
	//隐藏画面时触发
	virtual void OnPartHide();
#if ((defined WIN32) && (defined WIN32SCALE))
	virtual void ZoomPartSelf();
#endif
signals:
	void onClickdown(QMouseEvent *eventT, int nX, int nY);		//鼠标按下消息
	void onClickup(QMouseEvent *eventT, int nX, int nY);		//鼠标抬起消息
private:
	void DrawDownList(int drawtype);
	void SetDownListColor();
	bool AllowChanged();
	void showPopup();
protected:
	void mouseReleaseEvent(QMouseEvent *eventT);
	void mousePressEvent(QMouseEvent *eventT);
private slots:
	void DownListIndexChanged(int nIndex);
	void SetShowRang(int);
    int getItemHeight();

private:
	hmiproto::hmidownlist m_part;
	QRect               m_Rect;
	QString             m_strPartType;
	int                 m_nScrNo;
	QPushButton			*m_comboxbtn;
	QStringList			strItemList;
	QString				m_comboxFontStyle;
    QStyledItemDelegate	*m_QStyledItemDelegate;
};

#endif // _COMBOBOX_H_20211130_