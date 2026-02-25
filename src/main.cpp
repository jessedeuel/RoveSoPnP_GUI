#include <QApplication>
#include <QLocale>

#include "mainwindow.h"
#include "Logging.h" // Include Logging to access InitializeLoggers
#include "gui_constants.h"

int main(int argc, char *argv[])
{
    logging::InitializeLoggers(UI_LOGGING_OUTPUT_PATH);

    QApplication a(argc, argv);

    MainWindow w;
    w.resize(1200, 600);
    w.show();
    return a.exec();
}