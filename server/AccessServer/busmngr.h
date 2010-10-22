//Copyright Royce Pipkins 2010
//May be used under the terms of the GPL V3 or higher. http://www.gnu.org/licenses/gpl.html
#ifndef BUSMNGR_H
#define BUSMNGR_H
#include <QString>
#include <QList>
#include <QObject>
#include "qextserialport.h"
#include <QTimer>
#include <QTime>
#include <QWaitCondition>
#include <QMutex>
#include "ASCIIProtocol.h"
#include <QSqlDatabase>

struct AuthReply
{
    bool accessGranted;
    QString line1;
    QString line2;
};

struct BusDevice
{
    int addr;
    QDateTime last_reply_time;
    bool offline;
};

class BusMngr : public QObject
{
    Q_OBJECT

public:
    BusMngr(QString comPort);
public slots:
    void onReadyRead();
    void onBytesWritten(qint64 byteCount);
    void onTimeout();
    void onRtsTimeout();
    void onAboutToClose();
    void reconnect();

protected:
    enum BusState
    {
        IDLE,
        WAITING_IDCHECK_REPLY,
        WAITING_ACCESS_ACK,
        DISCONNECTED
    };

    QextSerialPort bus;
    Print busPrinter;
    ASCIIProtocol protocolDriver;
    void issueIdCheck();
    QList<BusDevice> deviceList;
    int deviceIdx;
    void advanceDevice();
    void startBus();
    void DelayMs(int ms);
    BusState busState;
    static const int timeout = 300; //TODO lower this value

    QTimer busTimer;
    QTimer rtsTimer;
    QTimer reconnectTimer;

    QMutex dummytex;
    QWaitCondition sleep;



    void checkID();
    void recvAck();
    void Log(QString msg);
    void Log(int user_id, QString username, bool accessGranted, int door_address, QString door_desc, int role_id, QString role);
    void LogNetEvent(int door_addr, bool online);
    QString GetDoorDesc(int door_addr);

    AuthReply isAuthorized(QString id, QString addr);
    AuthReply authFromFile(QString id, QString addr);
    QSqlDatabase db;

};

#endif // BUSMNGR_H
