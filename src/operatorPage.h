#pragma once

#include <iostream>
#include <string.h>

#include <QImage>
#include <QPixmap>
#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QTimer>

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

class operatorPage : public QWidget
{
    Q_OBJECT

public:
    operatorPage(QWidget *parent = nullptr);
    ~operatorPage();

private:
    QGridLayout* m_pOperatorPageLayout;
    QPushButton* m_pRunJobButton;
    QTextEdit* m_pGCodeEntryTextBox;
    QPushButton* m_pGCodeSendButton;

    QLabel *m_pCameraDisplayLabel;
    QTimer *m_pCameraTimer;

    // Vision member variables
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
};