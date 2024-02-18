#include "cwecontabledelegate.h"
#include <QAbstractItemDelegate>
#include "./cwecontable.h"
#include "../config/tableconfig.h"
#include "multiplatform/multiplatform.h"
#include <QDebug>

CWeconTableDelegate::CWeconTableDelegate(QObject *parent)
	: QStyledItemDelegate(parent) {
	m_pValidator = nullptr;
	m_pValidator = new CTextValidator(parent);
}

QWidget *CWeconTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
	const QModelIndex &index) const {
	if (!index.isValid()) {
		return nullptr;
	}
	QLineEdit *lineEdit = new QLineEdit(parent);
	lineEdit->setStyleSheet("selection-color: rgb(99, 99, 99);selection-background-color: rgb(150, 150, 150);");
	// get table data
	CWeconTableModel * cmodel = (CWeconTableModel *)(index.model());
	if (nullptr == cmodel) {
		return nullptr;
	}
	TableSpace::DATAFORMAT_T dataFormat = cmodel->getColumnDataFormat(index.column());
	m_pValidator->config(dataFormat.nDataFormat, dataFormat.sMin, dataFormat.sMax, dataFormat.nLength, dataFormat.nDecimal);
	lineEdit->setValidator(m_pValidator);
	m_originalText = cmodel->data(index, Qt::EditRole).toString();
	QFont textFont;
	textFont.setFamily("simsun");
	textFont.setPixelSize(cmodel->getFontSize());
	lineEdit->setFont(textFont);
	lineEdit->setText(m_originalText);
	if (cmodel->getDataFormatType(index.column()) != _String) {
		lineEdit->setInputMethodHints(Qt::ImhDigitsOnly);
	}
	return lineEdit;
}

void CWeconTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
	QString sText = index.model()->data(index, Qt::EditRole).toString();
	QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
	if (lineEdit == nullptr) {
		return;
	}
	lineEdit->setText(sText);
}

int CWeconTableDelegate::reset(QObject *object) {
	QLineEdit *lineEdit = qobject_cast<QLineEdit *>(object);
	if (lineEdit == nullptr) {
		return -1;
	}
	lineEdit->setText(m_originalText);
	return 0;
}

QString CWeconTableDelegate::text(QObject *object) {
	QLineEdit *lineEdit = qobject_cast<QLineEdit *>(object);
	if (lineEdit == nullptr) {
		return "";
	}
	return lineEdit->text();
}

void CWeconTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
	QLineEdit *lineEditor = static_cast<QLineEdit*>(editor);
	QString sText = lineEditor->text();
	model->setData(index, sText, Qt::EditRole);
}

bool CWeconTableDelegate::eventFilter(QObject *object, QEvent *ev) {
	if (ev->type() == QEvent::KeyPress) {
		QKeyEvent *pKeyEvent = static_cast<QKeyEvent *>(ev);//将事件转化为键盘事件
		if (pKeyEvent->key() == Qt::Key_Close) {
			this->reset(object);
			/*QWidget *editor = qobject_cast<QWidget*>(object->parent());
			if (editor) {
				emit closeEditor(editor, QAbstractItemDelegate::RevertModelCache);
			}*/
			ev->accept();
			return true;
		}
		else if (pKeyEvent->key() == Qt::Key_Return ||
			pKeyEvent->key() == Qt::Key_Enter) {
			QString sbuffer = this->text(object);
			if (m_pValidator->isValid(sbuffer)) {

			}
			else {

			}
		}
	}
#if defined(_DEBUG) || defined(_LINUX_)
	if (ev->type() == QEvent::FocusIn) {
		if (!QApplication::inputMethod()->isVisible()) {
			QApplication::inputMethod()->show();
		}
	}
	else if (ev->type() == QEvent::FocusOut) {
		if (QApplication::inputMethod()->isVisible()) {
			QApplication::inputMethod()->hide();
		}
	}
#endif
	return QStyledItemDelegate::eventFilter(object, ev);
}