#include "calibrationPage.h"
#include "flowControl.h"
#include <QFileDialog>
#include <QFont>
#include <QMessageBox>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

CalibrationPage::CalibrationPage(std::shared_ptr<BasicCam> cam, QWidget* parent) : QWidget(parent), m_camera(cam)
{
    setupUI();
}

CalibrationPage::~CalibrationPage() {}

void CalibrationPage::bindFlowControl(FlowControl* fc)
{
    if (!fc)
        return;
    m_fc = fc;
    connect(m_fc, &FlowControl::requestCameraFrameUpdate, this, &CalibrationPage::updateCameraFrame);
}

void CalibrationPage::setupUI()
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);

    // --- LEFT: Camera Display ---
    QVBoxLayout* visionLayout = new QVBoxLayout();
    QLabel* titleLabel        = new QLabel("Live Calibration Feed", this);
    QFont font                = titleLabel->font();
    font.setPointSize(14);
    font.setBold(true);
    titleLabel->setFont(font);

    m_feedLabel = new QLabel("NO SIGNAL", this);
    m_feedLabel->setAlignment(Qt::AlignCenter);
    m_feedLabel->setStyleSheet(
        "QLabel { background-color: #000000; color: #ff0000; font-weight: bold; font-size: 24px; border: 2px solid #34495e; border-radius: 5px; }");
    m_feedLabel->setMinimumSize(640, 480);
    m_feedLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    visionLayout->addWidget(titleLabel);
    visionLayout->addWidget(m_feedLabel);
    mainLayout->addLayout(visionLayout, 6);    // 60% width

    // --- RIGHT: Controls ---
    QVBoxLayout* controlLayout = new QVBoxLayout();

    // 1. Parameters Group
    QGroupBox* paramGroup  = new QGroupBox("Chessboard Parameters");
    QGridLayout* paramGrid = new QGridLayout(paramGroup);

    m_spinBoardX           = new QSpinBox();
    m_spinBoardX->setRange(3, 30);
    m_spinBoardX->setValue(9);    // Inner corners X

    m_spinBoardY = new QSpinBox();
    m_spinBoardY->setRange(3, 30);
    m_spinBoardY->setValue(6);    // Inner corners Y

    m_spinSquareSize = new QDoubleSpinBox();
    m_spinSquareSize->setRange(1.0, 100.0);
    m_spinSquareSize->setValue(25.0);    // mm

    paramGrid->addWidget(new QLabel("Inner Corners X:"), 0, 0);
    paramGrid->addWidget(m_spinBoardX, 0, 1);
    paramGrid->addWidget(new QLabel("Inner Corners Y:"), 1, 0);
    paramGrid->addWidget(m_spinBoardY, 1, 1);
    paramGrid->addWidget(new QLabel("Square Size (mm):"), 2, 0);
    paramGrid->addWidget(m_spinSquareSize, 2, 1);

    // 2. Capture Group
    QGroupBox* captureGroup    = new QGroupBox("Image Capture");
    QVBoxLayout* captureLayout = new QVBoxLayout(captureGroup);

    m_lblCaptureCount          = new QLabel("Valid Captures: 0");
    m_lblCaptureCount->setStyleSheet("font-weight: bold; color: #2980b9;");

    QPushButton* btnCapture = new QPushButton("Capture View");
    btnCapture->setStyleSheet("background-color: #27ae60; color: white; font-weight: bold; padding: 10px;");
    QPushButton* btnClear = new QPushButton("Clear All Captures");

    connect(btnCapture, &QPushButton::clicked, this, &CalibrationPage::onCaptureClicked);
    connect(btnClear, &QPushButton::clicked, this, &CalibrationPage::onClearClicked);

    captureLayout->addWidget(m_lblCaptureCount);
    captureLayout->addWidget(btnCapture);
    captureLayout->addWidget(btnClear);

    // 3. Calibration Group
    QGroupBox* calibGroup     = new QGroupBox("Run Calibration");
    QVBoxLayout* calibLayout  = new QVBoxLayout(calibGroup);

    m_chkFisheye              = new QCheckBox("Use Fisheye Lens Model");
    QPushButton* btnCalibrate = new QPushButton("Compute Calibration");
    btnCalibrate->setStyleSheet("background-color: #8e44ad; color: white; font-weight: bold; padding: 10px;");

    m_lblRmsError = new QLabel("RMS Error: --");
    m_lblRmsError->setStyleSheet("font-weight: bold;");

    connect(btnCalibrate, &QPushButton::clicked, this, &CalibrationPage::onCalibrateClicked);

    calibLayout->addWidget(m_chkFisheye);
    calibLayout->addWidget(btnCalibrate);
    calibLayout->addWidget(m_lblRmsError);

    // 4. Test & Save Group
    QGroupBox* saveGroup    = new QGroupBox("Test & Save");
    QVBoxLayout* saveLayout = new QVBoxLayout(saveGroup);

    m_chkPreviewUndistorted = new QCheckBox("Show Undistorted Live Preview");
    QPushButton* btnSave    = new QPushButton("Save Config to File...");
    btnSave->setStyleSheet("background-color: #f39c12; color: white; font-weight: bold; padding: 10px;");

    connect(btnSave, &QPushButton::clicked, this, &CalibrationPage::onSaveClicked);

    saveLayout->addWidget(m_chkPreviewUndistorted);
    saveLayout->addWidget(btnSave);

    controlLayout->addWidget(paramGroup);
    controlLayout->addWidget(captureGroup);
    controlLayout->addWidget(calibGroup);
    controlLayout->addWidget(saveGroup);
    controlLayout->addStretch();

    mainLayout->addLayout(controlLayout, 4);    // 40% width
}

