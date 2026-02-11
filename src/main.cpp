#include "mainwindow.h"
#include "Logging.h" // Include Logging to access InitializeLoggers

#include <QApplication>
#include <QLocale>

int main(int argc, char *argv[])
{
    logging::InitializeLoggers("./logs");

    QApplication a(argc, argv);

    MainWindow w;
    w.resize(1200, 600);
    w.show();
    return a.exec();
}