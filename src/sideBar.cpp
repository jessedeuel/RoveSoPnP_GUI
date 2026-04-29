#include "sideBar.h"
#include "flowControl.h"

sideBar::sideBar(QWidget* parent) : QWidget(parent)
{
    m_pSideBarLayout = new QGridLayout();
    this->setLayout(m_pSideBarLayout);

    m_pStateLabel = new QLabel("State: Offline", this);
    m_pStateLabel->setStyleSheet("font-weight: bold; font-size: 14px;");

    m_pConnectionStatusLabel = new QLabel("Connection Status: Unknown", this);
    m_pPositionLabel         = new QLabel("Position: X: 0.00 Y: 0.00 Z: 0.00", this);
    m_pCurrentComponentLabel = new QLabel("Current Component: N/A", this);
    m_pCurrentJobLabel       = new QLabel("Current Job: N/A", this);
    m_pPauseButton           = new QPushButton("Pause", this);
    m_pStartEndProgramButton = new QPushButton("Start Program", this);
    m_pStartEndProgramButton->setStyleSheet("background-color: green; color: white; font-weight: bold;");

    m_pSideBarLayout->addWidget(m_pStateLabel);
    m_pSideBarLayout->addWidget(m_pConnectionStatusLabel);
    m_pSideBarLayout->addWidget(m_pPositionLabel);
    m_pSideBarLayout->addWidget(m_pCurrentComponentLabel);
    m_pSideBarLayout->addWidget(m_pCurrentJobLabel);
    m_pSideBarLayout->addWidget(m_pPauseButton);
    m_pSideBarLayout->addWidget(m_pStartEndProgramButton);

    connect(m_pPauseButton, &QPushButton::clicked, this, &sideBar::onPauseButtonClicked);
    connect(m_pStartEndProgramButton, &QPushButton::clicked, this, &sideBar::onStartEndProgramButtonClicked);

    qDebug() << "sideBar initialized.";
}

sideBar::~sideBar()
{
    qDebug() << "Destroying sideBar...";
}

void sideBar::bindFlowControl(FlowControl* fc)
{
    if (!fc)
        return;
    m_fc = fc;

    // Connect the live UI update signals
    connect(m_fc, &FlowControl::stateChanged, this, &sideBar::updateState);
    connect(m_fc, &FlowControl::positionUpdated, this, &sideBar::updatePosition);

    // Give it an initial push so the text doesn't say offline at boot
    updateState(m_fc->getStateString());
}

void sideBar::updateState(const QString& stateStr)
{
    m_pStateLabel->setText("State: " + stateStr);
}

void sideBar::updatePosition(float x, float y, float z)
{
    // Format to 2 decimal places to keep the UI clean
    m_pPositionLabel->setText(QString("Position: X: %1  Y: %2  Z: %3").arg(x, 0, 'f', 2).arg(y, 0, 'f', 2).arg(z, 0, 'f', 2));
}

void sideBar::onPauseButtonClicked()
{
    if (m_pPauseButton->text() == "Pause")
    {
        m_pPauseButton->setText("Resume");
        if (m_fc)
            m_fc->pauseJob();
    }
    else
    {
        m_pPauseButton->setText("Pause");
        if (m_fc)
            m_fc->resumeJob();
    }
}

void sideBar::onStartEndProgramButtonClicked()
{
    qDebug() << "EndProgramButton Clicked";
    if (m_pStartEndProgramButton->styleSheet().contains("background-color: red;"))
    {
        // Program is currently running, so stop it
        if (m_fc)
            m_fc->stopJob();

        m_pStartEndProgramButton->setStyleSheet("background-color: green; color: white; font-weight: bold;");
        m_pStartEndProgramButton->setText("Start Program");
    }
    else
    {
        // No program is currently running, so start it
        if (m_fc)
            m_fc->startJob();

        m_pStartEndProgramButton->setStyleSheet("background-color: red; color: white; font-weight: bold;");
        m_pStartEndProgramButton->setText("End Program");
    }
}
