#include "mainwindow.h"

#include <QDebug>
#include <QImage>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QTimer>
#include <future>
#include <regex.h>

#include "Logging.h"
#include "calibrationPage.h"
#include "customMenuBar.h"
#include "debugPnPTestPage.h"
#include "flowControl.h"
#include "jobsPage.h"
#include "operatorPage.h"
#include "settingsPage.h"
#include "sideBar.h"
#include "vision/cameras/BasicCam.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    qDebug() << "Initializing MainWindow...";

    QGridLayout* mainLayout         = new QGridLayout();

    customMenuBar* menuBar_instance = new customMenuBar(this);
    this->setMenuBar(menuBar_instance);

    m_pGRBL_instance = std::make_shared<GRBL>();

    // ---------------------------------------------------------
    // VISION SYSTEM INTEGRATION (Must happen before UI classes that need it)
    // ---------------------------------------------------------
    try
    {
        gantryCam = std::make_shared<BasicCam>("/dev/v4l/by-id/usb-8SSC21C16294V1SR34S00CW_Integrated_Camera_200901010001-video-index0",
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

    // Initialize UI Pages
    sideBar* sideBar_instance                 = new sideBar(this);
    jobsPage* jobsPage_instance               = new jobsPage(this);
    OperatorPage* operatorPage_instance       = new OperatorPage(this);
    settingsPage* settingsPage_instance       = new settingsPage(m_pGRBL_instance, this);
    debugPnPTestPage* debugPage_instance      = new debugPnPTestPage(m_pGRBL_instance, this);
    CalibrationPage* calibrationPage_instance = new CalibrationPage(gantryCam, this);

    QTabWidget* tabs                          = new QTabWidget();
    tabs->addTab(jobsPage_instance, "Jobs");
    tabs->addTab(operatorPage_instance, "Operation");
    tabs->addTab(calibrationPage_instance, "Camera Calibration");
    tabs->addTab(settingsPage_instance, "Settings");
    tabs->addTab(debugPage_instance, "Debug FC Test");

    mainLayout->addWidget(sideBar_instance, 0, 0);
    mainLayout->addWidget(tabs, 0, 1);

    // Set the central widget of MainWindow
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    connect(tabs, &QTabWidget::tabBarClicked, this, &MainWindow::onTabBarClicked);

    // ---------------------------------------------------------
    // FLOW CONTROL INTEGRATION
    // ---------------------------------------------------------

    // 1. Initialize FlowControl. We allocate it on the heap so it lives for the app duration.
    FlowControl* flowControl_instance = new FlowControl(m_pGRBL_instance, gantryCam, nullptr);
    flowControl_instance->setParent(this);    // Added parent pointer to clean memory and stop SIGSEGV upon app exit

    // 2. Bind the UI pages directly to the state machine
    operatorPage_instance->bindFlowControl(flowControl_instance);
    sideBar_instance->bindFlowControl(flowControl_instance);
    calibrationPage_instance->bindFlowControl(flowControl_instance);

    qDebug() << "MainWindow initialized.";
}

MainWindow::~MainWindow()
{
    qDebug() << "Destroying MainWindow...";

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
        case 2: qDebug() << "Pressed Calibration tab"; break;
        case 3: qDebug() << "Pressed Settings tab"; break;
        case 4: qDebug() << "Pressed Debug tab"; break;
        default: break;
    }
}
