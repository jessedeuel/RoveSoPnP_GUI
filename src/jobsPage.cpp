#include "jobsPage.h"

#include "board.hpp"

jobsPage::jobsPage(QWidget* parent) : QWidget(parent)
{
    m_pJobsPageLayout = new QGridLayout();
    this->setLayout(m_pJobsPageLayout);

    // TODO: Add components to table through PnP->Components.getPlacementMap() and update table when new job is uploaded.
    m_pJobsTable = new QTableWidget(10, 4);
    QStringList horzHeaders;
    horzHeaders << "Reference" << "Value" << "Cut Tape ID" << "Package";
    m_pJobsTable->setHorizontalHeaderLabels(horzHeaders);

    m_pUploadJobButton    = new QPushButton("Upload Job", this);
    m_pCurrentJobTextEdit = new QTextEdit("", this);

    QFontMetrics fm       = QFontMetrics(m_pCurrentJobTextEdit->font());
    int line_height       = fm.lineSpacing();
    int margins           = m_pCurrentJobTextEdit->document()->documentMargin() + m_pCurrentJobTextEdit->frameWidth() * 2;
    m_pCurrentJobTextEdit->setMaximumHeight(line_height + (int) margins * 2);

    m_pJobsPageLayout->addWidget(m_pJobsTable, 0, 0, 1, 2);
    m_pJobsPageLayout->addWidget(m_pCurrentJobTextEdit, 1, 0);
    m_pJobsPageLayout->addWidget(m_pUploadJobButton, 1, 1);

    connect(m_pUploadJobButton, &QPushButton::clicked, this, &jobsPage::onUploadJobButtonClicked);

    qDebug() << "jobsPage initialized.";
}

jobsPage::~jobsPage()
{
    qDebug() << "jobsPage destroyed.";
}

void jobsPage::onUploadJobButtonClicked()
{
    qDebug("Upload Job Button clicked");
    const QStringList filters({"PnP Files (*.csv *.xlsx *.ods)", "All Files (*)"});
    QFileDialog dialog(this);
    dialog.setNameFilters(filters);
    dialog.setDirectory("./");
    connect(&dialog,
            &QFileDialog::fileSelected,
            this,
            [&](const QString& filePath) mutable
            {
                qDebug() << "Selected file: " << filePath;
                m_pCurrentJobTextEdit->setPlainText(filePath);
            });

    dialog.exec();

    m_sJobFilePath = m_pCurrentJobTextEdit->toPlainText().toStdString();
    qDebug() << "Job File Path: |" << m_sJobFilePath.c_str() << "|";

    // TODO: Update to go through pnp class when components class is integrated
    Components components(m_sJobFilePath.c_str());

    map<tuple<string, cuttape_t>, vector<component_t>> component_map = components.getPlacementMap();

    for (const auto& pair : component_map)
    {
        auto test3 = pair.second.begin();

        qDebug() << test3->ref << ", " << test3->value << ", " << std::get<1>(pair.first).ID << ", " << test3->package;
    }
}
