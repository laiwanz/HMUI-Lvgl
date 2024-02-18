#include "ComboBox.h"
#include <QDebug>
#include <QComboBox>
#include <QLabel>
#include <QtSvg/QSvgRenderer>
#include <QMovie>
#include <QPainter>
#include <QtGui>
#include <QAbstractItemView>
#ifdef WIN32
#include <QApplication>
#include <QDesktopWidget>
#endif // 
#include "hmiutility.h"
#include "EventMgr/EventMgr.h"
#include "imgcachemgr.h"
#include "hmiscreen.h"
#include "platform/platform.h"
#include "macrosplatform/macrosplatform.h"

CComboBox::CComboBox(const hmiproto::hmidownlist &downlist, QWidget *parent) {
	m_part.Clear();
	m_comboxFontStyle = "";
	m_strPartType = "";
	m_nScrNo = -1;
	m_comboxbtn = NULL;
    m_QStyledItemDelegate = NULL;
	m_part.MergeFrom(downlist);
}

CComboBox::~CComboBox() {
	try {
        if (NULL != m_QStyledItemDelegate) {
            delete m_QStyledItemDelegate;
        }

		if (m_comboxbtn != NULL) {
			delete m_comboxbtn;
			m_comboxbtn = NULL;
		}
	}
	catch (...) {
	}
}

//初始化数据
void CComboBox::Init(QWidget * pWidget) {
	RECT	rc;
	rc.left = m_part.basic().left();
	rc.right = m_part.basic().right();
	rc.top = m_part.basic().top();
	rc.bottom = m_part.basic().bottom();
	m_sPartName = m_part.basic().name();
	QString strPartType(m_part.basic().type().c_str());
	m_strPartType = strPartType;

	m_nScrNo = m_part.basic().scrno();

	//位置及大小
	int nLeft = rc.left;
	int nTop = rc.top;
	int nWidth = rc.right - rc.left;
	int nHeight = rc.bottom - rc.top;

	this->setParent(pWidget);					//指定父窗体
	m_comboxbtn = new QPushButton(pWidget);
	m_comboxbtn->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	if (m_part.basic().has_font()) {
		m_comboxFontStyle = QString("%1").arg(m_part.basic().font().c_str());
	}
	else {
		m_comboxFontStyle = QString("%1px SimSun").arg(nHeight);
	}

    m_QStyledItemDelegate = new QStyledItemDelegate();
    this->setItemDelegate(m_QStyledItemDelegate);

	//this->setStyleSheet(QString("font:%1").arg(m_comboxFontStyle));

	connect(this, SIGNAL(activated(int)), this, SLOT(DownListIndexChanged(int)));
#ifdef WIN32
	connect(this, SIGNAL(highlighted(int)), this, SLOT(SetShowRang(int)));
#endif
	m_comboxbtn->setGeometry(nLeft + 1, nTop + 1, nWidth - 16, nHeight - 2); //为了保留边框和下拉箭头作出微调
	if (m_part.center()) //文本显示方式
	{
		m_comboxbtn->setStyleSheet(QString("QPushButton{font:%1;background-color:white;border:0px;}").arg(m_comboxFontStyle));
	}
	else {
		m_comboxbtn->setStyleSheet(QString("QPushButton{font:%1;background-color:white;border:0px;text-align:left center;}").arg(m_comboxFontStyle));
	}
	this->setGeometry(nLeft, nTop, nWidth, nHeight);
	m_Rect = QRect(nLeft, nTop, nWidth, nHeight);

	connect(this, SIGNAL(onClickdown(QMouseEvent *, int, int)), static_cast<HMIScreen *>( this->parent() ), SLOT(RecvOnclickedDown(QMouseEvent *, int, int)));
	connect(this, SIGNAL(onClickup(QMouseEvent *, int, int)), static_cast<HMIScreen *>( this->parent() ), SLOT(RecvOnclickedUp(QMouseEvent *)));

#if ((defined WIN32) && (defined WIN32SCALE))
	memcpy(&m_OriginalRect, &rc, sizeof(RECT));
	ZoomPartSelf();
#endif
}

