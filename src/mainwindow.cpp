#include "mainwindow.h"
#include <regex.h>
#include <QDebug>
#include <QTextEdit>
#include <QPlainTextEdit>

#include "jobsPage.h"
#include "operatorPage.h"

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

    jobsPage* jobsPage_instance = new jobsPage(this);
    operatorPage* operatorPage_instance = new operatorPage();

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
    tabs->addTab(operatorPage_instance, "Operation");
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

    qDebug() << "MainWindow initialized.";
}

MainWindow::~MainWindow()
{
    qDebug() << "Destroying MainWindow...";
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