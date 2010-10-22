/********************************************************************************
** Form generated from reading UI file 'adminrfid.ui'
**
** Created: Wed Apr 21 18:10:24 2010
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ADMINRFID_H
#define UI_ADMINRFID_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_AdminRFID
{
public:
    QLabel *label;
    QComboBox *cbPortList;
    QPushButton *pbConnect;
    QLabel *label_2;
    QLabel *lblID;
    QLabel *label_3;
    QPushButton *pbScan;

    void setupUi(QDialog *AdminRFID)
    {
        if (AdminRFID->objectName().isEmpty())
            AdminRFID->setObjectName(QString::fromUtf8("AdminRFID"));
        AdminRFID->resize(329, 176);
        label = new QLabel(AdminRFID);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 20, 121, 31));
        QFont font;
        font.setFamily(QString::fromUtf8("Arial"));
        font.setPointSize(16);
        font.setBold(true);
        font.setItalic(true);
        font.setWeight(75);
        label->setFont(font);
        cbPortList = new QComboBox(AdminRFID);
        cbPortList->setObjectName(QString::fromUtf8("cbPortList"));
        cbPortList->setGeometry(QRect(20, 70, 211, 22));
        QFont font1;
        font1.setFamily(QString::fromUtf8("Arial"));
        cbPortList->setFont(font1);
        pbConnect = new QPushButton(AdminRFID);
        pbConnect->setObjectName(QString::fromUtf8("pbConnect"));
        pbConnect->setGeometry(QRect(240, 70, 75, 23));
        pbConnect->setFont(font1);
        label_2 = new QLabel(AdminRFID);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 50, 221, 16));
        label_2->setFont(font1);
        lblID = new QLabel(AdminRFID);
        lblID->setObjectName(QString::fromUtf8("lblID"));
        lblID->setGeometry(QRect(20, 110, 271, 31));
        QFont font2;
        font2.setFamily(QString::fromUtf8("Arial"));
        font2.setPointSize(12);
        font2.setBold(true);
        font2.setWeight(75);
        lblID->setFont(font2);
        lblID->setFrameShape(QFrame::Box);
        lblID->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);
        label_3 = new QLabel(AdminRFID);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(20, 150, 271, 16));
        label_3->setFont(font1);
        pbScan = new QPushButton(AdminRFID);
        pbScan->setObjectName(QString::fromUtf8("pbScan"));
        pbScan->setGeometry(QRect(230, 20, 91, 23));

        retranslateUi(AdminRFID);
        QObject::connect(pbConnect, SIGNAL(clicked()), AdminRFID, SLOT(onConnect()));
        QObject::connect(pbScan, SIGNAL(clicked()), AdminRFID, SLOT(scanPorts()));

        QMetaObject::connectSlotsByName(AdminRFID);
    } // setupUi

    void retranslateUi(QDialog *AdminRFID)
    {
        AdminRFID->setWindowTitle(QApplication::translate("AdminRFID", "AdminRFID", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("AdminRFID", "AdminRFID", 0, QApplication::UnicodeUTF8));
        pbConnect->setText(QApplication::translate("AdminRFID", "Connect", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("AdminRFID", "RFID Reader Communications Port:", 0, QApplication::UnicodeUTF8));
        lblID->setText(QApplication::translate("AdminRFID", "No card has been detected yet", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("AdminRFID", "ID is copied to clipboard as soon as it appears.", 0, QApplication::UnicodeUTF8));
        pbScan->setText(QApplication::translate("AdminRFID", "Rescan Ports", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class AdminRFID: public Ui_AdminRFID {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADMINRFID_H
