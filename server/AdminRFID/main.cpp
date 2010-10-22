#include <QtGui/QApplication>
#include "adminrfid.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AdminRFID w;
    w.show();
    return a.exec();
}
