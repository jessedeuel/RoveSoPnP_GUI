#include <QMessageBox>

#include <operatorPage.h>

operatorPage::operatorPage(std::shared_ptr<PnPRunner> pPnPRunner_instance, QWidget *parent) : m_eVisionMode(VisionMode::None)
{
    this->m_pPnPRunner_instance = std::move(pPnPRunner_instance);

    m_pOperatorPageLayout = new QGridLayout();
    this->setLayout(m_pOperatorPageLayout);
    m_pRunJobButton = new QPushButton("Run Job", this);
    m_pOperatorPageLayout->addWidget(m_pRunJobButton, 0, 0, 1, 2);

    // --- Vision Pipeline Controls Setup ---
    m_pVisionGroupBox = new QGroupBox("Live Vision Pipelines", this);
    QHBoxLayout *visionLayout = new QHBoxLayout();

    m_pModeNoneBtn = new QPushButton("Raw Feed", this);
    m_pModeFiducialBtn = new QPushButton("Detect Fiducials", this);
    m_pModeComponentBtn = new QPushButton("Component Align", this);
    m_pModeHomingBtn = new QPushButton("Visual Homing", this);

    visionLayout->addWidget(m_pModeNoneBtn);
    visionLayout->addWidget(m_pModeFiducialBtn);
    visionLayout->addWidget(m_pModeComponentBtn);
    visionLayout->addWidget(m_pModeHomingBtn);
    m_pVisionGroupBox->setLayout(visionLayout);

    m_pOperatorPageLayout->addWidget(m_pVisionGroupBox, 1, 0, 1, 2);

    // Connect Vision Buttons
    connect(m_pModeNoneBtn, &QPushButton::clicked, this, &operatorPage::setVisionModeNone);
    connect(m_pModeFiducialBtn, &QPushButton::clicked, this, &operatorPage::setVisionModeFiducial);
    connect(m_pModeComponentBtn, &QPushButton::clicked, this, &operatorPage::setVisionModeComponent);
    connect(m_pModeHomingBtn, &QPushButton::clicked, this, &operatorPage::setVisionModeHoming);

    // --- Camera Display Setup ---
    m_pCameraDisplayLabel = new QLabel("Camera Feed Loading...", this);
    m_pCameraDisplayLabel->setObjectName("cameraDisplayLabel");
    m_pCameraDisplayLabel->setMinimumSize(640, 480);
    m_pCameraDisplayLabel->setAlignment(Qt::AlignCenter);
    m_pCameraDisplayLabel->setStyleSheet("border: 1px solid black; background-color: #333; color: white;");
    m_pOperatorPageLayout->addWidget(m_pCameraDisplayLabel, 2, 0, 1, 2);

    // --- GCode Entry Setup ---
    m_pGCodeEntryTextBox = new QTextEdit("Enter GCode", this);
    m_pGCodeEntryTextBox->setMaximumHeight(50);
    m_pGCodeSendButton = new QPushButton("Send GCode", this);
    m_pOperatorPageLayout->addWidget(m_pGCodeEntryTextBox, 3, 0);
    m_pOperatorPageLayout->addWidget(m_pGCodeSendButton, 3, 1);
    connect(m_pGCodeSendButton, &QPushButton::clicked, this, &operatorPage::onGCodeSendButtonClicked);

    // --- Camera Initialization ---
    qDebug() << "Attempting to open camera /dev/video0...";
    try
    {
        // Initialize BasicCam (device 0, 640x480, 30FPS)
        m_pGantryCam = std::make_unique<BasicCam>("/dev/video0", 640, 480, 30, PIXEL_FORMATS::eBGR, 90.0, 90.0, false, 1);
        qDebug() << "Camera opened successfully. Starting camera thread...";
        m_pGantryCam->Start();
        qDebug() << "Camera started successfully.";
    }
    catch (const std::exception &e)
    {
        qDebug() << "CRITICAL: Camera initialization failed:" << e.what();
        m_pCameraDisplayLabel->setText("Camera Error: Initialization Failed");
    }

    // Start Timer to poll frames at ~30 FPS (33ms)
    m_pCameraTimer = new QTimer(this);
    connect(m_pCameraTimer, &QTimer::timeout, this, &operatorPage::updateCameraDisplay);
    m_pCameraTimer->start(33);

    qDebug() << "operatorPage initialized.";
}

