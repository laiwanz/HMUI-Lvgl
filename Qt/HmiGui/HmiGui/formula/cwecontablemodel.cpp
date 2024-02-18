#include "cwecontablemodel.h"
#include <QTableView>
#include "cbl/cbl.h"
#ifndef HMIPREVIEW
#include "ProtoXML/ProtoXML.h"
#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#endif

#define formulaExpandDataCount  3
std::string CWeconTableModel::g_sFormulaImagePath = "";
CWeconTableModel::CWeconTableModel(QObject * parent)
	: QAbstractTableModel(parent)
{
	m_nRowCount = 0;
	m_bIsEdit = false;
	m_nFontSize = 12;
	m_rowColor1.setRgb(255, 255, 255);
	m_rowColor2.setRgb(239, 244, 248);
}

Qt::ItemFlags CWeconTableModel::flags(const QModelIndex &index) const
{
	if (!index.isValid()) {
		return Qt::NoItemFlags;
	}
	if (m_bIsDisablesCellEditing) {
		return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	}
	return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

int CWeconTableModel::rowCount(const QModelIndex &parent) const
{
	return m_nRowCount;
}

int CWeconTableModel::columnCount(const QModelIndex &parent) const
{
	return m_hHeaderData.size();
}

QVariant CWeconTableModel::data(const QModelIndex &index, int role) const
{
	switch (role) {
	case Qt::DisplayRole:
	case Qt::EditRole:
	{
		QTableView * table = dynamic_cast<QTableView *>(parent());
		if (nullptr != table && role == Qt::DisplayRole && m_bIsEdit && table->currentIndex() == m_currentEditIndex && table->currentIndex() == index) {

			return "";
		}

		auto nRow = index.row();
		auto nColumn = index.column();//table data is start tableRowExpand::maxColum
		if (nRow < m_nRowCount && nColumn < (int)m_hHeaderData.size()) {
			auto cellData = this->getTableData(nRow, nColumn);
			return cellData;
		}
		else {

			return "";
		}
		break;
	}
	case Qt::ForegroundRole: {

		QBrush itemBrush(m_textColor1);
		return itemBrush;
	}
							 break;
	case Qt::BackgroundRole:
	{
		if (index.row() % 2 == 0) {

			QBrush itemBrush(m_rowColor2);
			return itemBrush;
		}
		else {

			QBrush itemBrush(m_rowColor1);
			return itemBrush;
		}
	}
	case Qt::TextAlignmentRole:
	{

		return Qt::AlignCenter;

	}
	case Qt::FontRole:
	{
		QFont cellFont;
		cellFont.setFamily("simsun");
		cellFont.setPixelSize(m_nFontSize);
		return cellFont;
	}
	case tableRowExpand::rowId:
	{
		auto nRow = index.row();
		if (nRow < m_nRowCount && 0 < m_hHeaderData.size()) {
			return getTableData(nRow, tableRowExpand::rowId - Qt::UserRole - 1, 1);
		}
		break;
	}
	default:
		return QVariant();
	}
	return QVariant();
}

QVariant CWeconTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole) {
		switch (orientation) {
		case Qt::Horizontal:
			if (section < m_hHeaderData.size()) {
				return m_hHeaderData.at(section);
			}
			break;
		case Qt::Vertical:
			if (section < (int)m_nRowCount) {
				QString result;
				result = "   " + getTableData(section, tableRowExpand::vHeadName - Qt::UserRole - 1, 1) + "   ";
				return result;
			}
			break;
		default:
			return QString::number(section + 1);
		}
	}
	else if (Qt::TextAlignmentRole == role) {

		return Qt::AlignCenter;
	}
	else if (Qt::FontRole == role) {

		QFont cellFont;
		cellFont.setFamily("simsun");
		cellFont.setPixelSize(m_nFontSize);
		return cellFont;
	}
	return QVariant();
}

int CWeconTableModel::setHeadData(int section, const QString &sText, Qt::Orientation orientation)
{
	int nRet = -1;
	switch (orientation) {
	case Qt::Horizontal:
		if (section < m_hHeaderData.size()) {
			m_hHeaderData[section] = sText;
			nRet = 0;
		}
		break;
	case Qt::Vertical:
		//     if(section < (int)m_nRowCount && m_hHeaderData.size() >= formulaExpandDataCount) {
				 ////需要修改
		//         //m_data[section][tableRowExpand::formulaName - Qt::UserRole - 1] = sText;
		//         nRet = 0;
		//     }
		break;
	default:
		nRet = 0;
		break;
	}

	return nRet;
}

