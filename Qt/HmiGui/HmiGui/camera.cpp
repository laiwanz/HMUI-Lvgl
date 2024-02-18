#include "camera.h"

#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include "hmiutility.h"
#include "commonpart.h"

CCamera::CCamera(const hmiproto::hmiipcamaro &camera, QWidget *parent)
{
	m_Img = NULL;
	memset(&m_rc, 0, sizeof(m_rc));
	m_part.Clear();
	m_part.MergeFrom(camera);
}

CCamera::~CCamera()
{
	try {
		if (m_Img != NULL)
		{
			delete m_Img;
			m_Img = NULL;
		}
	}
	catch (...) {}
}

//初始化数据
void CCamera::Init(QWidget * pWidget)
{
	m_rc.left = m_part.basic().left();
	m_rc.right = m_part.basic().right();
	m_rc.top = m_part.basic().top();
	m_rc.bottom = m_part.basic().bottom();
	
	QString strPartType(m_part.basic().type().c_str());

	if (m_Img == NULL) {
		m_Img = new CommonPart(pWidget, strPartType);
		m_Img->setAttribute(Qt::WA_TransparentForMouseEvents, true);//允许鼠标点击穿透
		//设置大小
		m_Img->setGeometry(m_rc.left, m_rc.top, m_rc.right - m_rc.left, m_rc.bottom - m_rc.top);
		m_Img->SetIMGIndex(m_part.basic().imgindex());
	}

#if ((defined WIN32) && (defined WIN32SCALE))
	memcpy(&m_OriginalRect, &m_rc, sizeof(RECT));
	ZoomPartSelf();
#endif
}

//收到更新数据
void CCamera::Update(const hmiproto::hmiipcamaro &camera, QWidget * pWidget)
{
	RECT	rc;
	(void)pWidget;
	m_part.MergeFrom(camera);

	rc.left = m_part.basic().left();
	rc.right = m_part.basic().right();
	rc.top = m_part.basic().top();
	rc.bottom = m_part.basic().bottom();
	if (memcmp(&m_rc, &rc, sizeof(RECT)) != 0)
	{
		memcpy(&m_rc, &rc, sizeof(RECT));
		if (m_Img)
		{
			m_Img->setGeometry(m_rc.left, m_rc.top, m_rc.right - m_rc.left,
				m_rc.bottom - m_rc.top);
		}
	}
#if ((defined WIN32) && (defined WIN32SCALE))

	if (memcmp(&m_rc, &rc, sizeof(RECT)) != 0)
	{
		memcpy(&m_OriginalRect, &rc, sizeof(RECT));
		ZoomPartSelf();
	}
#endif
	m_Img->update();
}

//绘图
void CCamera::Draw(QWidget * pWidget, int)
{
	(void)pWidget;
	if (NULL == m_Img)
	{
		return;
	}

	m_Img->SetIMGIndex(m_part.basic().imgindex());
	m_Img->update();
}

#if ((defined WIN32) && (defined WIN32SCALE))
//比例缩放
void CCamera::ZoomPartSelf()
{
	if (NULL == m_Img)
		return;

	RECT rectTemp = m_OriginalRect;
	HMIUtility::CountScaleRect(&rectTemp);

	int nLeft = rectTemp.left;
	int nTop = rectTemp.top;
	int nWidth = rectTemp.right - rectTemp.left;
	int nHeight = rectTemp.bottom - rectTemp.top;

	m_Img->setGeometry(nLeft, nTop, nWidth, nHeight);
}
#endif
