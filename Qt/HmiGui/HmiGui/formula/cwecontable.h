#ifndef CWECONTABLE_H
#define CWECONTABLE_H
#include <QMap>
#include <QFont>
#include <QWidget>
#include <QLineEdit>
#include <QModelIndex>
#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QAbstractItemDelegate>
#include <QApplication>

#include "cbl/cbl.h"
#include "cformulasearchwnd.h"
#include "cwecontablemodel.h"
#include "../config/tableconfig.h"

class QLabel;
class QComboBox;
class QTableView;
class CSvgButton;
class QHBoxLayout;
class QVBoxLayout;
class CChangePageEdit;
class CWeconTableModel;

class CWeconTable : public QWidget
{
public:
	enum LanguageType {
		EnglishLanguage = 0,
		ChineseLanguage,
		LanguageTypeMax,
	};

	using FUNC_ON_SEEARCH_BUTTON_CLICK
		= function<int()>;
	using FUNC_ON_TURN_PAGE
		= function<void(const int & nPageNum)>;
	using FUNC_ON_CELL_FINISHEDIT
		= function<int(const int & nId, const std::string & sHead, const std::string & sData)>;
	using FUNC_ON_CHANGE_SHOWCOUNT
		= function<int(const int & nShowCount)>;
	using FUNC_ON_TIPS_MOVEMENT
		= function<int()>;

    Q_OBJECT
public:
    CWeconTable(QWidget *parent = nullptr);
    CWeconTable(TableSpace::TABLESTYLE & style, QWidget *parent = nullptr);

    int init(TableSpace::TABLESTYLE & style);
	int initPosition();

	int updateLockImage(const QString & imagePath);
	int updateLockRect();
	int hideLockImage();

	int setFormulaEnable(bool bIsEnable);

	int setHeadData(QList<QString> & headDatas);

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	bool setData(const QStringList & datas, int role = Qt::EditRole);

	int getLanguageType();
	void setLanguageType(CWeconTable::LanguageType & type);

	bool isNumberic(const QString & str);

	int addHeadData(const QString & sText, Qt::Orientation orientation, int section = -1);
	int initHHead(QList<QString> & heads);
	int initData(QList<QStringList> & allData);
	int initData(const std::string & allData);
	int addDatas(const QList<QStringList> & datas);
	int addData(const QStringList & data, const bool isUpdate = false);
	int clearData();

	int insertDataFormat(const int & nColumn, TableSpace::DATAFORMAT_T & dataFormat);

    static QString Int16ToRgbStr(int colorValue);
	static QString IntToRGB(int nColorValue, int nAlipha);
    int getPreview(const QString & sPath);

	int setSrcno(int nNum);
	int setCurrentPageNum(const int & nNum);
	int setTotalPageNum(const int & nNum);
	int setOnePageShowCount(const int & nNum);
	int setTableName(const QString& sName);
	int setPosition(const int & nPosition);
	int setExeWidth(const int & nWidth);
	int setExeHeight(const int & nHeight);
	
	QModelIndex getCurrentSelectIndex();
	int getSrcno();
	int getCurrentPageNum();
	int getOnePageShowCount();
	int getExeWidth();
	int getExeHeight();
	QString getSvgPath();
	int getPosition();

	void updateTable();

	QPushButton * AddLeftBottomBtn(const QString & svgName);
	int AddItemToPageCountComBox(const QString & sText);
	virtual QString getSgvNameByFuncType(const int & type);

	//call back func
	int setSeachBtnClickFunc(FUNC_ON_SEEARCH_BUTTON_CLICK func);
	int setTurnPageFunc(FUNC_ON_TURN_PAGE func);
	int setCellEditFinishFunc(FUNC_ON_CELL_FINISHEDIT func);
	int setChangeShowCountFunc(FUNC_ON_CHANGE_SHOWCOUNT func);

	virtual int initLanguageInfo();
	virtual int initFuncBtnConnect(QPushButton * btn, const int & nFuncCode);

public:
	static int showMsgWnd(const QString & sTitleText, const QString & sMsgText, QMessageBox::StandardButton buttons = QMessageBox::NoButton, QWidget * parent = nullptr);

protected:
	virtual void getScaleWH();
	bool IsTableSelect();
	virtual void resizeEvent(QResizeEvent *event);
	virtual void paintEvent(QPaintEvent *event);

public slots:
	void OnClickSearchBtn();
	void OnTurnToPageUp();
	void ClickPageUpButton();
	void ClickPageDownButton();
	void ClickExpandButton();
	void ChangeShowPageNumber(int nCurIndex);
    void DoubleClickCell(const QModelIndex & index);
    void CellCloseEdit(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);
	void OnClickVHead(int nLogicalIndex);
	void OnShowMessageWnd(QString & sMsgText);
signals:
	void showMessageWnd(QString & sMsgText);
	void updateOnePageShowCount();
private:
    QVBoxLayout * m_mainLayout;
    QHBoxLayout * m_headLayout;
    QHBoxLayout * m_bottomLayout;
    QTableView * m_table;
    CWeconTableModel * m_model;
    QLabel * m_tableHeadName;

    QMap<int, CSvgButton *> m_functionBtn;

    QComboBox * m_onePageShowCount;
    CSvgButton * m_upPage;
    CSvgButton * m_downPage;
	CSvgButton * m_expandBtn;
	CSvgButton * m_searchBtn;
    CChangePageEdit * m_pageNum;

    QString m_svgPath;

	QLabel * m_lockImg;

	int m_nScrno;
	bool m_bIsExpand;
	int m_nPositionId;
	bool m_bIsSendChangePageShowCount;
	bool m_bIsDisablesCellEditing;
	static int m_nWidth;
	static int m_nHeight;
	QModelIndex m_lastClickIndex;
	QVariant m_lastEditText;
	int m_nVHeadWidth;
	LanguageType m_languageType;

private:
	FUNC_ON_TURN_PAGE m_turnPageFunc;
	FUNC_ON_CELL_FINISHEDIT m_cellFinishEditFunc;
	FUNC_ON_SEEARCH_BUTTON_CLICK m_searchBtnClickFunc;
	FUNC_ON_CHANGE_SHOWCOUNT m_changeShowCountFunc;
};

#endif // CWECONTABLE_H
