#ifndef GUI_FLOWCONTROL_H
#define GUI_FLOWCONTROL_H

#include <QImage>
#include <QObject>
#include <QString>
#include <QTimer>
#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

// --- Low-Level Hardware ---
#include "../libdeps/RoveSoPnP_FlowControl/include/LED.hpp"
#include "../libdeps/RoveSoPnP_FlowControl/include/board.hpp"
#include "../libdeps/RoveSoPnP_FlowControl/include/feeder.hpp"
#include "../libdeps/RoveSoPnP_FlowControl/include/gantry.hpp"
#include "../libdeps/RoveSoPnP_FlowControl/include/grbl.hpp"
#include "../libdeps/RoveSoPnP_FlowControl/include/head.hpp"
#include "../libdeps/RoveSoPnP_FlowControl/include/tapeLookup.hpp"

// --- Vision Library ---
#include "../libdeps/RoveSoPnP_Vision/src/vision/algorithms/ComponentDetector.hpp"
#include "../libdeps/RoveSoPnP_Vision/src/vision/algorithms/FicucialDetector.hpp"
#include "../libdeps/RoveSoPnP_Vision/src/vision/algorithms/PixelTo3D.hpp"
#include "../libdeps/RoveSoPnP_Vision/src/vision/cameras/BasicCam.h"
#include <opencv2/opencv.hpp>

// --- Board Placement Files ---
#define ARM_FlowControl_FILE  "../board/ArmBoard_Hardware-all-pos.csv"
#define CORE_FlowControl_FILE "../board/CoreBoard-all-pos.csv"

enum class FlowState
{
    // --- System States ---
    IDLE,
    RUNNING,
    JOB_FAILED,

    // --- Vision Calibration Sequence ---
    BOARD_DETECT_SAFE_START_STATE,
    CALIBRATION_HOME,
    CALIBRATION_HOMING,
    MOVE_Z_SAFE_CALIBRATION,
    MOVE_TO_FIDUCIAL_1,
    DETECT_FIDUCIAL_1,
    MOVE_TO_FIDUCIAL_2,
    DETECT_FIDUCIAL_2,
    MOVE_TO_FIDUCIAL_3,
    DETECT_FIDUCIAL_3,
    MOVE_TO_FIDUCIAL_4,
    DETECT_FIDUCIAL_4,
    CALCULATE_BOARD_TRANSFORM,

    // --- Pick and Place Job Sequence ---
    FEEDER_SAFE_START_STATE,
    GET_NEXT_COMPONENT,
    WAIT_FOR_USER_CUTTAPE_RELOAD,
    ADVANCE_FEEDER,
    PNP_HOME,
    PNP_HOMING,
    PICKUP_SAFE_START_STATE,
    MOVE_Z_SAFE_PICK,
    MOVE_XY_TO_FEEDER,
    LOWER_Z_TO_PICK,
    ENABLE_VACUUM,
    DWELL_FOR_VACUUM,
    RAISE_Z_FROM_PICK,
    PLACE_DETECT_SAFE_START_STATE,
    MOVE_XY_TO_UPWARD_CAMERA,
    DETECT_COMPONENT_POSE,
    MOVING_TO_CORRECTED_POSITION,
    ROTATE_HEAD_A_AXIS,
    LOWER_Z_TO_DROP,
    DISABLE_VACUUM,
    DWELL_FOR_RELEASE,
    RAISE_Z_FROM_DROP,

    // --- Error Handling & Interrupts ---
    PAUSE,
    AWAIT_OPERATOR_RESUME,
    MACHINE_IS_STUPID
};

class FlowControl : public QObject
{
        Q_OBJECT

    public:
        FlowControl(std::shared_ptr<GRBL> grbl_instance, std::shared_ptr<BasicCam> gantryCam, std::shared_ptr<BasicCam> upwardCam);
        ~FlowControl();

        void tickStateMachine();
        void setState(FlowState state);

        FlowState getState() const { return m_currentState; }

        QString getStateString() const;

        void setUserConfirmsPosition(bool confirmed) { m_userConfirmsPosition = confirmed; }

        void setFeederReady(bool ready) { m_userConfirmsFeederReady = ready; }

        void resumeMachine() { m_userResumesMachine = true; }

        FlowState advanceComponent();
        void updateComponents(const char* posFile);

    public slots:
        // --- Machine Control Handlers (Triggered by UI) ---
        void startJob();
        void pauseJob();
        void resumeJob();
        void stopJob();
        void jogMachine(const QString& axis, double distance);

    signals:
        // Signals for OperatorPage & SideBar integration
        void requestCameraFrameUpdate(const QImage& image, const QString& activeCameraName);
        void sendLogMessage(const QString& msg);
        void stateChanged(const QString& stateName);
        void positionUpdated(float x, float y, float z);

        // Signal to dynamically ask the UI to unlock jog controls and show a lock button
        void requestUserFiducialLock(int fiducialIndex);

    private:
        std::shared_ptr<GRBL> grbl;
        std::unique_ptr<Head> head;
        std::unique_ptr<Gantry> gantry;
        std::unique_ptr<Feeder> feeder;
        std::unique_ptr<LED> led1;
        std::unique_ptr<LED> led2;
        std::unique_ptr<Components> components;

        // Internal Timer to continuously tick the state machine
        QTimer* m_tickTimer;

        // Vision Components
        std::shared_ptr<BasicCam> m_gantryCam;
        std::shared_ptr<BasicCam> m_upwardCam;
        cv::Mat m_cvCurrentFrame;
        CameraConfig m_gantryCamConfig;    // Setup with your real K/D matrices
        std::vector<board_coords_t> m_fiducialWorldCoords;

        // State management
        FlowState m_currentState  = FlowState::IDLE;
        FlowState m_previousState = FlowState::IDLE;

        // Track last reported position to prevent UI spam
        float m_lastReportedX = -999.0f;
        float m_lastReportedY = -999.0f;
        float m_lastReportedZ = -999.0f;

        // Timers for dwells
        std::chrono::steady_clock::time_point m_dwellStartTime;

        // Timers for hardware polling
        std::chrono::steady_clock::time_point m_lastPositionPollTime;
        std::chrono::steady_clock::time_point m_lastBusyPollTime;
        bool m_cachedBusyState = false;

        // Job Data
        struct ComponentData
        {
                float feeder_x, feeder_y;
                float pickup_z;
                float place_x, place_y;
                float place_z;
                float tape_pitch;
        } current_component;

        // Computed Corrections
        float m_correctedPlaceX      = 0.0f;
        float m_correctedPlaceY      = 0.0f;
        double m_correctedPlaceAngle = 0.0;

        // Configuration
        const float SAFE_Z_HEIGHT = 0.0f;
        const int DWELL_TIME_MS   = 500;

        // Status Flags (Set via GUI)
        bool m_userConfirmsPosition    = false;
        bool m_userConfirmsFeederReady = false;
        bool m_userResumesMachine      = false;

        // Throttled busy check to prevent serial spam
        bool isMachineBusy();

        // Helper to process a fiducial state
        void processFiducialDetection(FlowState nextState);

        // Helper to continuously push frames with vision overlay to the UI while jogging
        void updateLiveVisionFeed(const QString& cameraName);
};

#endif    // GUI_FLOWCONTROL_H