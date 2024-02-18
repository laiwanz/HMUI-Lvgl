/********************************************************************************
** Form generated from reading UI file 'AddFormulaGroupWnd.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ADDFORMULAGROUPWND_H
#define UI_ADDFORMULAGROUPWND_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <formula/csvgbutton.h>

QT_BEGIN_NAMESPACE

class Ui_AddFormulaGroupWnd
{
public:
    QVBoxLayout *verticalLayout_3;
    QSpacerItem *verticalSpacer_10;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer_4;
    CSvgButton *closeBtn;
    QSpacerItem *horizontalSpacer_9;
    QHBoxLayout *horizontalLayout_5;
    QSpacerItem *horizontalSpacer_7;
    QVBoxLayout *verticalLayout_2;
    QSpacerItem *verticalSpacer_5;
    QLineEdit *groupNameEdit;
    QSpacerItem *verticalSpacer_6;
    QHBoxLayout *horizontalLayout_2;
    QLabel *labDesc;
    QSpacerItem *horizontalSpacer_3;
    QLabel *labCurentChoose;
    QSpacerItem *verticalSpacer_7;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QVBoxLayout *verticalLayout;
    QPushButton *labTop;
    QSpacerItem *verticalSpacer;
    QPushButton *labUp;
    QSpacerItem *verticalSpacer_2;
    QPushButton *labDown;
    QSpacerItem *verticalSpacer_3;
    QPushButton *labBottom;
    QSpacerItem *verticalSpacer_4;
    QSpacerItem *horizontalSpacer_2;
    QFrame *line;
    QSpacerItem *verticalSpacer_8;
    QHBoxLayout *horizontalLayout_4;
    QSpacerItem *horizontalSpacer_5;
    QPushButton *btnAdd;
    QSpacerItem *horizontalSpacer_6;
    QSpacerItem *verticalSpacer_9;
    QSpacerItem *horizontalSpacer_8;
    QButtonGroup *buttonGroup;

    void setupUi(QDialog *AddFormulaGroupWnd)
    {
        if (AddFormulaGroupWnd->objectName().isEmpty())
            AddFormulaGroupWnd->setObjectName(QString::fromUtf8("AddFormulaGroupWnd"));
        AddFormulaGroupWnd->resize(480, 400);
        AddFormulaGroupWnd->setStyleSheet(QString::fromUtf8("#AddFormulaGroupWnd{background-color: rgb(255, 255, 255);}\n"
""));
        verticalLayout_3 = new QVBoxLayout(AddFormulaGroupWnd);
        verticalLayout_3->setSpacing(0);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        verticalSpacer_10 = new QSpacerItem(20, 7, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_10);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(0);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_4);

        closeBtn = new CSvgButton(AddFormulaGroupWnd);
        closeBtn->setObjectName(QString::fromUtf8("closeBtn"));
        QSizePolicy sizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(closeBtn->sizePolicy().hasHeightForWidth());
        closeBtn->setSizePolicy(sizePolicy);
        closeBtn->setStyleSheet(QString::fromUtf8("QPushButton{border-radius: 2px 2px 2px 2px;}"));

        horizontalLayout_3->addWidget(closeBtn);

        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_9);

        horizontalLayout_3->setStretch(0, 418);
        horizontalLayout_3->setStretch(1, 40);
        horizontalLayout_3->setStretch(2, 10);

        verticalLayout_3->addLayout(horizontalLayout_3);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(0);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        horizontalSpacer_7 = new QSpacerItem(39, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_7);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(-1, 0, -1, -1);
        verticalSpacer_5 = new QSpacerItem(20, 18, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_5);

        groupNameEdit = new QLineEdit(AddFormulaGroupWnd);
        groupNameEdit->setObjectName(QString::fromUtf8("groupNameEdit"));
        sizePolicy.setHeightForWidth(groupNameEdit->sizePolicy().hasHeightForWidth());
        groupNameEdit->setSizePolicy(sizePolicy);
        groupNameEdit->setStyleSheet(QString::fromUtf8("QLineEdit{border-radius: 2px 2px 2px 2px;border: 0px solid #0C5CB2;background: #EFF4F8;}"));

        verticalLayout_2->addWidget(groupNameEdit);

        verticalSpacer_6 = new QSpacerItem(20, 18, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_6);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        labDesc = new QLabel(AddFormulaGroupWnd);
        labDesc->setObjectName(QString::fromUtf8("labDesc"));
        labDesc->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        horizontalLayout_2->addWidget(labDesc);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_3);

        labCurentChoose = new QLabel(AddFormulaGroupWnd);
        labCurentChoose->setObjectName(QString::fromUtf8("labCurentChoose"));
        labCurentChoose->setStyleSheet(QString::fromUtf8("color: #0964B7;"));

        horizontalLayout_2->addWidget(labCurentChoose);

        horizontalLayout_2->setStretch(0, 100);
        horizontalLayout_2->setStretch(1, 8);
        horizontalLayout_2->setStretch(2, 276);

        verticalLayout_2->addLayout(horizontalLayout_2);

        verticalSpacer_7 = new QSpacerItem(20, 17, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_7);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        labTop = new QPushButton(AddFormulaGroupWnd);
        buttonGroup = new QButtonGroup(AddFormulaGroupWnd);
        buttonGroup->setObjectName(QString::fromUtf8("buttonGroup"));
        buttonGroup->addButton(labTop);
        labTop->setObjectName(QString::fromUtf8("labTop"));
        sizePolicy.setHeightForWidth(labTop->sizePolicy().hasHeightForWidth());
        labTop->setSizePolicy(sizePolicy);
        labTop->setStyleSheet(QString::fromUtf8("QPushButton{border-radius: 2px 2px 2px 2px;border: 1px solid #0C5CB2;color:#0C5CB2;}"));

        verticalLayout->addWidget(labTop);

        verticalSpacer = new QSpacerItem(20, 17, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        labUp = new QPushButton(AddFormulaGroupWnd);
        buttonGroup->addButton(labUp);
        labUp->setObjectName(QString::fromUtf8("labUp"));
        sizePolicy.setHeightForWidth(labUp->sizePolicy().hasHeightForWidth());
        labUp->setSizePolicy(sizePolicy);
        labUp->setStyleSheet(QString::fromUtf8("QPushButton{border-radius: 2px 2px 2px 2px;border: 0px solid #0C5CB2;background: #EFF4F8;}"));

        verticalLayout->addWidget(labUp);

        verticalSpacer_2 = new QSpacerItem(20, 17, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);

        labDown = new QPushButton(AddFormulaGroupWnd);
        buttonGroup->addButton(labDown);
        labDown->setObjectName(QString::fromUtf8("labDown"));
        sizePolicy.setHeightForWidth(labDown->sizePolicy().hasHeightForWidth());
        labDown->setSizePolicy(sizePolicy);
        labDown->setStyleSheet(QString::fromUtf8("QPushButton{border-radius: 2px 2px 2px 2px;border: 0px solid #0C5CB2;background: #EFF4F8;}"));

        verticalLayout->addWidget(labDown);

        verticalSpacer_3 = new QSpacerItem(20, 17, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_3);

        labBottom = new QPushButton(AddFormulaGroupWnd);
        buttonGroup->addButton(labBottom);
        labBottom->setObjectName(QString::fromUtf8("labBottom"));
        sizePolicy.setHeightForWidth(labBottom->sizePolicy().hasHeightForWidth());
        labBottom->setSizePolicy(sizePolicy);
        labBottom->setStyleSheet(QString::fromUtf8("QPushButton{border-radius: 2px 2px 2px 2px;border: 0px solid #0C5CB2;background: #EFF4F8;}"));

        verticalLayout->addWidget(labBottom);

        verticalSpacer_4 = new QSpacerItem(20, 17, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_4);

        verticalLayout->setStretch(0, 40);
        verticalLayout->setStretch(1, 12);
        verticalLayout->setStretch(2, 40);
        verticalLayout->setStretch(3, 12);
        verticalLayout->setStretch(4, 40);
        verticalLayout->setStretch(5, 12);
        verticalLayout->setStretch(6, 40);
        verticalLayout->setStretch(7, 12);

        horizontalLayout->addLayout(verticalLayout);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        horizontalLayout->setStretch(0, 114);
        horizontalLayout->setStretch(1, 240);
        horizontalLayout->setStretch(2, 114);

        verticalLayout_2->addLayout(horizontalLayout);

        line = new QFrame(AddFormulaGroupWnd);
        line->setObjectName(QString::fromUtf8("line"));
        sizePolicy.setHeightForWidth(line->sizePolicy().hasHeightForWidth());
        line->setSizePolicy(sizePolicy);
        line->setMinimumSize(QSize(0, 1));
        line->setMaximumSize(QSize(16777215, 1));
        line->setStyleSheet(QString::fromUtf8("background-color:#CCCCCC;;"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout_2->addWidget(line);

        verticalSpacer_8 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_8);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(0);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_5);

        btnAdd = new QPushButton(AddFormulaGroupWnd);
        btnAdd->setObjectName(QString::fromUtf8("btnAdd"));
        sizePolicy.setHeightForWidth(btnAdd->sizePolicy().hasHeightForWidth());
        btnAdd->setSizePolicy(sizePolicy);
        btnAdd->setStyleSheet(QString::fromUtf8("QPushButton{border-radius: 2px 2px 2px 2px;border: 0px solid #0C5CB2;background: #0661B3; color:white;}"));

        horizontalLayout_4->addWidget(btnAdd);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_6);

        horizontalLayout_4->setStretch(0, 132);
        horizontalLayout_4->setStretch(1, 120);
        horizontalLayout_4->setStretch(2, 132);

        verticalLayout_2->addLayout(horizontalLayout_4);

        verticalSpacer_9 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_9);

        verticalLayout_2->setStretch(0, 4);
        verticalLayout_2->setStretch(1, 40);
        verticalLayout_2->setStretch(2, 11);
        verticalLayout_2->setStretch(3, 40);
        verticalLayout_2->setStretch(4, 4);
        verticalLayout_2->setStretch(5, 212);
        verticalLayout_2->setStretch(6, 1);
        verticalLayout_2->setStretch(7, 12);
        verticalLayout_2->setStretch(8, 40);
        verticalLayout_2->setStretch(9, 16);

        horizontalLayout_5->addLayout(verticalLayout_2);

        horizontalSpacer_8 = new QSpacerItem(39, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_8);

        horizontalLayout_5->setStretch(0, 42);
        horizontalLayout_5->setStretch(1, 384);
        horizontalLayout_5->setStretch(2, 42);

        verticalLayout_3->addLayout(horizontalLayout_5);

        verticalLayout_3->setStretch(0, 10);
        verticalLayout_3->setStretch(1, 40);
        verticalLayout_3->setStretch(2, 380);

        retranslateUi(AddFormulaGroupWnd);

        QMetaObject::connectSlotsByName(AddFormulaGroupWnd);
    } // setupUi

    void retranslateUi(QDialog *AddFormulaGroupWnd)
    {
        AddFormulaGroupWnd->setWindowTitle(QApplication::translate("AddFormulaGroupWnd", "Dialog", nullptr));
        closeBtn->setText(QString());
        groupNameEdit->setPlaceholderText(QApplication::translate("AddFormulaGroupWnd", "\350\257\267\350\276\223\345\205\245", nullptr));
        labDesc->setText(QApplication::translate("AddFormulaGroupWnd", "\351\200\211\346\213\251\347\273\204\344\275\215\347\275\256", nullptr));
        labCurentChoose->setText(QApplication::translate("AddFormulaGroupWnd", "\347\275\256\351\241\266", nullptr));
        labTop->setText(QApplication::translate("AddFormulaGroupWnd", "\347\275\256\351\241\266", nullptr));
        labUp->setText(QApplication::translate("AddFormulaGroupWnd", "\344\270\212\344\270\200\350\241\214", nullptr));
        labDown->setText(QApplication::translate("AddFormulaGroupWnd", "\344\270\213\344\270\200\350\241\214", nullptr));
        labBottom->setText(QApplication::translate("AddFormulaGroupWnd", "\347\275\256\345\272\225", nullptr));
        btnAdd->setText(QApplication::translate("AddFormulaGroupWnd", "\346\267\273\345\212\240", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AddFormulaGroupWnd: public Ui_AddFormulaGroupWnd {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADDFORMULAGROUPWND_H
