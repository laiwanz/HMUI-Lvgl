#include "slider.h"
#include <cstdlib>
#include <QDebug>
#include <QFile>
#include <QBitArray>
#include <QPainter>
#include <QtCore/qmath.h>  
#include <QtSvg/QSvgRenderer>
#include <QByteArray>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "hmiutility.h"
#include "EventMgr/EventMgr.h"
#include "hmimgr.h"

CSlider::CSlider(const hmiproto::hmislider &slider, QWidget *parent) {
	m_part.Clear();
	m_isSlideColor = "";
	m_isBgColor = "";
	m_isBorderColor = "";
	m_isShapColor = "";
	m_stylesheet = "";
	m_minLabel = NULL;
	m_maxLabel = NULL;
	m_bWaring = true;
	m_bNormal = true;
	m_bMove = false;
	m_displayLabel = NULL;
	m_commandpart = NULL;
	m_bMoveState = false;
	m_nClickedDownStartHorPos = 0;
	m_nClickedDownStartVerPos = 0;
	m_nClickedDownstartValue = 0;
	m_nLableFontSize = 0;
	m_nMarginSize = 0;
	nLeft = 0;
	nTop = 0;
	nWidth = 0;
	nHeight = 0;
	m_nScrNo = 0;
	m_part.MergeFrom(slider);
}

CSlider::~CSlider() {
	if (m_commandpart) {
		delete m_commandpart;
	}

	if (m_minLabel) {
		delete m_minLabel;
	}

	if (m_maxLabel) {
		delete m_maxLabel;
	}

	if (m_displayLabel) {
		delete m_displayLabel;
	}
}

void CSlider::Init(QWidget * pWidget) {
	RECT	rc;

	rc.left = m_part.basic().left();
	rc.right = m_part.basic().right();
	rc.top = m_part.basic().top();
	rc.bottom = m_part.basic().bottom();
	m_sPartName = m_part.basic().name();
	m_nScrNo = m_part.basic().scrno();

	//获取坐标值
	nLeft = rc.left;
	nTop = rc.top;
	nWidth = rc.right - rc.left;
	nHeight = rc.bottom - rc.top;

	//获取前景色和背景色，解决修改WEB颜色的	bug xk 20200324
	m_dwSlidersubColor = m_part.surpluscolor();
	m_dwSlideraddColor = m_part.slidercolor();

	//卷动模式和最小刻度互斥
	if (m_part.scroll()) {
		m_part.set_scale(1);
	}
	this->DrawImg(pWidget);		//绘制背景图
	this->setParent(pWidget);			//指定父窗体
	this->IsShowUseBgColor(); //判断是否设置背景色，为之后样式设置做铺垫
	this->ParseLabelFontSize();	//解析字体样式
	this->setGeometry(nLeft, nTop, nWidth, nHeight); //设置坐标
	this->setMinimum(m_part.minvalue());//设置滑动条控件的最小值
	this->setMaximum(m_part.maxvalue());//设置滑动条控件的最大值
	this->setTracking(m_part.rightnow());//是否立即写入，是则为ture 否则为false
	this->setPageStep(0);
	this->InitValueDisplayLabel();	//显示当前值，最大值，最小值
	this->InitSliderStyle();//设置初始化样式
	this->DoSliderAlarmColor(m_part.curvalue());//设置初始化警戒色
	this->setValue(m_part.curvalue());
	this->ChangeCurValue(m_part.curvalue());
	//关联信号和槽
	 //connect(this,SIGNAL(sliderReleased()),this,SLOT(EndTheSlide())); //松开滑块
	connect(this, SIGNAL(sliderMoved(int)), this, SLOT(SliderMove(int)));//移动滑块
	connect(this, SIGNAL(sendSliderOnclickedDown(QMouseEvent *, int, int)), static_cast<HMIScreen *>( this->parent() ), SLOT(RecvOnclickedDown(QMouseEvent *, int, int)));//按下部件
	connect(this, SIGNAL(sendSliderOnclickedUp(QMouseEvent *, int, int)), static_cast<HMIScreen *>( this->parent() ), SLOT(RecvOnclickedUp(QMouseEvent *)));//抬起部件

#if ((defined WIN32) && (defined WIN32SCALE))
	memcpy(&m_OriginalRect, &rc, sizeof(RECT));
	ZoomPartSelf(); //缩放
#endif
}

void CSlider::Update(const hmiproto::hmislider &slider, QWidget * pWidget) {
	(void) pWidget;
	m_part.MergeFrom(slider);

	if (m_part.has_minvalue()) //最小值地址有变化
	{
		this->setMinimum(m_part.minvalue());
		if (m_minLabel) {
			m_minLabel->setText(CutDataStrByDigitCount(m_part.minvalue()));
			m_minLabel->adjustSize();
			if (m_part.direction() == RightToLeft) {
				m_minLabel->move(this->width() - m_minLabel->width(), this->height() - m_minLabel->height());
			}
		}
	}

	if (m_part.has_maxvalue()) //最大值地址有变化
	{
		this->setMaximum(m_part.maxvalue());
		if (m_maxLabel) {
			m_maxLabel->setText(CutDataStrByDigitCount(m_part.maxvalue()));
			m_maxLabel->adjustSize();
			if (m_part.direction() == LeftToRight) {
				m_maxLabel->move(this->width() - m_maxLabel->width(), this->height() - m_maxLabel->height());
			}
		}
	}


	if (m_displayLabel) {
		this->InitLableStyle(m_displayLabel);
	}

	if (m_minLabel && m_maxLabel) {
		this->InitLableStyle(m_minLabel);
		this->InitLableStyle(m_maxLabel);

		if (m_part.has_direction()) {
			if (m_part.direction() == LeftToRight) {
				m_maxLabel->move(this->width() - m_maxLabel->width(), this->height() - m_maxLabel->height());
			}

			if (m_part.direction() == RightToLeft) {
				m_minLabel->move(this->width() - m_minLabel->width(), this->height() - m_minLabel->height());
			}
		}
	}

	if (!m_bMove) //不滑动时更新
	{
		if (m_part.has_curvalue()) {
			this->setValue(m_part.curvalue());
			this->ChangeCurValue(m_part.curvalue());
		}

		if (m_displayLabel) {
			m_displayLabel->adjustSize();
		}
	}

#if ((defined WIN32) && (defined WIN32SCALE))
	RECT	rc;

	rc.left = m_part.basic().left();
	rc.right = m_part.basic().right();
	rc.top = m_part.basic().top();
	rc.bottom = m_part.basic().bottom();
	memcpy(&m_OriginalRect, &rc, sizeof(RECT));
	ZoomPartSelf();
#endif
}

void CSlider::DrawImg(QWidget * pWidget) {
	if (m_part.basic().imgindex() < 0) //如果没有背景图则直接退出
	{
		return;
	}

	m_commandpart = new CommonPart(pWidget);
	m_commandpart->setGeometry(nLeft, nTop, nWidth, nHeight);
	m_commandpart->SetIMGIndex(m_part.basic().imgindex());
	m_commandpart->update();
}

void CSlider::Draw(QWidget *, int /* =DRAW_INIT */) {
}

#if ((defined WIN32) && (defined WIN32SCALE))
void CSlider::ZoomPartSelf() {
	rectTemp.left = m_part.basic().left();
	rectTemp.right = m_part.basic().right();
	rectTemp.top = m_part.basic().top();
	rectTemp.bottom = m_part.basic().bottom();
	HMIUtility::CountScaleRect(&rectTemp);

	int nLeft = rectTemp.left;
	int nTop = rectTemp.top;
	int nWidth = rectTemp.right - rectTemp.left;
	int nHeight = rectTemp.bottom - rectTemp.top;

	if (NULL != this) {
		this->setGeometry(nLeft, nTop, nWidth, nHeight);
	}

	int nFontSizeH = displayLabelFt.pixelSize();
	int nFontSizeW = displayLabelFt.pixelSize();

	HMIUtility::CountScaleWH(&nFontSizeW, &nFontSizeH);

	if (nFontSizeW > nFontSizeH) {
		displayLabelFt.setPixelSize(nFontSizeW);
		if (m_displayLabel)
			m_displayLabel->setFont(displayLabelFt);
	}
	else {
		displayLabelFt.setPixelSize(nFontSizeH);
		if (m_displayLabel)
			m_displayLabel->setFont(displayLabelFt);
	}
}
#endif

void CSlider::IsShowUseBgColor() {

	if (!m_part.has_bgcolor() || m_part.basic().imgindex() > -1) //是否有设置背景颜色
	{
		m_isBgColor = "0";
	}

	if (!m_part.useborder()) //是否有设置背景颜色
	{
		m_isBorderColor = "0";
	}

	if (!m_part.useslidercolor()) //是否有设置背景颜色
	{
		m_isSlideColor = "0";
	}
}

void CSlider::InitValueDisplayLabel() {
	if (m_part.usecur()) {
		m_displayLabel = new QLabel(this);
		m_displayLabel->setText(CutDataStrByDigitCount(m_part.curvalue()));
		this->InitLableStyle(m_displayLabel);
	}

	if (m_part.showvalue()) {
		m_minLabel = new QLabel(this);
		m_maxLabel = new QLabel(this);
		m_minLabel->setText(CutDataStrByDigitCount(m_part.minvalue()));
		m_maxLabel->setText(CutDataStrByDigitCount(m_part.maxvalue()));
		this->InitLableStyle(m_minLabel);
		this->InitLableStyle(m_maxLabel);
	}
}

void CSlider::InitSliderStyle() {

	if (m_part.direction() == TopToButtom || m_part.direction() == ButtomToTop)  //竖直方向滑动条
	{
		if (m_part.direction() == TopToButtom) //如果是从上到下则
		{
			if (m_minLabel) {
				m_minLabel->move(0, 0);
			}
			if (m_maxLabel) {
				m_maxLabel->move(0, this->height() - m_minLabel->height());
			}

			this->setInvertedAppearance(true);	//设置从上到下显示。
			//颠倒警戒色
			unsigned int tempColor = m_dwSlidersubColor;
			m_dwSlidersubColor = m_dwSlideraddColor;
			m_dwSlideraddColor = tempColor;
		}
		else {
			if (m_minLabel)
				m_minLabel->move(0, this->height() - m_minLabel->height());
			if (m_maxLabel)
				m_maxLabel->move(0, 0);
		}

		//遵循样式设置圆形的规则QSlider::handle:Horizontal中的width - QSlider::groove:Horizontal中的
		//heigth  >= 2*QSlider::handle:Horizontal中的margin
		if (( m_part.width() - m_part.railheight() ) % 2) {
			m_nMarginSize = ( m_part.width() - m_part.railheight() ) / 2 + 1;
		}
		else {
			m_nMarginSize = ( m_part.width() - m_part.railheight() ) / 2;
		}

		this->setOrientation(Qt::Vertical);//设置滑动条 方向

		if (m_displayLabel)
			m_displayLabel->move(GetVerMoveWidth(), GetVerMoveHeight(0));

		m_stylesheet = QString("\
							 QSlider{border:%1px solid #%2;}\
							 QSlider::groove:vertical{border:0px;width:%3px;height:%4px;}  \
							 QSlider::sub-page:vertical{background:#%6;border-top-left-radius:%5px;border-top-right-radius:%5px;} \
							 QSlider::add-page:vertical{background:#%7;border-bottom-left-radius:%5px;border-bottom-right-radius:%5px;}  \
							 QSlider::handle:vertical{background:#%8;height:%9px;border-radius:%10px;margin:-%12px -%11px -%12px -%11px;}\
							 ")
			.arg(m_part.useborder())
			.arg(QString("%1 %2").arg(QString::number(m_part.bordercolor(), 16).rightJustified(6, QLatin1Char('0'))).arg(m_isBorderColor))
			.arg(QString::number(m_part.railheight()))
			.arg(QString::number(this->height() - m_part.height()))
			.arg(QString::number(m_part.railheight()*m_part.railtype() / 2))//此处-1解决滑轨高度除2成小数舍去导致圆角不圆的BUG
			.arg(QString("%1 %2").arg(QString::number(m_dwSlideraddColor, 16).rightJustified(6, QLatin1Char('0'))).arg(m_isSlideColor))
			.arg(QString("%1 %2").arg(QString::number(m_dwSlidersubColor, 16).rightJustified(6, QLatin1Char('0'))).arg(m_isSlideColor))
			.arg(QString("%1 %2").arg(QString::number(m_part.frncolor(), 16).rightJustified(6, QLatin1Char('0'))).arg(m_isShapColor))
			.arg(QString::number(m_part.height()))
			.arg(QString::number(m_part.radius() / 2))
			.arg(QString::number(m_nMarginSize))
			.arg(QString::number(m_part.height() / 2));
	}
	else {
		if (m_part.direction() == RightToLeft) //从右到左
		{
			if (m_maxLabel)
				m_maxLabel->move(0, this->height() - m_minLabel->height());
			if (m_minLabel)
				m_minLabel->move(this->width() - m_minLabel->width(), this->height() - m_minLabel->height());

			this->setInvertedAppearance(true);	//设置从右到左显示滑动条

			unsigned int tempColor = m_dwSlidersubColor; //警戒值颠倒
			m_dwSlidersubColor = m_dwSlideraddColor;
			m_dwSlideraddColor = tempColor;
		}
		else {
			if (m_maxLabel)
				m_minLabel->move(0, this->height() - m_minLabel->height());
			if (m_minLabel)
				m_maxLabel->move(this->width() - m_maxLabel->width(), this->height() - m_minLabel->height());
		}

		//遵循样式设置圆形的规则QSlider::handle:Horizontal中的width - QSlider::groove:Horizontal中的
		//heigth  >= 2*QSlider::handle:Horizontal中的margin
		if (( m_part.height() - m_part.railheight() ) % 2) {
			m_nMarginSize = ( m_part.height() - m_part.railheight() ) / 2 + 1;
		}
		else {
			m_nMarginSize = ( m_part.height() - m_part.railheight() ) / 2;
		}
		this->setOrientation(Qt::Horizontal);//设置滑动条 方向

		if (m_displayLabel)
			m_displayLabel->move(GetHorMoveWidth(0), GetHorMoveHeight());

		m_stylesheet = QString("\
							 QSlider{border:%1px solid #%2;}\
							 QSlider::groove:horizontal{border:0px;height:%3px;width:%4px;}  \
							 QSlider::sub-page:horizontal{background:#%6;border-bottom-left-radius:%5px;border-top-left-radius:%5px}  \
							 QSlider::add-page:horizontal{background:#%7;border-top-right-radius:%5px;border-bottom-right-radius:%5px;} \
							 QSlider::handle:horizontal{background:#%8;width:%9px;border-radius:%10px;margin:-%11px -%12px -%11px -%12px;} \
							 ")
			.arg(m_part.useborder())
			.arg(QString("%1 %2").arg(QString::number(m_part.bordercolor(), 16).rightJustified(6, QLatin1Char('0'))).arg(m_isBorderColor))
			.arg(QString::number(m_part.railheight()))
			.arg(this->width() - m_part.width())
			.arg(QString::number(( m_part.railheight() )*m_part.railtype() / 2))
			.arg(QString("%1 %2").arg(QString::number(m_dwSlidersubColor, 16).rightJustified(6, QLatin1Char('0'))).arg(m_isSlideColor))
			.arg(QString("%1 %2").arg(QString::number(m_dwSlideraddColor, 16).rightJustified(6, QLatin1Char('0'))).arg(m_isSlideColor))
			.arg(QString("%1 %2").arg(QString::number(m_part.frncolor(), 16).rightJustified(6, QLatin1Char('0'))).arg(m_isShapColor))
			.arg(QString::number(m_part.width()))
			.arg(QString::number(m_part.radius() / 2))
			.arg(QString::number(m_nMarginSize))
			.arg(QString::number(m_part.width() / 2));
	}

	this->setStyleSheet(m_stylesheet);
}

