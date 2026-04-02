#include "settingsPage.h"
#include <QDebug>
#include <array>

settingsPage::settingsPage(std::shared_ptr<GRBL> GRBL_instance, QWidget* parent) : QWidget(parent)
{
    m_pSettingsPageLayout = new QGridLayout();
    this->setLayout(m_pSettingsPageLayout);

    m_pGRBL_instance       = GRBL_instance;

    m_pComPortSelectionBox = new QComboBox();
    m_pRefreshPortsButton  = new QPushButton("Refresh");    // Initialize refresh button

    m_lPorts               = listPorts();
    m_pComPortSelectionBox->addItems(m_lPorts);

    m_pComPortConnectButton = new QPushButton("Connect");

    // Adjust grid layout to fit the new button
    m_pSettingsPageLayout->addWidget(m_pComPortSelectionBox, 0, 0);
    m_pSettingsPageLayout->addWidget(m_pRefreshPortsButton, 0, 1);      // Inserted into column 1
    m_pSettingsPageLayout->addWidget(m_pComPortConnectButton, 0, 2);    // Shifted to column 2

    connect(m_pComPortConnectButton, &QPushButton::clicked, this, &settingsPage::onComPortSetButtonClicked);
    connect(m_pRefreshPortsButton, &QPushButton::clicked, this, &settingsPage::onRefreshPortsButtonClicked);    // Connect slot

    qDebug() << "settingsPage initialized.";
}

settingsPage::~settingsPage()
{
    qDebug() << "settingsPage destroyed.";
}

// Rewritten to be completely Cross-Platform
QList<QString> settingsPage::listPorts()
{
    QList<QString> result;

    // QSerialPortInfo works automatically across Windows, Linux, and macOS
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& portInfo : serialPortInfos)
    {
        // systemLocation() returns the full path (e.g., "/dev/ttyACM0" or "COM3")
        result.push_back(portInfo.systemLocation());
    }

    return result;
}

int settingsPage::connectPnPMachine(QString comPort, QString csvFile)
{
    return 0;
}

void settingsPage::onComPortSetButtonClicked()
{
    QString comPortSelectionBoxText = m_pComPortSelectionBox->currentText().trimmed();
    m_sComPort                      = comPortSelectionBoxText.toStdString();

    qDebug() << "Connect Port: |" << m_sComPort.c_str() << "|";

    m_pGRBL_instance->connect(m_sComPort.c_str());
}

// Logic to dynamically refresh the dropdown
void settingsPage::onRefreshPortsButtonClicked()
{
    m_pComPortSelectionBox->clear();
    m_lPorts = listPorts();
    m_pComPortSelectionBox->addItems(m_lPorts);
    qDebug() << "Available ports refreshed.";
}
