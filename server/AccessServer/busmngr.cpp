//Copyright Royce Pipkins 2010
//May be used under the terms of the GPL V3 or higher. http://www.gnu.org/licenses/gpl.html
#include "busmngr.h"
#include "qextserialport.h"
#include <QtDebug>
#include <QSettings>
#include <QVariant>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QStringList>
#include <QCoreApplication>
#include <QFile>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDate>



#define ID_CHECK 1
#define ACCESS_REQ 2
#define NO_ACTIVITY 3
#define ACCESS_GRANTED 4
#define ACCESS_DENIED 5
#define SET_IDLE 6
#define ACKNOWLEDGE 7

BusMngr::BusMngr(QString comPort):
        bus(),
        busPrinter(bus),
        protocolDriver(busPrinter),
        deviceIdx(0),
        db(QSqlDatabase::addDatabase("QMYSQL"))
{

    QString conffile = QString(QCoreApplication::applicationDirPath() + "/AccessServer.conf");
    QSettings settings(conffile, QSettings::IniFormat);

    if (settings.status() != QSettings::NoError)
    {
        int f;
        f = 1;
    }
    QVariant addrV = settings.value("doorAddresses", "1");
    QStringList addrs = addrV.toStringList();
    QString addr;
    foreach (addr, addrs)
    {
        BusDevice device;
        device.addr = addr.toInt();
        device.last_reply_time = QDateTime::currentDateTime();
        device.offline = false;
        deviceList.append(device);
    }


    dummytex.lock();



    bus.setQueryMode(QextSerialPort::EventDriven);
    bus.setPortName(settings.value("commPort", "COM10").toString());
    qDebug() << "BusMngr startup";
    bus.setBaudRate(BAUD9600);
    bus.setParity(PAR_NONE);
    bus.setDataBits(DATA_8);
    bus.setStopBits(STOP_1);
    bus.setFlowControl(FLOW_OFF);
    bus.setTimeout(timeout);

    db.setHostName(settings.value("dbHostName", "127.0.0.1").toString());
    db.setPort(settings.value("dbPort", 3306).toInt());
    db.setDatabaseName(settings.value("dbName", "AccessControl").toString());
    db.setUserName(settings.value("dbUserName", "AccessServer").toString());
    db.setPassword(settings.value("dbPassword", "").toString());
    db.setConnectOptions(settings.value("dbConnectionOptions", "MYSQL_OPT_RECONNECT=1").toString());

    if (!db.open())
    {
        Log("Unable to open the database!");
    }


    if (bus.open(QIODevice::ReadWrite) == true) {
        connect(&bus, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        connect(&bus, SIGNAL(bytesWritten(qint64)), this, SLOT(onBytesWritten(qint64)));
        connect(&bus, SIGNAL(aboutToClose()), this, SLOT(onAboutToClose()));
        connect(&busTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
        connect(&rtsTimer, SIGNAL(timeout()), this, SLOT(onRtsTimeout()));
        connect(&reconnectTimer, SIGNAL(timeout()), this, SLOT(reconnect()));
        startBus();
    }
    else {
        qDebug() << "ERROR failed to open :" << bus.portName() << ": " << bus.errorString();
        Log("ERROR - Unable to open " + bus.portName() + " please double check that the device name is correct. Quitting");
        busState = DISCONNECTED;
        QCoreApplication::quit();
    }


}


void BusMngr::issueIdCheck()
{
    qDebug() << "issueIdCheck()";
    sleep.wait(&dummytex, 10);
    bus.setRts(false);
    protocolDriver.send(deviceList[deviceIdx].addr, ID_CHECK, (uint8_t*)"ID_CHECK");
    busState = WAITING_IDCHECK_REPLY;
    busTimer.start(timeout);
    rtsTimer.start(37); //approx 1ms per char
}

void BusMngr::onTimeout()
{
    qDebug() << "timeout";
    //if we timed out for any reason, give up on device for now
    //and just poll the next device
    if (QDateTime::currentDateTime().toTime_t()
        - deviceList[deviceIdx].last_reply_time.toTime_t()
        >= 30 &&
        deviceList[deviceIdx].offline == false)
    {
        deviceList[deviceIdx].offline = true;
        QString door_desc = GetDoorDesc(deviceList[deviceIdx].addr);
        Log(door_desc + " is not responding and is now considered offline.");
        LogNetEvent(deviceList[deviceIdx].addr, false);
    }
    advanceDevice();
    issueIdCheck();
}


void BusMngr::onRtsTimeout()
{
    bus.setRts(true);
}


void BusMngr::onBytesWritten(qint64 byteCount)
{   


}

void BusMngr::advanceDevice()
{
    deviceIdx++;
    if (deviceIdx >= deviceList.size()) deviceIdx = 0;
}

void BusMngr::onReadyRead()
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
            deviceList[deviceIdx].last_reply_time = QDateTime::currentDateTime();
            if (deviceList[deviceIdx].offline)
            {
                deviceList[deviceIdx].offline = false;
                Log(GetDoorDesc(deviceList[deviceIdx].addr) + " is back online.");
                LogNetEvent(deviceList[deviceIdx].addr, true);
            }
            switch(busState)
            {
            case IDLE:
                //we got a packet when we were not waiting for one
                //ignore the packet and poll the current device
                issueIdCheck();
                break;
            case WAITING_IDCHECK_REPLY:
                checkID();
                break;
            case WAITING_ACCESS_ACK:
                recvAck();
                break;
            case DISCONNECTED:
                //depend onAboutToClose to fix this
                break;
            }

            protocolDriver.erasePkt();
        }
        else qDebug() << "bad/incomplete packet";
    }
}

