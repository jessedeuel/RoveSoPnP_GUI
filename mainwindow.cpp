#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QMenuBar *menuBar = new QMenuBar();
    menuBar->addMenu("Files");
    menuBar->addMenu("Edit");

    QGridLayout *sideBarLayout = new QGridLayout();
    QPushButton *button = new QPushButton("Click Me", this);
    sideBarLayout->addWidget(button);
    QLabel *label = new QLabel("Hello, Qt!", this);
    label->setObjectName("label");
    sideBarLayout->addWidget(label);

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

    connect(button, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
}

void MainWindow::onButtonClicked()
{
    findChild<QLabel*>("label")->setText("Button Clicked!");
}

MainWindow::~MainWindow()
{
    delete ui;
}
