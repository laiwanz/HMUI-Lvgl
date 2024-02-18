#include "weatherUi.h"
#include <QPainter>
#include <QString>
#include "hmiutility.h"
#include "EventMgr/EventMgr.h"
#include "hmimgr.h"

static std::string toUtf8(const std::string &sStr) {

	cbl::CODE_FORMAT_T nSrcFormat = cbl::getCodeFormat(sStr);

	std::string sTagStr;
	int nRet = cbl::encode(nSrcFormat, cbl::UTF8, sStr, sTagStr);
	if (nRet < 0) {
		return sStr;
	}

	return sTagStr;
}


CWeatherUi::CWeatherUi(const hmiproto::hmiweather &weather, QWidget *parent){

	m_part.Clear();

	m_svg = NULL;

	m_labHeadBg = new QLabel(this);
	m_labProvince = new QLabel(this);
	m_labCity = new QLabel(this);
	m_model = new QStringListModel();
	m_listView = new QListView(this);
	m_btnCancel = new QPushButton("X", this);

	m_listView->setModel(m_model);

	m_labHeadBg->hide();
	m_labCity->hide();
	m_labProvince->hide();
	m_btnCancel->hide();
	m_listView->hide();
	
	if (weather.has_designeddata()) {
		m_part.clear_designeddata();
	}
	m_part.MergeFrom(weather);
}

CWeatherUi::~CWeatherUi()
{
	try {
		if (m_svg) {
			delete m_svg;
			m_svg = nullptr;
		}
		if(m_labHeadBg){
			delete m_labHeadBg;
			m_labHeadBg = nullptr;
		}
		if(m_labProvince){
			delete m_labProvince;
			m_labProvince = nullptr;
		}
		if(m_labCity){
			delete m_labCity;
			m_labCity = nullptr;
		}
		if(m_model){
			delete m_model;
			m_model = nullptr;
		}
		if(m_listView){
			delete m_listView;
			m_listView = nullptr;
		}
		if(m_btnCancel){
			delete m_btnCancel;
			m_btnCancel = nullptr;
		}
	}
	catch (...) {
	}
}

void CWeatherUi::Init(QWidget * pWidget){

	m_rc.left = m_part.basic().left();
	m_rc.right = m_part.basic().right();
	m_rc.top = m_part.basic().top();
	m_rc.bottom = m_part.basic().bottom();

	this->setParent(pWidget);
	this->setGeometry(m_rc.left, m_rc.top, m_rc.right - m_rc.left, m_rc.bottom - m_rc.top);

	QString strPartType(m_part.basic().type().c_str());
	m_sPartName = m_part.basic().name();
	m_nScrNo = m_part.basic().scrno();
	
	getDesignedConfig(m_part);

#if ((defined WIN32) && (defined WIN32SCALE))
	memcpy(&m_OriginalRect, &m_rc, sizeof(RECT));
	ZoomPartSelf();
#endif

	updateBackground(m_part);
	updateSeltUi(m_part);

	connect(m_listView, SIGNAL(clicked(const QModelIndex)), this, SLOT(slotSelectItem(const QModelIndex)));
	connect(m_btnCancel, SIGNAL(clicked()), this, SLOT(slotBtnCancel()));
	connect(this, SIGNAL(sendWeatherOnclickedDown(QMouseEvent *, int, int)), static_cast<HMIScreen *>(this->parent()), SLOT(RecvOnclickedDown(QMouseEvent *, int, int)));
	connect(this, SIGNAL(sendWeatherOnclickedUp(QMouseEvent *, int, int)), static_cast<HMIScreen *>(this->parent()), SLOT(RecvOnclickedUp(QMouseEvent *)));
	
}

