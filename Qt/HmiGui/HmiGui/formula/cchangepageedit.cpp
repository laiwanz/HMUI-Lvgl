#include "cchangepageedit.h"

#include <QMouseEvent>
#include <QIntValidator>

CChangePageEdit::CChangePageEdit(const int nTotalPage, const int nCurrentPage, QWidget *parent)
	:QLineEdit(parent)
{
	m_bIsEdit = false;
	m_nTotalPage = nTotalPage;
	m_nCurrentPage = nCurrentPage;
	QString sText = QString::number(m_nCurrentPage) + "/" + QString::number(m_nTotalPage);
	setText(sText);
	connect(this, &CChangePageEdit::editingFinished, this, &CChangePageEdit::finishPageEdit);
	this->installEventFilter(this);
}

int CChangePageEdit::setCurrentPage(const int nPage) {
	if (nPage > m_nTotalPage) {
		m_nCurrentPage = m_nTotalPage;
	}
	else if (nPage < 1) {
		m_nCurrentPage = 1;
	}
	else {
		m_nCurrentPage = nPage;
	}
	UpdataEdit();
	return 0;
}

int CChangePageEdit::setTotalPage(const int nPage) {
	m_nTotalPage = nPage;
	UpdataEdit();
	return 0;
}

int CChangePageEdit::getCurrentPage() {
	return m_nCurrentPage;
}

int CChangePageEdit::getTotalPage() {
	return m_nTotalPage;
}

int CChangePageEdit::UpdataEdit() {
	QString sText;
	if (m_bIsEdit) {
		sText = QString::number(m_nCurrentPage);
	}
	else {
		sText = QString::number(m_nCurrentPage) + "/" + QString::number(m_nTotalPage);
	}
	setText(sText);
	return 0;
}

int CChangePageEdit::editTextChange() {
	int nNum = text().toInt();
	if (nNum < 1) {
		m_nCurrentPage = 1;
	}
	else if (nNum > m_nTotalPage) {
		m_nCurrentPage = m_nTotalPage;
	}
	else {
		m_nCurrentPage = nNum;
	}
	return 0;
}

int CChangePageEdit::finishPageEdit() {
	m_bIsEdit = false;
	QString sText = text();
	if (sText.contains('/')) {
		return -1;
	}
	int nCurrentPage = sText.toInt();
	if (nCurrentPage > m_nTotalPage) {
		m_nCurrentPage = m_nTotalPage;
	}
	else if (nCurrentPage <= 0) {
		m_nCurrentPage = 1;
	}
	else {
		m_nCurrentPage = nCurrentPage;
	}
	sText = QString::number(m_nCurrentPage) + "/" + QString::number(m_nTotalPage);
	this->setText(sText);
	OnEditFinish(m_nCurrentPage);
	return 0;
}

//void CChangePageEdit::mousePressEvent(QMouseEvent *event) {
//	m_bIsEdit = true;
//	setText(QString::number(m_nCurrentPage));
//	QLineEdit::mousePressEvent(event);
//}

bool CChangePageEdit::eventFilter(QObject *obj, QEvent *ev) {
	return QObject::eventFilter(obj, ev);
}