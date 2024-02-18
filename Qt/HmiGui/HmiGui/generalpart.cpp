#include "generalpart.h"
#include <QRect>
#include <QDebug>
#include <QTimer>
#include <QLabel>
#include "imgcachemgr.h"
#include "hmiutility.h"
#include "commonpart.h"

GeneralPart::GeneralPart(const hmiproto::hmipartcommon &common, QWidget *parent) {
	m_part.Clear();
	m_parttype = NORMALPART;
	m_commandpart = NULL;
	m_glintTimer = NULL;
	m_qrlabel = NULL;


	m_bUpdateImg = false;
	m_bUpdateText = false;
	m_bUpdateHide = false;
	m_bUpdateGlint = false;
	m_bUpdateArea = false;

	m_TxtCrc32 = 0;            //文本校验和
	memset(&m_rc, 0, sizeof(RECT));//轮廓
	m_bUseShowHide = false;		//是否隐藏
	m_bUseGlint = false;		//是否闪烁
	m_strPartType = "";
	m_bIsShowGrayScale = false;
	m_nImgIndex = 0;
	m_nLockImgIndex = 0;
	m_part.MergeFrom(common);
}

GeneralPart::~GeneralPart() {
	try {
		if (m_commandpart != NULL) {
			delete m_commandpart;
			m_commandpart = NULL;
		}

		if (m_glintTimer != NULL) {
			delete m_glintTimer;
			m_glintTimer = NULL;
		}

		if (m_qrlabel != NULL) {
			delete m_qrlabel;
			m_qrlabel = NULL;
		}
	}
	catch (...) {
	}
}

void GeneralPart::Init(QWidget * pWidget) {
	//图片序号
	m_nImgIndex = m_part.basic().imgindex();
	m_nLockImgIndex = m_part.lockimg();
	m_bIsShowGrayScale = m_part.gray();

	//位图预览图片路径
	QString strTemp(m_part.imgpath().c_str());
	if (( BITMAPDISPLAY_INDEX == m_part.basic().imgindex() ) && m_part.has_imgpath()) {
		m_strImgPath = strTemp;
	}

	unsigned int unLen = 0;

	unLen = m_part.basic().text().length();
	m_TxtCrc32 = cbl::crc32(0, (const unsigned char*) m_part.basic().text().c_str(), unLen * sizeof(char));
	m_rc.left = m_part.basic().left();
	m_rc.right = m_part.basic().right();
	m_rc.top = m_part.basic().top();
	m_rc.bottom = m_part.basic().bottom();
	m_bUseShowHide = m_part.basic().hide();		//是否隐藏
	m_bUseGlint = m_part.basic().glint();			//是否闪烁

	//部件类型
	QString strPartType(m_part.basic().type().c_str());
	m_strPartType = strPartType;

	if (m_strPartType.compare("QRBitmap") == 0)//二维码部件)
	{
		m_parttype = QRPART;
		m_qrlabel = new QRLabel(pWidget);
		m_qrlabel->setGeometry(m_rc.left, m_rc.top, m_rc.right - m_rc.left, m_rc.bottom - m_rc.top);
	}
	else {
		m_commandpart = new CommonPart(pWidget, m_strPartType);
		m_commandpart->setAttribute(Qt::WA_TransparentForMouseEvents, true);

		if (m_strPartType.compare("RotatePic") == 0)//旋转位图
		{
			m_parttype = ROTATEPIC;
			QRect rect(m_rc.left, m_rc.top, m_rc.right - m_rc.left, m_rc.bottom - m_rc.top);
			m_commandpart->setRoateRect(rect);
		}
		else {
			m_parttype = NORMALPART;
			//设置大小
			m_commandpart->setGeometry(m_rc.left, m_rc.top, m_rc.right - m_rc.left, m_rc.bottom - m_rc.top);
			if (m_part.basic().dispdirect() > 0)//部件启用旋转功能
			{
				QRect rect(m_rc.left, m_rc.top, m_rc.right - m_rc.left, m_rc.bottom - m_rc.top);
				m_commandpart->setRoateRect(rect);
				m_part.set_degrees(90 * m_part.basic().dispdirect());
			}
        }
    }

	GetDrawFunc(DRAW_INIT);

#if ((defined WIN32) && (defined WIN32SCALE))
	if (NULL != m_commandpart) {
		m_commandpart->GetTextRect(m_part.basic(), &( m_commandpart->m_OriginalTextRect ));
	}

	memcpy(&m_OriginalRect, &m_rc, sizeof(RECT));
	ZoomPartSelf();
#endif
}

