#include "settingsPage.h"

settingsPage::settingsPage(QWidget *parent) : QWidget(parent)
{
    m_pSettingsPageLayout = new QGridLayout();
    this->setLayout(m_pSettingsPageLayout);

    m_pComPortSelectionBox = new QComboBox();

    m_lPorts = MainWindow::listPorts();
    m_pComPortSelectionBox->addItems(m_lPorts);

    m_pComPortConnectButton = new QPushButton("Connect");
    m_pSettingsPageLayout->addWidget(comPortSelectionBox, 0, 0);
    m_pSettingsPageLayout->addWidget(comPortConnectButton, 0, 1);

    connect(comPortConnectButton, &QPushButton::clicked, this, &MainWindow::onComPortSetButtonClicked);

    qDebug() << "settingsPage initialized.";
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

void settingsPage::onComPortSetButtonClicked()
{
    m_PNPMachine = Comm();
    QString comPortSelectionBoxText = comPortSelectionBox->currentText().trimmed();
    QByteArray array = comPortSelectionBoxText.toLocal8Bit();
    qDebug() << "Connect Port: |" << array.constData() << "|";
    if (m_PNPMachineComm.setupComm(array.constData()) == false)
    {
        m_pComPortConnectButton->setStyleSheet("background-color: red;");
    }
    else
    {
        m_pComPortConnectButton->setStyleSheet("background-color: green;");
    }
}