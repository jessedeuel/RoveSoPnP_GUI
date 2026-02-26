#include <QApplication>
#include <QLocale>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.resize(1200, 600);
    w.show();
    return a.exec();
}