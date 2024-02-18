#include "commonpart.h"
#include <QDebug>
#include <QLabel>
#include <QPainter>
#include <QColor>
#include <QFont>
#include <QtSvg/QSvgRenderer>
#include <QMovie>
#include <QPaintEvent>
#include <QLabel>
#include <QBitmap>
#include "hmiutility.h"
#include "macrosplatform/path.h"
#include "GlobalModule/GlobalModule.h"

FONTINFO *CommonPart::m_Qpf2Info = NULL;
int CommonPart::m_nQpf2fFileCount = 0;
unsigned short CommonPart::m_wQpf2fFileVersion = 0;

enum Align {
	TextCenter = 0,
	TextLeft = 1,
	TextRight = 2,
};

void CommonPart::setLockImgInfo(int nLockImgIndex, int nLockWidth, bool bIsShowGrayScale) {
	m_nLockImgIndex = nLockImgIndex;
	m_nLockWidth = nLockWidth;
	m_bIsShowGrayScale = bIsShowGrayScale;
}

CommonPart::CommonPart(QWidget *pWidget, QString parttype) : QWidget(pWidget) {
	m_nImgIndex = BITMAPDISPLAY_INDEX;
	m_strImgPath = "";
	m_pImgInfo = NULL;
	m_pCustomImage = NULL;
	m_text = "";
	m_bTxtColor = false;
	m_bTxtBkColor = false;
	m_svg = NULL;
	m_movie = NULL;
	m_degrees = 0;
	m_bRoate = false;
	m_parttype = parttype;
	m_txtFlags = 0;
	m_strGifpath = "";
	m_pSvgImage = NULL;
	m_nLockImgIndex = -1;
	m_nLockWidth = 0;
	m_bIsShowGrayScale = false;
	m_nFreq = 0;
	memset(&m_Rect, 0, sizeof(RECT));
	m_nAlign = 0;
	m_bShowSelect = false;
	m_nSelectMode = CurSorMode_Default;
}

CommonPart::~CommonPart() {
	try {
		if (m_svg != NULL) {
			m_svgmutex.lock();
			delete m_svg;
			m_svg = NULL;
			m_svgmutex.unlock();
		}

		if (m_movie != NULL) {
			delete m_movie;
			m_movie = NULL;
		}

		if (m_pSvgImage) {
			delete m_pSvgImage;
			m_pSvgImage = NULL;
		}

		if (m_pCustomImage) {
			delete m_pCustomImage;
			m_pCustomImage = NULL;
		}
	}
	catch (...) {
	}
}

void CommonPart::ReadQpf2File() {
#ifdef WIN32
	return;
#else
	FILE		*fp = NULL;

	fp = fopen(PROJECT_FONT_PATH, "rb");
	if (fp != NULL) {
		//文件前6个字节为版本号与qpf2文件个数
		fread(&m_wQpf2fFileVersion, 2, 1, fp);
		fread(&m_nQpf2fFileCount, 4, 1, fp);
		m_Qpf2Info = (FONTINFO *) malloc(m_nQpf2fFileCount * sizeof(FONTINFO) + 1);
		if (m_Qpf2Info == NULL) {
			fclose(fp);
			return;
		}
		memset(m_Qpf2Info, 0, m_nQpf2fFileCount * sizeof(FONTINFO) + 1);
		fread(m_Qpf2Info, sizeof(FONTINFO) * m_nQpf2fFileCount, 1, fp);
		fclose(fp);;
	}
#endif
}

bool CommonPart::FindFontInfo(int nWeight, int nStyle, QString &strFontFamily, QString strFontSize) {
#ifdef WIN32 
	nWeight = nWeight;
	nStyle = nStyle;
	strFontFamily = strFontFamily;
	strFontSize = strFontSize;
#else
	QString strFontName;

	for (int nI = 0; nI < m_nQpf2fFileCount; nI++) {
		QStringList strList;

		strFontName.clear();
		strFontName = m_Qpf2Info[nI].szFontFamily;
		strList = strFontName.split("_");
		// 如果部件使用的字体类型xxx在qpf文件字体中有对应的xxx_qpf2类型，
		// 则需要替换成xxx_qpf2类型字体，判断对应字体是不能用包含关系判断，
		// 否则会出现宋体匹配到新宋体这种情况 xqh 20200718
		//if(strFontName.contains(strFontFamily)  &&
		if (strList.size() > 0 &&
			strList[0] == strFontFamily &&
			m_Qpf2Info[nI].nStyle == nStyle &&
			m_Qpf2Info[nI].nWeight == nWeight &&
			m_Qpf2Info[nI].nFontSize == strFontSize.toInt()) {
			strFontFamily.clear();
			strFontFamily = strFontName;

			return true;
		}
	}
#endif

	return false;
}