void BusMngr::recvAck()
{
    if (protocolDriver.getType() == ACKNOWLEDGE)
    {
        qDebug() << "recv'ed ACK";
        advanceDevice();
        issueIdCheck();
    }
    else qDebug() << "unexpected msg type. wanted ACK";

}

void BusMngr::checkID()
{
    if (protocolDriver.getType() == ACCESS_REQ)
    {
        qDebug() << "Access Request";
        QString id = (char*)protocolDriver.getBody();
        QString addr;
        addr.setNum(protocolDriver.getAddress(), 16);
        AuthReply reply = isAuthorized(id, addr);
        QString body;
        uint8_t code;
        body = reply.line1;
        if (reply.line1.length() < 16)  body += QString( 16 - reply.line1.length(), QChar(' '));
        body += reply.line2;
        if (reply.line2.length() < 16)  body += QString( 16 - reply.line2.length(), QChar(' '));
        reply.accessGranted ? code = ACCESS_GRANTED : code = ACCESS_DENIED;
        sleep.wait(&dummytex, 10);
        bus.setRts(false);
        protocolDriver.send(deviceList[deviceIdx].addr, code, (uint8_t*)body.toAscii().constData());
        busState = WAITING_ACCESS_ACK;
        busTimer.start(timeout);
        rtsTimer.start(body.length()+25); //approx 1ms per char
    }
    else if (protocolDriver.getType() == NO_ACTIVITY)
    {
        advanceDevice();
        issueIdCheck();
    }
}

AuthReply BusMngr::isAuthorized(QString id, QString addr)
{
    QString msg;
    QString conffile = QString(QCoreApplication::applicationDirPath() + "/AccessServer.conf");
    QSettings settings(conffile, QSettings::IniFormat);
    AuthReply reply;
    bool convert_ok = false;
    int door_addr = addr.toInt(&convert_ok, 16);
    QString query_str =  "select username,role,door_desc,user_roles.user_id,roles.role_id "
                     "from hashes,roles,doors,door_roles,user_roles "
                     "where roles.role_id=door_roles.role_id "
                     "and doors.door_address=door_roles.door_address "
                     "and door_roles.role_id = user_roles.role_id "
                     "and hashes.user_id = user_roles.user_id "
                     "and hashes.hash_id = md5(\"" + id + "\") "
                     "and doors.door_address = "
                     + QString::number(door_addr, 10) + ";";

    QSqlQuery query(db);
    if (query.exec(query_str))
    {
        int username_field = query.record().indexOf("username");
        int user_id_field = query.record().indexOf("user_id");
        int role_field = query.record().indexOf("role");
        int role_id_field = query.record().indexOf("role_id");
        int door_field = query.record().indexOf("door_desc");

        if (query.next())
        {
            //access granted
            reply.accessGranted = true;
            reply.line1 = "    Welcome    ";
            QString member_name = query.value(username_field).toString();
            if (member_name.length() > 16) member_name.chop(member_name.length() - 16);
            if (member_name.length() < 16)
            {
                int spaces = 16 - member_name.length();
                int rspaces = spaces / 2;
                int lspaces = spaces - rspaces;
                reply.line2 = QString(lspaces, QChar(' ')) + member_name + QString(rspaces, QChar(' '));
            }
            //todo log file and db
            Log(query.value(user_id_field).toInt(), member_name, true, door_addr, query.value(door_field).toString(), query.value(role_id_field).toInt(), query.value(role_field).toString());
        }
        else
        {
            //access denied
            //todo auxillary query to determine if it was a role failure.
            reply.accessGranted = false;
            reply.line1 = "     Access";
            reply.line2 = "     Denied!";
            //todo log file and db

            query_str = "select user_id, username from hashes where hash_id = md5(\"" + id + "\");";
            query.exec(query_str);
            if (query.next())
            {
                int uid = query.value(0).toInt();
                QString uname = query.value(1).toString();
                QString door_desc = GetDoorDesc(door_addr);
                Log(uid, uname , false, door_addr, door_desc, -1, "No Role" );
            }
            else
            {
                QString door_desc = GetDoorDesc(door_addr);
                Log(-1, "Unknown user", false, door_addr, door_desc, -1, "No Role");
            }
        }
    }
    else
    {
        return authFromFile(id, addr);
    }
    return reply;
}


