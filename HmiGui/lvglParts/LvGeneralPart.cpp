#include "LvGeneralPart.h"
#include "../lvglParts/LvImgCacheMgr.h"


LvGeneralPart::LvGeneralPart(const hmiproto::hmipartcommon & common, HMILvScreen * parent)
{

	std::cout << "LvGeneralPart Construction " << std::endl;
	m_part.Clear();
	m_parttype = NORMALPART;
	m_commandpart = NULL;
	//m_glintTimer = NULL;
	m_qrlabel = NULL;


	m_bUpdateImg = false;
	m_bUpdateText = false;
	m_bUpdateHide = false;
	m_bUpdateGlint = false;
	m_bUpdateArea = false;

	m_TxtCrc32 = 0;            //�ı�У���
	memset(&m_rc, 0, sizeof(RECT));//����
	m_bUseShowHide = false;		//�Ƿ�����
	m_bUseGlint = false;		//�Ƿ���˸
	m_strPartType = "";
	m_bIsShowGrayScale = false;
	m_nImgIndex = 0;
	m_nLockImgIndex = 0;
	m_part.MergeFrom(common);

}

LvGeneralPart::~LvGeneralPart()
{
	try {
		if (m_commandpart != NULL) {
			delete m_commandpart;
			m_commandpart = NULL;
		}

		//if (m_glintTimer != NULL) {
		//	delete m_glintTimer;
		//	m_glintTimer = NULL;
		//}

		if (m_qrlabel != NULL) {
			delete m_qrlabel;
			m_qrlabel = NULL;
		}
	}
	catch (...) {
	}
}

void LvGeneralPart::Init(lv_obj_t * pWidget)
{
	//std::cout << "LvGeneralPart Init" << std::endl;

	m_nImgIndex = m_part.basic().imgindex();
	m_nLockImgIndex = m_part.lockimg();
	m_bIsShowGrayScale = m_part.gray();

	string strTemp(m_part.imgpath().c_str());
	//std::cout << "LvGeneralPart Init strTemp: " << strTemp <<std::endl;

	if ((BITMAPDISPLAY_INDEX == m_part.basic().imgindex()) && m_part.has_imgpath()) {
		m_strImgPath = strTemp;
	}

	unsigned int unLen = 0;

	unLen = m_part.basic().text().length();
	m_TxtCrc32 = cbl::crc32(0, (const unsigned char*)m_part.basic().text().c_str(), unLen * sizeof(char));
	m_rc.left = m_part.basic().left();
	m_rc.right = m_part.basic().right();
	m_rc.top = m_part.basic().top();
	m_rc.bottom = m_part.basic().bottom();
	m_bUseShowHide = m_part.basic().hide();		//�Ƿ�����
	m_bUseGlint = m_part.basic().glint();			//�Ƿ���˸

	string strPartType(m_part.basic().type().c_str());
	m_strPartType = strPartType;

	if (m_strPartType.compare("QRBitmap") == 0)//��ά�벿��)
	{
		//std::cout << "This is QRBitMap This is a QRLabel " << std::endl;
		//std::cout << std::endl;
		m_parttype = QRPART;
		m_qrlabel = new LvQRLabel(pWidget);
		m_qrlabel->setLvGeometry(m_rc.left, m_rc.top, m_rc.right - m_rc.left, m_rc.bottom - m_rc.top);
		
	}
	else 
	{
		//std::cout << "This is Img show" << std::endl;

		m_commandpart = new LvCommonPart(pWidget, m_strPartType);
		if (m_strPartType.compare("RotatePic") == 0)//��תλͼ
		{
			m_parttype = ROTATEPIC;
			//QRect rect(m_rc.left, m_rc.top, m_rc.right - m_rc.left, m_rc.bottom - m_rc.top);
			m_commandpart->LvsetRoateRect(m_rc.left, m_rc.top, m_rc.right - m_rc.left, m_rc.bottom - m_rc.top);
		}
		else
		{
			m_parttype = NORMALPART;
			m_commandpart->LvsetGeometry(m_rc.left, m_rc.top, m_rc.right - m_rc.left, m_rc.bottom - m_rc.top);
			//std::cout << "left:" << m_rc.left << "right: " << m_rc.top << "width: " << m_rc.right - m_rc.left << "height: " << m_rc.bottom - m_rc.top << std::endl;
			if (m_part.basic().dispdirect() > 0)//����������ת����
			{
				m_commandpart->LvsetRoateRect(m_rc.left, m_rc.top, m_rc.right - m_rc.left, m_rc.bottom - m_rc.top);;
				m_part.set_degrees(90 * m_part.basic().dispdirect());
			}
		}
		m_commandpart->setPos(m_rc);
		
	}

	
	GetDrawFunc(DRAW_INIT);

#if ((defined WIN32) && (defined WIN32SCALE))
	if (NULL != m_commandpart) {
		//m_commandpart->GetTextRect(m_part.basic(), &(m_commandpart->m_OriginalTextRect));
	}

	memcpy(&m_OriginalRect, &m_rc, sizeof(RECT));
	ZoomPartSelf();
#endif
}

