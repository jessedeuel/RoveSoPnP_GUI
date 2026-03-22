#include "operatorPage.h"
#include <QDebug>
#include <QImage>
#include <QMessageBox>
#include <QPixmap>
#include <QTimer>    // Added QTimer include

#include "vision/algorithms/ComponentDetector.hpp"
#include "vision/algorithms/FicucialDetector.hpp"
#include "vision/algorithms/VisualHoming.hpp"

// std::shared_ptr<PnPRunner> pnpRunner
// m_pPnPRunner_instance(pnpRunner)
operatorPage::operatorPage(QWidget* parent) : QWidget(parent), m_eVisionMode(VisionMode::None)
{
    m_pOperatorPageLayout = new QGridLayout(this);

    // ==========================================
    // 1. MACHINE STATUS PANEL (Top Left)
    // ==========================================
    m_pStatusGroup            = new QGroupBox("Machine Status", this);
    QVBoxLayout* statusLayout = new QVBoxLayout(m_pStatusGroup);
    m_pStateLabel             = new QLabel("State: DISCONNECTED", this);
    m_pStateLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: red;");
    m_pPositionLabel = new QLabel("X: 0.00  Y: 0.00  Z: 0.00  A: 0.00", this);

    statusLayout->addWidget(m_pStateLabel);
    statusLayout->addWidget(m_pPositionLabel);
    m_pOperatorPageLayout->addWidget(m_pStatusGroup, 0, 0);

    // ==========================================
    // 2. CONTROL PANEL (Bottom Left)
    // ==========================================
    m_pControlGroup            = new QGroupBox("Machine Controls", this);
    QVBoxLayout* controlLayout = new QVBoxLayout(m_pControlGroup);

    m_pHomeBtn                 = new QPushButton("Home Machine", this);
    m_pCalibrateVisionBtn      = new QPushButton("Calibrate PCB Vision", this);
    m_pStartJobBtn             = new QPushButton("Start Job", this);
    m_pPauseBtn                = new QPushButton("Pause", this);
    m_pAbortBtn                = new QPushButton("ABORT / E-STOP", this);
    m_pAbortBtn->setStyleSheet("background-color: red; color: white; font-weight: bold;");

    controlLayout->addWidget(m_pHomeBtn);
    controlLayout->addWidget(m_pCalibrateVisionBtn);
    controlLayout->addWidget(m_pStartJobBtn);
    controlLayout->addWidget(m_pPauseBtn);
    controlLayout->addWidget(m_pAbortBtn);
    m_pOperatorPageLayout->addWidget(m_pControlGroup, 1, 0);

    // Connect control buttons to PnPRunner state machine
    connect(m_pHomeBtn, &QPushButton::clicked, this, &operatorPage::onHomeClicked);
    connect(m_pCalibrateVisionBtn, &QPushButton::clicked, this, &operatorPage::onCalibrateClicked);
    connect(m_pStartJobBtn, &QPushButton::clicked, this, &operatorPage::onStartJobClicked);
    connect(m_pPauseBtn, &QPushButton::clicked, this, &operatorPage::onPauseClicked);
    connect(m_pAbortBtn, &QPushButton::clicked, this, &operatorPage::onAbortClicked);

    // ==========================================
    // 3. VISION PANEL (Right Side)
    // ==========================================
    m_pVisionGroup               = new QGroupBox("Live Vision Feed", this);
    QVBoxLayout* visionLayout    = new QVBoxLayout(m_pVisionGroup);

    QHBoxLayout* visionBtnLayout = new QHBoxLayout();
    m_pModeNoneBtn               = new QPushButton("Raw", this);
    m_pModeFiducialBtn           = new QPushButton("Fiducial", this);
    m_pModeComponentBtn          = new QPushButton("Component", this);
    m_pModeHomingBtn             = new QPushButton("Homing", this);
    visionBtnLayout->addWidget(m_pModeNoneBtn);
    visionBtnLayout->addWidget(m_pModeFiducialBtn);
    visionBtnLayout->addWidget(m_pModeComponentBtn);
    visionBtnLayout->addWidget(m_pModeHomingBtn);

    m_pCameraDisplayLabel = new QLabel("Loading Camera...", this);
    m_pCameraDisplayLabel->setMinimumSize(640, 480);
    m_pCameraDisplayLabel->setAlignment(Qt::AlignCenter);
    m_pCameraDisplayLabel->setStyleSheet("background-color: #222; color: white;");

    visionLayout->addLayout(visionBtnLayout);
    visionLayout->addWidget(m_pCameraDisplayLabel);
    m_pOperatorPageLayout->addWidget(m_pVisionGroup, 0, 1, 2, 1);    // Span 2 rows

    // --- GCode Entry Setup ---
    m_pGCodeEntryTextBox = new QTextEdit("Enter GCode", this);
    m_pGCodeEntryTextBox->setMaximumHeight(50);
    m_pGCodeSendButton = new QPushButton("Send GCode", this);
    m_pOperatorPageLayout->addWidget(m_pGCodeEntryTextBox, 3, 0);
    m_pOperatorPageLayout->addWidget(m_pGCodeSendButton, 3, 1);
    // Connect GCode Send Button
    connect(m_pGCodeSendButton, &QPushButton::clicked, this, &operatorPage::onGCodeSendButtonClicked);

    // Connect Vision Buttons
    connect(m_pModeNoneBtn, &QPushButton::clicked, this, &operatorPage::setVisionNone);
    connect(m_pModeFiducialBtn, &QPushButton::clicked, this, &operatorPage::setVisionFiducial);
    connect(m_pModeComponentBtn, &QPushButton::clicked, this, &operatorPage::setVisionComponent);
    connect(m_pModeHomingBtn, &QPushButton::clicked, this, &operatorPage::setVisionHoming);

    // ==========================================
    // Initialization & Timers
    // ==========================================
    try
    {
        m_pGantryCam = std::make_unique<BasicCam>("/dev/v4l/by-id/usb-SunplusIT_Inc_USB_2.0_Camera_20201211V0-video-index0",
                                                  640,
                                                  480,
                                                  30,
                                                  PIXEL_FORMATS::eBGR,
                                                  90.0,
                                                  90.0,
                                                  false,
                                                  1);
        qDebug() << "Camera opened successfully. Starting camera thread...";
        m_pGantryCam->Start();
    }
    catch (...)
    {
        m_pCameraDisplayLabel->setText("Camera Error");
    }

    // --- Added Timer Setup ---
    QTimer* updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &operatorPage::updateUIAndCamera);
    updateTimer->start(33);    // Check for new frames at ~30 FPS

    qDebug() << "operatorPage initialized.";
}

