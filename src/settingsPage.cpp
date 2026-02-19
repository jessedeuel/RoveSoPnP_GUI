#include "settingsPage.h"

settingsPage::settingsPage(QWidget *parent) : QWidget(parent)
{
    m_pSettingsPageLayout = new QGridLayout();
    this->setLayout(m_pSettingsPageLayout);

    m_pComPortSelectionBox = new QComboBox();

    m_lPorts = listPorts();
    m_pComPortSelectionBox->addItems(m_lPorts);

    m_pComPortConnectButton = new QPushButton("Connect");

    m_pUploadJobButton = new QPushButton("Upload Job", this);
    m_pCurrentJobTextEdit = new QTextEdit("", this);
    
    QFontMetrics fm = QFontMetrics(m_pCurrentJobTextEdit->font());
    int line_height = fm.lineSpacing();
    int margins = m_pCurrentJobTextEdit->document()->documentMargin() + m_pCurrentJobTextEdit->frameWidth() * 2;
    m_pCurrentJobTextEdit->setMaximumHeight(line_height + (int)margins * 2);

    m_pSettingsPageLayout->addWidget(m_pComPortSelectionBox, 0, 0);
    m_pSettingsPageLayout->addWidget(m_pComPortConnectButton, 0, 1);
    m_pSettingsPageLayout->addWidget(m_pCurrentJobTextEdit, 1, 0);
    m_pSettingsPageLayout->addWidget(m_pUploadJobButton, 1, 1);

    connect(m_pComPortConnectButton, &QPushButton::clicked, this, &settingsPage::onComPortSetButtonClicked);
    connect(m_pUploadJobButton, &QPushButton::clicked, this, &settingsPage::onUploadJobButtonClicked);

    qDebug() << "settingsPage initialized.";
}

settingsPage::~settingsPage()
{
    qDebug() << "settingsPage destroyed.";
}

QList<QString> settingsPage::listPorts()
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
    // TODO: Implement Windows version of listPorts()
#elif __APPLE__
    qDebug("On MacOS");
    // TODO: Implement MacOS version of listPorts()
#endif

    return result;
}

int settingsPage::connectPnPMachine(QString comPort, QString csvFile)
{
    m_pPnPRunner = std::make_unique<PnPRunner>(comPort.toStdString(), csvFile.toStdString());

    return 0;
}

void settingsPage::onComPortSetButtonClicked()
{
    QString comPortSelectionBoxText = m_pComPortSelectionBox->currentText().trimmed();
    QByteArray array = comPortSelectionBoxText.toLocal8Bit();
    m_sComPort = comPortSelectionBoxText.toStdString();

    qDebug() << "Connect Port: |" << m_sComPort.c_str() << "|";
    /*
    if (m_PNPMachineComm.setupComm(array.constData()) == false)
    {
        m_pComPortConnectButton->setStyleSheet("background-color: red;");
    }
    else
    {
        m_pComPortConnectButton->setStyleSheet("background-color: green;");
    }
    */
}

void settingsPage::onUploadJobButtonClicked()
{
    qDebug("Upload Job Button clicked");
    const QStringList filters({"PnP Files (*.csv *.xlsx *.ods)", 
                                "All Files (*)"
                              });
    QFileDialog dialog(this);
    dialog.setNameFilters(filters);
    dialog.setDirectory("./");
    connect(&dialog, &QFileDialog::fileSelected, this, [&](const QString &filePath) mutable {
        qDebug() << "Selected file: " << filePath;
        m_pCurrentJobTextEdit->setPlainText(filePath);
    });

    dialog.exec();

    m_sJobFilePath = m_pCurrentJobTextEdit->toPlainText().toStdString();
    qDebug() << "Job File Path: |" << m_sJobFilePath.c_str() << "|";
}