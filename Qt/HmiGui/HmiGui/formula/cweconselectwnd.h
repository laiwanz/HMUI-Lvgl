#ifndef CWECONSELECTWND_H
#define CWECONSELECTWND_H

#include <QDialog>
#include <QLineEdit>

class QLabel;
class QCheckBox;
class QPushButton;
class QListWidget;
class QVBoxLayout;
class QHBoxLayout;
class QListWidgetItem;
class CSvgButton;


class CWeconSelectWnd :public QDialog {
	Q_OBJECT
public:
	enum SelectWndType {
		TypeFile = 0,
		TypeDocument = 1,
		TypeMax,
	};

	enum WndMode {
		ImportWnd = 0,
		ExportWnd = 1,
	};

public:
    CWeconSelectWnd(const int &nWidth, const int &nHeight, const WndMode wndType, const QString & sPath, const int & nWndType,
                    const QString & sInitName = "", const QString & sName2 = "", const QString & sPath2 = "",
                    QWidget * parent = nullptr);

	void init();

public slots:
	void OnClickCloseWndBtn();
	void OnClickSelectBtn();

	void OnClickListItem(QListWidgetItem *item);
	void OnDbClickListItem(QListWidgetItem *item);

    void OnClickSDCardCheck();
    void OnClickUDiskCheck();

    void OnClickPathForward();
    void OnClickPathStandBack();
    void OnclickCreateFile();
    void OnClickAddNewFile();
    void OnClickCancelAddFileName();

signals:
	void OnSelectResult(const QString & sPath);

private:
	int getAllFiles(const QString & sPath, QStringList & files);
	int getAllDocuments(const QString & sPath, QStringList & documents);

private:
    QVBoxLayout * m_mainLayout;             // 主
    QHBoxLayout * m_headLayout;             // 头
    QHBoxLayout * m_pathLayout;             // 路径
    QHBoxLayout * m_fileLayout;             // 文件
    QHBoxLayout * m_nameLayout;             // 文件名
    QHBoxLayout * m_selectLayout;           // 选择框
    /* path */
    QHBoxLayout * m_pathShowLayout;         // 路径显示
    /* file */
    QVBoxLayout * m_fileCutLayout;          // 文件切换
    QHBoxLayout * m_uDiskLayout;            // U盘
    QHBoxLayout * m_SDLayout;               // sd卡

    QPushButton * m_btnSelect;              //确认按钮
    QPushButton * m_btnClose;               //关闭按钮
    QPushButton * m_btnUDisk;               //切换U盘
    QPushButton * m_btnSDCard;              //切换SD
    QPushButton * m_btnForward;             //前进
    QPushButton * m_btnStandBack;           //后退
	CSvgButton  * m_btnCreate;              //新建文件夹

    QLabel * m_labPath;
    QLabel * m_labHead;
    QLabel * m_labPathImg;
    QLabel * m_labDiskImg;
    QLabel * m_labDiskText;
    QLabel * m_labSDImg;
    QLabel * m_labSDText;
    QLabel * m_labFileName;
    QLabel * m_labFileText;

    QListWidget     * m_list;
    QListWidgetItem * m_lastSelectItem;
    QWidget         * m_widgetPath;
    QLineEdit       * m_lineFileName;
    QDialog         * m_dlgFillInFileName;

    std::list<std::string> m_listFilePathSelete;
    std::string     m_sImgPath;
    QString			m_sInitPath;
    QString			m_sUPath;
    QString			m_sSdPath;
    QString			m_showPath;
    int				m_nCurrentPos;
    int				m_nWndType;
	int				m_nFontSize;
	int				m_languageType;
    bool			bIsSDExit;
    double			m_dWidthScale;
    double			m_dHeightScale;
};

#endif
