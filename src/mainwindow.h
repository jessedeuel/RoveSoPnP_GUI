#pragma once

#include <QComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QtMultimedia/QCamera>
#include <QtMultimedia/QMediaCaptureSession>
#include <QtMultimediaWidgets/QVideoWidget>

#include "grbl.hpp"

// #include "comm.hpp"
//  #include "pnpRunner.h"

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
        MainWindow(QWidget* parent = nullptr);
        ~MainWindow();

    private:
        // UI Elements
        QComboBox* comPortSelectionBox;

        // Comm and flow control member variables
        QList<QString> listPorts();
        // Comm m_PNPMachineComm;
        std::shared_ptr<GRBL> m_pGrbl;

    private slots:
        void onTabBarClicked(int index);
};