void CommonPart::setRoateRect(QRect r) {
	m_roateRect.setRect(r.x(), r.y(), r.width(), r.height());
}

void CommonPart::setRotate(int degress) {
	m_bRoate = true;
	m_degrees = degress;
}

int CommonPart::getRotate() {
	return m_degrees;
}

void CommonPart::calculateCustomTextArea() {
	QFontMetrics    fm(m_txtFont);
	QRect           partRect = this->geometry();
	QRect           rc = fm.boundingRect(m_txtRect, m_txtFlags, m_text);

	if (m_txtRect.left() + rc.width() > partRect.width()) // 宽度超出了部件的宽度
	{
		m_txtRect.setLeft(0);
		m_txtRect.setWidth(partRect.width());
	}
	else if (rc.width() > m_txtRect.width())  // 宽度超出了上位机打包的宽度
	{
		m_txtRect.setWidth(rc.width());
	}

	if (m_txtRect.top() + rc.height() > partRect.height()) // 高度超出了部件的高度
	{
		m_txtRect.setTop(0);
		m_txtRect.setHeight(partRect.height());
	}
	else if (rc.height() > m_txtRect.height())  // 高度超出了上位机打包的高度
	{
		m_txtRect.setHeight(rc.height());
	}

	return;

}

void CommonPart::calculateStaticTextArea() {
	if (0 != m_parttype.compare("Text")) {
		return;
	}

	QFontMetrics    fm(m_txtFont);
	QRect           partRect = this->geometry();
	QRect           rc = fm.boundingRect(partRect, m_txtFlags, m_text);
	int             nMove = 0;
	bool            bReset = false;

	m_txtBgRect = rc;

	if (rc.width() > partRect.width()) {
		switch (m_nAlign) {
		case TextCenter:    // 居中
			{
				nMove = ( rc.width() - partRect.width() ) / 2;
			}
			break;
		case TextLeft:      // 居左
			{
				nMove = 0;
			}
			break;
		case TextRight:     // 居右
			{
				nMove = rc.width() - partRect.width();
			}
			break;
		default:
			{
				return;
			}
			break;
		}

		bReset = true;
		partRect.setLeft(partRect.left() - nMove);
		partRect.setWidth(rc.width());
	}

	if (rc.height() > partRect.height()) {
		partRect.setHeight(rc.height());
		bReset = true;
	}

	if (false == bReset) {
		return;
	}

	m_txtRect.setWidth(partRect.width());
	m_txtRect.setHeight(partRect.height());
	this->setGeometry(partRect);
	return;
}

void CommonPart::CalculateTextArea() {
	if (m_text.length() <= 0) {
		return;
	}
	if (0 == m_parttype.compare("Text")) // 静态文本
	{
		calculateStaticTextArea();
	}
	else if (3 == m_nAlign) // 自定义文本位置
	{
		calculateCustomTextArea();
	}
	return;
}


void CommonPart::setAlign(int nAlign) {

	if (0 == m_parttype.compare("Text")) {
		if (TextCenter == nAlign) {
			m_txtFlags = Qt::AlignHCenter | Qt::AlignTop;
		}
		else if (TextLeft == nAlign) {
			m_txtFlags = Qt::AlignLeft | Qt::AlignTop;
		}
		else if (TextRight == nAlign) {
			m_txtFlags = Qt::AlignRight | Qt::AlignTop;
		}
	}
	else {
		if (TextCenter == nAlign) {
			m_txtFlags = Qt::AlignCenter;
		}
		else if (TextLeft == nAlign) {
			m_txtFlags = Qt::AlignVCenter | Qt::AlignLeft;
		}
		else if (TextRight == nAlign) {
			m_txtFlags = Qt::AlignVCenter | Qt::AlignRight;
		}
	}

	return;
}