void CSlider::ParseLabelFontSize() {
	QString sfontsty(m_part.basic().font().c_str());
	QStringList szlist = sfontsty.split(" ");
	switch (szlist.size()) {
	case 2:
		m_nLableFontSize = szlist.at(0).split("px").at(0).toInt();
		break;
	case 3:
		m_nLableFontSize = szlist.at(1).split("px").at(0).toInt();
		break;
	case 4:
		m_nLableFontSize = szlist.at(2).split("px").at(0).toInt();
		break;
	default:
		break;
	}

	for (int nI = 0; nI < szlist.size(); nI++) {
		if (szlist.at(nI) == "italic") {
			displayLabelFt.setItalic(true);
		}

		if (szlist.at(nI) == "bold") {
			displayLabelFt.setItalic(true);
		}
	}

	displayLabelFt.setPixelSize(m_nLableFontSize);

}

void CSlider::DoDIsplyCurValueMove(int nValue) {
	m_part.set_curvalue(nValue);
	int nShowValue;

	if (m_part.curvalue() % m_part.scale() > m_part.scale() / 2) //当前值取最小刻度的余数大于最小刻度的一半则下一刻度
	{
		if ((long long) m_part.curvalue() + ( m_part.scale() - m_part.curvalue() % m_part.scale() ) <= m_part.maxvalue() &&
			(long long) m_part.curvalue() + ( m_part.scale() - m_part.curvalue() % m_part.scale() ) >= m_part.minvalue()) //判断是否超出范围
		{
			nShowValue = (long long) m_part.curvalue() + ( m_part.scale() - m_part.curvalue() % m_part.scale() );
		}
		else {
			nShowValue = (long long) m_part.curvalue() - m_part.curvalue() % m_part.scale();
		}

	}
	else {
		long long nDataRange = (long long) m_part.curvalue() - m_part.minvalue();
		if (nDataRange < m_part.scale()) {
			nShowValue = m_part.minvalue();
		}
		else {
			nShowValue = m_part.curvalue() - m_part.curvalue() % m_part.scale();
		}
		if ((long long) nShowValue < m_part.minvalue())//判断是否超出范围
		{
			nShowValue = (long long) m_part.curvalue() + ( m_part.scale() - m_part.curvalue() % m_part.scale() );
		}

		if ((long long) nShowValue > m_part.maxvalue()) {
			nShowValue = (long long) m_part.curvalue() - m_part.curvalue() % m_part.scale();
		}
	}

	m_part.set_curvalue(nShowValue);
	if (m_displayLabel) {
		m_displayLabel->setText(CutDataStrByDigitCount(nShowValue));
		m_displayLabel->adjustSize(); //更新当前值

		if (m_part.direction() == TopToButtom || m_part.direction() == ButtomToTop) //移动当前值
		{
			m_displayLabel->move(GetVerMoveWidth(), GetVerMoveHeight(nValue));

		}
		else {
			m_displayLabel->move(GetHorMoveWidth(nValue), GetHorMoveHeight());
		}
	}

	if (m_part.rightnow()) {
		QString strBuffer = QString("%1").arg(nShowValue);
		EventMgr::SendDataTransEvent(m_nScrNo, QString(m_sPartName.c_str()), strBuffer); //发送给hmi
	}
}

