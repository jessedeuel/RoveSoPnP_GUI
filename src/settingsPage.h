#pragma once

#include <QComboBox>
#include <QDebug>
#include <QFileDialog>
#include <QFontMetrics>
#include <QGridLayout>
#include <QPushButton>
#include <QSerialPortInfo>
#include <QTextEdit>
#include <QWidget>

#include <memory.h>

#include "grbl.hpp"

class settingsPage : public QWidget
{
        Q_OBJECT

    public:
        settingsPage(std::shared_ptr<GRBL> GRBL_instance, QWidget* parent = nullptr);
        ~settingsPage();

        QList<QString> listPorts();
        int connectPnPMachine(QString comPort, QString csvFile);

    private:
        QGridLayout* m_pSettingsPageLayout;
        QComboBox* m_pComPortSelectionBox;
        QList<QString> m_lPorts;

        QPushButton* m_pRefreshPortsButton;
        QPushButton* m_pComPortConnectButton;

        std::shared_ptr<GRBL> m_pGRBL_instance;
        std::atomic<bool> m_bPnPThreadRunning;

        std::string m_sComPort;

    private slots:
        void onComPortSetButtonClicked();
        void onRefreshPortsButtonClicked();
};
