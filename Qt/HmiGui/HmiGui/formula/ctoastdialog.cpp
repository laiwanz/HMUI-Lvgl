#include "ctoastdialog.h"
#include <QHBoxLayout>
#include <QDialog>
#include <QEvent>
#include <QBitmap>
#include <QPainter>
#include <QStyleOption>

ToastDialog::ToastDialog(const int &nWidth, const int &nHeight, QWidget *parent) : QDialog(parent->parentWidget(), Qt::FramelessWindowHint){

    auto layout = new QHBoxLayout;
    mLabel = new QLabel;
    mLabel->setStyleSheet("color: black; background-color: rgba(200, 200, 200, 200); border-radius:5px;");
	mLabel->setAlignment(Qt::AlignCenter);
	mLabel->setWordWrap(true);
    layout->addWidget(mLabel, 1);
    setLayout(layout);
	layout->setContentsMargins(0,0,0,0);
	this->setStyleSheet("background-color: transparent; border-radius:5px;");
	QPoint ponint = QPoint(0, 0);
#ifndef _LINUX_
	ponint = parent->parentWidget()->mapToGlobal(QPoint(0, 0));
#endif // !_LINUX_
	// 计算长宽缩放比
	if (nWidth > nHeight) {
		m_dWidthScale = (double)nWidth / 800;
		m_dHeightScale = (double)nHeight / 480;
	}
	else {
		m_dWidthScale = (double)nWidth / 480;
		m_dHeightScale = (double)nHeight / 800;
	}
	// 移动当前画面 
	mLabel->setFixedWidth(300 * m_dWidthScale);
	mLabel->setMinimumHeight(40 * m_dHeightScale);
	auto nWndWidth = m_dWidthScale * 300;
	auto nWndHeight = m_dHeightScale * 40;
	this->setFixedWidth(nWndWidth);
	this->move(ponint.x() + (nWidth - nWndWidth) / 2, ponint.y() + (nHeight - nWndHeight) / 2);
	// 计算字体
	int nFontSize = nHeight * 0.03;
	if (nWidth < nHeight) {
		nFontSize = nWidth * 0.03;
	}
	if (nFontSize < 8) {
		nFontSize = 8;
	}
	QString sStyle = "QLabel{color: black; background-color: rgba(200, 200, 200, 200); border-radius:5px;font-size:" + QString::number(nFontSize) + "px;}";
	mLabel->setStyleSheet(sStyle);
    setAttribute(Qt::WA_TranslucentBackground,true);
	this->accept();
}

ToastDialog::~ToastDialog() {
	if (mTimerId != 0) {
		killTimer(mTimerId);
	}
	this->accept();
}

void ToastDialog::show(DIALOGLEVEL level, const QString& text){
	this->accept();
    QPalette p = palette();
    p.setColor(QPalette::Window, QColor(0,0,0,200));
    if (INFO == level){
        p.setColor(QPalette::Window, QColor(255,255,255,200));//黑色
    }
    else if (WARN == level){
        p.setColor(QPalette::Window, QColor(0,0,255,200));
    }
    else if(ERRORDLG == level){
        p.setColor(QPalette::Window, QColor(255,0,0,200));
    }
    setPalette(p);
    mLabel->setText(text);
	this->raise();
	if (mTimerId != 0) {
		killTimer(mTimerId);
	}
	mTimerId = startTimer(2000);
    QDialog::show();
}

void ToastDialog::timerEvent(QTimerEvent *event){
    killTimer(mTimerId);
    mTimerId = 0;
    this->accept();
}
