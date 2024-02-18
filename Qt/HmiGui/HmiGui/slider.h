#ifndef SLIDER_H
#define SLIDER_H
#include <QLabel>
#include <QSlider>
#include <QMouseEvent>
#include "generalpart.h"

#define SPACERLEN	20
class CSlider :public QSlider, public HMIPart {
	Q_OBJECT
		enum {
		LeftToRight = 0,
		RightToLeft,
		TopToButtom,
		ButtomToTop,
	};

#define  MINCOTRLSIZE 40
public:
	explicit CSlider(const hmiproto::hmislider &slider, QWidget *parent = nullptr);
	~CSlider();
	//初始化数据
	virtual void Init(QWidget * pWidget);
	//收到更新数据
	virtual void Update(const hmiproto::hmislider &slider, QWidget * pWidget);
	//绘图
	virtual void Draw(QWidget * pWidget, int drawtype = DRAW_INIT);
#if ((defined WIN32) && (defined WIN32SCALE))
	virtual void ZoomPartSelf();	//缩放
#endif
signals:
	void sendSliderOnclickedDown(QMouseEvent *eventT, int nX, int nY);	//鼠标按下消息
	void sendSliderOnclickedUp(QMouseEvent *eventT, int nX, int nY);		//鼠标抬起消息
public slots:
	void EndTheSlide();										//滑块抬起事件处理函数
	void SliderMove(int nValue);							//滑块移动事件处理函数
protected:
	bool CheckIsSlide(QMouseEvent *eventT);					//判断是否需要移动
	int GetHorMoveWidth(int nValue);						//获取水平滑块移动x距离
	int GetHorMoveHeight();									//获取水平滑块移动y距离
	int GetVerMoveHeight(int nValue);						//获取竖直滑块移动y距离
	int GetVerMoveWidth();									//获取竖直滑块移动x距离
	void DrawImg(QWidget * pWidget);						//画背景图
	void ChangeCurValue(int nValue);							//滑动处理
	void IsShowUseBgColor();								//判断是否显示背景图片
	void InitValueDisplayLabel();							//设置值显示标签
	void InitSliderStyle();									//设置滑动条初始化样式
	void DoDIsplyCurValueMove(int nValue);					//当前值显示移动
	void DoSliderAlarmColor(int nValue);						//处理警戒色
	void ParseLabelFontSize();								//解析字体样式
	void GetHandleAddress(int nValue);						//获取滑块位置
	void InitLableStyle(QLabel *label);						//初始化字体样式
	void DoScrollControl(QMouseEvent *eventT);				//卷动控制
	void mousePressEvent(QMouseEvent *eventT);				//鼠标按下处理函数
	void mouseMoveEvent(QMouseEvent *eventT);				//鼠标移动处理函数
	void mouseReleaseEvent(QMouseEvent *eventT);				//鼠标抬起处理函数
	QString	CutDataStrByDigitCount(int nValue);				//将int转Qstring显示函数
	bool AllowChanged();
private:
	bool				m_bWaring;						//是否设置能设置警戒色 0 不能 1 能
	bool				m_bNormal;						//是否设置能设置正常色 0 不能 1 能
	bool				m_bMoveState;					//是否是移动状态 0 不是 1 是
	bool				m_bMove;						//滑是否移动 0不是 1 是
	int					m_nClickedDownStartHorPos;		//起始点x坐标
	int					m_nClickedDownStartVerPos;		//起始点y坐标
	int					m_nClickedDownstartValue;		//起始点移动值
	int					m_nLableFontSize;				//显示字体大小
	int					m_nMarginSize;					//水平滑块上下边界值
	int					nLeft;							//部件x坐标
	int					nTop;							//部件y坐标
	int					nWidth;							//宽度
	int					nHeight;						//高度
	int                 m_nScrNo;						//画面号
	unsigned int		m_dwSlideraddColor;				//滑轨未滑过的颜色颜色
	unsigned int		m_dwSlidersubColor;				//滑轨滑过的颜色
	long long			m_nSliderX;						//滑块坐标x值
	long long			m_nSliderY;						//滑块坐标Y值
	QString				m_isBgColor;					//是否有背景色 ""有 "0" 无
	QString				m_isSlideColor;					//是否有滑轨色 ""有 "0" 无
	QString				m_isBorderColor;				//是否有边框颜色 ""有 "0" 无
	QString				m_isShapColor;					//是否有边框颜色 ""有 "0" 无
	QString				m_stylesheet;					//当前不讲样式
	CommonPart			*m_commandpart;					//背景图指针
	RECT				rectTemp;						//坐标
	QFont				displayLabelFt;					//字体类型
	QLabel				*m_displayLabel;				//跟踪标签
	QLabel				*m_minLabel;					//最小值标签
	QLabel				*m_maxLabel;					//最大值标签
	hmiproto::hmislider m_part;							//滑动开关属性
};
#endif