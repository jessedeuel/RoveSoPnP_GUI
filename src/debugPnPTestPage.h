#pragma once

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>
#include <memory>

#include "flowControl.hpp"

class debugPnPTestPage : public QWidget
{
        Q_OBJECT

    public:
        explicit debugPnPTestPage(QWidget* parent = nullptr);
        ~debugPnPTestPage();

    private:
        std::unique_ptr<FlowControl> m_pFlowControl;

        // UI Elements
        QLineEdit* m_pComPortEdit;
        QPushButton* m_pConnectBtn;

        // Gantry Elements
        QDoubleSpinBox* m_pXSpin;
        QDoubleSpinBox* m_pYSpin;
        QPushButton* m_pMoveXYBtn;
        QDoubleSpinBox* m_pZSpin;
        QPushButton* m_pMoveZBtn;
        QPushButton* m_pHomeGantryBtn;

        // Head Elements
        QSpinBox* m_pHeadAngleSpin;
        QPushButton* m_pRotateHeadBtn;
        QPushButton* m_pVacuumOnBtn;
        QPushButton* m_pVacuumOffBtn;

        // Feeder Elements
        QDoubleSpinBox* m_pFeederLengthSpin;
        QPushButton* m_pFeedBtn;

        // LED Elements
        QComboBox* m_pLedColorCombo;
        QPushButton* m_pLed1OnBtn;
        QPushButton* m_pLed1OffBtn;
        QPushButton* m_pLed2OnBtn;
        QPushButton* m_pLed2OffBtn;

        // State Machine Elements
        QPushButton* m_pTickStateBtn;
        QPushButton* m_pAdvanceCompBtn;

        // Helper
        bool checkConnection();

    private slots:
        void onConnectClicked();
        void onMoveXYClicked();
        void onMoveZClicked();
        void onHomeGantryClicked();
        void onRotateHeadClicked();
        void onVacuumOnClicked();
        void onVacuumOffClicked();
        void onFeedClicked();
        void onLed1OnClicked();
        void onLed1OffClicked();
        void onLed2OnClicked();
        void onLed2OffClicked();
        void onTickStateClicked();
        void onAdvanceCompClicked();
};