void LvGeneralPart::Update(const hmiproto::hmipartcommon & common, HMILvScreen * parent)
{
	std::cout << "LvGeneralPart Update" << std::endl;

	m_part.MergeFrom(common);

	//�Ƿ����ͼƬ
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

	string strTemp(m_part.imgpath().c_str());
	if ((BITMAPDISPLAY_INDEX == m_part.basic().imgindex())
		&& m_part.has_imgpath()						//���� λͼ->λͼԤ��
		&& (0 != m_strImgPath.compare(strTemp)) ) 
	{
		m_strImgPath = strTemp;
		m_bUpdateImg = true;//����
	}

	//�Ƿ�����ı�
	unsigned long TxtCrc32 = 0;

	unsigned int unLen = 0;

	unLen = m_part.basic().text().length();
	TxtCrc32 = cbl::crc32(0, (const unsigned char*)m_part.basic().text().c_str(), unLen * sizeof(char));

	m_TxtCrc32 = TxtCrc32;

	m_bUpdateText = true;

	RECT	rc;

	rc.left = m_part.basic().left();
	rc.right = m_part.basic().right();
	rc.top = m_part.basic().top();
	rc.bottom = m_part.basic().bottom();

	if (memcmp(&rc, &m_rc, sizeof(RECT)) != 0) {
		memcpy(&m_rc, &rc, sizeof(RECT));
		m_bUpdateArea = true;
	}

	if (m_bUseShowHide != (m_part.basic().hide() == true)) {
		m_bUseShowHide = (m_part.basic().hide() == true);
		m_bUpdateHide = true;
	}

	if (m_bUseGlint != m_part.basic().glint()) {
		m_bUseGlint = m_part.basic().glint();
		m_bUpdateGlint = true;
	}

	GetDrawFunc(DRAW_UPDATE);

#if ((defined WIN32) && (defined WIN32SCALE))
	//�Ƿ��������
	if (memcmp(&rc, &m_OriginalRect, sizeof(RECT)) != 0) {
		if (NULL != m_commandpart) {
			//m_commandpart->GetTextRect(common.basic(), &(m_commandpart->m_OriginalTextRect));
		}

		if (memcmp(&rc, &m_OriginalRect, sizeof(RECT)) != 0) {
			memcpy(&m_OriginalRect, &rc, sizeof(RECT));
			ZoomPartSelf();
		}
	}
#endif
}



void LvGeneralPart::Draw(HMILvScreen * parent, int drawtype)
{

	m_strImgPath = LvImgCacheMgr::GetImgPath(m_nImgIndex);
	for(DRAW_FUN draw_fun : m_drawList) {
		(this->*draw_fun)(parent, drawtype);
	}
}

