#pragma once

#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

#include <memory>

// #include "pnpRunner.h"

class sideBar : public QWidget
{
        Q_OBJECT

    public:
        // std::shared_ptr<PnPRunner> pPnPRunner_instance
        sideBar(QWidget* parent = nullptr);
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

        // std::shared_ptr<PnPRunner> m_pPnPRunner_instance;

    private slots:
        void onPauseButtonClicked();
        void onStartEndProgramButtonClicked();
};
