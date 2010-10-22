#include "adminrfid.h"
#include "ui_adminrfid.h"
#include "qextserialenumerator.h"
#include <QClipboard>
#include <QSettings>
#include <QMessageBox>

#define STX 2
#define ETX 3

AdminRFID::AdminRFID(QWidget *parent)
    : QDialog(parent), ui(new Ui::AdminRFID)
{
    int idx;
    ui->setupUi(this);
    bus = NULL;
    scanPorts();

    QSettings settings("Milwaukee Makerspace", "AdminRFID");
    QString port = settings.value("port").toString();
    if (port.length())
    {
        idx = ui->cbPortList->findText(port);
        if (idx != -1)
        {
            ui->cbPortList->setCurrentIndex(idx);
            onConnect();
        }
    }


}

AdminRFID::~AdminRFID()
{
    delete ui;
}

void AdminRFID::onConnect()
{
    //TODO connect to comm port
    if (bus)
    {
        bus->close();
        delete bus;
        bus = NULL;
        ui->pbConnect->setText("Connect");
        ui->cbPortList->setEnabled(true);
        QSettings settings("Milwaukee Makerspace", "AdminRFID");
        settings.setValue("port", "");
    }
    else
    {
        CreatePort();
        ui->pbConnect->setText("Disconnect");
        ui->cbPortList->setEnabled(false);
    }
}

void AdminRFID::CreatePort()
{
    bus = new QextSerialPort(ui->cbPortList->currentText(), QextSerialPort::EventDriven);
    bus->setBaudRate(BAUD9600);
    bus->setParity(PAR_NONE);
    bus->setDataBits(DATA_8);
    bus->setStopBits(STOP_1);
    bus->setFlowControl(FLOW_OFF);


    if (bus->open(QIODevice::ReadWrite) == true) {
        connect(bus, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        QSettings settings("Milwaukee Makerspace", "AdminRFID");
        settings.setValue("port", ui->cbPortList->currentText());
    }
    else {
        QMessageBox msgBox;
        msgBox.setText("Unable to connect to serial port");
        msgBox.exec();

    }
}

void AdminRFID::scanPorts()
{
    ui->cbPortList->clear();
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    foreach( QextPortInfo port, ports ) {
        ui->cbPortList->addItem(port.physName);
    }

}

void AdminRFID::onReadyRead()
{
    char data;
    char id[11];
    QClipboard* clip;
    while (bus->bytesAvailable() > 0)
    {
      bus->getChar((char*)&data);
      rfid_packet_recv(data);
    }

    if (is_good_rfid_packet())
    {
     strncpy(id, rfid_packet.fields.id, 10);
     id[10] = 0;
     ui->lblID->setText(id);

     clip = QApplication::clipboard();
     clip->setText(id);
     rfid_packet.fields.stx = 0;
     rfid_packet.fields.etx = 0;
    }

}


bool AdminRFID::is_good_rfid_packet()
{
  if (rfid_packet.fields.stx == STX && rfid_packet.fields.etx == ETX)
  {
    //TODO perform checksum on ID field.
    return (true);
  }
  return (false);
}

void AdminRFID::rfid_packet_recv(char data)
{
  unsigned int idx;

  if (data == STX)
  {
    for(idx = 0; idx < sizeof(RFID_packet); idx++)
    {
     rfid_packet.packet_data[idx] = 0;
    }
    rfid_packet.fields.packet_end++;
    rfid_packet.fields.stx = STX;

  }
  else if (data == ETX)
  {
    rfid_packet.fields.packet_end++;
    rfid_packet.fields.etx = ETX;
  }
  else if (rfid_packet.fields.stx == STX && rfid_packet.fields.etx != ETX)
  {
    rfid_packet.fields.packet_end++;
    if ((unsigned int)rfid_packet.fields.packet_end >=  sizeof(RFID_packet))
    {
     for(idx = 0; idx < sizeof(RFID_packet); idx++)
     {
      rfid_packet.packet_data[idx] = 0;
     }
    }
    else
     rfid_packet.packet_data[(int)rfid_packet.fields.packet_end] = data;
  }
  else
  {
    //data is garbage
  }
}


