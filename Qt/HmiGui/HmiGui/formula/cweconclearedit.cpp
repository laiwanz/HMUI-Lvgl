#include "cweconclearedit.h"

#include <QRect>
#include <QSize>
#include <QKeyEvent>
#include <QListWidget>
#include <QResizeEvent>
#include <QCompleter>
#include <QInputMethod>
#include <QGuiApplication>
#include <QAbstractItemModel>
#include <QStringListModel>
#include <QAbstractItemView>

#include "csvgbutton.h"

CWeconClearEdit::CWeconClearEdit(const QString & sSvgPath, const QString & sSvgName, QWidget * parent)
{
	m_sSvgName = sSvgName;
	m_sSvgPath = sSvgPath;

	m_clearBtn = new CSvgButton(sSvgPath + sSvgName, this);
	connect(m_clearBtn, &CSvgButton::clicked, this, &CWeconClearEdit::OnClickClear);

	m_downEdit = new QLineEdit(this);
	m_downEdit->hide();
	m_downEdit->setFixedSize(this->size());
	m_downList = new QCompleter(m_downEdit);
	m_searchModel = new QStringListModel(m_downEdit);
	m_downList->setModel(m_searchModel);
	m_downList->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
	m_downEdit->setCompleter(m_downList);

	connect(m_downEdit, &QLineEdit::textChanged, this, &CWeconClearEdit::OnHideDownList);
	connect(m_downList->popup(), &QAbstractItemView::clicked, this, &CWeconClearEdit::OnSelectPopItem);
	connect(this, &CWeconClearEdit::OnShowDownList, this, &CWeconClearEdit::OnShowDownListWnd, Qt::QueuedConnection);
}

void CWeconClearEdit::resizeEvent(QResizeEvent * event)
{
	QLineEdit::resizeEvent(event);

	QRect rect;

	event->size();
	rect.setTop(event->size().height() * 0.1);
	rect.setLeft(event->size().width() - event->size().height() * 0.8 - 10);
	rect.setWidth(event->size().height() * 0.8);
	rect.setHeight(event->size().height() * 0.8);

	m_clearBtn->setGeometry(rect);

	m_downEdit->setFixedSize(this->size());
}

int CWeconClearEdit::setListInfo(QStringList & datas)
{
	m_listDatas.clear();
	m_listDatas = datas;

	m_downList->popup();
	m_searchModel->setStringList(datas);
	m_downList->popup()->update();
	return 0;
}

void CWeconClearEdit::OnShowDownListWnd()
{
	m_downList->complete();
	this->setFocus();
#ifndef WIN32
	QInputMethod *inputMethod = QGuiApplication::inputMethod();
	inputMethod->show();
#endif // WIN32
}

void CWeconClearEdit::OnSelectPopItem(const QModelIndex &index)
{
	auto sText = m_downList->popup()->model()->data(index);
	if ("" != sText) {
	
		this->setText(sText.toString());
	}
}

void CWeconClearEdit::OnHideDownList(const QString & sText)
{
	if (!m_downList->popup()->isHidden() && m_downEdit->text() != "") {
	
		auto sTempText = this->text() + sText;
		this->setText(sTempText);
		m_downEdit->setText("");
	}
}

void CWeconClearEdit::keyPressEvent(QKeyEvent * e)
{
	QLineEdit::keyPressEvent(e);
	if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
	
		m_downList->popup()->hide();
		textEditFinish();
	}
}

void CWeconClearEdit::mousePressEvent(QMouseEvent * event)
{
	OnShowDownList();
	QLineEdit::mousePressEvent(event);
}

void CWeconClearEdit::OnClickClear() {

	this->setText("");
	OnClearText();
}

int CWeconClearEdit::setDownListStyle(const QString & sStyle) {
	
	m_downList->popup()->setStyleSheet(sStyle);
	return 0;
}