void CSlider::DoSliderAlarmColor(int nValue) {
	if (m_part.alarm()) //是否启用警戒色
	{
		if (nValue > m_part.alarmvalue() && m_bWaring) // 如果当前值大于警戒值，且能够设置警戒色
		{
			QString newsty;
			QString oldsty = this->styleSheet(); //获取当前样式

			if (m_part.direction() == LeftToRight)  //竖直方向滑动条
			{
				newsty = QString("QSlider::sub-page:horizontal{background:#%1;}").arg(QString::number(m_part.alarmcolor(), 16).rightJustified(6, QLatin1Char('0')));
			}
			else if (m_part.direction() == RightToLeft) {
				newsty = QString("QSlider::add-page:horizontal{background:#%1;}").arg(QString::number(m_part.alarmcolor(), 16).rightJustified(6, QLatin1Char('0')));
			}
			else if (m_part.direction() == TopToButtom) {
				newsty = QString("QSlider::sub-page:vertical{background:#%1;}").arg(QString::number(m_part.alarmcolor(), 16).rightJustified(6, QLatin1Char('0')));
			}
			else if (m_part.direction() == ButtomToTop) {
				newsty = QString("QSlider::add-page:vertical{background:#%1;}").arg(QString::number(m_part.alarmcolor(), 16).rightJustified(6, QLatin1Char('0')));
			}
			this->setStyleSheet(oldsty + newsty);
			m_bWaring = false; //不能设置警戒色
			m_bNormal = true; //能设置正常色
		}

		if (nValue <= m_part.alarmvalue() && m_bNormal) // 如果当前值小于等于警戒值，且能够设置正常色
		{
			this->setStyleSheet(m_stylesheet);
			m_bNormal = false;//不能设置正常色
			m_bWaring = true;//能设置警戒色
		}
	}
}

int CSlider::GetHorMoveWidth(int nValue) {
	if (!m_displayLabel) {
		return 0;
	}
	unsigned int  nDataLimt = m_part.maxvalue() - m_part.minvalue(); //数据范围
	unsigned int  nvalue = 0; //当前值

	if (nValue < m_part.minvalue()) {
		nValue = m_part.minvalue();
	}

	if (nValue > m_part.maxvalue()) {
		nValue = m_part.maxvalue();
	}

	if (m_part.direction() == LeftToRight) {
		nvalue = nValue - m_part.minvalue();
	}
	else {
		nvalue = m_part.maxvalue() - nValue;
	}

	double	fscle = nvalue * 1.0 / nDataLimt; //计算滑块的移动占总体的比例
	int   nlableleft = ( this->width() - m_part.width() )*fscle + ( m_part.width() - m_displayLabel->width() ) / 2;//计算左边坐标值
	int   nlableright = nlableleft + m_displayLabel->width(); //计算右边坐标值

	if (nlableleft >= 0) {
		if (nlableright >= this->width()) //如果右边坐标值大于宽度（即超出边界），
		{
			return this->width() - m_displayLabel->width();
		}
		else {
			return nlableleft;
		}
	}
	return 0;
}

int CSlider::GetVerMoveWidth() {
	if (!m_displayLabel) {
		return 0;
	}
	int	  nvaluelen = QString(this->m_displayLabel->text()).size(); //当前值
	int   nlableleft = this->width() / 2 - displayLabelFt.pixelSize()*nvaluelen / 4; //计算左边坐标值
	int   nlableright = nlableleft + displayLabelFt.pixelSize()*nvaluelen / 2;//计算右边坐标值
	if (nlableleft >= 0) {
		if (nlableright >= this->width()) //如果超出边界则左对齐
		{
			return 0;
		}
		else {
			return nlableleft;
		}
	}
	return 0;
}

int CSlider::GetHorMoveHeight() {
	if (!m_displayLabel) {
		return 0;
	}
	int  nlabletop = ( nHeight - m_part.height() ) / 2 - m_displayLabel->height(); //计算当前值的Y轴坐标

	if (nlabletop > 0)//不在最顶上则显示滑块最上方，否者显示在最上面
	{
		return nlabletop;
	}
	return 0;
}

