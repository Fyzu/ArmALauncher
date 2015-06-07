#include "launcher.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    launcher w;

    //Отобразить виджет
    w.show();

    return a.exec();
}