void CWeatherUi::Update(const hmiproto::hmiweather &weather, QWidget * pWidget){

	RECT	rc;
	(void)pWidget;
	if (weather.has_designeddata()) {
		m_part.clear_designeddata();
	}
	m_part.MergeFrom(weather);

	rc.left = m_part.basic().left();
	rc.right = m_part.basic().right();
	rc.top = m_part.basic().top();
	rc.bottom = m_part.basic().bottom();

	if (memcmp(&m_rc, &rc, sizeof(RECT)) != 0) {
		memcpy(&m_rc, &rc, sizeof(RECT));
	}
	this->setGeometry(m_rc.left, m_rc.top, m_rc.right - m_rc.left, m_rc.bottom - m_rc.top);
	
	getDesignedConfig(m_part);
	
#if ((defined WIN32) && (defined WIN32SCALE))

	if (NULL != m_svg) {
		m_svg->GetTextRect(m_part.basic(), &(m_svg->m_OriginalTextRect));
	}

	if (memcmp(&m_rc, &rc, sizeof(RECT)) != 0) {
		memcpy(&m_OriginalRect, &rc, sizeof(RECT));
		ZoomPartSelf();
	}
#endif

	updateBackground(weather);
	updateSeltUi(weather);
	this->update();
}



void CWeatherUi::Draw(QWidget * pWidget, int drawtype /*= DRAW_INIT*/)
{
	

}

#if ((defined WIN32) && (defined WIN32SCALE))
void CWeatherUi::ZoomPartSelf()
{

}
#endif 

void CWeatherUi::paintEvent(QPaintEvent *event){

	QPainter painter(this);
	if (m_imageCache.isValid()) {

		QImage img;
		m_imageCache.releaseTo(img);
		painter.drawImage(QRect(0, 0, this->width(), this->height()), img);
	}
}

void CWeatherUi::mousePressEvent(QMouseEvent *eventT){

	int nX = eventT->x() + this->x();
	int nY = eventT->y() + this->y();

	emit sendWeatherOnclickedDown(eventT, nX, nY); //告诉hmiscreen鼠标按下的位置
}



void CWeatherUi::mouseReleaseEvent(QMouseEvent *eventT){

	int nX = eventT->x() + this->x();
	int nY = eventT->y() + this->y();

	emit sendWeatherOnclickedUp(eventT, nX, nY);
}

int CWeatherUi::updateBackground(const hmiproto::hmiweather &weather){

	if (!weather.has_designeddata()) {
		return 0;
	}

	QPixmap pix(this->size());
	pix.fill(Qt::transparent);

	for (auto &item : m_config.vecRect) {

		QPainter painter(&pix);
		QColor color(item.sFillColor.c_str());
		color.setAlphaF(item.fOpacity);
		painter.fillRect(item.nX, item.nY, item.nW, item.nH, color);
	}

	for (auto &item : m_config.vecLine) {
		QPainter painter(&pix);
		painter.setPen(QColor(item.sColor.c_str()));
		painter.drawLine(item.nX1, item.nY1, item.nX2, item.nY2);
	}
	for (auto &item : m_config.vecText) {
		QPainter painter(&pix);
		QFont font;
		font.setPixelSize(std::stoi(item.sFont));
		painter.setFont(font);
		painter.setPen(QColor(item.sColor.c_str()));
		int align = Qt::AlignCenter;
		if (item.nAlignType == WEATHER_DESIGNED_CONFIG_T::TEXT_ELEMENT_T::ALIGN_LEFT) {
			align = Qt::AlignLeft | Qt::AlignVCenter;
		}
		else if (item.nAlignType == WEATHER_DESIGNED_CONFIG_T::TEXT_ELEMENT_T::ALIGN_MID) {
			align = Qt::AlignCenter | Qt::AlignVCenter;
		}
		else if (item.nAlignType == WEATHER_DESIGNED_CONFIG_T::TEXT_ELEMENT_T::ALIGN_RIGHT) {
			align = Qt::AlignRight | Qt::AlignVCenter;
		}
		painter.drawText(QRectF(item.nX, item.nY, item.nW, item.nH), align, item.sText.c_str());
	}
	for (auto &item : m_config.vecImage) {
		QPainter painter(&pix);
		painter.setOpacity(item.fOpacity);
		painter.drawPixmap(QRect(item.nX, item.nY, item.nW, item.nH), QPixmap(item.sFilePath.c_str()));
	}

	m_imageCache.createFrom(pix.toImage());
	this->update();

	return 0;
}