void CommonPart::setText(const hmiproto::hmipartcommon common, int drawtype) {
	int		nWeight = 50;
	int		nStyle = 0;

	(void) drawtype;
	QString strText(common.basic().text().c_str());
	m_text = strText;
	//字体
	QString strFont(common.basic().font().c_str());
	if (strFont.isEmpty() /*|| strFont.length() > 32*/)//封掉对长度的判断，否则会导致部分字体不显示 Lizh 20191220
	{                                                  //如：（1）italic 26px Franklin Gothic Medium（2）italic 26px Leelawadee UI Semilight（3）italic 26px Malgun Gothic Semilight等
		return;
	}
	QString family = "";
	QString fontsize = "";
	int nIndex = 0;
	QString strItalic = "italic";
	QString strBold = "bold";
	if (strFont.contains(strItalic)) {
		nStyle = 1;
		m_txtFont.setItalic(true);
		nIndex = nIndex + strItalic.length() + 1;
	}
	else {
		m_txtFont.setItalic(false);
	}

	if (strFont.contains(strBold)) {
		nWeight = 75;
		m_txtFont.setBold(true);
		nIndex = nIndex + strBold.length() + 1;
	}
	else
		m_txtFont.setBold(false);
	for (; nIndex < strFont.length(); nIndex++) {
		if (strFont.at(nIndex) == 'p') {
			break;
		}

		fontsize.append(strFont.at(nIndex));
	}
	nIndex += 3;
	for (; nIndex < strFont.length(); nIndex++) {
		family.append(strFont.at(nIndex));
	}

	if (common.basic().type().compare("String"))//只有非动态文本才qpf2文件中获取字体名（family） Lizh 20191223
	{
		FindFontInfo(nWeight, nStyle, family, fontsize);
	}

	m_txtFont.setFamily(family);
	m_txtFont.setPixelSize(fontsize.toInt());

	//文本颜色
	m_txtColor.setRgb(QRgb(common.basic().textcolor()));
	m_bTxtColor = true;

	//背景颜色
	m_bTxtBkColor = false;
	if (common.has_bgcolor()) {
		m_txtBkColor.setRgb(QRgb(common.bgcolor()));
		m_bTxtBkColor = true;
	}

	//对齐方式
    
	if (m_bRoate)
	{
		m_txtRect = m_roateRect;
	}
	else
	{
		m_txtRect = this->rect();
	}
	m_nAlign = common.basic().align();
	if (common.basic().align() == 3)//自定义位置
	{
		int		nX = 0, nY = 0;
		int     nHeight = 0, nWidth = 0;

		sscanf(common.basic().textcoordinate().c_str(), "%d %d", &nX, &nY);
		sscanf(common.basic().textarea().c_str(), "%d %d", &nWidth, &nHeight);
		m_txtRect.setLeft(nX);
		m_txtRect.setTop(nY);
		m_txtRect.setWidth(nWidth);
		m_txtRect.setHeight(nHeight);

		switch (common.basic().textalign()) {
		case TextCenter:
			{
				m_txtFlags = Qt::AlignCenter;
			}
			break;
		case TextLeft:
			{
				m_txtFlags = Qt::AlignLeft;
			}
			break;
		case TextRight:
			{
				m_txtFlags = Qt::AlignRight;
			}
			break;
		default:
			{
				m_txtFlags = Qt::AlignLeft;
			}
			break;
		}

	}
	else {
		setAlign(common.basic().align());
	}

#if ((defined WIN32) && (defined WIN32SCALE))
	m_OriginalTxtFont = m_txtFont;
#endif
}

void CommonPart::drawImg_NORMAL(QPainter& p) {
	if (m_pImgInfo == NULL ||
		m_pImgInfo->nImgType != ImgCacheMgr::NORMALIMG ||
		m_pImgInfo->pix == NULL) {
		return;
	}

	if (m_movie != NULL) {
		m_movie->stop();
	}

	if (m_bRoate) {
		p.drawPixmap(m_roateRect, *m_pImgInfo->pix);
	}
	else {
		p.drawPixmap(this->rect(), *m_pImgInfo->pix);
	}
}

