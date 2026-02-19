#pragma once

#include <QWidget>
#include <QDebug>

class settingsPage : public QWidget
{
    Q_OBJECT

public:
    settingsPage(QWidget *parent = nullptr);
    ~settingsPage();
    
    QList<QString> settingsPage::listPorts()

private:
    QGridLayout* settingsPageLayout;
    QComboBox* m_pComPortSelectionBox;
    QList<QString> m_lPorts;
    QPushButton* m_pComPortConnectButton;

    PnP m_PnPMachine

private slots:

};