void GeneralPart::Update(const hmiproto::hmipartcommon &common, QWidget * pWidget) {
	(void) pWidget;
	m_part.MergeFrom(common);

	//是否更新图片
	if (m_nImgIndex != m_part.basic().imgindex() ||
		m_nLockImgIndex != m_part.lockimg() ||
		m_bIsShowGrayScale != (m_part.gray() == true) || 
		m_nLockWidth != (int)m_part.lockimgwidth())
    {
		m_nImgIndex = m_part.basic().imgindex();
		m_nLockImgIndex = m_part.lockimg();
		m_bIsShowGrayScale = m_part.gray();
		m_nLockWidth = m_part.lockimgwidth();
        m_bUpdateImg = true;
	}

	//这个判断针对位图预览，不影响之前
	QString strTemp(m_part.imgpath().c_str());
	if (( BITMAPDISPLAY_INDEX == m_part.basic().imgindex() )
		&& m_part.has_imgpath()						//仅当 位图->位图预览
		&& ( 0 != m_strImgPath.compare(strTemp) ))	//路径变化
	{
		m_strImgPath = strTemp;
		m_bUpdateImg = true;//更新
	}

	//是否更新文本
	unsigned long TxtCrc32 = 0;

	unsigned int unLen = 0;


	unLen = m_part.basic().text().length();
	TxtCrc32 = cbl::crc32(0, (const unsigned char*) m_part.basic().text().c_str(), unLen * sizeof(char));

	m_TxtCrc32 = TxtCrc32;

	/*这里先强制设为true，因为这里判断错误，
	 * 因为这里只单单对文本内容是否改变判断，
	 * 还涉及到文本样式，颜色等
	 */
	m_bUpdateText = true;

	RECT	rc;

	rc.left = m_part.basic().left();
	rc.right = m_part.basic().right();
	rc.top = m_part.basic().top();
	rc.bottom = m_part.basic().bottom();

	//是否更新轮廓
	if (memcmp(&rc, &m_rc, sizeof(RECT)) != 0) {
		memcpy(&m_rc, &rc, sizeof(RECT));
		m_bUpdateArea = true;
	}

	//是否更新隐藏
	if (m_bUseShowHide != ( m_part.basic().hide() == true )) {
		m_bUseShowHide = ( m_part.basic().hide() == true );
		m_bUpdateHide = true;
	}

	//是否更新闪烁
	if (m_bUseGlint != m_part.basic().glint()) {
		m_bUseGlint = m_part.basic().glint();
		m_bUpdateGlint = true;
	}

	GetDrawFunc(DRAW_UPDATE);

#if ((defined WIN32) && (defined WIN32SCALE))
	//是否更新轮廓
	if (memcmp(&rc, &m_OriginalRect, sizeof(RECT)) != 0) {
		if (NULL != m_commandpart) {
			m_commandpart->GetTextRect(common.basic(), &( m_commandpart->m_OriginalTextRect ));
		}

		if (memcmp(&rc, &m_OriginalRect, sizeof(RECT)) != 0) {
			memcpy(&m_OriginalRect, &rc, sizeof(RECT));
			ZoomPartSelf();
		}
	}
#endif
}

void GeneralPart::Draw(QWidget * pWidget, int drawtype) {
	foreach(DRAW_FUN draw_fun, m_drawList) {
		( this->*draw_fun )( pWidget, drawtype );
	}
}

void GeneralPart::OnPartHide() {
	if (m_commandpart) {
		m_commandpart->OnPartHide();
	}
}

void GeneralPart::OnPartShow() {
	if (m_commandpart) {
		m_commandpart->OnPartShow();
	}
}

#if ((defined WIN32) && (defined WIN32SCALE))
void GeneralPart::ZoomPartSelf() {
	if (NULL == m_commandpart)
		return;

	//部件显示区域设置
	RECT rectTemp = m_OriginalRect;
	HMIUtility::CountScaleRect(&rectTemp);

	int nLeft = rectTemp.left;
	int nTop = rectTemp.top;
	int nWidth = rectTemp.right - rectTemp.left;
	int nHeight = rectTemp.bottom - rectTemp.top;

    if (m_strPartType.compare("RotatePic") == 0 || m_part.basic().dispdirect() > 0)//旋转位图
	{
		QRect rect(nLeft, nTop, nWidth, nHeight);
		m_commandpart->setRoateRect(rect);


        QRect rect2(0,0,HMIUtility::m_nInitWidth,HMIUtility::m_nInitHeight);
		HMIUtility::CountScaleRect(&rect2);
		m_commandpart->setGeometry(rect2);
	}
	else {
		m_commandpart->setGeometry(nLeft, nTop, nWidth, nHeight);
	}

	if (!( m_commandpart->m_text.isEmpty() )) {
		//字体显示区域设置
		QRect rectTextTemp = m_commandpart->m_OriginalTextRect;
		HMIUtility::CountScaleRect(&rectTextTemp);
		m_commandpart->m_txtRect = rectTextTemp;

		//字体大小设置
		int nFontSizeH = m_commandpart->m_OriginalTxtFont.pixelSize();
		int nFontSizeW = m_commandpart->m_OriginalTxtFont.pixelSize();
		int nFontResult = 0;

		HMIUtility::CountScaleWH(&nFontSizeW, &nFontSizeH);
		if (nFontSizeW > nFontSizeH) {
			nFontResult = nFontSizeW;
		}
		else {
			nFontResult = nFontSizeH;
		}

		m_commandpart->m_txtFont.setPixelSize(nFontResult);
		m_commandpart->CalculateTextArea();
	}
}
#endif

void GeneralPart::DrawImg(QWidget * pWidget, int drawtype) {
	(void) pWidget;
	(void) drawtype;

	if (NULL == m_commandpart) {
		return;
	}

	if (BITMAPDISPLAY_INDEX == m_part.basic().imgindex() &&
		m_part.has_imgpath())//位图预览
	{
		QString strTemp(m_part.imgpath().c_str());
		RECT	rc = { 0 };

		rc.left = m_part.basic().left();
		rc.right = m_part.basic().right();
		rc.top = m_part.basic().top();
		rc.bottom = m_part.basic().bottom();

		m_commandpart->SetIMGPath(strTemp, rc);
	}
	else//原先的方式
	{
		m_commandpart->SetIMGIndex(m_part.basic().imgindex());
	}

	if (m_commandpart != NULL &&//二维码部件m_commandpart变量为NULL，因此需要加上判断 Lizh 20200225
		true == m_part.has_lockimg() &&
		true == m_part.has_lockimgwidth()) {
		m_nLockImgIndex = m_part.lockimg();
		m_commandpart->setLockImgInfo(m_part.lockimg(), m_part.lockimgwidth(), m_part.gray());
	}
}

void GeneralPart::DrawTxt(QWidget * pWidget, int drawtype) {
	(void) pWidget;
	if (m_parttype == NORMALPART) {

#if ((defined WIN32) && (defined WIN32SCALE))
		int nLeft = m_OriginalRect.left;
		int nTop = m_OriginalRect.top;
		int nWidth = m_OriginalRect.right - m_OriginalRect.left;
		int nHeight = m_OriginalRect.bottom - m_OriginalRect.top;
		m_commandpart->setGeometry(nLeft, nTop, nWidth, nHeight);
#endif

		m_commandpart->setText(m_part, drawtype);

#if ((defined WIN32) && (defined WIN32SCALE))
		memcpy(&m_commandpart->m_OriginalTextRect, &( m_commandpart->m_txtRect ), sizeof(RECT));
		ZoomPartSelf();
#else
		m_commandpart->CalculateTextArea();
#endif
	}
	else if (m_parttype == QRPART) {
		QString strText(m_part.basic().text().c_str());
		m_qrlabel->setQRData(strText);
	}
}

void GeneralPart::DrawShowHide(QWidget * pWidget, int drawtype) {
	(void) pWidget;
	//初始化并且不使用隐藏
	if (drawtype == DRAW_INIT && !m_part.basic().hide()) {
		return;
	}

	//初始化并且使用隐藏
	if (drawtype == DRAW_INIT && m_part.basic().hide()) {
		DoShowHide(false);
		return;
	}

	//更新处理
	if (drawtype == DRAW_UPDATE) {
		DoShowHide(!m_part.basic().hide());
	}
}

void GeneralPart::DoShowHide(bool bShow) {
	
	QWidget *part = NULL;
    if (m_parttype == NORMALPART || m_parttype == ROTATEPIC)
    {
        part = static_cast<QWidget *>(m_commandpart);
	}
	else if (m_parttype == QRPART) {
		part = static_cast<QWidget *>( m_qrlabel );
	}

	if (part == NULL) {
		return;
	}

	if (bShow) {
		part->show();
	}
	else {
		part->hide();
	}

}

