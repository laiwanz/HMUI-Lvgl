#ifndef _REMARKSWIDGET_H_20231103_
#define _REMARKSWIDGET_H_20231103_
#include <QWidget>
#include <QLabel>

typedef enum eWIDGETDIRECTION {
	eDirection_NONE = 0,
	eDirection_top_right = 1,
	eDirection_top_left = 2,
	eDirection_bottom_right = 3,
	eDirection_bottom_left = 4,
	eDirection_left = 5,
	eDirection_right = 6
} eWIDGETDIRECTION_T;

class CRemarkWidget : public QWidget {
	Q_OBJECT
public:
	explicit CRemarkWidget(QWidget *parent = nullptr);
	~CRemarkWidget();
	int setTriangeOffset(int nOffset);
	int setTriangeSize(unsigned int nWidth, unsigned int nHeight);
	int setText(const QString &sText);
	int setDirection(eWIDGETDIRECTION_T eDirection);
signals:
	void commitMode(bool isWarning);
protected:
	void paintEvent(QPaintEvent *event) override;
private:
	int m_nOffset;
	unsigned int m_nTriangleWidth, m_nTriangleHeight, m_nRadius;
	eWIDGETDIRECTION_T m_eDirection;
	QLabel *m_pLabel;
	bool m_bWarning;
};
#endif // _REMARKSWIDGET_H_20231103_