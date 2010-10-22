#ifndef ADMINRFID_H
#define ADMINRFID_H

#include <QtGui/QDialog>
#include <qextserialport.h>

namespace Ui
{
    class AdminRFID;
}

struct RFID_fields
{
 char packet_end;
 char stx;
 char id[10];
 char checksum[2];
 char crlf[2];
 char etx;
};

union RFID_packet
{
 char packet_data[17];
 RFID_fields fields;
};



class AdminRFID : public QDialog
{
    Q_OBJECT

public:
    AdminRFID(QWidget *parent = 0);
    ~AdminRFID();

private:
    Ui::AdminRFID *ui;
    QextSerialPort* bus;
    void CreatePort();
    void rfid_packet_recv(char data);
    bool is_good_rfid_packet();
    RFID_packet rfid_packet;

public slots:
    void onConnect();
    void onReadyRead();
    void scanPorts();
};

#endif // ADMINRFID_H
