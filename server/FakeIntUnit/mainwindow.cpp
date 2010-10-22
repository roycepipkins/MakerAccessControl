//Copyright Royce Pipkins 2010
//May be used under the terms of the GPL V3 or higher. http://www.gnu.org/licenses/gpl.html
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

#define ID_CHECK 1
#define ACCESS_REQ 2
#define NO_ACTIVITY 3
#define ACCESS_GRANTED 4
#define ACCESS_DENIED 5
#define SET_IDLE 6
#define ACKNOWLEDGE 7

#define ADDRESS 1

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    bus(),
    busPrinter(bus),
    protocolDriver(busPrinter)
{
    ui->setupUi(this);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::on_pushButton_clicked()
{
    ui->lblId->setText(ui->leId->text());
    ui->leId->clear();
}

void MainWindow::onReadyRead()
{
    unsigned char data;
    while (bus.bytesAvailable())
    {
        bus.getChar((char*)&data);
        protocolDriver.recv(data);
    }

    if (data == 3)
    {
        if (protocolDriver.isValidPacket())
        {

            switch(protocolDriver.getType())
            {
            case ID_CHECK:
                DoIdCheck();
                break;
            case ACCESS_GRANTED:
                GrantAccess();
                break;
            case ACCESS_DENIED:
                DenyAccess();
                break;
            }

            protocolDriver.erasePkt();
        }
        //else qDebug() << "bad/incomplete packet";
    }
}

void MainWindow::DoIdCheck()
{
    if (ui->lblId->text().length() == 0)
    {
        protocolDriver.send(ADDRESS, NO_ACTIVITY, (uint8_t*)"NO_ACTIVITY");
    }
    else
    {
        protocolDriver.send(ADDRESS, ACCESS_REQ, (uint8_t*)ui->lblId->text().toAscii().constData());
    }
}

void MainWindow::GrantAccess()
{
    ui->lblId->clear();
    ui->lblStatus->setText("Granted");
    protocolDriver.send(ADDRESS, ACKNOWLEDGE, (uint8_t*)"ACKNOWLEDGE");
}

void MainWindow::DenyAccess()
{
    ui->lblId->clear();
    ui->lblStatus->setText("Denied");
    protocolDriver.send(ADDRESS, ACKNOWLEDGE, (uint8_t*)"ACKNOWLEDGE");
}

void MainWindow::on_pushButton_2_clicked()
{
    QMessageBox msgbox(this);
    bus.setQueryMode(QextSerialPort::EventDriven);
    bus.setPortName(ui->lePortName->text());

    bus.setBaudRate(BAUD9600);
    bus.setParity(PAR_NONE);
    bus.setDataBits(DATA_8);
    bus.setStopBits(STOP_1);
    bus.setFlowControl(FLOW_OFF);
    bus.setTimeout(timeout);

    if (bus.open(QIODevice::ReadWrite) == true) {
        connect(&bus, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        msgbox.setText("Port open");
    }
    else {
        msgbox.setText("Failed to open port");

    }
    msgbox.exec();
}
