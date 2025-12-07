#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define _GLIBCXX_USE_CXX11_ABI 0

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QTableWidget>
#include <QTabWidget>
#include <QtMultimedia/QCamera>
#include <QMenuBar>
#include <QtMultimedia/QMediaCaptureSession>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QComboBox>

#include <cstdio>
#include <memory>
#include <vector>

#include "comm.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Comm m_PNPMachineComm;
    Ui::MainWindow *ui;

    QComboBox *comPortSelectionBox;

    QList<QString> listPorts();

private slots:
    void onPauseButtonClicked();
    void onEndProgramButtonClicked();
    void onComPortSetButtonClicked();
};
#endif // MAINWINDOW_H