int CWeconTableModel::setHeadData(QList<QString>& headDatas)
{
	if (headDatas.size() != m_hHeaderData.size()) {

		return -1;
	}

	m_hHeaderData.clear();
	m_hHeaderData = headDatas;

	beginResetModel();
	endResetModel();

	return 0;
}

bool CWeconTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid()) {

		return false;
	}
	int nRet = -1;
	switch (role) {
	case Qt::EditRole: {
		//需要修改
			//auto& row = m_data[index.row()];
			//row[index.column() + formulaExpandDataCount] = value.toString();
		return true;
	};
	case tableRowExpand::rowId:
	{
		//需要修改
		//auto nRow = index.row();
		//if(nRow < m_data.size() && 0 < m_data[nRow].size()) {

		//    m_data[nRow][tableRowExpand::formulaId - Qt::UserRole - 1] = value.toString();
		//    return true;
		//}
		break;
	}
	default:
		nRet = 0;
		break;
	}

	return nRet;
}

bool CWeconTableModel::setData(const QVariant & uid, const QStringList & datas, int role)
{
	QModelIndex index = getIndexByUid(uid);
	if (index.isValid()) {

		//需要修改
		//m_data[index.row()] = datas;
	}
	return false;
}

bool CWeconTableModel::setData(const int & nRow, QStringList & datas)
{
	if (nRow > rowCount()) {

		return false;
	}

	//需要修改
	//for (int i = 0; i < (int)m_data[nRow-1].size(); i++)
	//{
	//	m_data[nRow - 1][i] = datas[i];
	//}

	return true;
}

int CWeconTableModel::setFontSize(const int & nSize)
{
	m_nFontSize = nSize;
	this->beginResetModel();
	this->endResetModel();
	return 0;
}

int CWeconTableModel::getFontSize()
{
	return this->m_nFontSize;
}

int CWeconTableModel::addHeadData(const QString &sText, Qt::Orientation orientation, int section)
{
	switch (orientation) {
	case Qt::Horizontal:
		m_hHeaderData.push_back(sText);
		break;
	case Qt::Vertical:
		//需要修改
		std::string datas;
		datas += (char)0x02;
		datas += (char)0x02;
		datas += sText.toStdString();
		for (auto i = 0; i < columnCount(); i++) {
			datas += (char)0x02 + QString::number(0).toStdString();
		}
		m_listData.push_back(datas);
		m_nRowCount += 1;
		break;
	}
	beginResetModel();
	endResetModel();

	return 0;
}

int CWeconTableModel::initHHead(QList<QString>& heads)
{
	m_hHeaderData.clear();
	m_hHeaderData = heads;
	beginResetModel();
	endResetModel();
	return 0;
}

int CWeconTableModel::initData(QList<QStringList>& allData)
{
	beginResetModel();
	endResetModel();
	return 0;
}

int CWeconTableModel::initData(const std::string & sData)
{
	int nRtn = 0;
	if ((nRtn = this->uncompressFormulaData(sData, m_listData)) < 0) {
		printf("%s:uncompress formula data error:%d\r\n", __FUNCTION__, nRtn);
		return -1;
	}
	m_nRowCount = m_listData.size();
	beginResetModel();
	endResetModel();
	return 0;
}

int CWeconTableModel::addDatas(QList<QStringList>& allData)
{
	beginResetModel();
	endResetModel();
	return 0;
}

int CWeconTableModel::addData(const QStringList &datas)
{
	int nRet = 0;
	nRet = insertRows(rowCount(), 1);

	return nRet;
}

int CWeconTableModel::clearData()
{
	removeRows(0, rowCount());
	return 0;
}

bool CWeconTableModel::insertRows(int row, int count, const QModelIndex & parent)
{
	beginInsertRows(parent, row, row + count - 1);
	endInsertRows();
	return true;
}

bool CWeconTableModel::removeRows(int row, int count, const QModelIndex & parent)
{
	if (count <= 0) {

		return true;
	}
	beginRemoveRows(parent, row, row + count - 1);

	endRemoveRows();
	return true;
}

