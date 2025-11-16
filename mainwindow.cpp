#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
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


    QWidget *jobsPage = new QWidget();
    QGridLayout *jobsPageLayout = new QGridLayout();
    jobsPage->setLayout(jobsPageLayout);
    QTableWidget *table = new QTableWidget(10, 10);
    jobsPageLayout->addWidget(table, 0, 0);
    QPushButton *uploadJobButton = new QPushButton("Upload Job", this);
    jobsPageLayout->addWidget(uploadJobButton, 1, 0);

    QWidget *operatorPage = new QWidget();
    QGridLayout *operatorPageLayout = new QGridLayout();
    operatorPage->setLayout(operatorPageLayout);
    QPushButton *runJobButton = new QPushButton("Run Job", this);
    
    operatorPageLayout->addWidget(runJobButton);

    QGridLayout *mainLayout = new QGridLayout();
    QTabWidget *tabs = new QTabWidget();
    tabs->addTab(jobsPage, "Jobs");
    tabs->addTab(operatorPage, "Operation");

    mainLayout->addWidget(menuBar, 0, 0, 1, 2);
    mainLayout->addLayout(sideBarLayout, 1, 0);
    mainLayout->addWidget(tabs, 1, 1);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    qDebug() << "Test";
    //camera->start();

    connect(pauseButton, &QPushButton::clicked, this, &MainWindow::onPauseButtonClicked);
    connect(endProgramButton, &QPushButton::clicked, this, &MainWindow::onEndProgramButtonClicked);
}

void MainWindow::onPauseButtonClicked()
{
    //findChild<QLabel*>("label")->setText("Button Clicked!");
}

void MainWindow::onEndProgramButtonClicked()
{
    // If job not running set to green
    QPushButton* endProgramButton = findChild<QPushButton*>("endProgramButton");
    qDebug() << "Test";
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

MainWindow::~MainWindow()
{
    delete ui;
}