//收到更新数据
void CComboBox::Update(const hmiproto::hmidownlist &downlist, QWidget * pWidget) {
	(void) pWidget;
	if (downlist.pencolor_size() > 0) {
		m_part.clear_pencolor();
	}
	m_part.MergeFrom(downlist);

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

//绘图
void CComboBox::Draw(QWidget*, int drawtype) {

	DrawDownList(drawtype);
}

void CComboBox::DrawDownList(int drawtype) {
	(void) drawtype;


	if (m_part.basic().has_font()) {
		m_comboxFontStyle = QString("%1").arg(m_part.basic().font().c_str());
	}
	else {
		m_comboxFontStyle = QString("%1px SimSun").arg(m_Rect.height());
	}

	//是否隐藏
	if (m_part.basic().hide()) {
		this->hide();
		m_comboxbtn->hide();
		return;
	}
	else {
		this->show();
		m_comboxbtn->show();
	}

	//禁止信号处理
	this->blockSignals(true);

	//清空所有
	this->clear();

	char	szRowSeparation[2] = { 0x01 };
	QString strOptions(m_part.content().c_str());
	strItemList = strOptions.split(szRowSeparation);
	this->addItems(strItemList);
	int nNum = strItemList.size();
	if (m_part.center())//下拉框居中显示
	{
		for (int nI = 0; nI < nNum; nI++) {
			static_cast<QStandardItemModel*>( this->view()->model() )->item(nI)->setTextAlignment(Qt::AlignCenter);
		}
	}

	//设置每一项颜色
	for (int nI = 0; nI < MAX_TypeLen; nI++) {

		if (nI >= m_part.pencolor_size()) {

			break;
		}
		this->setItemData(nI, QColor(m_part.pencolor(nI)), Qt::ForegroundRole);
	}

	//选中指定项
	this->setCurrentIndex(m_part.selected());

	SetDownListColor();//每一次都得设置颜色，解决权限不足时颜色改变 xk 20191110

	unsigned int dwColor = 0;
	if (this->currentIndex() >= 0) {
		dwColor = m_part.pencolor(this->currentIndex());
	}

    QString strStyle = QString("QComboBox {font:%1;}").arg(m_comboxFontStyle);
	this->setStyleSheet(strStyle);
    QString strColor = QString::number(dwColor, 16).rightJustified(6, QLatin1Char('0'));
    strStyle += QString("QComboBox {color:#%1;}").arg(strColor);

    int nHeight = getItemHeight();
    strStyle += QString("QComboBox QAbstractItemView:item{min-height:%1px;}").arg(nHeight);
	
    this->setStyleSheet(strStyle);
	this->blockSignals(false);//允许信号处理
}

void CComboBox::mousePressEvent(QMouseEvent *eventT) {

	QComboBox::mousePressEvent(eventT);

	emit onClickdown(eventT, eventT->x() + m_Rect.left(), eventT->y() + m_Rect.top());
	mouseReleaseEvent(eventT);
}

void CComboBox::mouseReleaseEvent(QMouseEvent *eventT) {

	QComboBox::mouseReleaseEvent(eventT);

	emit onClickup(eventT, eventT->x() + m_Rect.left(), eventT->y() + m_Rect.top());
}

bool CComboBox::AllowChanged() {
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

void CComboBox::showPopup() {
#ifndef WIN32
	SetShowRang(0);
#endif
	QComboBox::showPopup();
}

void CComboBox::DownListIndexChanged(int nIndex) {
	if (false == AllowChanged()) {
		return;
	}

	//改变颜色
	SetDownListColor();

	//发送给HMI
	QString strBuffer = QString("%1").arg(nIndex);
	EventMgr::SendDataTransEvent(m_nScrNo, QString(m_sPartName.c_str()), strBuffer);
}

//从缓存中显示出来
void CComboBox::OnPartShow() {

}

//隐藏画面
void CComboBox::OnPartHide() {

}

#if ((defined WIN32) && (defined WIN32SCALE))
void CComboBox::ZoomPartSelf() {
	RECT rectTemp = m_OriginalRect;

	HMIUtility::CountScaleRect(&rectTemp);

	int nLeft = rectTemp.left;
	int nTop = rectTemp.top;
	int nWidth = rectTemp.right - rectTemp.left;
	int nHeight = rectTemp.bottom - rectTemp.top;

	this->setGeometry(nLeft, nTop, nWidth, nHeight);
	m_comboxbtn->setGeometry(nLeft + 1, nTop + 1, nWidth - 16, nHeight - 2); //下拉清单部件由按钮和下拉清单叠加组成，缩放需一起处理。 linzhen20200327
}
#endif

void CComboBox::SetDownListColor() {
	unsigned int dwColor = 0;
	if (this->currentIndex() >= 0) {
		m_comboxbtn->setText(strItemList.at(this->currentIndex()));
		dwColor = m_part.pencolor(this->currentIndex());
	}
	else {
		m_comboxbtn->setText("");
	}
	if (m_part.center()) //文本显示方式
	{
		m_comboxbtn->setStyleSheet(QString("QPushButton{font:%1;background-color:white;border:0px;}").arg(m_comboxFontStyle));
	}
	else {
		m_comboxbtn->setStyleSheet(QString("QPushButton{font:%1;background-color:white;border:0px;text-align:left center;}").arg(m_comboxFontStyle));
	}
	QPalette   pal;
	pal.setColor(QPalette::ButtonText, QColor(dwColor));
	m_comboxbtn->setPalette(pal);
}

void CComboBox::SetShowRang(int) {
	int	nPartTop = 0;
	int nPartBottom = 0;
	int	nLenth = 0;
	int nScreenHeight = 0;
	int nControlNum = strItemList.size();
	int nPartHeight = getItemHeight();

#ifdef _LINUX_

	int	nWinWidth = 0;
	int	nBitPerPixel = 0;

	nPartTop = m_part.basic().top();
	nPartBottom = m_part.basic().bottom();
	cbl::CLcd devLcd;
	devLcd.getResolution(nWinWidth, nScreenHeight, nBitPerPixel);

#else

	QDesktopWidget* d = QApplication::desktop();
	nScreenHeight = d->height();
	nPartTop = this->mapToGlobal(this->pos()).y();
	nPartBottom = nPartTop + nPartHeight;

#endif


	if (nPartTop + nPartHeight / 2 >= nScreenHeight / 2)//部件中心点 在 画面中心点 的下面（可能存在向下显示）
	{
		if (nControlNum > ( nScreenHeight - nPartBottom ) / nPartHeight)//如果部件底部与画面底部之间的空间足够显示所有状态则向下显示，否则向上显示
		{
			nLenth = nPartTop - ( nPartTop / nPartHeight )*nPartHeight;//部件向上显示区域与画面顶部之间的空间长度
			if (nLenth <= nPartHeight * 0.4)//如果部件向上显示区域与画面顶部之间的空间长度太小，光标移至下拉区域最后一个状态时，下拉清单会下移
			{
				this->setMaxVisibleItems(nPartTop / nPartHeight - 1);//显示的状态数减1，避免部件向上显示区域与画面顶部之间的空间长度太小
			}
			else {
				this->setMaxVisibleItems(nPartTop / nPartHeight);
			}

		}
		else {
			this->setMaxVisibleItems(this->maxCount());
		}
	}
	else//部件中心点 在 画面中心点 的上面（向下显示）
	{
		if (nControlNum > ( nScreenHeight - nPartBottom ) / nPartHeight) {
			nLenth = ( nScreenHeight - nPartBottom ) - ( ( nScreenHeight - nPartBottom ) / nPartHeight )*nPartHeight;//部件向下显示区域与画面底部之间的空间长度
			if (nLenth <= nPartHeight * 0.4)//如果部件向下显示区域与画面顶部之间的空间长度太小，光标移至下拉区域最后一个状态时，下拉清单会下移
			{
				this->setMaxVisibleItems(( nScreenHeight - nPartBottom ) / nPartHeight - 1);//显示的状态数减1，避免部件向上显示区域与画面顶部之间的空间长度太小
			}
			else {
				this->setMaxVisibleItems(( nScreenHeight - nPartBottom ) / nPartHeight);
			}
		}
		else {
			this->setMaxVisibleItems(this->maxCount());
		}

	}
}

int CComboBox::getItemHeight() {

    QFontMetrics    fm(this->font());
    int             nHeight = fm.height();

    if (m_part.itemheight() > 50) {
        nHeight = nHeight * 5;
    }
    else if ((m_part.itemheight() > 10) && (m_part.itemheight() <= 50)) {

        float   fValue = (float)m_part.itemheight() / 10;
        nHeight = nHeight * fValue;
    }

    if (0 == nHeight) {
        nHeight = 12;
    }

    return nHeight;
}
