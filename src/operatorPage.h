#pragma once

#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <future>
#include <memory>

// #include "pnpRunner.h"
#include "vision/cameras/BasicCam.h"

// Enum to manage which vision pipeline is currently active
enum class VisionMode
{
    None,
    Fiducial,
    Component,
    Homing
};

class operatorPage : public QWidget
{
        Q_OBJECT

    public:
        // std::shared_ptr<PnPRunner> pPnPRunner_instance
        operatorPage(QWidget* parent = nullptr);
        ~operatorPage();

    private:
        // std::shared_ptr<PnPRunner> m_pPnPRunner_instance;

        // --- UI Layouts ---
        QGridLayout* m_pOperatorPageLayout;

        // --- Machine Status Panel ---
        QGroupBox* m_pStatusGroup;
        QLabel* m_pStateLabel;
        QLabel* m_pPositionLabel;

        // --- Control Panel ---
        QGroupBox* m_pControlGroup;
        QPushButton* m_pHomeBtn;
        QPushButton* m_pCalibrateVisionBtn;
        QPushButton* m_pStartJobBtn;
        QPushButton* m_pPauseBtn;
        QPushButton* m_pAbortBtn;

        // --- Vision Panel ---
        QGroupBox* m_pVisionGroup;
        QLabel* m_pCameraDisplayLabel;
        QPushButton* m_pModeNoneBtn;
        QPushButton* m_pModeFiducialBtn;
        QPushButton* m_pModeComponentBtn;
        QPushButton* m_pModeHomingBtn;

        // --- Manual Control ---
        QTextEdit* m_pGCodeEntryTextBox;
        QPushButton* m_pGCodeSendButton;

        // --- Vision & Update Logic ---
        QTimer* m_pUpdateTimer;
        VisionMode m_eVisionMode;
        cv::Mat m_currentFrame;
        std::future<bool> m_frameReadyFuture;
        std::unique_ptr<BasicCam> m_pGantryCam;

    private slots:
        void updateUIAndCamera();
        void onGCodeSendButtonClicked();

        // Machine Controls
        void onHomeClicked();
        void onCalibrateClicked();
        void onStartJobClicked();
        void onPauseClicked();
        void onAbortClicked();

        // Vision Toggles
        void setVisionNone() { m_eVisionMode = VisionMode::None; }

        void setVisionFiducial() { m_eVisionMode = VisionMode::Fiducial; }

        void setVisionComponent() { m_eVisionMode = VisionMode::Component; }

        void setVisionHoming() { m_eVisionMode = VisionMode::Homing; }
};
