#ifndef DEBUGPNPTESTPAGE_H
#define DEBUGPNPTESTPAGE_H

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>
#include <memory>

// Hardware and Flow Control Includes
#include "LED.hpp"
#include "feeder.hpp"
#include "flowControl.h"    // Ensure this matches the exact casing of your header file
#include "gantry.hpp"
#include "grbl.hpp"
#include "head.hpp"

class debugPnPTestPage : public QWidget
{
        Q_OBJECT

    public:
        explicit debugPnPTestPage(QWidget* parent = nullptr);
        ~debugPnPTestPage();

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
        void onAdvanceCompClicked();
        void onTickStateClicked();

    private:
        bool checkConnection();

        // --- UI Elements ---
        QLineEdit* m_pComPortEdit;
        QPushButton* m_pConnectBtn;

        QDoubleSpinBox* m_pXSpin;
        QDoubleSpinBox* m_pYSpin;
        QPushButton* m_pMoveXYBtn;

        QDoubleSpinBox* m_pZSpin;
        QPushButton* m_pMoveZBtn;
        QPushButton* m_pHomeGantryBtn;

        QSpinBox* m_pHeadAngleSpin;
        QPushButton* m_pRotateHeadBtn;
        QPushButton* m_pVacuumOnBtn;
        QPushButton* m_pVacuumOffBtn;

        QDoubleSpinBox* m_pFeederLengthSpin;
        QPushButton* m_pFeedBtn;

        QComboBox* m_pLedColorCombo;
        QPushButton* m_pLed1OnBtn;
        QPushButton* m_pLed1OffBtn;
        QPushButton* m_pLed2OnBtn;
        QPushButton* m_pLed2OffBtn;

        QPushButton* m_pAdvanceCompBtn;
        QPushButton* m_pTickStateBtn;

        // --- Hardware Control Instances ---
        std::shared_ptr<GRBL> m_grbl;
        std::unique_ptr<Gantry> m_gantry;
        std::unique_ptr<Head> m_head;
        std::unique_ptr<Feeder> m_feeder;
        std::unique_ptr<LED> m_led1;
        std::unique_ptr<LED> m_led2;

        std::unique_ptr<FlowControl> m_pFlowControl;
};

#endif    // DEBUGPNPTESTPAGE_H
