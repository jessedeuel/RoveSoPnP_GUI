#include "mainwindow.h"

#include <QDebug>
#include <QImage>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QTimer>
#include <future>
#include <regex.h>

#include "Logging.h"
#include "customMenuBar.h"
#include "debugPnPTestPage.h"
#include "jobsPage.h"
#include "operatorPage.h"
#include "settingsPage.h"
#include "sideBar.h"
#include "vision/cameras/BasicCam.h"

// #include "pnpRunner.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    qDebug() << "Initializing MainWindow...";

    QGridLayout* mainLayout         = new QGridLayout();

    customMenuBar* menuBar_instance = new customMenuBar(this);
    this->setMenuBar(menuBar_instance);

    m_pGRBL_instance                     = std::make_shared<GRBL>();

    sideBar* sideBar_instance            = new sideBar(this);
    jobsPage* jobsPage_instance          = new jobsPage(this);
    OperatorPage* operatorPage_instance  = new OperatorPage(this);
    settingsPage* settingsPage_instance  = new settingsPage(m_pGRBL_instance, this);
    debugPnPTestPage* debugPage_instance = new debugPnPTestPage(m_pGRBL_instance, this);

    QTabWidget* tabs                     = new QTabWidget();
    tabs->addTab(jobsPage_instance, "Jobs");
    tabs->addTab(operatorPage_instance, "Operation");
    tabs->addTab(settingsPage_instance, "Settings");
    tabs->addTab(debugPage_instance, "Debug FC Test");

    mainLayout->addWidget(sideBar_instance, 0, 0);
    mainLayout->addWidget(tabs, 0, 1);

    // Set the central widget of MainWindow
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
https:    // www.ebay.com/mye/myebay/bidsoffers

    connect(tabs, &QTabWidget::tabBarClicked, this, &MainWindow::onTabBarClicked);

    // ---------------------------------------------------------
    // VISION SYSTEM INTEGRATION
    // ---------------------------------------------------------

    // Initialize the BasicCam (Using std::make_shared to match std::shared_ptr)
    try
    {
        gantryCam = std::make_unique<BasicCam>("/dev/v4l/by-id/usb-8MP_USB_Camera_8MP_USB_Camera_2022051301-video-index0",
                                               640,
                                               480,
                                               30,
                                               PIXEL_FORMATS::eBGR,
                                               90.0,
                                               90.0,
                                               false,
                                               1);
        qDebug() << "Camera opened successfully. Starting camera thread...";
        gantryCam->Start();
    }
    catch (...)
    {
        qDebug() << "Failed to open camera. Check the camera path and connection.";
    }

    // Setup a QTimer to act as the camera polling loop
    cameraTimer = new QTimer(this);

    // Capture 'operatorPage_instance' in the lambda so we can pass the frame to it
    connect(cameraTimer,
            &QTimer::timeout,
            this,
            [this, operatorPage_instance]()
            {
                cv::Mat cvNormalFrame;

                // Request frame from camera
                std::future<bool> fuCopyStatus = gantryCam->RequestFrameCopy(cvNormalFrame);

                // .get() blocks the main thread very briefly until the frame is ready
                if (fuCopyStatus.get() && !cvNormalFrame.empty())
                {
                    // Convert OpenCV Mat (BGR) to Qt QImage (RGB)
                    QImage qimg(cvNormalFrame.data, cvNormalFrame.cols, cvNormalFrame.rows, cvNormalFrame.step, QImage::Format_RGB888);

                    // OpenCV defaults to BGR, so we swap it to RGB for Qt rendering
                    QImage finalImage = qimg.rgbSwapped();

                    // Pass the frame to the Operator Page
                    operatorPage_instance->updateCameraFrame(finalImage, "Gantry Camera - Live Feed");
                }
            });

    // Start timer at roughly 30 FPS (~33ms interval)
    cameraTimer->start(33);

    qDebug() << "MainWindow initialized.";
}

MainWindow::~MainWindow()
{
    qDebug() << "Destroying MainWindow...";

    // Stop the timer so it stops requesting frames
    if (cameraTimer)
    {
        cameraTimer->stop();
    }

    // Safely stop the camera thread before destruction
    if (gantryCam)
    {
        gantryCam->RequestStop();
        gantryCam->Join();
    }
}

void MainWindow::onTabBarClicked(int index)
{
    switch (index)
    {
        case 0: qDebug() << "Pressed Jobs tab"; break;
        case 1: qDebug() << "Pressed Operator tab"; break;
        case 2: qDebug() << "Pressed Settings tab"; break;
        default: break;
    }
}