int CSlider::GetVerMoveHeight(int nValue) {
	if (!m_displayLabel) {
		return 0;
	}
	unsigned int  nDataLimt = this->maximum() - this->minimum(); //数据范围
	unsigned int  nvalue;

	if (m_part.direction() == ButtomToTop) {
		nvalue = this->maximum() - nValue;
	}
	else {
		nvalue = nValue - this->minimum();
	}
	double fscle = nvalue * 1.0 / nDataLimt; //计算滑块的移动占总体的比例

	int	nlabletop = ( this->height() - m_part.height() )*fscle - m_nLableFontSize; //计算最顶上的坐标值

	if (m_minLabel &&m_maxLabel) //有最大最小值标签时
	{
		if (m_part.direction() == TopToButtom) {
			if (nlabletop < m_minLabel->height()) // 最顶上不超过最顶上值的高度
			{
				return m_minLabel->height();
			}

			if (nlabletop > m_maxLabel->y() - m_displayLabel->height())// 最底上不超过最底下值的高度
			{
				return m_maxLabel->y() - m_displayLabel->height();
			}

		}
		else {
			if (nlabletop < m_maxLabel->height()) // 最顶上不超过最顶上值的高度
			{
				return m_minLabel->height();
			}

			if (nlabletop > m_minLabel->y() - m_displayLabel->height())// 最底上不超过最底下值的高度
			{
				return m_minLabel->y() - m_displayLabel->height();
			}
		}

	}
	//没有显示上下限时，最上方不隐藏
	if (nlabletop > 0) {
		return nlabletop;
	}
	else {
		return 0;
	}
}

void CSlider::InitLableStyle(QLabel *label) {
	if (!m_part.basic().has_font()) {
		return;
	}
	if (displayLabelFt.italic())//如果说是斜体则设置居左显示，解决居中显示情况下斜体时右边缺少部分
	{
		label->setAlignment(Qt::AlignLeft);
	}
	else {
		label->setAlignment(Qt::AlignCenter);
	}

	label->setStyleSheet(QString("font:%1;color:#%2;") //设置字体和颜色
		.arg(m_part.basic().font().c_str())
		.arg(QString::number(m_part.basic().textcolor(), 16).rightJustified(6, QLatin1Char('0'))));
	label->adjustSize();
}

void CSlider::EndTheSlide() {
	if (!m_bMoveState) //非移动情况下不处理，过滤卷动模式抬起动作
	{
		return;
	}
	int		nValue = m_part.curvalue();
	m_bMoveState = false;

	if (0 != nValue % m_part.scale()) {

		if (nValue % m_part.scale() > m_part.scale() / 2) //当前值取最小刻度的余数大于最小刻度的一半则下一刻度
		{
			if ((long long) ( nValue + m_part.scale() - nValue % m_part.scale() ) > m_part.scale() &&
				( (long long) nValue + m_part.scale() - nValue % m_part.scale() ) <= m_part.scale())//判断是否超出范围
			{
				nValue = (long long) nValue + ( m_part.scale() - nValue % m_part.scale() );
			}
			else {
				nValue = (long long) nValue - nValue % m_part.scale();
			}
		}
		else {
			nValue = (long long) nValue - nValue % m_part.scale();
		}
	}
	else //如果在最小刻度上要判断是否超出最大最小值范围，否则会出现停留在位置的BUG
	{
		if ((long long) nValue < m_part.minvalue())//判断是否超出范围
		{
			nValue = (long long) nValue + ( m_part.scale() - nValue % m_part.scale() );
		}

		if ((long long) nValue > m_part.maxvalue()) {
			nValue = (long long) nValue - nValue % m_part.scale();
		}
	}
	m_part.set_curvalue(nValue);
	this->setValue(nValue);
	this->ChangeCurValue(nValue);
	QString strBuffer = QString("%1").arg(nValue);
	EventMgr::SendDataTransEvent(m_nScrNo, QString(m_sPartName.c_str()), strBuffer);
}

void CSlider::ChangeCurValue(int nValue) {

	this->DoSliderAlarmColor(nValue); //处理警戒色

	if (!m_displayLabel) {
		return;
	}
	if (nValue > m_part.maxvalue()) {
		nValue = m_part.maxvalue();
	}

	if (nValue < m_part.minvalue()) {
		nValue = m_part.minvalue();
	}

	m_displayLabel->setText(CutDataStrByDigitCount(nValue));
	m_displayLabel->adjustSize();
	m_part.set_curvalue(nValue);

	if (m_part.direction() == TopToButtom || m_part.direction() == ButtomToTop) //移动当前值位置
	{
		m_displayLabel->move(GetVerMoveWidth(), GetVerMoveHeight(nValue));

	}
	else {
		m_displayLabel->move(GetHorMoveWidth(nValue), GetHorMoveHeight());
	}
}