void GeneralPart::DrawArea(QWidget * pWidget, int drawtype) {
	(void) pWidget;
	(void) drawtype;

	RECT	rc;

	rc.left = m_part.basic().left();
	rc.right = m_part.basic().right();
	rc.top = m_part.basic().top();
	rc.bottom = m_part.basic().bottom();

#if ((defined WIN32) && (defined WIN32SCALE))
	RECT	rectTemp;

	memcpy(&rectTemp, &rc, sizeof(RECT));
	if (m_part.basic().dispdirect() > 0)//旋转部件
	{
		QRect rect(m_rc.left, m_rc.top, m_rc.right - m_rc.left, m_rc.bottom - m_rc.top);
		m_commandpart->setRoateRect(rect);
		QRect rect2(0, 0, HMIUtility::m_nInitWidth, HMIUtility::m_nInitHeight);
		HMIUtility::CountScaleRect(&rect2);
		m_commandpart->setGeometry(rect2);
	}
	else
	{
		HMIUtility::CountScaleRect(&rectTemp);
		m_commandpart->setGeometry(rectTemp.left, rectTemp.top, rectTemp.right - rectTemp.left, rectTemp.bottom - rectTemp.top);
	}
   
#else
	if (m_part.basic().dispdirect() > 0)//旋转部件
	{
		QRect rect(m_rc.left, m_rc.top, m_rc.right - m_rc.left, m_rc.bottom - m_rc.top);
		m_commandpart->setRoateRect(rect);
		m_commandpart->setGeometry(0, 0, pWidget->width(), pWidget->height());
	}
	else
	{
		m_commandpart->setGeometry(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
	}
   
#endif
}

void GeneralPart::DrawGlint(QWidget * pWidget, int drawtype) {
	(void) drawtype;

	if (m_part.basic().glint()) {
		//创建定时器
		if (m_glintTimer == NULL) {
			m_glintTimer = new QTimer(pWidget);
			connect(m_glintTimer, SIGNAL(timeout()), this, SLOT(DoGlint()));
		}

		//启动
		m_glintTimer->start(m_part.basic().glintfreq());
	}
	else {
		//停止
		if (m_glintTimer != NULL) {
			m_glintTimer->stop();
			if (!m_commandpart->isVisible())
				m_commandpart->show();
		}
	}

}

void GeneralPart::DoGlint() {
	m_commandpart->setVisible(!m_commandpart->isVisible());
}

void GeneralPart::DrawRotatePic(QWidget * pWidget, int drawtype) {
	(void) pWidget;
	(void) drawtype;

	//设置旋转
	m_commandpart->setRotate(m_part.degrees());
	if (m_commandpart->geometry() != pWidget->geometry()) {
		m_commandpart->setGeometry(0, 0, pWidget->width(), pWidget->height());
	}
}

void GeneralPart::DrawSelect(QWidget * pWidget, int drawtype) {
	(void)pWidget;
	(void)drawtype;

	if (nullptr != m_commandpart){
		//设置选中
		m_commandpart->setSelect(m_part.basic().showselect(),m_part.basic().selectmode());
	}

}

void GeneralPart::DrawUpdate(QWidget * pWidget, int drawtype) {
	(void) pWidget;
	(void) drawtype;
	if (m_parttype == NORMALPART || m_parttype == ROTATEPIC) {
		m_commandpart->update();
	}
}

void GeneralPart::GetDrawFunc(int drawtype) {
	m_drawList.clear();
	if (m_parttype == NORMALPART && m_commandpart == NULL) {
		return;
	}
	else if (m_parttype == QRPART && (m_qrlabel == NULL)) {
		return;
	}

	// 将部件轮廓处理逻辑移到最开始，防止其它功能会使用到部件的轮廓属性 xqh 20210114
	if (m_bUpdateArea)	// 移动缩放，只在更新时才处理
	{
		m_drawList.append(&GeneralPart::DrawArea);
		m_bUpdateArea = false;
	}

    if (m_parttype == ROTATEPIC || m_part.basic().dispdirect() > 0 )
    {
        if (drawtype == DRAW_INIT ||(m_commandpart != NULL && m_commandpart->getRotate() != m_part.degrees()))
        {

			m_drawList.append(&GeneralPart::DrawRotatePic);
		}
	}

	if (drawtype == DRAW_INIT || m_bUpdateImg)//显示图片
	{
		m_drawList.append(&GeneralPart::DrawImg);
		m_bUpdateImg = false;
	}

	if (drawtype == DRAW_INIT || m_bUpdateText) {
		m_drawList.append(&GeneralPart::DrawTxt);
		m_bUpdateText = false;
	}

	//闪烁处理
	if (drawtype == DRAW_INIT || m_bUpdateGlint) {
		m_drawList.append(&GeneralPart::DrawGlint);
		m_bUpdateGlint = false;
	}

	//隐藏处理
	if (drawtype == DRAW_INIT || m_bUpdateHide) {
		m_drawList.append(&GeneralPart::DrawShowHide);
		m_bUpdateHide = false;
	}

	if (m_commandpart != NULL &&
		(drawtype == DRAW_INIT ||
		m_commandpart->getSelect() != m_part.basic().showselect() ||
		m_commandpart->getSelectMode() != m_part.basic().selectmode())){

		m_drawList.append(&GeneralPart::DrawSelect);
	}

	//更新
	if (m_drawList.size() > 0) {
		m_drawList.append(&GeneralPart::DrawUpdate);
	}
}