int CWeatherUi::updateSeltUi(const hmiproto::hmiweather &weather){

	if (weather.has_isshowselectui() ? weather.isshowselectui() : false) {
		m_labCity->show();
		m_labProvince->show();
		m_btnCancel->show();
		m_listView->show();
		m_labHeadBg->show();
	}
	else {
		m_labProvince->hide();
		m_labCity->hide();
		m_btnCancel->hide();
		m_listView->hide();
		m_labHeadBg->hide();
		return 0;
		
	}

	/* paint select ui */
	QString sProvince = weather.has_selectprovince() && !weather.selectprovince().empty() ? weather.selectprovince().c_str() : toUtf8("省份").c_str();
	QString sCity = toUtf8("城市").c_str();
	QStringList listData;
	for (int i = 0; i < weather.listcitys().size(); ++i) {
		listData.push_back(weather.listcitys(i).c_str());
	}
	m_labProvince->setText(sProvince);
	m_labCity->setText(sCity);
	m_labProvince->adjustSize();
	m_labCity->adjustSize();
	m_model->setStringList(listData);

	QFont font(weather.basic().font().c_str());
	m_labProvince->setFont(font);
	m_labProvince->setFont(font);
	m_listView->setFont(font);

	int nRowHeight = weather.has_rowwidth() ? weather.rowwidth() : this->width()/15;
	/* layout */
	{
		if (weather.basic().has_left() && weather.basic().has_top() && weather.basic().has_right() && weather.basic().has_bottom()) {

			this->setGeometry(weather.basic().left(), 
				weather.basic().top(), 
				weather.basic().right() - weather.basic().left(), 
				weather.basic().bottom() - weather.basic().top());
		}
		m_labHeadBg->setGeometry(0, 0, this->width(), nRowHeight*1.5);
		m_labProvince->setGeometry(0, 0, m_labProvince->width() + 5, nRowHeight*1.5);
		m_labCity->setGeometry(m_labProvince->width() + nRowHeight, 0, m_labCity->width(), nRowHeight*1.5);
		m_btnCancel->setGeometry(this->width() - nRowHeight * 1.5, 0, nRowHeight * 1.5, nRowHeight *1.5);
		m_listView->setGeometry(0, nRowHeight*1.5, this->width(), this->height() - nRowHeight);
	}

	m_listView->setEnabled(true);
	m_listView->setSelectionMode(QListView::SingleSelection);
	m_listView->setResizeMode(QListView::Fixed);

	/* style */
	{
		m_labCity->setAlignment(Qt::AlignCenter);
		m_labProvince->setAlignment(Qt::AlignCenter);
		this->setStyleSheet("background-color:white;");
		m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
		m_listView->setStyleSheet(QString("QListView{border-width: 2px;}QListView::item{height:%1px;}").arg(nRowHeight));
		if (weather.has_selectprovince() && weather.selectprovince().size() > 0) {
			m_labCity->setStyleSheet("QLabel{border:2px solid red;border-top:0px;border-right:0px;border-left:0px;background-color:#FFFFFF;color:red;}");
			m_labProvince->setStyleSheet("QLabel{border:0px solid red;border-top:0px;border-right:0px;border-left:0px;background-color:#FFFFFF;color:black;}");
		}
		else {
			m_labCity->setStyleSheet("QLabel{border:0px solid red;border-top:0px;border-right:0px;border-left:0px;background-color:#FFFFFF;color:black;}");
			m_labProvince->setStyleSheet("QLabel{border:2px solid red;border-top:0px;border-right:0px;border-left:0px;background-color:#FFFFFF;color:red;}");
		}
	}
	
	return 0;
}

