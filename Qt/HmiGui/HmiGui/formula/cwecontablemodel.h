#ifndef CWECONTABLEMODEL_H
#define CWECONTABLEMODEL_H
#include <QList>
#include <QHeaderView>
#include "../config/tableconfig.h"

class CWeconTableModel:public QAbstractTableModel
{
public:
    enum tableRowExpand {
		cellIndex = Qt::UserRole + 1,
		rowId = Qt::UserRole + 2,
		vHeadName = Qt::UserRole + 3,
        maxColum,
    };
public:
    CWeconTableModel(QObject * parent = nullptr);

    virtual Qt::ItemFlags flags(const QModelIndex & index) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    int setHeadData(int section, const QString & sText, Qt::Orientation orientation);
	int setHeadData(QList<QString> & headDatas);
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	virtual bool setData(const QVariant & uid, const QStringList & datas, int role = Qt::EditRole);
	bool setData(const int & nRow, QStringList & datas);
	int setFontSize(const int & nSize);
	int getFontSize();

    int addHeadData(const QString & sText, Qt::Orientation orientation, int section = -1);
	int initHHead(QList<QString> & heads);
	int initData(QList<QStringList> & allData);
	int initData(const std::string & sData);
	int addDatas(QList<QStringList> & allData);
    int addData(const QStringList & datas);
	int clearData();

	bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

	int insertDataFormat(const int & nColumn, TableSpace::DATAFORMAT_T & dataFormat);
	int getDataFormatType(const int & nColumn);

	QModelIndex getIndexByUid(const QVariant & uid);
	TableSpace::DATAFORMAT_T getColumnDataFormat(const int & nColumn);
	QModelIndex getCurrentEditIndex();

    int setCellBgColor1(const QString & sColor, int nAlpha);
    int setCellBgColor2(const QString & sColor, int nAlpha);
    int setCellTextColor(const QString & sColor);

	bool getEditFlag();
	int setEditFlag(bool bIsEdit);
	int setDisablesCellEditing(bool bIsDisablesCellEditing);
	int setCurrentEditIndex(const QModelIndex & index);
	int uncompressFormulaData(const std::string &sbuffer, QList<std::string> &listData);
private:
	QString getTableData(const QModelIndex &nIndex, const int & nMode = 0) const;
	QString getTableData(const int &nRow, const int &nColumn, const int & nMode = 0) const;
public:
	static std::string g_sFormulaImagePath;

private:
    QList<QString> m_hHeaderData;
	QList<std::string> m_listData;
	QMap<int, TableSpace::DATAFORMAT_T> m_columnDataFormats;
    QColor m_rowColor1;
    QColor m_rowColor2;
    QColor m_textColor1;
	int m_nFontSize;
	int m_nRowCount;

	bool m_bIsDisablesCellEditing;
	bool m_bIsEdit;
	QModelIndex m_currentEditIndex;
};

#endif // CWECONTABLEMODEL_H
