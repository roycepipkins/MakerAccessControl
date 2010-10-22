//Copyright Royce Pipkins 2010
//May be used under the terms of the GPL V3 or higher. http://www.gnu.org/licenses/gpl.html
#include <QtCore/QCoreApplication>
#include "busmngr.h"


int main(int argc, char *argv[])
{

    QCoreApplication a(argc, argv);
    QString port = "COM13";
    if (argc >= 2) port = argv[1];
    BusMngr mngr(port);

    return a.exec();
}