void LvGeneralPart::OnPartHide()
{
}

void LvGeneralPart::OnPartShow()
{
}


void LvGeneralPart::DrawImg(HMILvScreen * pWidget, int drawtype)
{
	//std::cout << "LvGeneralPart DrawImg" << std::endl;

	if (NULL == m_commandpart) {
		return;
	}
	if (BITMAPDISPLAY_INDEX == m_part.basic().imgindex() &&
		m_part.has_imgpath())//λͼԤ��
	{

		string strTemp(m_part.imgpath().c_str());
		//std::cout << "LvGeneralPart DrawImg: " << strTemp << std::endl;
		RECT	rc = { 0 };

		rc.left = m_part.basic().left();
		rc.right = m_part.basic().right();
		rc.top = m_part.basic().top();
		rc.bottom = m_part.basic().bottom();

		m_commandpart->SetIMGPath(strTemp, rc);
	}
	else {
		//std::cout << "LvGeneralPart DrawImg index: " << m_part.basic().imgindex() << std::endl;

		m_commandpart->SetIMGIndex(m_part.basic().imgindex());
		m_commandpart->drawImg();
	}

	if (m_commandpart != NULL &&//��ά�벿��m_commandpart����ΪNULL�������Ҫ�����ж� Lizh 20200225
		true == m_part.has_lockimg() &&
		true == m_part.has_lockimgwidth()) {
		m_nLockImgIndex = m_part.lockimg();
		//m_commandpart->setLockImgInfo(m_part.lockimg(), m_part.lockimgwidth(), m_part.gray());
	}
}

void LvGeneralPart::DrawTxt(HMILvScreen * pWidget, int drawtype)
{
	if (m_parttype == NORMALPART) {

#if ((defined WIN32) && (defined WIN32SCALE))
		int nLeft = m_OriginalRect.left;
		int nTop = m_OriginalRect.top;
		int nWidth = m_OriginalRect.right - m_OriginalRect.left;
		int nHeight = m_OriginalRect.bottom - m_OriginalRect.top;
		//m_commandpart->setGeometry(nLeft, nTop, nWidth, nHeight);
#endif

		m_commandpart->setText(m_part, drawtype);

#if ((defined WIN32) && (defined WIN32SCALE))
		//memcpy(&m_commandpart->m_OriginalTextRect, &(m_commandpart->m_txtRect), sizeof(RECT));
		ZoomPartSelf();
#else
		m_commandpart->CalculateTextArea();
#endif
	}
	else if (m_parttype == QRPART) {
		string strText(m_part.basic().text());
		m_qrlabel->setLabelData(strText);
	}
}

void LvGeneralPart::DrawArea(HMILvScreen * pWidget, int drawtype)
{
}

void LvGeneralPart::GetDrawFunc(int drawtype)
{
	//std::cout << "LvGeneralPart GetDrawFunc" << std::endl;

	m_drawList.clear();
	if (m_parttype == NORMALPART && m_commandpart == NULL) {
		return;
	}
	else if (m_parttype == QRPART && (m_qrlabel == NULL)) {
		return;
	}

	if (m_bUpdateArea)	// �ƶ����ţ�ֻ�ڸ���ʱ�Ŵ���
	{
		m_drawList.push_back(&LvGeneralPart::DrawArea);

		m_bUpdateArea = false;
	}


	if (drawtype == DRAW_INIT || m_bUpdateImg)//��ʾͼƬ
	{
		//std::cout << "LvGeneralPart Draw Image Init" << std::endl;

		m_drawList.push_back(&LvGeneralPart::DrawImg);
		m_bUpdateImg = false;
	}

	if (drawtype == DRAW_INIT || m_bUpdateText) {
		m_drawList.push_back(&LvGeneralPart::DrawTxt);
		m_bUpdateText = false;
	}

}
