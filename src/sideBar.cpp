#include "sideBar.h"

sideBar::sideBar(std::shared_ptr<PnPRunner> pPnPRunner_instance, QWidget *parent) : QWidget(parent)
{
    this->m_pPnPRunner_instance = pPnPRunner_instance;

    m_pSideBarLayout = new QGridLayout();
    this->setLayout(m_pSideBarLayout);
    
    m_pStateLabel = new QLabel("State: ", this);
    m_pConnectionStatusLabel = new QLabel("Connection Status: ", this);
    m_pPositionLabel = new QLabel("Position: ", this);
    m_pCurrentComponentLabel = new QLabel("Current Component: ", this);
    m_pCurrentJobLabel = new QLabel("Current Job: ", this);
    m_pPauseButton = new QPushButton("Pause", this);
    m_pStartEndProgramButton = new QPushButton("Start Program", this);
    m_pStartEndProgramButton->setStyleSheet("background-color: green;");

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

void sideBar::onPauseButtonClicked()
{
    // findChild<QLabel*("label")->setText("Button Clicked!");
}

void sideBar::onStartEndProgramButtonClicked()
{
    qDebug() << "EndProgramButton Clicked";
    if (m_pStartEndProgramButton->styleSheet().contains("background-color: red;"))
    {
        // This means program is currently running, so we want to stop it, then change the button to green and text to "Start Program"
        //m_pPnPRunner_instance->RequestStop();

        m_pStartEndProgramButton->setStyleSheet("background-color: green;");
        m_pStartEndProgramButton->setText("Start Program");
    }
    else
    {
        // This means that no program is currently running, so we want to start it, then change the button to red and text to "End Program"
        //m_pPnPRunner_instance->Start();

        m_pStartEndProgramButton->setStyleSheet("background-color: red;");
        m_pStartEndProgramButton->setText("End Program");
    }
}