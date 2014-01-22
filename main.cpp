#include <QtGui/QApplication>
#include "commanderdatalog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CommanderDataLog w;
    w.show();
    
    return a.exec();
}