void CSlider::SliderMove(int nValue) {
	m_bMoveState = true;
	this->DoSliderAlarmColor(nValue); //处理警戒色
	this->DoDIsplyCurValueMove(nValue); //处理当前值显示以及位置
}

void CSlider::GetHandleAddress(int nValue) {
	unsigned int  nDataLimt = m_part.maxvalue() - m_part.minvalue();
	unsigned int  nDataValue = 0;
	if (m_part.direction() == LeftToRight) {
		nDataValue = ( nValue - m_part.minvalue() );
		double temp = nDataValue * 1.0 / nDataLimt;
		m_nSliderX = ( nWidth - m_part.width() )*temp;
		m_nSliderY = ( nHeight - m_part.height() ) / 2;
	}

	if (m_part.direction() == RightToLeft) {
		nDataValue = ( m_part.maxvalue() - nValue );
		double temp = nDataValue * 1.0 / nDataLimt;
		m_nSliderX = ( nWidth - m_part.width() )*temp;
		m_nSliderY = ( nHeight - m_part.height() ) / 2;
	}

	if (m_part.direction() == TopToButtom) {
		nDataValue = ( nValue - m_part.minvalue() );
		double temp = nDataValue * 1.0 / nDataLimt;
		m_nSliderX = ( nWidth - m_part.width() ) / 2;
		m_nSliderY = ( nHeight - m_part.height() )*temp;
	}

	if (m_part.direction() == ButtomToTop) {
		nDataValue = ( m_part.maxvalue() - nValue );
		double temp = nDataValue * 1.0 / nDataLimt;
		m_nSliderX = ( nWidth - m_part.width() ) / 2;
		m_nSliderY = ( nHeight - m_part.height() )*temp;
	}

}

bool CSlider::CheckIsSlide(QMouseEvent *eventT) {
	double diameter;
	if (m_part.width() < MINCOTRLSIZE && m_part.height() < MINCOTRLSIZE) //当滑块宽度小于最小宽度时
	{
		diameter = MINCOTRLSIZE / 2;
	}
	else {
		diameter = sqrt(m_part.height()*m_part.height()*1.0 + m_part.width()*m_part.width()*1.0);  //计算当前滑块外切圆直径
		diameter = diameter * 1.5 / 2;	//计算当前滑块外切圆半径

	}

	int nX = qAbs(eventT->x() - ( m_nSliderX + m_part.width()*1.0 / 2 ));
	int nY = qAbs(eventT->y() - ( m_nSliderY + m_part.height()*1.0 / 2 ));

	if (nX*nX + nY * nY > diameter*diameter) //判断点是否在圆内
	{
		return false;
	}

	return true;
}

void CSlider::DoScrollControl(QMouseEvent *eventT) {
	if (!m_part.scroll()) //如果没有开启卷动模式
	{
		return;
	}

	int nCurValue = this->value();
	long long nValue = nCurValue;

	if (m_part.direction() == LeftToRight)	//水平方向
	{
		if (eventT->x() < m_nSliderX) {
			nValue = (long long) nCurValue - m_part.scrollvalue();
		}
		else if (eventT->x() > m_nSliderX + m_part.width()) {
			nValue = (long long) nCurValue + m_part.scrollvalue();
		}
	}
	else if (m_part.direction() == RightToLeft) {
		if (eventT->x() < m_nSliderX) {
			nValue = (long long) nCurValue + m_part.scrollvalue();
		}
		else if (eventT->x() > m_nSliderX + m_part.width()) {
			nValue = (long long) nCurValue - m_part.scrollvalue();
		}
	}
	else if (m_part.direction() == TopToButtom) {
		if (eventT->y() < m_nSliderY) {
			nValue = (long long) nCurValue - m_part.scrollvalue();
		}
		else if (eventT->y() > m_nSliderY + m_part.height()) {
			nValue = (long long) nCurValue + m_part.scrollvalue();
		}
	}
	else if (m_part.direction() == ButtomToTop) {
		if (eventT->y() < m_nSliderY) {
			nValue = (long long) nCurValue + m_part.scrollvalue();
		}
		else if (eventT->y() > m_nSliderY + m_part.height()) {
			nValue = (long long) nCurValue - m_part.scrollvalue();
		}
	}
	//避免超出边界
	if (nValue > m_part.maxvalue()) {
		nValue = m_part.maxvalue();
	}

	if (nValue < m_part.minvalue()) {
		nValue = m_part.minvalue();
	}

	this->setValue(nValue); //改变滑块位置
	this->ChangeCurValue(nValue);//改变当前值显示位置
	QString strBuffer = QString("%1").arg(m_part.curvalue());
	EventMgr::SendDataTransEvent(m_nScrNo, QString(m_sPartName.c_str()), strBuffer);
}