int CWeatherUi::getDesignedConfig(const hmiproto::hmiweather &weather){

	WEATHER_DESIGNED_CONFIG_T designedConfig;
	if (weather.has_designeddata()) {
		designedConfig.nViewHeight = weather.designeddata().height();
		designedConfig.nViewWidth = weather.designeddata().width();
		for (int nIndex = 0; nIndex < weather.designeddata().listimage_size(); ++nIndex) {
			WEATHER_DESIGNED_CONFIG_T::IMAGE_ELEMENT_T image;
			image.nX = weather.designeddata().listimage(nIndex).x();
			image.nY = weather.designeddata().listimage(nIndex).y();
			image.nW = weather.designeddata().listimage(nIndex).width();
			image.nH = weather.designeddata().listimage(nIndex).height();
			image.fOpacity = weather.designeddata().listimage(nIndex).opacity();
			image.sFilePath = weather.designeddata().listimage(nIndex).srcpath();
			designedConfig.vecImage.push_back(image);
		}
		for (int nIndex = 0; nIndex < weather.designeddata().listline_size(); ++nIndex) {
			WEATHER_DESIGNED_CONFIG_T::LINE_ELEMENT_T line;
			line.nX1 = weather.designeddata().listline(nIndex).x1();
			line.nY1 = weather.designeddata().listline(nIndex).y1();
			line.nX2 = weather.designeddata().listline(nIndex).x2();
			line.nY2 = weather.designeddata().listline(nIndex).y2();
			line.nStrokeWidth = weather.designeddata().listline(nIndex).strokewidth();
			line.sColor = weather.designeddata().listline(nIndex).color();
			designedConfig.vecLine.push_back(line);
		}
		for (int nIndex = 0; nIndex < weather.designeddata().listrect_size(); ++nIndex) {
			WEATHER_DESIGNED_CONFIG_T::RECT_ELEMENT_T rect;
			rect.nX = weather.designeddata().listrect(nIndex).x();
			rect.nY = weather.designeddata().listrect(nIndex).y();
			rect.nW = weather.designeddata().listrect(nIndex).width();
			rect.nH = weather.designeddata().listrect(nIndex).height();
			rect.fOpacity = weather.designeddata().listrect(nIndex).opacity();
			rect.sFillColor = weather.designeddata().listrect(nIndex).fillcolor();
			designedConfig.vecRect.push_back(rect);
		}
		for (int nIndex = 0; nIndex < weather.designeddata().listtext_size(); ++nIndex) {
			WEATHER_DESIGNED_CONFIG_T::TEXT_ELEMENT_T text;
			text.nX = weather.designeddata().listtext(nIndex).x();
			text.nY = weather.designeddata().listtext(nIndex).y();
			text.nW = weather.designeddata().listtext(nIndex).width();
			text.nH = weather.designeddata().listtext(nIndex).height();
			text.nAlignType = weather.designeddata().listtext(nIndex).align();
			text.sFont = weather.designeddata().listtext(nIndex).font();
			text.sText = weather.designeddata().listtext(nIndex).text();
			text.sColor = weather.designeddata().listtext(nIndex).color();
			designedConfig.vecText.push_back(text);
		}
	}
	m_config = designedConfig;

	return 0;
}

void CWeatherUi::slotSelectItem(const QModelIndex &index) {

	QString sSeltCity = index.data().toString();
	EventMgr::SendDataTransEvent(m_nScrNo, QString(m_sPartName.c_str()), sSeltCity);
	m_listView->setEnabled(false);
}

void CWeatherUi::slotBtnCancel() {

	EventMgr::SendDataTransEvent(m_nScrNo, QString(m_sPartName.c_str()), "0");
}