void CalibrationPage::updateCameraFrame(const QImage& image, const QString& /*activeCameraName*/)
{
    if (image.isNull())
        return;

    QImage displayImage = image;

    // Apply distortion correction to the live preview if calibrated & requested
    if (m_isCalibrated && m_chkPreviewUndistorted->isChecked())
    {
        // Wrap QImage memory directly into an OpenCV Mat
        cv::Mat mat(image.height(), image.width(), CV_8UC3, (void*) image.constBits(), image.bytesPerLine());

        cv::Mat rgbMat;
        mat.copyTo(rgbMat);    // Deep copy to prevent modifying the QImage buffer

        cv::Mat undistorted;
        if (m_chkFisheye->isChecked())
        {
            cv::fisheye::undistortImage(rgbMat, undistorted, m_K, m_D);
        }
        else
        {
            cv::undistort(rgbMat, undistorted, m_K, m_D);
        }

        displayImage = QImage(undistorted.data, undistorted.cols, undistorted.rows, undistorted.step, QImage::Format_RGB888).copy();
    }

    m_feedLabel->setPixmap(QPixmap::fromImage(displayImage).scaled(m_feedLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void CalibrationPage::onCaptureClicked()
{
    if (!m_camera)
        return;

    cv::Mat frame;
    // Request a raw, unscaled frame directly from the camera driver for maximum precision
    if (m_camera->RequestFrameCopy(frame).get() && !frame.empty())
    {
        m_imageSize = frame.size();

        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        cv::Size boardSize(m_spinBoardX->value(), m_spinBoardY->value());
        std::vector<cv::Point2f> corners;

        // Attempt to find the chessboard corners
        bool found = cv::findChessboardCorners(gray, boardSize, corners, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE | cv::CALIB_CB_FAST_CHECK);

        if (found)
        {
            // Increase corner precision
            cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 0.001));

            m_imagePoints.push_back(corners);

            // Build real-world 3D coordinates for these corners
            std::vector<cv::Point3f> objP;
            float squareSize = m_spinSquareSize->value();
            for (int i = 0; i < boardSize.height; ++i)
            {
                for (int j = 0; j < boardSize.width; ++j)
                {
                    objP.push_back(cv::Point3f(j * squareSize, i * squareSize, 0));
                }
            }
            m_objectPoints.push_back(objP);

            m_lblCaptureCount->setText(QString("Valid Captures: %1 (Last: SUCCESS)").arg(m_imagePoints.size()));
        }
        else
        {
            m_lblCaptureCount->setText(QString("Valid Captures: %1 (Last: REJECTED)").arg(m_imagePoints.size()));
            QMessageBox::warning(this, "Capture Failed", "Could not detect chessboard. Ensure the entire board is visible and well-lit.");
        }
    }
}

void CalibrationPage::onClearClicked()
{
    m_imagePoints.clear();
    m_objectPoints.clear();
    m_isCalibrated = false;
    m_lblCaptureCount->setText("Valid Captures: 0");
    m_lblRmsError->setText("RMS Error: --");
}

void CalibrationPage::onCalibrateClicked()
{
    if (m_imagePoints.empty())
    {
        QMessageBox::warning(this, "Error", "No captures available. Please capture valid chessboard images first.");
        return;
    }

    std::vector<cv::Mat> rvecs, tvecs;
    double rms = 0;

    try
    {
        if (m_chkFisheye->isChecked())
        {
            int flags = cv::fisheye::CALIB_RECOMPUTE_EXTRINSIC | cv::fisheye::CALIB_CHECK_COND | cv::fisheye::CALIB_FIX_SKEW;

            // Fisheye calibration requires double-precision vectors
            std::vector<std::vector<cv::Point2d>> imgPointsD(m_imagePoints.size());
            std::vector<std::vector<cv::Point3d>> objPointsD(m_objectPoints.size());

            for (size_t i = 0; i < m_imagePoints.size(); ++i)
            {
                for (auto& p : m_imagePoints[i])
                    imgPointsD[i].push_back(cv::Point2d(p.x, p.y));
                for (auto& p : m_objectPoints[i])
                    objPointsD[i].push_back(cv::Point3d(p.x, p.y, p.z));
            }

            rms = cv::fisheye::calibrate(objPointsD, imgPointsD, m_imageSize, m_K, m_D, rvecs, tvecs, flags);
        }
        else
        {
            rms = cv::calibrateCamera(m_objectPoints, m_imagePoints, m_imageSize, m_K, m_D, rvecs, tvecs);
        }

        m_isCalibrated = true;
        m_lblRmsError->setText(QString("RMS Error: %1").arg(rms, 0, 'f', 4));
        QMessageBox::information(this, "Success", QString("Calibration Complete!\n\nRMS Reprojection Error: %1\n\nA lower error is better (typically < 1.0).").arg(rms));
    }
    catch (cv::Exception& e)
    {
        QMessageBox::critical(this, "Calibration Failed", QString("OpenCV Error: %1").arg(e.what()));
    }
}

void CalibrationPage::onSaveClicked()
{
    if (!m_isCalibrated)
    {
        QMessageBox::warning(this, "Error", "You must successfully run a calibration before saving.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Save Camera Calibration Config", "", "YAML Config Files (*.yaml);;XML Config Files (*.xml)");
    if (fileName.isEmpty())
        return;

    try
    {
        cv::FileStorage fs(fileName.toStdString(), cv::FileStorage::WRITE);
        fs << "camera_matrix" << m_K;
        fs << "distortion_coefficients" << m_D;
        fs << "image_width" << m_imageSize.width;
        fs << "image_height" << m_imageSize.height;
        fs << "is_fisheye" << m_chkFisheye->isChecked();
        fs.release();

        QMessageBox::information(this, "Saved", "Camera configuration saved successfully.");
    }
    catch (cv::Exception& e)
    {
        QMessageBox::critical(this, "Save Failed", QString("Failed to write to file: %1").arg(e.what()));
    }
}
