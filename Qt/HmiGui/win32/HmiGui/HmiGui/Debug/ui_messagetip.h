/********************************************************************************
** Form generated from reading UI file 'messagetip.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MESSAGETIP_H
#define UI_MESSAGETIP_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_MessageTip
{
public:
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer;
    QLabel *label;
    QSpacerItem *verticalSpacer_2;
    QFrame *line;
    QSpacerItem *verticalSpacer_3;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *OK;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *cancel;
    QSpacerItem *horizontalSpacer_3;
    QSpacerItem *verticalSpacer_4;

    void setupUi(QDialog *MessageTip)
    {
        if (MessageTip->objectName().isEmpty())
            MessageTip->setObjectName(QString::fromUtf8("MessageTip"));
        MessageTip->setWindowModality(Qt::NonModal);
        MessageTip->setEnabled(true);
        MessageTip->resize(361, 204);
        MessageTip->setAutoFillBackground(false);
        verticalLayout = new QVBoxLayout(MessageTip);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        label = new QLabel(MessageTip);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);

        line = new QFrame(MessageTip);
        line->setObjectName(QString::fromUtf8("line"));
        line->setAutoFillBackground(false);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_3);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        OK = new QPushButton(MessageTip);
        OK->setObjectName(QString::fromUtf8("OK"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Ignored);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(OK->sizePolicy().hasHeightForWidth());
        OK->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(OK);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        cancel = new QPushButton(MessageTip);
        cancel->setObjectName(QString::fromUtf8("cancel"));
        sizePolicy.setHeightForWidth(cancel->sizePolicy().hasHeightForWidth());
        cancel->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(cancel);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);

        horizontalLayout->setStretch(0, 1);
        horizontalLayout->setStretch(1, 3);
        horizontalLayout->setStretch(2, 1);
        horizontalLayout->setStretch(3, 3);
        horizontalLayout->setStretch(4, 1);

        verticalLayout->addLayout(horizontalLayout);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_4);

        verticalLayout->setStretch(0, 1);
        verticalLayout->setStretch(1, 5);
        verticalLayout->setStretch(2, 2);
        verticalLayout->setStretch(3, 3);
        verticalLayout->setStretch(4, 1);
        verticalLayout->setStretch(5, 3);
        verticalLayout->setStretch(6, 1);

        retranslateUi(MessageTip);

        QMetaObject::connectSlotsByName(MessageTip);
    } // setupUi

    void retranslateUi(QDialog *MessageTip)
    {
        MessageTip->setWindowTitle(QApplication::translate("MessageTip", "MessageTip", nullptr));
        label->setText(QApplication::translate("MessageTip", "TextLabel", nullptr));
        OK->setText(QApplication::translate("MessageTip", "OK", nullptr));
        cancel->setText(QApplication::translate("MessageTip", "cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MessageTip: public Ui_MessageTip {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MESSAGETIP_H
