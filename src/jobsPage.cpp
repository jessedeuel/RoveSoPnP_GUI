#include "jobsPage.h"

jobsPage::jobsPage(QWidget *parent) : QWidget(parent)
{
    m_jobsPageLayout = new QGridLayout();
    this->setLayout(m_jobsPageLayout);

    // TODO: Add components to table through PnP->Components.getPlacementMap() and update table when new job is uploaded.
    m_jobsTable = new QTableWidget(10, 10);
    m_jobsPageLayout->addWidget(m_jobsTable, 0, 0, 1, 2);

    qDebug() << "jobsPage initialized.";
}

jobsPage::~jobsPage()
{
    qDebug() << "jobsPage destroyed.";
}