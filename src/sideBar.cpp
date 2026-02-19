#include "sideBar.h"

sideBar::sideBar(QWidget *parent) : QWidget(parent)
{
    m_pSideBarLayout = new QGridLayout();
    this->setLayout(m_pSideBarLayout);
    
    m_pStateLabel = new QLabel("State: ", this);
    m_pConnectionStatusLabel = new QLabel("Connection Status: ", this);
    m_pPositionLabel = new QLabel("Position: ", this);
    m_pCurrentComponentLabel = new QLabel("Current Component: ", this);
    m_pCurrentJobLabel = new QLabel("Current Job: ", this);
    m_pPauseButton = new QPushButton("Pause", this);
    m_pEndProgramButton = new QPushButton("Start Program", this);
    m_pEndProgramButton->setStyleSheet("background-color: green;");

    m_pSideBarLayout->addWidget(m_pStateLabel);
    m_pSideBarLayout->addWidget(m_pConnectionStatusLabel);
    m_pSideBarLayout->addWidget(m_pPositionLabel);
    m_pSideBarLayout->addWidget(m_pCurrentComponentLabel);
    m_pSideBarLayout->addWidget(m_pCurrentJobLabel);
    m_pSideBarLayout->addWidget(m_pPauseButton);
    m_pSideBarLayout->addWidget(m_pEndProgramButton);

    connect(m_pPauseButton, &QPushButton::clicked, this, &sideBar::onPauseButtonClicked);
    connect(m_pEndProgramButton, &QPushButton::clicked, this, &sideBar::onEndProgramButtonClicked);

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

void sideBar::onEndProgramButtonClicked()
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