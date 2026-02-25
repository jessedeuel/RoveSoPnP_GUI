#pragma once

#include <QLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

#include <memory>
#include "pnpRunner.h"

class sideBar : public QWidget
{
    Q_OBJECT

public:
    sideBar(std::shared_ptr<PnPRunner> pPnPRunner_instance, QWidget *parent = nullptr);
    ~sideBar();

private:
    QGridLayout* m_pSideBarLayout;
    QLabel* m_pStateLabel;
    QLabel* m_pConnectionStatusLabel;
    QLabel* m_pPositionLabel;
    QLabel* m_pCurrentComponentLabel;
    QLabel* m_pCurrentJobLabel;
    QPushButton* m_pPauseButton;
    QPushButton* m_pStartEndProgramButton;

private slots:
    void onPauseButtonClicked();
    void onStartEndProgramButtonClicked();
};