bool CSlider::AllowChanged() {
	HMIScreen	*hmiScr = static_cast<HMIScreen *>( this->parent() );
	HMIMgr		*hmiMgr = static_cast<HMIMgr *>( hmiScr->getParent() );
	HMIScreen	*topScr = hmiMgr->GetTopScr();

	if (topScr &&
		topScr->GetScrType() != SCREEN_INDIRECT &&
		hmiScr->GetScrNo() != topScr->GetScrNo()) {
		return false;
	}

	return true;
}

void CSlider::mousePressEvent(QMouseEvent *eventT) {
	if (false == AllowChanged()) {
		return;
	}

	QSlider::mousePressEvent(eventT);
	//获取点击坐标值，滑块当前值
	m_nClickedDownStartHorPos = eventT->x();
	m_nClickedDownStartVerPos = eventT->y();
	m_nClickedDownstartValue = this->value();
	this->GetHandleAddress(m_nClickedDownstartValue); //获取滑块相对部件的坐标
	this->DoScrollControl(eventT);//处理卷动模式
	if (CheckIsSlide(eventT)) // 是否可滑动
	{
		m_bMove = true;
	}
	else {
		m_bMove = false;
	}

	emit sendSliderOnclickedDown(eventT, m_nClickedDownStartHorPos + nLeft, m_nClickedDownStartVerPos + nTop); //告诉hmiscreen鼠标按下的位置
}

void CSlider::mouseMoveEvent(QMouseEvent *eventT) {
	QSlider::mouseMoveEvent(eventT);

	if (!m_bMove) // 点击的位置不能移动则返回
	{
		return;
	}

	double fscle = 0.0;
	long long nValue = 0;

	if (m_part.direction() == TopToButtom || m_part.direction() == ButtomToTop) {
		if (abs(eventT->y() - m_nClickedDownStartVerPos) <= ( this->height() - m_part.height() )) //计算滑块的移动占总体的比例
			fscle = ( eventT->y() - m_nClickedDownStartVerPos )*1.0 / ( this->height() - m_part.height() );
		else if (eventT->y() - m_nClickedDownStartVerPos < 0) // 超过1.0则为1.0
			fscle = -1.0;
		else if (eventT->y() - m_nClickedDownStartVerPos > 0) // 超过-1.0则为-1.0
			fscle = 1.0;

		unsigned int  nDataLimt = this->maximum() - this->minimum();
		if (m_part.direction() == ButtomToTop) //判断显示类型
			nValue = m_nClickedDownstartValue - fscle * nDataLimt + 0.5;
		else
			nValue = m_nClickedDownstartValue + fscle * nDataLimt + 0.5;
	}
	else {
		if (abs(eventT->x() - m_nClickedDownStartHorPos) <= ( this->width() - m_part.width() )) //计算滑块的移动占总体的比例
			fscle = ( eventT->x() - m_nClickedDownStartHorPos )*1.0 / ( this->width() - m_part.width() );
		else if (eventT->x() - m_nClickedDownStartHorPos < 0) // 超过1.0则为1.0
			fscle = -1.0;
		else if (eventT->x() - m_nClickedDownStartHorPos > 0) // 超过-1.0则为-1.0
			fscle = 1.0;

		unsigned int  nDataLimt = this->maximum() - this->minimum();
		if (m_part.direction() == LeftToRight) //判断显示类型
			nValue = m_nClickedDownstartValue + fscle * nDataLimt + 0.5;
		else
			nValue = m_nClickedDownstartValue - fscle * nDataLimt + 0.5;
	}


	if (nValue <= this->minimum()) {
		this->setValue(this->minimum());
		this->SliderMove(this->minimum());
	}
	else if (nValue >= this->maximum()) {
		this->setValue(this->maximum());
		this->SliderMove(this->maximum());
	}
	else {
		this->setValue(nValue);
		this->SliderMove(nValue);
	}
}

void CSlider::mouseReleaseEvent(QMouseEvent *eventT) {
	m_bMove = false;
	QSlider::mouseReleaseEvent(eventT);
	this->EndTheSlide(); //处理抬起动作
	emit sendSliderOnclickedUp(eventT, m_nClickedDownStartHorPos + nLeft, m_nClickedDownStartVerPos + nTop); //告诉hmiscreen鼠标抬起的位置
}

QString CSlider::CutDataStrByDigitCount(int nValue) {
	double temp = nValue * 1.0 / qPow(10, m_part.decnum());
	QString szStr = QString::number(temp, 'f', m_part.decnum()); //小数位数换算
	return szStr;
}