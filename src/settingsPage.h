#pragma once

#include <QComboBox>
#include <QDebug>
#include <QFileDialog>
#include <QFontMetrics>
#include <QGridLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

#include <memory.h>

class settingsPage : public QWidget
{
        Q_OBJECT

    public:
        // std::shared_ptr<PnPRunner> pPnPRunner_instance
        settingsPage(QWidget* parent = nullptr);
        ~settingsPage();

        QList<QString> listPorts();

        // PnP* getPnPMachine();
        int connectPnPMachine(QString comPort, QString csvFile);

    private:
        QGridLayout* m_pSettingsPageLayout;
        QComboBox* m_pComPortSelectionBox;
        QList<QString> m_lPorts;
        QPushButton* m_pComPortConnectButton;

        // std::unique_ptr<PnPRunner> m_pPnPRunner_instance;
        std::atomic<bool> m_bPnPThreadRunning;

        std::string m_sComPort;

        // TODO: Add thread for PnP ticking
        // void PnPThreadHandler();

    private slots:
        void onComPortSetButtonClicked();
};
