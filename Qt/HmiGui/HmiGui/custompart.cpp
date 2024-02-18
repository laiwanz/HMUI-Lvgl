#include "custompart.h"
#include <QDebug>
#include <QtSvg/QSvgRenderer>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include "hmiutility.h"
#include "commonpart.h"

custompart::custompart(const hmiproto::hmicustom &custom, QWidget *parent) {
	m_svg = NULL;
	m_bUseShowHide = false;
	m_part.Clear();
	m_part.MergeFrom(custom);
}

custompart::~custompart() {
	try {
		if (m_svg != NULL) {
			delete m_svg;
			m_svg = NULL;
		}
	}
	catch (...) {
	}
}

//初始化数据
void custompart::Init(QWidget * pWidget) {
	RECT	rc;

	rc.left = m_part.basic().left();
	rc.right = m_part.basic().right();
	rc.top = m_part.basic().top();
	rc.bottom = m_part.basic().bottom();
	QString strPartType(m_part.basic().type().c_str());

	if (m_svg == NULL) {
		m_svg = new CommonPart(pWidget, strPartType);
		m_svg->setAttribute(Qt::WA_TransparentForMouseEvents, true);//允许鼠标点击穿透
		int nLeft = rc.left;
		int nTop = rc.top;
		int nWidth = rc.right - rc.left;
		int nHeight = rc.bottom - rc.top;
		memcpy(&m_rc, &rc, sizeof(RECT));
		m_svg->setGeometry(nLeft, nTop, nWidth, nHeight);
		m_svg->setFreq(m_part.freq());
	}

#if ((defined WIN32) && (defined WIN32SCALE))
	if (NULL != m_svg) {
		m_svg->GetTextRect(m_part.basic(), &( m_svg->m_OriginalTextRect ));
	}
	memcpy(&m_OriginalRect, &rc, sizeof(RECT));
	ZoomPartSelf();
#endif
}

//收到更新数据
void custompart::Update(const hmiproto::hmicustom &custom, QWidget * pWidget) {
	RECT	rc;
	(void) pWidget;
	m_part.MergeFrom(custom);

	rc.left = m_part.basic().left();
	rc.right = m_part.basic().right();
	rc.top = m_part.basic().top();
	rc.bottom = m_part.basic().bottom();
	if (memcmp(&m_rc, &rc, sizeof(RECT)) != 0) {
		memcpy(&m_rc, &rc, sizeof(RECT));
		if (m_svg) {
			m_svg->setGeometry(m_rc.left, m_rc.top, m_rc.right - m_rc.left,
				m_rc.bottom - m_rc.top);
		}
	}

	//帧率更新处理 Linzhen 20200612
	unsigned int nFreq = m_svg->getFreq();
	if (nFreq != m_part.freq()) {
		m_svg->setFreq(m_part.freq());
	}

#if ((defined WIN32) && (defined WIN32SCALE))

	if (NULL != m_svg) {
		m_svg->GetTextRect(m_part.basic(), &( m_svg->m_OriginalTextRect ));
	}

	if (memcmp(&m_rc, &rc, sizeof(RECT)) != 0) {
		memcpy(&m_OriginalRect, &rc, sizeof(RECT));
		ZoomPartSelf();
	}
#endif
}

//绘图
void custompart::Draw(QWidget * pWidget, int) {
	(void) pWidget;
	if (!m_part.has_content()) {
		return;
	}

	bool		bAll;
	QString		strSvg;
	QByteArray	contents;

	if (m_part.has_updatecontent()) {
		bAll = false;
		strSvg = m_part.updatecontent().c_str();
	}
	else {
		bAll = true;
		strSvg = m_part.content().c_str();
	}
	contents = strSvg.toUtf8();
	if (m_part.basic().dispdirect() == 999)//用图片显示
	{
		RECT	rc;

		rc.left = m_part.basic().left();
		rc.right = m_part.basic().right();
		rc.top = m_part.basic().top();
		rc.bottom = m_part.basic().bottom();
		m_svg->setImg(contents, rc);
	}
	else {
		m_svg->setSvg(contents, m_part.basic().type(), bAll);
	}

	//触发隐藏处理 Linzhen 20200612
	if (m_bUseShowHide != ( m_part.basic().hide() == true )) {
		m_bUseShowHide = m_part.basic().hide();
		if (m_bUseShowHide) {
			m_svg->hide();
		}
		else {
			m_svg->show();
		}
	}
	m_part.clear_updatecontent();
	m_svg->update();
}

#if ((defined WIN32) && (defined WIN32SCALE))
//比例缩放
void custompart::ZoomPartSelf() {
	if (NULL == m_svg)
		return;

	RECT rectTemp = m_OriginalRect;
	HMIUtility::CountScaleRect(&rectTemp);

	int nLeft = rectTemp.left;
	int nTop = rectTemp.top;
	int nWidth = rectTemp.right - rectTemp.left;
	int nHeight = rectTemp.bottom - rectTemp.top;

	m_svg->setGeometry(nLeft, nTop, nWidth, nHeight);
}
#endif