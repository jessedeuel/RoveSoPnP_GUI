#include <operatorPage.h>

operatorPage::operatorPage(QWidget *parent)
{
    m_pOperatorPageLayout = new QGridLayout();
    this->setLayout(m_pOperatorPageLayout);
    m_pRunJobButton = new QPushButton("Run Job", this);
    m_pOperatorPageLayout->addWidget(m_pRunJobButton);

    // Camera Display Setup
    m_pCameraDisplayLabel = new QLabel("Camera Feed Loading...", this);
    m_pCameraDisplayLabel->setObjectName("cameraDisplayLabel");
    m_pCameraDisplayLabel->setMinimumSize(640, 480);
    m_pCameraDisplayLabel->setAlignment(Qt::AlignCenter);
    m_pCameraDisplayLabel->setStyleSheet("border: 1px solid black; background-color: #333; color: white;");
    m_pOperatorPageLayout->addWidget(m_pCameraDisplayLabel);

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
    connect(m_pCameraTimer, &QTimer::timeout, this, &operatorPage::updateCameraDisplay);
    m_pCameraTimer->start(33);

    m_pGCodeEntryTextBox = new QTextEdit("Enter GCode", this);
    m_pGCodeSendButton = new QPushButton("Send GCode", this);
    m_pOperatorPageLayout->addWidget(m_pGCodeEntryTextBox);
    m_pOperatorPageLayout->addWidget(m_pGCodeSendButton);

    connect(m_pGCodeSendButton, &QPushButton::clicked, this, &operatorPage::onGCodeSendButtonClicked);
}

operatorPage::~operatorPage()
{
    qDebug() << "Destroying operatorPage...";

    // Stop camera and thread pool safely before destruction
    if (m_pGantryCam)
    {
        qDebug() << "Stopping Camera...";
        m_pGantryCam->RequestStop();
        m_pGantryCam->Join();
    }
}

void operatorPage::onRunJobButtonClicked()
{
    qDebug("Run Job Button clicked");
}

void operatorPage::updateCameraDisplay()
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

void operatorPage::onGCodeSendButtonClicked()
{
    qDebug("GCode Send Button Clicked");

}