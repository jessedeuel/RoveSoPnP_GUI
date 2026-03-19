#include "settingsPage.h"

// std::shared_ptr<PnPRunner> pPnPRunner_instance
settingsPage::settingsPage(QWidget* parent) : QWidget(parent)
{
    m_pSettingsPageLayout = new QGridLayout();
    this->setLayout(m_pSettingsPageLayout);

    m_pComPortSelectionBox = new QComboBox();

    m_lPorts               = listPorts();
    m_pComPortSelectionBox->addItems(m_lPorts);

    m_pComPortConnectButton = new QPushButton("Connect");

    m_pSettingsPageLayout->addWidget(m_pComPortSelectionBox, 0, 0);
    m_pSettingsPageLayout->addWidget(m_pComPortConnectButton, 0, 1);

    connect(m_pComPortConnectButton, &QPushButton::clicked, this, &settingsPage::onComPortSetButtonClicked);

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
    FILE* pipe = popen("ls /dev/ttyA*", "r");    // Open pipe for reading
    if (!pipe)
    {
        qDebug("Error: popen() failed!");
    }
    else
    {
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
        {
            result.push_back(((QString) buffer.data()).trimmed());
        }
    }
    if (pipe)
        pclose(pipe);    // Close the pipe
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
    return 0;
}

void settingsPage::onComPortSetButtonClicked()
{
    QString comPortSelectionBoxText = m_pComPortSelectionBox->currentText().trimmed();
    QByteArray array                = comPortSelectionBoxText.toLocal8Bit();
    m_sComPort                      = comPortSelectionBoxText.toStdString();

    qDebug() << "Connect Port: |" << m_sComPort.c_str() << "|";

    // m_pPnPRunner_instance = std::make_unique<PnPRunner>(m_sComPort.c_str());

    // if (m_pPnPRunner_instance->getPnPMachine()->getState() == IDLE)
    // {
    //     m_pComPortConnectButton->setStyleSheet("background-color: green;");
    //     m_pPnPRunner_instance->Start();
    // }
    // else
    // {
    //     m_pComPortConnectButton->setStyleSheet("background-color: red;");
    //     m_pPnPRunner_instance->RequestStop();
    // }
}
