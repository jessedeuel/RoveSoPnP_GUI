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

#include "vision/algorithms/CameraCalibration.hpp"
#include "vision/algorithms/ComponentDetector.hpp"
#include "vision/algorithms/FicucialDetector.hpp"
#include "vision/algorithms/PixelTo3D.hpp"
#include "vision/algorithms/PlacementVerifier.hpp"
#include "vision/algorithms/VisualHoming.hpp"
#include "vision/cameras/BasicCam.h"

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
    Ui::MainWindow *ui;
    QComboBox *comPortSelectionBox;

    // Comm and flow control member variables
    QList<QString> listPorts();
    Comm m_PNPMachineComm;

    // Vision member variables
    IPS m_IterPersecond;
    std::unique_ptr<BasicCam> m_pGantryCam;
    std::vector<uint32_t> m_vThreadFPSValue;
    std::vector<cv::Mat> m_vCalibrationImages;

private slots:
    void onPauseButtonClicked();
    void onEndProgramButtonClicked();
    void onComPortSetButtonClicked();
    void onTabBarClicked(int index);
};
#endif // MAINWINDOW_H
