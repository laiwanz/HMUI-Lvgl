#ifndef CWECONTABLEDELEGATE_H
#define CWECONTABLEDELEGATE_H
#include <QStyledItemDelegate>
#include "../common/textValidator.h"

class CWeconTableDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	CWeconTableDelegate(QObject *parent = nullptr);
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
		const QModelIndex &index) const override;
	void setEditorData(QWidget *editor, const QModelIndex &index) const override;
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
	bool eventFilter(QObject *object, QEvent *ev) override;
private:
	int reset(QObject *object);
	QString text(QObject *object);
private:
	mutable QString m_originalText;
	CTextValidator *m_pValidator;
};
#endif // CWECONTABLEDELEGATE_H