operatorPage::~operatorPage()
{
    if (m_pGantryCam)
    {
        m_pGantryCam->RequestStop();
        m_pGantryCam->Join();
    }
}

// --- User Interaction Hooks ---
void operatorPage::onHomeClicked()
{
    // m_pPnPRunner_instance->CommandHomeMachine();
}

void operatorPage::onCalibrateClicked()
{
    // m_pPnPRunner_instance->CommandCalibrateVision();
}

void operatorPage::onStartJobClicked()
{
    // m_pPnPRunner_instance->CommandStartJob("board/CoreBoard-all-pos.csv");
}

void operatorPage::onAbortClicked()
{
    // m_pPnPRunner_instance->CommandAbort();
}

void operatorPage::onPauseClicked()
{
    // if (m_pPnPRunner_instance->GetCurrentState() == MachineState::RUNNING_JOB)
    // {
    //     m_pPnPRunner_instance->CommandPauseJob();
    // }
    // else if (m_pPnPRunner_instance->GetCurrentState() == MachineState::PAUSED)
    // {
    //     m_pPnPRunner_instance->CommandResumeJob();
    // }
}

// --- Main Update Loop ---
void operatorPage::updateUIAndCamera()
{
    // 1. Update UI Status based on PnPRunner State
    // MachineState currentState = m_pPnPRunner_instance->GetCurrentState();

    // switch (currentState)
    // {
    //     case MachineState::DISCONNECTED:
    //         m_pStateLabel->setText("State: DISCONNECTED");
    //         m_pStateLabel->setStyleSheet("color: red;");
    //         break;
    //     case MachineState::IDLE:
    //         m_pStateLabel->setText("State: IDLE");
    //         m_pStateLabel->setStyleSheet("color: green;");
    //         break;
    //     case MachineState::HOMING:
    //         m_pStateLabel->setText("State: HOMING");
    //         m_pStateLabel->setStyleSheet("color: orange;");
    //         break;
    //     case MachineState::VISION_CALIBRATION:
    //         m_pStateLabel->setText("State: CALIBRATING VISION");
    //         m_pStateLabel->setStyleSheet("color: orange;");
    //         break;
    //     case MachineState::RUNNING_JOB:
    //         m_pStateLabel->setText("State: RUNNING JOB");
    //         m_pStateLabel->setStyleSheet("color: blue;");
    //         break;
    //     case MachineState::PAUSED:
    //         m_pStateLabel->setText("State: PAUSED");
    //         m_pStateLabel->setStyleSheet("color: orange;");
    //         break;
    //     case MachineState::ERROR_STATE:
    //         m_pStateLabel->setText("State: ERROR");
    //         m_pStateLabel->setStyleSheet("color: red;");
    //         break;
    // }

    // // Dynamic Button Toggling
    // m_pHomeBtn->setEnabled(currentState == MachineState::IDLE || currentState == MachineState::ERROR_STATE);
    // m_pCalibrateVisionBtn->setEnabled(currentState == MachineState::IDLE);
    // m_pStartJobBtn->setEnabled(currentState == MachineState::IDLE);
    // m_pPauseBtn->setEnabled(currentState == MachineState::RUNNING_JOB || currentState == MachineState::PAUSED);
    // m_pPauseBtn->setText(currentState == MachineState::PAUSED ? "Resume" : "Pause");

    // 2. Update Camera Feed
    if (!m_pGantryCam || m_pGantryCam->GetThreadState() != Thread<void>::ThreadState::eRunning)
        return;

    if (!m_frameReadyFuture.valid())
    {
        m_frameReadyFuture = m_pGantryCam->RequestFrameCopy(m_currentFrame);
        return;
    }

    if (m_frameReadyFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
    {
        try
        {
            if (m_frameReadyFuture.get() && !m_currentFrame.empty())
            {
                cv::Mat displayFrame = m_currentFrame.clone();

                // Run Vision Overlay based on selected mode
                if (m_eVisionMode == VisionMode::Fiducial)
                {
                    auto fiducials = FiducialDetector::DetectFiducials(displayFrame);
                    for (const auto& pt : fiducials)
                    {
                        cv::circle(displayFrame, pt, 15, cv::Scalar(0, 255, 0), 2);
                        cv::drawMarker(displayFrame, pt, cv::Scalar(0, 0, 255), cv::MARKER_CROSS, 20, 2);
                    }
                }
                else if (m_eVisionMode == VisionMode::Component)
                {
                    auto pose = ComponentDetector::DetectComponentPose(displayFrame);
                    if (pose.bFound)
                    {
                        cv::Point2f rect_points[4];
                        pose.cvBoundingBox.points(rect_points);
                        for (int j = 0; j < 4; j++)
                            cv::line(displayFrame, rect_points[j], rect_points[(j + 1) % 4], cv::Scalar(255, 0, 0), 2);
                        cv::putText(displayFrame,
                                    "Angle: " + std::to_string(pose.dRotationDegrees),
                                    cv::Point(20, 40),
                                    cv::FONT_HERSHEY_SIMPLEX,
                                    0.8,
                                    cv::Scalar(0, 255, 255),
                                    2);
                    }
                }
                else if (m_eVisionMode == VisionMode::Homing)
                {
                    CameraConfig mockConfig;
                    mockConfig.cvK = (cv::Mat_<double>(3, 3) << 800, 0, 320, 0, 800, 240, 0, 0, 1);
                    mockConfig.cvD = cv::Mat::zeros(4, 1, CV_64F);
                    VisualHoming::FindHomeMarker(displayFrame, mockConfig, 50.0f, 0, cv::aruco::DICT_4X4_50, &displayFrame);
                }

                // Push to UI
                cv::Mat rgbFrame;
                cv::cvtColor(displayFrame, rgbFrame, cv::COLOR_BGR2RGB);
                QImage qimg((uchar*) rgbFrame.data, rgbFrame.cols, rgbFrame.rows, rgbFrame.step, QImage::Format_RGB888);
                m_pCameraDisplayLabel->setPixmap(QPixmap::fromImage(qimg.copy()));
            }
        }
        catch (...)
        {}
        m_frameReadyFuture = m_pGantryCam->RequestFrameCopy(m_currentFrame);
    }
}

void operatorPage::onGCodeSendButtonClicked()
{
    qDebug("GCode Send Button Clicked");
    QString gcode = m_pGCodeEntryTextBox->toPlainText();
    qDebug() << "GCode to send:" << gcode;

    // if (this->m_pPnPRunner_instance == nullptr)
    // {
    //     qDebug() << "PnPRunner not instantiated.";
    //     QMessageBox msgBox;
    //     msgBox.setText("Machine not connected.");
    //     msgBox.exec();
    // }
    // else
    // {
    //     if (this->m_pPnPRunner_instance->GetCurrentState() == MachineState::IDLE)
    //     {
    //         qDebug() << "PnP is currently IDLE. Sending GCode command.";
    //         this->m_pPnPRunner_instance->sendGCode(gcode.toStdString());
    //     }
    //     else
    //     {
    //         qDebug() << "PnP is currently not IDLE. Current state:" << QString::fromStdString(m_pPnPRunner_instance->GetCurrentStateString())
    //                  << ". GCode command cannot be sent.";
    //     }
    // }
}
