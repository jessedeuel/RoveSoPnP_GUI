#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QTableWidget>
#include <QTabWidget>
#include <QtMultimedia/QCamera>
#include <QtMultimedia/QMediaCaptureSession>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QComboBox>
#include <QTimer>

#include "comm.hpp"


QT_BEGIN_NAMESPACE
namespace Ui
{
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
    // UI Elements
    QComboBox *comPortSelectionBox;

    // Comm and flow control member variables
    QList<QString> listPorts();
    Comm m_PNPMachineComm;

private slots:
    void onPauseButtonClicked();
    void onEndProgramButtonClicked();
    void onComPortSetButtonClicked();
    void onTabBarClicked(int index);
};
