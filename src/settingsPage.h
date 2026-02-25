#pragma once

#include <QWidget>
#include <QDebug>
#include <QGridLayout>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QFileDialog>
#include <QFontMetrics>

#include <memory.h>

#include "pnpRunner.h"

class settingsPage : public QWidget
{
    Q_OBJECT

public:
    settingsPage(std::shared_ptr<PnPRunner> pPnPRunner_instance, QWidget *parent = nullptr);
    ~settingsPage();
    
    QList<QString> listPorts();
    
    PnP* getPnPMachine();
    int connectPnPMachine(QString comPort, QString csvFile);

private:
    QGridLayout* m_pSettingsPageLayout;
    QComboBox* m_pComPortSelectionBox;
    QList<QString> m_lPorts;
    QPushButton* m_pComPortConnectButton;
    QPushButton* m_pUploadJobButton;
    QTextEdit* m_pCurrentJobTextEdit;

    std::unique_ptr<PnPRunner> m_pPnPRunner_instance;
    std::atomic<bool> m_bPnPThreadRunning;

    std::string m_sJobFilePath;
    std::string m_sComPort;

    // TODO: Add thread for PnP ticking
    // void PnPThreadHandler();

private slots:
    void onComPortSetButtonClicked();
    void onUploadJobButtonClicked();
};