operatorPage::~operatorPage()
{
    qDebug() << "Destroying operatorPage...";
    if (m_pGantryCam)
    {
        m_pGantryCam->RequestStop();
        m_pGantryCam->Join();
    }
}

// Vision Toggles
void operatorPage::setVisionModeNone() { m_eVisionMode = VisionMode::None; }
void operatorPage::setVisionModeFiducial() { m_eVisionMode = VisionMode::Fiducial; }
void operatorPage::setVisionModeComponent() { m_eVisionMode = VisionMode::Component; }
void operatorPage::setVisionModeHoming() { m_eVisionMode = VisionMode::Homing; }

void operatorPage::onRunJobButtonClicked()
{
    qDebug("Run Job Button clicked");
}

void operatorPage::updateCameraDisplay()
{
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
            bool success = m_frameReadyFuture.get();

            if (success && !m_currentFrame.empty())
            {
                // Create a working copy for drawing OpenCV overlays
                cv::Mat displayFrame = m_currentFrame.clone();

                // --- INTEGRATE VISION PIPELINES HERE ---
                if (m_eVisionMode == VisionMode::Fiducial)
                {
                    auto fiducials = FiducialDetector::DetectFiducials(displayFrame);
                    for (const auto &pt : fiducials)
                    {
                        cv::circle(displayFrame, pt, 15, cv::Scalar(0, 255, 0), 2);                       // Green Circle
                        cv::drawMarker(displayFrame, pt, cv::Scalar(0, 0, 255), cv::MARKER_CROSS, 20, 2); // Red Crosshair
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
                        {
                            cv::line(displayFrame, rect_points[j], rect_points[(j + 1) % 4], cv::Scalar(255, 0, 0), 2); // Blue Box
                        }
                        std::string text = "Angle: " + std::to_string(pose.dRotationDegrees);
                        cv::putText(displayFrame, text, cv::Point(20, 40), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 255), 2);
                    }
                }
                else if (m_eVisionMode == VisionMode::Homing)
                {
                    // For homing, you need a camera config. We mock a pinhole config here for live UI testing.
                    CameraConfig mockConfig;
                    mockConfig.cvK = (cv::Mat_<double>(3, 3) << 800, 0, 320, 0, 800, 240, 0, 0, 1);
                    mockConfig.cvD = cv::Mat::zeros(4, 1, CV_64F);

                    // Pass displayFrame pointer to get debug drawing of axes automatically
                    VisualHoming::FindHomeMarker(displayFrame, mockConfig, 50.0f, 0, cv::aruco::DICT_4X4_50, &displayFrame);
                }

                // Convert processed BGR OpenCV Mat to RGB Qt Image
                cv::Mat rgbFrame;
                cv::cvtColor(displayFrame, rgbFrame, cv::COLOR_BGR2RGB);

                if (rgbFrame.cols > 0 && rgbFrame.rows > 0)
                {
                    QImage qimg((uchar *)rgbFrame.data, rgbFrame.cols, rgbFrame.rows, rgbFrame.step, QImage::Format_RGB888);
                    m_pCameraDisplayLabel->setPixmap(QPixmap::fromImage(qimg.copy()));
                }
            }
        }
        catch (cv::Exception &e)
        {
            qDebug() << "OpenCV Error: " << e.what();
        }
        catch (...)
        {
            qDebug() << "Unknown Error in updateCameraDisplay";
        }

        // Request the NEXT frame immediately
        m_frameReadyFuture = m_pGantryCam->RequestFrameCopy(m_currentFrame);
    }
}

void operatorPage::onGCodeSendButtonClicked()
{
    qDebug("GCode Send Button Clicked");
    QString gcode = m_pGCodeEntryTextBox->toPlainText();
    qDebug() << "GCode to send:" << gcode;

    if (this->m_pPnPRunner_instance == nullptr)
    {
        qDebug() << "PnPRunner not instantiated.";
        QMessageBox msgBox;
        msgBox.setText("Machine not connected.");
        msgBox.exec();
    }
    else
    {
        if (this->m_pPnPRunner_instance->getState() == "IDLE")
        {
            qDebug() << "PnP is currently IDLE. Sending GCode command.";
            this->m_pPnPRunner_instance->sendGCode(gcode.toStdString());
        }
        else
        {
            qDebug() << "PnP is currently not IDLE. Current state:" << QString::fromStdString(m_pPnPRunner_instance->getState()) << ". GCode command cannot be sent.";
        }
    }
}