int CWeconTableModel::insertDataFormat(const int & nColumn, TableSpace::DATAFORMAT_T & dataFormat)
{
	m_columnDataFormats.insert(nColumn, dataFormat);
	return 0;
}

int CWeconTableModel::getDataFormatType(const int & nColumn)
{
	if (nColumn < 0 || nColumn >= m_columnDataFormats.size()) {

		return -1;
	}

	return m_columnDataFormats[nColumn].nDataFormat;
}

QModelIndex CWeconTableModel::getIndexByUid(const QVariant & uid)
{
	for (int i = 0; i < this->rowCount(); i++) {
		QModelIndex modelIndex = this->index(i, 0);
		QVariant cellId = this->data(modelIndex, tableRowExpand::rowId);
		if (cellId == uid)
			return modelIndex;
	}
	return QModelIndex();
}

TableSpace::DATAFORMAT_T CWeconTableModel::getColumnDataFormat(const int & nColumn)
{
	auto dataFormat = m_columnDataFormats.find(nColumn);
	if (dataFormat != m_columnDataFormats.end()) {
		return dataFormat.value();
	}
	return TableSpace::DATAFORMAT_T();
}

QModelIndex CWeconTableModel::getCurrentEditIndex()
{
	return m_currentEditIndex;
}

int CWeconTableModel::setCellBgColor1(const QString &sColor, int nAlpha)
{
	m_rowColor1 = sColor;
	m_rowColor1.setAlpha(nAlpha);
	return 0;
}

int CWeconTableModel::setCellBgColor2(const QString &sColor, int nAlpha)
{
	m_rowColor2 = sColor;
	m_rowColor2.setAlpha(nAlpha);
	return 0;
}


int CWeconTableModel::setCellTextColor(const QString &sColor)
{
	m_textColor1 = sColor;
	return 0;
}

bool CWeconTableModel::getEditFlag()
{
	return m_bIsEdit;
}

int CWeconTableModel::setEditFlag(bool bIsEdit)
{
	m_bIsEdit = bIsEdit;
	return 0;
}

int CWeconTableModel::setDisablesCellEditing(bool bIsDisablesCellEditing) {

	m_bIsDisablesCellEditing = bIsDisablesCellEditing;
	return 0;
}

int CWeconTableModel::setCurrentEditIndex(const QModelIndex & index)
{
	m_currentEditIndex = index;
	return 0;
}

QString CWeconTableModel::getTableData(const QModelIndex &nIndex, const int & nMode) const
{
	return getTableData(nIndex.row(), nIndex.column(), nMode);
}

int CWeconTableModel::uncompressFormulaData(const std::string &sbuffer, QList<std::string> &listData) {
#ifndef HMIPREVIEW
	listData.clear();
	if (sbuffer.empty()) {
		return 0;
	}
	hmiproto::ctabledata ccompressdata;
	google::protobuf::io::ArrayInputStream input(sbuffer.data(), sbuffer.size());
	google::protobuf::io::GzipInputStream gzip(&input, google::protobuf::io::GzipInputStream::ZLIB);
	if (!ccompressdata.ParseFromZeroCopyStream(&gzip)) {
		return -1;
	}
	for (int nI = 0; nI < ccompressdata.value_size(); nI++) {
		listData.push_back(ccompressdata.value(nI));
	}
#endif
	return 0;
}

QString CWeconTableModel::getTableData(const int & nRow, const int & nColumn, const int & nMode) const
{
	if (nRow >= (int)m_listData.size() || nColumn >= m_hHeaderData.size() + formulaExpandDataCount) {

		return "";
	}

	std::string sSpliteFlag;
	sSpliteFlag += (char)2;
	std::vector<std::string> vecRowData;
	int ret = cbl::split(m_listData.at(nRow), sSpliteFlag, &vecRowData);
	if (ret <= 0) {

		return "";
	}
	int nExpandCount = maxColum - 1 - Qt::UserRole;

	if ((int)vecRowData.size() != (m_hHeaderData.size() + nExpandCount) || nRow >= (int)m_listData.size()) {

		return "";
	}

	if (nMode != 0) {

		nExpandCount = 0;
	}

	return vecRowData[nColumn + nExpandCount].c_str();
}