void CommonPart::drawImg_GIF(QPainter& p) {
	if (m_pImgInfo == NULL ||
		m_pImgInfo->nImgType != ImgCacheMgr::GIFIMG) {
		return;
	}

	if (m_movie == NULL) {
		qDebug() << "draw gif...";
		m_movie = new QMovie();
		m_movie->setFileName(m_pImgInfo->strImgPath);
		m_strGifpath = m_pImgInfo->strImgPath;
		if (m_movie->speed() > 50) {
			m_movie->setSpeed(50);//这里先限速50
		}
		connect(m_movie, SIGNAL(updated(QRect)), this, SLOT(movieUpdated(QRect)));
	}
	else {
		if (0 != m_strGifpath.compare(m_pImgInfo->strImgPath)) {
			m_movie->stop();
			m_movie->setFileName(m_pImgInfo->strImgPath);
			m_strGifpath = m_pImgInfo->strImgPath;
		}
	}
	m_movie->start();
	if (m_bRoate) {
		p.drawPixmap(m_roateRect, m_movie->currentPixmap());
	}
	else {
		p.drawPixmap(this->rect(), m_movie->currentPixmap());
	}
}

void CommonPart::drawImg_SVG(QPainter& p) {
	if (m_pImgInfo == NULL ||
		m_pImgInfo->svg == NULL ||
		m_pImgInfo->nImgType != ImgCacheMgr::SVGIMG) {
		return;
	}

	if (m_movie != NULL) {
		m_movie->stop();
	}

	if (m_bRoate) {
		m_pImgInfo->svg->render(&p, QRectF(m_roateRect));
	}
	else {
		m_pImgInfo->svg->render(&p);
	}
}

void CommonPart::drawImg_LOCK(QPainter& p) {

	if (m_nLockImgIndex != -1) {
		IMG_T	*pImgInfo = NULL;
		QRect		rc(this->rect().top(), this->rect().left(), this->rect().left() + m_nLockWidth, this->rect().top() + m_nLockWidth);
		if (m_bRoate)
		{
			rc.setLeft(m_roateRect.x());
			rc.setTop(m_roateRect.y());
			rc.setWidth(m_nLockWidth);
			rc.setHeight(m_nLockWidth);
		}
		pImgInfo = ImgCacheMgr::GetImgInfo(m_nLockImgIndex);

		pImgInfo->mutex.lock();
		p.drawPixmap(rc, *pImgInfo->pix);
		pImgInfo->mutex.unlock();
	}
}

void CommonPart::drawImg(QPainter& p) {
	//在缓存
	if (m_nImgIndex != BITMAPDISPLAY_INDEX) {
		m_pImgInfo = ImgCacheMgr::GetImgInfo(m_nImgIndex);
		if (m_pImgInfo == NULL) {
			return;
		}

		m_pImgInfo->mutex.lock();
		if (m_pImgInfo->nImgType == ImgCacheMgr::SVGIMG) {
			drawImg_SVG(p);
		}
		else if (m_pImgInfo->nImgType == ImgCacheMgr::GIFIMG) {
			drawImg_GIF(p);
		}
		else if (m_pImgInfo->nImgType == ImgCacheMgr::NORMALIMG) {
			drawImg_NORMAL(p);
		}
		m_pImgInfo->mutex.unlock();
	}
	else//不在缓存
	{
		if (m_strImgPath.size() <= 0) {
			return;
		}

		if (m_pCustomImage == NULL) {
			m_pCustomImage = new QImage;
		}

		ImgCacheMgr::LoadCustomIMG(m_strImgPath, *m_pCustomImage, m_Rect);

		if (m_movie != NULL) {
			m_movie->stop();
		}

		if (m_bRoate) {
			p.drawImage(m_roateRect, *m_pCustomImage);
		}
		else {
			p.drawImage(this->rect(), *m_pCustomImage);
		}

		if (m_pCustomImage) {
			delete m_pCustomImage;
			m_pCustomImage = NULL;
		}
	}

	drawImg_LOCK(p);//画部件锁
}