AuthReply BusMngr::authFromFile(QString id, QString addr)
{
    AuthReply reply;
    QString conffile = QString(QCoreApplication::applicationDirPath() + "/idlist.ini");
    QSettings idList(conffile, QSettings::IniFormat);
    QVariant notlisted("not listed");
    QString member_name = idList.value(id, notlisted).toString();
    QString msg;
    if (member_name == notlisted.toString())
    {
        reply.accessGranted = false;
        reply.line1 = "     Access";
        reply.line2 = "     Denied!";
        msg = "Access Denied for " + id + " at the door addressed " + addr + " (FILE)";
        Log(msg);
    }
    else
    {
        reply.accessGranted = true;
        reply.line1 = "    Welcome    ";
        if (member_name.length() > 16) member_name.chop(member_name.length() - 16);
        if (member_name.length() < 16)
        {
            int spaces = 16 - member_name.length();
            int rspaces = spaces / 2;
            int lspaces = spaces - rspaces;
            reply.line2 = QString(lspaces, QChar(' ')) + member_name + QString(rspaces, QChar(' '));
        }
        msg = "Access Granted at the door addressed " + addr + " for user " + member_name + " (FILE)";
        Log(msg);
    }

    return reply;
}


void BusMngr::LogNetEvent(int door_addr, bool online)
{
    QString query_str = "insert into network_events(door_address, online, event_time) ";
            query_str += "values(" + QString::number(door_addr, 10) + "," + (online ? "true," : "false,") +
                    "'" + QDateTime::currentDateTime().toString(Qt::ISODate) + "');";
    QSqlQuery query(db);
    query.exec(query_str);
}

void BusMngr::Log(int user_id, QString username, bool accessGranted, int door_address, QString door_desc, int role_id, QString role)
{
    QString ctime = QDate::currentDate().toString(Qt::ISODate) + " " + QTime::currentTime().toString(Qt::ISODate);
    QString query_str = "insert into access_attempts(user_id,door_address,role_id,access_granted,attempt_time) "
                        "values(" + QString::number(user_id, 10) + "," + QString::number(door_address, 10) +
                        "," + QString::number(role_id, 10) + "," + (accessGranted ? "true," : "false,") +
                        "'" + ctime + "');";

    QSqlQuery query(db);
    if (!query.exec(query_str))
    {
        Log("Unable to write log entry to database.");
    }

    QString msg = "Access " + (accessGranted ? QString("Granted") : QString("Denied")) + " at " + door_desc +
    " for user " + username;

    if (accessGranted)
    {
        msg = msg + " due to assigned role '" + role + "' (DB)";
    }

    QString conffile = QString(QCoreApplication::applicationDirPath() + "/AccessServer.conf");
    QSettings settings(conffile, QSettings::IniFormat);
    QVariant notlisted("/var/log/AccessServer.log");
    QString log_fname = settings.value("logFile", notlisted).toString();

    QFile file(log_fname);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
             return;

    QTextStream out(&file);
    out << ctime << ": " << msg << endl;

}

void BusMngr::Log(QString msg)
{
    QString conffile = QString(QCoreApplication::applicationDirPath() + "/AccessServer.conf");
    QSettings settings(conffile, QSettings::IniFormat);
    QVariant notlisted("/var/log/AccessServer.log");
    QString log_fname = settings.value("logFile", notlisted).toString();

    QFile file(log_fname);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
             return;

    QTextStream out(&file);
    QString ctime = QDate::currentDate().toString(Qt::ISODate) + " " + QTime::currentTime().toString(Qt::ISODate);
    out << ctime << ": " << msg << endl;

}



void BusMngr::DelayMs(int ms)
{
    QEventLoop eloop;
    QTimer dlay;
    connect(&dlay, SIGNAL(timeout()), &eloop, SLOT(quit()));
    dlay.start(ms);
    eloop.exec();
}

void BusMngr::startBus()
{
    busState = IDLE;
    busTimer.setSingleShot(true);
    rtsTimer.setSingleShot(true);

    Log("Access Server Started.");
    issueIdCheck();

}

void BusMngr::reconnect()
{
    DelayMs(2000);
    Log("ERROR - the Serial port has shutdown unexpectedly. Attempting to reconnect.");

    if (bus.open(QIODevice::ReadWrite) == true)
    {
        startBus();
    }
    else
    {
        reconnectTimer.setSingleShot(true);
        reconnectTimer.start(2000);
    }
}

void BusMngr::onAboutToClose()
{
    busState = DISCONNECTED;
    reconnectTimer.setSingleShot(true);
    reconnectTimer.start(2000);
}

QString BusMngr::GetDoorDesc(int door_addr)
{
    QString query_str = "select door_desc from doors where door_address = " + QString::number(door_addr, 10) + ";";
    QSqlQuery query(db);
    query.exec(query_str);
    QString door_desc = "Unnamed Door";
    if (query.next())
    {
        door_desc = query.value(0).toString();
    }
    return door_desc;
}
