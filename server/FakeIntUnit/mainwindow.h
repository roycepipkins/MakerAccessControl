//Copyright Royce Pipkins 2010
//May be used under the terms of the GPL V3 or higher. http://www.gnu.org/licenses/gpl.html
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qextserialport.h"
#include <QTimer>
#include <QWaitCondition>
#include <QMutex>
#include "ASCIIProtocol.h"


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);
    static const int timeout = 300; //TODO lower this value

private:
    Ui::MainWindow *ui;
    QextSerialPort bus;
    Print busPrinter;
    ASCIIProtocol protocolDriver;

    void DoIdCheck();
    void GrantAccess();
    void DenyAccess();


private slots:
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();
    void onReadyRead();
};

#endif // MAINWINDOW_H