void CommonPart::drawText(QPainter& p) {
	if (m_text.isEmpty()) {
		return;
	}
	p.setFont(m_txtFont);

	if (m_bTxtColor) {
		if (m_bIsShowGrayScale) {
			m_txtColor.setRgb(220, 220, 220);
		}
		p.setPen(m_txtColor);
	}

	if (m_bTxtBkColor) {
		p.setBackgroundMode(Qt::OpaqueMode);
		p.setBackground(QBrush(m_txtBkColor));

		m_txtBgRect.setLeft(m_txtRect.left());
		m_txtBgRect.setTop(m_txtRect.top());
		p.fillRect(m_txtBgRect, QBrush(m_txtBkColor));
	}

	QRect brRect;
	if (m_bRoate)
	{
		QRect roateRect;
		if (m_nAlign == 3)//自定义位置
		{
			roateRect.setLeft(m_txtRect.x() + m_roateRect.x());
			roateRect.setTop(m_txtRect.y() + m_roateRect.y());
			roateRect.setWidth(m_txtRect.width());
			roateRect.setHeight(m_txtRect.height());
		}
		else
		{
			roateRect = m_roateRect;
		}
		

		p.drawText(roateRect, m_txtFlags, m_text, &brRect);
	}
	else
	{
		p.drawText(m_txtRect, m_txtFlags, m_text, &brRect);
	}
}

void CommonPart::paintEvent(QPaintEvent *qpevent) {
	(void) qpevent;
	QPainter p(this);

	if (m_pSvgImage) {
		p.drawImage(m_SvgRect, *m_pSvgImage);//LYQ 20180517 将SVG变成图片进行显示
	}
	//旋转
	if (m_bRoate) {
		p.resetTransform();

		//中心点
		qreal dX = m_roateRect.x() + m_roateRect.width() / 2;
		qreal dY = m_roateRect.y() + m_roateRect.height() / 2;

		p.translate(dX, dY);

		//旋转角度
		p.rotate(m_degrees);

		p.translate(-dX, -dY);
	}


	//显示图片
	drawImg(p);

	//显示文本
	drawText(p);

	if (m_bShowSelect) {
		if (CurSorMode_Rect == m_nSelectMode){
			QPen  pen;
			pen.setColor("red");
			pen.setWidth(5);
			pen.setStyle(Qt::SolidLine);
			p.setPen(pen);
			p.drawRoundedRect(this->rect(), 2, 2);
		}
		else if(CurSorMode_GrayPlacement == m_nSelectMode){
			p.setBrush(QBrush(QColor(0, 0, 0, 100)));
			p.drawRoundedRect(this->rect(), 2, 2);
		}
	}
	//画SVG
	if (m_svg != NULL) {
		m_svgmutex.lock();
		m_svg->render(&p);
		m_svgmutex.unlock();
	}
}

int CommonPart::getFreq() {
	return m_nFreq;
}

void CommonPart::setFreq(int nFreq) {
	m_nFreq = nFreq;
}

void CommonPart::setSvg(const QByteArray &contents, const std::string &strPartName, bool bAll) {
	m_svgmutex.lock();
	if (true == bAll) {
		if (NULL != m_svg) {
			delete m_svg;
			m_svg = NULL;
		}
		m_svg = new QSvgRenderer();
		m_svg->setFramesPerSecond(m_nFreq);
		m_svg->load(contents);
		QObject::connect(m_svg, SIGNAL(repaintNeeded()), this, SLOT(svgRepaintNeeded()));
	}
	else if (false == bAll &&
		NULL != m_svg &&
		( "MeterGraph" == strPartName ||
		"Clock" == strPartName ||
		"BarGraph" == strPartName ||
		"FlowPart" == strPartName )) {
		QString						sId;
		QXmlStreamAttributes		qXmlAttributes;
		QList<QByteArray>			qListLabels;
		QList<QByteArray>			qListAttributes;
		QList<QByteArray>			qListAttributePairs;
		QList<QByteArray>			qListValue;

		if (!contents.isEmpty()) {
			qListLabels = contents.split('|');
			for (auto &iter : qListLabels) {
				if (iter.isEmpty()) {
					continue;
				}
				qListAttributes = iter.split(':');
				if (qListAttributes.size() < 2) {
					continue;
				}
				sId.append(qListAttributes[0]);
				qListAttributePairs = qListAttributes[1].split(',');
				for (auto &iterAttr : qListAttributePairs) {
					if (iterAttr.isEmpty()) {
						continue;
					}
					qListValue = iterAttr.split('=');
					if (qListValue.size() < 2) {
						continue;
					}
					qXmlAttributes.append(qListValue[0], qListValue[1]);
				}
#ifdef _LINUX_ //模拟时不使用局部更新机制
				m_svg->load(sId, qXmlAttributes);
#endif
				sId.clear();
				qXmlAttributes.clear();
			}
		}
		if ("FlowPart" == strPartName) {
			m_svg->setFramesPerSecond(m_nFreq);
		}
	}

	m_svgmutex.unlock();
}

