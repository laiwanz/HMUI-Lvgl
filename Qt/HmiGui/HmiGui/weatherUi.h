#ifndef WEATHER_H__20230303
#define WEATHER_H__20230303

#include <QWidget>
#include <QListView>
#include <QStringListModel>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsOpacityEffect>
#include "generalpart.h"
#include "QImageZCache.h"
#include "SVGParts/SVGPartsDef.h"

class CWeatherUi :public QWidget, public HMIPart {
	Q_OBJECT
public:
	explicit CWeatherUi(const hmiproto::hmiweather &weather, QWidget *parent = nullptr);
	~CWeatherUi();
	//初始化数据
	virtual void Init(QWidget * pWidget);
	//收到更新数据
	virtual void Update(const hmiproto::hmiweather &weather, QWidget * pWidget);
	//绘图
	virtual void Draw(QWidget * pWidget, int drawtype = DRAW_INIT);
#if ((defined WIN32) && (defined WIN32SCALE))
	virtual void ZoomPartSelf();	//缩放
#endif
signals:
	void sendWeatherOnclickedDown(QMouseEvent *eventT, int nX, int nY);		//鼠标按下消息
	void sendWeatherOnclickedUp(QMouseEvent *eventT, int nX, int nY);		//鼠标抬起消息
protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *eventT);
	void mouseReleaseEvent(QMouseEvent *eventT);
private:
	int updateBackground(const hmiproto::hmiweather &weather);
	int updateSeltUi(const hmiproto::hmiweather &weather);
	int getDesignedConfig(const hmiproto::hmiweather &weather);
private slots:
	void slotSelectItem(const QModelIndex &index);
	void slotBtnCancel();
private:
	RECT				 m_rc;
	int					 m_nScrNo;
	hmiproto::hmiweather m_part;	/* weather config info */
	/* Qt ui */
	QListView*			m_listView;
	QStringListModel*	m_model;
	QLabel*				m_labProvince;
	QLabel*				m_labCity;
	QLabel*				m_labHeadBg;
	QPushButton*		m_btnCancel;
	QImageZCache		m_imageCache;

	WEATHER_DESIGNED_CONFIG_T m_config;
	CommonPart      *m_svg;
};



#endif // WEATHER_H__20230303