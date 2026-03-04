#include "mainwindow.h"

#include <regex.h>
#include <QDebug>
#include <QTextEdit>
#include <QPlainTextEdit>

#include "Logging.h"
#include "jobsPage.h"
#include "operatorPage.h"
#include "customMenuBar.h"
#include "sideBar.h"
#include "settingsPage.h"
#include "pnpRunner.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    qDebug() << "Initializing MainWindow...";

    // Initialize the shared runner instance before passing it into UI modules
    m_pPnPRunner_instance = std::make_shared<PnPRunner>("/dev/ttyUSB0"); // Default COM port; update as needed

    QGridLayout *mainLayout = new QGridLayout();

    customMenuBar *menuBar_instance = new customMenuBar(this);
    this->setMenuBar(menuBar_instance);

    sideBar *sideBar_instance = new sideBar(m_pPnPRunner_instance, this);
    jobsPage *jobsPage_instance = new jobsPage(this);
    operatorPage *operatorPage_instance = new operatorPage(m_pPnPRunner_instance, this);
    settingsPage *settingsPage_instance = new settingsPage(m_pPnPRunner_instance, this);

    QTabWidget *tabs = new QTabWidget();
    tabs->addTab(jobsPage_instance, "Jobs");
    tabs->addTab(operatorPage_instance, "Operation");
    tabs->addTab(settingsPage_instance, "Settings");

    mainLayout->addWidget(sideBar_instance, 0, 0);
    mainLayout->addWidget(tabs, 0, 1);

    // Set the central widget of MainWindow
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    connect(tabs, &QTabWidget::tabBarClicked, this, &MainWindow::onTabBarClicked);

    qDebug() << "MainWindow initialized.";
}

MainWindow::~MainWindow()
{
    qDebug() << "Destroying MainWindow...";
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