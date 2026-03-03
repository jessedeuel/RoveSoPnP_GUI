#pragma once

#include <iostream>
#include <string.h>

#include <QImage>
#include <QPixmap>
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QTimer>
#include <QGroupBox>

#include <cstdio>
#include <memory>
#include <vector>
#include <future>

#include "vision/algorithms/CameraCalibration.hpp"
#include "vision/algorithms/ComponentDetector.hpp"
#include "vision/algorithms/FicucialDetector.hpp"
#include "vision/algorithms/PixelTo3D.hpp"
#include "vision/algorithms/PlacementVerifier.hpp"
#include "vision/algorithms/VisualHoming.hpp"
#include "vision/cameras/BasicCam.h"

#include "pnpRunner.h"

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
    operatorPage(std::shared_ptr<PnPRunner> pPnPRunner_instance, QWidget *parent = nullptr);
    ~operatorPage();

private:
    QGridLayout *m_pOperatorPageLayout;
    std::shared_ptr<PnPRunner> m_pPnPRunner_instance;

    // UI Controls
    QPushButton *m_pRunJobButton;
    QTextEdit *m_pGCodeEntryTextBox;
    QPushButton *m_pGCodeSendButton;

    // Vision Pipeline Toggles
    QGroupBox *m_pVisionGroupBox;
    QPushButton *m_pModeNoneBtn;
    QPushButton *m_pModeFiducialBtn;
    QPushButton *m_pModeComponentBtn;
    QPushButton *m_pModeHomingBtn;

    // Camera
    QLabel *m_pCameraDisplayLabel;
    QTimer *m_pCameraTimer;

    // Vision member variables
    VisionMode m_eVisionMode;
    cv::Mat m_currentFrame;
    std::future<bool> m_frameReadyFuture;

    IPS m_IterPersecond;
    std::unique_ptr<BasicCam> m_pGantryCam;
    std::vector<uint32_t> m_vThreadFPSValue;
    std::vector<cv::Mat> m_vCalibrationImages;

private slots:
    void onRunJobButtonClicked();
    void updateCameraDisplay();
    void onGCodeSendButtonClicked();

    // Vision Toggle Slots
    void setVisionModeNone();
    void setVisionModeFiducial();
    void setVisionModeComponent();
    void setVisionModeHoming();
};