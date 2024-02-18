#ifndef CDIALOGBOX_H
#define CDIALOGBOX_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include "csvgbutton.h"

enum DIALOGBOXTYPE{
    DialogInfo,
    DialogWarn,
    DialogError,
};

class CDialogBox : public QDialog
{
public:
    CDialogBox(const int& nWidth, const int& nHeigth,QWidget *parent);
	int config(const std::string& sTitle, const std::string& sNote);
private:
    void initState();
    void initWarn(const std::string &sTitle, const std::string &sNote);

signals:

public slots:
    void dealbtnSureClicked();
    void dealbtnCancelClicked();
    void dealbtnForkClicked();

private:
	QHBoxLayout		*m_layoutMain;	//主布局
	QVBoxLayout		*m_layoutFrame;	//
	QFrame			*m_frameMain;
    QLabel          *m_labTitle;	//标题
    QLabel          *m_labNote;		//提示文本
    QPushButton     *m_btnSure;		//确认按钮
    QPushButton     *m_btnCancle;	//取消按钮
	CSvgButton      *m_btnFork;		//×
	std::string		m_sIconPath;	//图标路径
	double			m_dWidthScale;
	double			m_dHeightScale;
	int				m_nFontSize;
	int				m_languageType;
};

#endif // MESSAGEBOX_H
