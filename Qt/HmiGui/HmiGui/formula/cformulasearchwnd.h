#ifndef CFORMULASEARCHWND_H
#define CFORMULASEARCHWND_H

#include <QMap>
#include <QVariant>
#include <QDialog>
#include <QModelIndex>

class QLabel;
class QLineEdit;
class QVBoxLayout;
class QHBoxLayout;
class CSvgButton;
class QPushButton;
class CWeconClearEdit;
class CChangePageEdit;
class QListWidget;

class CFormulaSearchWnd:public QDialog
{
	Q_OBJECT
public:
	typedef struct tagSearchData {

		QString m_sShowName;
		QVariant m_id;
	}SearchData;

	typedef struct tagButtonList {

		QPushButton * m_btn;
		int m_id;
	}ButtonList;
public:
	enum WndMode {
		SearchWnd = 0,
		ChangeWnd = 1,
	};
public:
    CFormulaSearchWnd(const int &nWidth, const int &nHeight, const QString & sPath, WndMode wndType = SearchWnd, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

	void init(QVector<SearchData> & datas);
	void updateList();
	int setListInfo(const QStringList & searchHistory);

	QVariant getResult();
	QStringList getSearchHistory();

public:
	void OnClickGroup();

	void OnClickPageDown();
	void OnClickPageUp();
	void OnTurnToPageNum();
	void OnClickClose();
	void OnClearHistory();

	void OnSearch();

	void keyPressEvent(QKeyEvent * e) override;

signals:
	void OnSearchFunction(int & nWndType, QVariant & id);

private:
	int getIdByButton(QPushButton * btn);

private:
    QVBoxLayout * m_mainLayout;
    QHBoxLayout * m_closeLayout;
	QHBoxLayout * m_searchLayout;
    QHBoxLayout * m_labGroupLayout;
    QHBoxLayout * m_pageLayout;

    QLabel * m_labAllGroup;
	QWidget * m_paretnptr;

    CSvgButton * m_closeWnd;
	CWeconClearEdit * m_searchEdit;
    CSvgButton * m_pageUp;
    CSvgButton * m_pageDown;
    CChangePageEdit * m_pageEdit;
	CSvgButton * m_searchBtn;

	QVector<SearchData> m_totalDatas;
	QVector<SearchData> m_showTotalDatas;
	QListWidget * m_infoList;
	int m_nPageCount;

	QModelIndex m_lastClickItem;
	QStringList m_searchList;
	QString m_sLastSearchText;
    QString m_sPath;
	QVariant m_result;
	WndMode m_wndType;
	int m_languageType;
	double	m_dWidthScale;
	double	m_dHeightScale;
};

#endif // CFORMULASEARCHWND_H
