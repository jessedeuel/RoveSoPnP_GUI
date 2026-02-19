#include "mainwindow.h"

#include <regex.h>
#include <QDebug>
#include <QTextEdit>
#include <QPlainTextEdit>

#include "jobsPage.h"
#include "operatorPage.h"
#include "customMenuBar.h"
#include "sideBar.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    qDebug() << "Initializing MainWindow...";

    QGridLayout *mainLayout = new QGridLayout();

    customMenuBar* menuBar_instance = new customMenuBar(this);
    this->setMenuBar(menuBar_instance);
    
    sideBar* sideBar_instance = new sideBar(this);
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

    QTabWidget *tabs = new QTabWidget();
    tabs->addTab(jobsPage_instance, "Jobs");
    tabs->addTab(operatorPage_instance, "Operation");
    tabs->addTab(settingsPage, "Settings");

    //mainLayout->addWidget(menuBar, 0, 0, 1, 2);
    mainLayout->addWidget(sideBar_instance, 0, 0);
    mainLayout->addWidget(tabs, 0, 1);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

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