#include "mainwindow.h"
#include <regex.h>
#include <string.h>
#include <QImage>
#include <QPixmap>
#include <iostream>
#include <QDebug>
#include <QTextEdit>
#include <QPlainTextEdit>
#include "jobsPage.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    qDebug() << "Initializing MainWindow...";

    QMenuBar *menuBar = new QMenuBar();
    menuBar->addMenu("File");
    menuBar->addMenu("Edit");

    QGridLayout *sideBarLayout = new QGridLayout();
    QLabel *stateLabel = new QLabel("State: ", this);
    stateLabel->setObjectName("stateLabel");
    QLabel *connectionStatusLabel = new QLabel("Connection Status: ", this);
    connectionStatusLabel->setObjectName("connectionStatus");
    QLabel *positionLabel = new QLabel("Position: ", this);
    positionLabel->setObjectName("positionLabel");
    QLabel *currentComponentLabel = new QLabel("Current Component: ", this);
    currentComponentLabel->setObjectName("currentComponentLabel");
    QLabel *currentJobLabel = new QLabel("Current Job: ", this);
    currentJobLabel->setObjectName("currentJobLabel");
    QPushButton *pauseButton = new QPushButton("Pause", this);
    pauseButton->setObjectName("pauseButton");
    QPushButton *endProgramButton = new QPushButton("Start Program", this);
    endProgramButton->setStyleSheet("background-color: green;");
    endProgramButton->setObjectName("endProgramButton");

    sideBarLayout->addWidget(stateLabel);
    sideBarLayout->addWidget(connectionStatusLabel);
    sideBarLayout->addWidget(positionLabel);
    sideBarLayout->addWidget(currentComponentLabel);
    sideBarLayout->addWidget(currentJobLabel);
    sideBarLayout->addWidget(pauseButton);
    sideBarLayout->addWidget(endProgramButton);

    jobsPage *jobsPage_instance = new jobsPage(this);

    QWidget *operatorPage = new QWidget();
    QGridLayout *operatorPageLayout = new QGridLayout();
    operatorPage->setLayout(operatorPageLayout);
    QPushButton *runJobButton = new QPushButton("Run Job", this);
    operatorPageLayout->addWidget(runJobButton);

    // Camera Display Setup
    m_pCameraDisplayLabel = new QLabel("Camera Feed Loading...", this);
    m_pCameraDisplayLabel->setObjectName("cameraDisplayLabel");
    m_pCameraDisplayLabel->setMinimumSize(640, 480);
    m_pCameraDisplayLabel->setAlignment(Qt::AlignCenter);
    m_pCameraDisplayLabel->setStyleSheet("border: 1px solid black; background-color: #333; color: white;");
    operatorPageLayout->addWidget(m_pCameraDisplayLabel);

    // Camera Initialization
    qDebug() << "Attempting to open camera /dev/video0...";
    try
    {
        // Initialize BasicCam (device 0, 640x480, 30FPS)
        m_pGantryCam = std::make_unique<BasicCam>("/dev/video0", 640, 480, 30, PIXEL_FORMATS::eBGR, 90.0, 90.0, false, 1);
        m_pGantryCam->Start();
        qDebug() << "Camera started successfully.";
    }
    catch (const std::exception &e)
    {
        qDebug() << "CRITICAL: Camera initialization failed:" << e.what();
        m_pCameraDisplayLabel->setText("Camera Error: Initialization Failed");
    }
    catch (...)
    {
        qDebug() << "CRITICAL: Camera initialization failed with unknown error.";
    }

    // Start Timer to poll frames at ~30 FPS (33ms)
    m_pCameraTimer = new QTimer(this);
    connect(m_pCameraTimer, &QTimer::timeout, this, &MainWindow::updateCameraDisplay);
    m_pCameraTimer->start(33);

    QTextEdit *gCodeEntryTextBox = new QTextEdit("Enter GCode", this);
    QPushButton *gCodeSendButton = new QPushButton("Send GCode", this);
    operatorPageLayout->addWidget(gCodeEntryTextBox);
    operatorPageLayout->addWidget(gCodeSendButton);

    QWidget *settingsPage = new QWidget();
    QGridLayout *settingsPageLayout = new QGridLayout();
    settingsPage->setLayout(settingsPageLayout);
    comPortSelectionBox = new QComboBox();

    QList<QString> ports = MainWindow::listPorts();
    comPortSelectionBox->addItems(ports);

    QPushButton *comPortConnectButton = new QPushButton("Connect");
    comPortConnectButton->setObjectName("comPortConnectButton");
    settingsPageLayout->addWidget(comPortSelectionBox, 0, 0);
    settingsPageLayout->addWidget(comPortConnectButton, 0, 1);

    QGridLayout *mainLayout = new QGridLayout();
    QTabWidget *tabs = new QTabWidget();
    tabs->addTab(jobsPage_instance, "Jobs");
    tabs->addTab(operatorPage, "Operation");
    tabs->addTab(settingsPage, "Settings");

    mainLayout->addWidget(menuBar, 0, 0, 1, 2);
    mainLayout->addLayout(sideBarLayout, 1, 0);
    mainLayout->addWidget(tabs, 1, 1);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    connect(pauseButton, &QPushButton::clicked, this, &MainWindow::onPauseButtonClicked);
    connect(endProgramButton, &QPushButton::clicked, this, &MainWindow::onEndProgramButtonClicked);
    connect(comPortConnectButton, &QPushButton::clicked, this, &MainWindow::onComPortSetButtonClicked);
    connect(tabs, &QTabWidget::tabBarClicked, this, &MainWindow::onTabBarClicked);
    connect(gCodeSendButton, &QPushButton::clicked, this, &MainWindow::onGCodeSendButtonClicked);

    qDebug() << "MainWindow initialized.";
}

