#include "jobsPage.h"

jobsPage::jobsPage(QWidget *parent) : QWidget(parent)
{
    m_jobsPageLayout = new QGridLayout();
    this->setLayout(m_jobsPageLayout);

    m_jobsTable = new QTableWidget(10, 10);
    m_jobsPageLayout->addWidget(m_jobsTable, 0, 0, 1, 2);
    m_uploadJobButton = new QPushButton("Upload Job", this);
    m_currentJobTextEdit = new QTextEdit("", this);
    
    QFontMetrics fm = QFontMetrics(m_currentJobTextEdit->font());
    int line_height = fm.lineSpacing();
    int margins = m_currentJobTextEdit->document()->documentMargin() + m_currentJobTextEdit->frameWidth() * 2;
    m_currentJobTextEdit->setMaximumHeight(line_height + (int)margins * 2);
    
    //currentJobTextEdit->setMaximumBlockCount(1);
    m_jobsPageLayout->addWidget(m_currentJobTextEdit, 1, 0);
    m_jobsPageLayout->addWidget(m_uploadJobButton, 1, 1);

    connect(m_uploadJobButton, &QPushButton::clicked, this, &jobsPage::onUploadJobButtonClicked);

    qDebug() << "jobsPage initialized.";
}

jobsPage::~jobsPage()
{
    qDebug() << "jobsPage destroyed.";
}

void jobsPage::onUploadJobButtonClicked()
{
    qDebug("Upload Job Button clicked");
    const QStringList filters({"PnP Files (*.csv *.xlsx *.ods)", 
                                "All Files (*)"
                              });
    QFileDialog dialog(this);
    dialog.setNameFilters(filters);
    dialog.setDirectory("./");
    //QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "./", );
    connect(&dialog, &QFileDialog::fileSelected, this, [&](const QString &filePath) mutable {
        qDebug() << "Selected file: " << filePath;
        m_currentJobTextEdit->setPlainText(filePath);
    });

    dialog.exec();
}