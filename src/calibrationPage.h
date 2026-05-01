#ifndef CALIBRATIONPAGE_H
#define CALIBRATIONPAGE_H

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>
#include <opencv2/opencv.hpp>
#include <vector>

#include "vision/cameras/BasicCam.h"

class FlowControl;    // Forward declaration

class CalibrationPage : public QWidget
{
        Q_OBJECT

    public:
        explicit CalibrationPage(std::shared_ptr<BasicCam> cam, QWidget* parent = nullptr);
        ~CalibrationPage();

        // Bind to the state machine to get the live video feed
        void bindFlowControl(FlowControl* fc);

    public slots:
        // Slot to receive frames from the FlowControl background loop
        void updateCameraFrame(const QImage& image, const QString& activeCameraName);

    private slots:
        // UI Interaction Handlers
        void onCaptureClicked();
        void onClearClicked();
        void onCalibrateClicked();
        void onSaveClicked();

    private:
        std::shared_ptr<BasicCam> m_camera;
        FlowControl* m_fc = nullptr;

        // UI Elements
        QLabel* m_feedLabel;
        QSpinBox* m_spinBoardX;
        QSpinBox* m_spinBoardY;
        QDoubleSpinBox* m_spinSquareSize;
        QCheckBox* m_chkFisheye;
        QCheckBox* m_chkPreviewUndistorted;
        QLabel* m_lblCaptureCount;
        QLabel* m_lblRmsError;

        // Calibration Data Storage
        std::vector<std::vector<cv::Point2f>> m_imagePoints;
        std::vector<std::vector<cv::Point3f>> m_objectPoints;
        cv::Size m_imageSize;

        // Result Matrices
        cv::Mat m_K;
        cv::Mat m_D;
        bool m_isCalibrated = false;

        void setupUI();
};

#endif    // CALIBRATIONPAGE_H
