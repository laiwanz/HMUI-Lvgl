#ifndef CWECONCLEAREDIT_H
#define CWECONCLEAREDIT_H

#include <QLineEdit>
#include <QStringList>

class CSvgButton;
class QCompleter;
class QStringListModel;

class CWeconClearEdit :public QLineEdit {

	Q_OBJECT
public:
	CWeconClearEdit(const QString & sSvgPath, const QString & sSvgName, QWidget * parent = nullptr);

	virtual void resizeEvent(QResizeEvent *event);

	int setListInfo(QStringList & datas);

	int setDownListStyle(const QString & sStyle);

public slots:
	void OnClickClear();
	void OnShowDownListWnd();
	void OnSelectPopItem(const QModelIndex &index);
	void OnHideDownList(const QString & sText);

	void keyPressEvent(QKeyEvent * e) override;
	void mousePressEvent(QMouseEvent * event) override;

signals:
	void textEditFinish();
	void OnClearText();
	void OnShowDownList();

private:
	CSvgButton * m_clearBtn;
	QString m_sSvgPath;
	QString m_sSvgName;
	QString m_sSvgClickName;

	QLineEdit * m_downEdit;
	QStringList m_listDatas;
	QCompleter * m_downList;
	QStringListModel * m_searchModel;
};

#endif