void CommonPart::setImg(const QByteArray &contents, RECT rc) {
	m_svgmutex.lock();

	QSvgRenderer *pSvg = new QSvgRenderer(contents);
	if (pSvg != NULL) {
		if (m_pSvgImage != NULL) {
			delete m_pSvgImage;
			m_pSvgImage = NULL;
		}

		m_SvgRect.setRect(0, 0, rc.right - rc.left, rc.bottom - rc.top);
		m_pSvgImage = new QImage(rc.right - rc.left, rc.bottom - rc.top, QImage::Format_RGB32);
		QPainter painter(m_pSvgImage);
		pSvg->render(&painter);

		delete pSvg;
	}

	m_svgmutex.unlock();
}

void CommonPart::svgRepaintNeeded() {
	this->update();
}

void CommonPart::movieUpdated(const QRect& rc) {
	(void) rc;
	this->update();
}

void CommonPart::OnPartHide() {
	if (m_movie) {
		m_movie->stop();
	}
}

void CommonPart::OnPartShow() {
	if (m_movie) {
		m_movie->start();
	}
}
#if ((defined WIN32) && (defined WIN32SCALE))
void CommonPart::GetTextRect(const hmiproto::hmipartbasic &basic, QRect *rectTemp) {
	QString strText(basic.text().c_str());

	QString strFont(basic.font().c_str());
	if (strFont.isEmpty() || strFont.length() > 32) {
		return;
	}

	QFont font;
	QString family = "";
	QString fontsize = "";
	int nIndex = 0;
	QString strItalic = "italic";
	QString strBold = "bold";
	if (strFont.contains(strItalic)) {
		font.setItalic(true);
		nIndex = nIndex + strItalic.length() + 1;
	}
	if (strFont.contains(strBold)) {
		font.setBold(true);
		nIndex = nIndex + strBold.length() + 1;
	}
	for (; nIndex < strFont.length(); nIndex++) {
		if (strFont.at(nIndex) == 'p') {
			break;
		}

		fontsize.append(strFont.at(nIndex));
	}
	nIndex += 3;
	for (; nIndex < strFont.length(); nIndex++) {
		family.append(strFont.at(nIndex));
	}

	font.setFamily(family);
	font.setPixelSize(fontsize.toInt());

	*rectTemp = this->rect();

	if (basic.align() == 3)//自定义位置
	{
		int x, y;

		sscanf(basic.textcoordinate().c_str(), "%d %d", &x, &y);

		rectTemp->setLeft(x);
		rectTemp->setTop(y);
	}

	QString strPartType(basic.type().c_str());
	//如果是文本部件，要重新设置下的宽度，高度
	if (strPartType.compare("Text") == 0)//文本部件
	{
		QLabel lblTxt(strText);
		lblTxt.setFont(font);
		lblTxt.setGeometry(this->geometry());
		lblTxt.adjustSize();
		QRect rect = this->geometry();
		if (lblTxt.geometry().width() > rect.width()) {
			rectTemp->setWidth(lblTxt.width());
		}
		if (lblTxt.geometry().height() > rect.height()) {
			rectTemp->setHeight(lblTxt.height());
		}
	}
}
#endif

void CommonPart::SetIMGPath(const QString &strImgPath, const RECT &rc) {
	m_strImgPath = strImgPath;
	m_nImgIndex = BITMAPDISPLAY_INDEX;
	memcpy(&m_Rect, &rc, sizeof(RECT));
}

void CommonPart::SetIMGIndex(const int &nIndex) {
	m_strImgPath = "";
	m_nImgIndex = nIndex;
}

void CommonPart::setSelect(bool bShowSelect,int nSelectMode) {
	m_bShowSelect = bShowSelect;
	m_nSelectMode = nSelectMode;
}

bool CommonPart::getSelect() {
	return m_bShowSelect;
}

int CommonPart::getSelectMode() {
	return m_nSelectMode;
}