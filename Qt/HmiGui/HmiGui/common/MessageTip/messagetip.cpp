#include "messagetip.h"
#include "ui_messagetip.h"
#include <QPoint>
#include <QPainter>
#include <QDebug>
#include "utility/utility.h"
#include "../../HmiGui/HmiGui.h"
#ifdef _LINUX_
#include "btl/btl.h"
#endif

MessageTip::MessageTip(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint)
    , ui(new Ui::MessageTip) {
	ui->setupUi(this);
	this->setAttribute(Qt::WA_TranslucentBackground);
#ifdef _WIN32
	HmiGui::GetScaleWH(m_nWidth, m_nHeight);
#else
	int nBitPerPixel = 0;
	if (UTILITY::CState::getLcdResolutionByHMIUIRot(m_nWidth, m_nHeight, nBitPerPixel, true) < 0) {
		m_nWidth = 800;
		m_nHeight = 480;
	}
#endif
	QPoint point = this->parentWidget()->mapToGlobal(QPoint(0, 0));
	this->setFixedSize(m_nWidth / 2, m_nHeight / 2);
	this->move(m_nWidth / 4 + point.x(), m_nHeight / 4 + point.y());
	m_nRadius = (unsigned int)(m_nWidth * 0.005);
	this->setStyleSheet(QString("QPushButton{border-radius:5px 5px 5px 5px; background-color: rgb(200, 200, 200);}"
		//"QPushButton:hover{background-color:rgb(200, 200, 200)}"
		"QPushButton:pressed{background-color:rgb(47, 127, 192)}"
		"QPushButton:checked{background-color:rgb(47, 127, 192)}"
		"QPushButton{font-family: SimSun;font-size: %1px;}"
		"QLabel{font-family: SimSun;font-size: %1px;}").arg((int)(m_nWidth * 0.025)));
	ui->label->setAlignment(Qt::AlignCenter);
	ui->OK->setCheckable(true);
	ui->OK->setChecked(true);
	this->parentWidget()->installEventFilter(this);
}

MessageTip::~MessageTip() {
	if (m_pTimer) {
		delete m_pTimer;
		m_pTimer = nullptr;
	}
    delete ui;
}

void MessageTip::paintEvent(QPaintEvent *event) {
	QPoint point = this->parentWidget()->mapToGlobal(QPoint(0, 0));
	this->move(m_nWidth / 4 + point.x(), m_nHeight / 4 + point.y());

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setPen(Qt::NoPen);

	painter.setBrush(QColor(255, 255, 255, 255));
	painter.setPen(Qt::NoPen);
	painter.drawRoundRect(this->rect(), m_nRadius, m_nRadius);
	QDialog::paintEvent(event);
}

void MessageTip::on_OK_pressed() {
#ifdef _LINUX_
	btl::beep(true);
#endif
	this->accept();
}

void MessageTip::on_cancel_pressed() {
#ifdef _LINUX_
	btl::beep(true);
#endif
	this->reject();
}

int MessageTip::setText(const QString &sbuffer) {
	ui->label->setText(sbuffer);
	return 0;
}

int MessageTip::confirmParam(bool bFlag, unsigned int nTimeout) {
	if (!bFlag) {
		return -1;
	}
	m_pTimer = new QTimer(this);
	m_pTimer->setInterval(nTimeout * 1000);
	connect(m_pTimer, &QTimer::timeout, [=]() {
		m_pTimer->stop();
		this->reject();
		return 0;
	});
	m_pTimer->start();
	return 0;
}

bool MessageTip::eventFilter(QObject *obj, QEvent *ev) {
	if (obj != nullptr) {
		if (ev->type() == QEvent::Move) {
			this->update();
		}
	}
	return QObject::eventFilter(obj, ev);
}

int MessageTip::setOKText(const QString &sbuffer) {
	ui->OK->setText(sbuffer);
	return 0;
}
int MessageTip::setCancelText(const QString &sbuffer) {
	ui->cancel->setText(sbuffer);
	return 0;
}