MainWindow::~MainWindow()
{
    qDebug() << "Destroying MainWindow...";
    // Stop camera and thread pool safely before destruction
    if (m_pGantryCam)
    {
        qDebug() << "Stopping Camera...";
        m_pGantryCam->RequestStop();
        m_pGantryCam->Join();
    }
}

void MainWindow::updateCameraDisplay()
{
    // Safety check: verify camera exists and is running
    if (!m_pGantryCam || m_pGantryCam->GetThreadState() != Thread<void>::ThreadState::eRunning)
    {
        return;
    }

    // 1. If we have no active request, make one.
    if (!m_frameReadyFuture.valid())
    {
        m_frameReadyFuture = m_pGantryCam->RequestFrameCopy(m_currentFrame);
        return; // Return and wait for next timer tick
    }

    // 2. If we have a request, check if it is ready (without blocking).
    if (m_frameReadyFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
    {
        try
        {
            // Retrieve result
            bool success = m_frameReadyFuture.get();

            if (success && !m_currentFrame.empty())
            {
                // Convert OpenCV BGR to Qt RGB
                cv::Mat rgbFrame;
                cv::cvtColor(m_currentFrame, rgbFrame, cv::COLOR_BGR2RGB);

                // Check for valid dimensions
                if (rgbFrame.cols > 0 && rgbFrame.rows > 0)
                {
                    // Create QImage with deep copy to prevent segfaults
                    QImage qimg((uchar *)rgbFrame.data, rgbFrame.cols, rgbFrame.rows, rgbFrame.step, QImage::Format_RGB888);
                    m_pCameraDisplayLabel->setPixmap(QPixmap::fromImage(qimg.copy()));
                }
            }
        }
        catch (cv::Exception &e)
        {
            qDebug() << "OpenCV Error in updateCameraDisplay: " << e.what();
        }
        catch (...)
        {
            qDebug() << "Unknown Error in updateCameraDisplay";
        }

        // 3. Request the NEXT frame immediately
        m_frameReadyFuture = m_pGantryCam->RequestFrameCopy(m_currentFrame);
    }
}

QList<QString> MainWindow::listPorts()
{
    QList<QString> result;
    std::array<char, 128> buffer;

#ifdef __linux__
    FILE *pipe = popen("ls /dev/tty*", "r"); // Open pipe for reading
    if (!pipe)
    {
        qDebug("Error: popen() failed!");
    }
    else
    {
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
        {
            result.push_back(((QString)buffer.data()).trimmed());
        }
    }
    if (pipe)
        pclose(pipe); // Close the pipe
#elif __windows__
    qDebug("On Windows");
#endif

    return result;
}

void MainWindow::onPauseButtonClicked()
{
    // findChild<QLabel*("label")->setText("Button Clicked!");
}

void MainWindow::onEndProgramButtonClicked()
{
    QPushButton *endProgramButton = findChild<QPushButton *>("endProgramButton");
    qDebug() << "EndProgramButton Clicked";
    if (endProgramButton->styleSheet().contains("background-color: red;"))
    {
        endProgramButton->setStyleSheet("background-color: green;");
        endProgramButton->setText("Start Program");
    }
    else
    {
        endProgramButton->setStyleSheet("background-color: red;");
        endProgramButton->setText("End Program");
    }
}

void MainWindow::onComPortSetButtonClicked()
{
    QPushButton *comPortConnectButton = findChild<QPushButton *>("comPortConnectButton");
    m_PNPMachineComm = Comm();
    QString comPortSelectionBoxText = comPortSelectionBox->currentText().trimmed();
    QByteArray array = comPortSelectionBoxText.toLocal8Bit();
    qDebug() << "Connect Port: |" << array.constData() << "|";
    if (m_PNPMachineComm.setupComm(array.constData()) == false)
    {
        comPortConnectButton->setStyleSheet("background-color: red;");
    }
    else
    {
        comPortConnectButton->setStyleSheet("background-color: green;");
    }
}

void MainWindow::onTabBarClicked(int index)
{
    switch (index)
    {
    case 0:
        qDebug() << "Pressed Jobs tab";
        break;
    case 1:
        qDebug() << "Pressed Operator tab";
        break;
    case 2:
        qDebug() << "Pressed Settings tab";
        break;
    default:
        break;
    }
}

void MainWindow::onGCodeSendButtonClicked()
{
    qDebug("GCode Send Button Clicked");

}