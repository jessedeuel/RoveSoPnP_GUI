#include "flowControl.h"

FlowControl::FlowControl(std::shared_ptr<GRBL> grbl_instance, std::shared_ptr<BasicCam> gantryCam, std::shared_ptr<BasicCam> upwardCam) :
    QObject(nullptr),    // Inherit QObject parent safely
    m_gantryCam(gantryCam), m_upwardCam(upwardCam)
{
    grbl   = grbl_instance;
    head   = std::make_unique<Head>(grbl);
    gantry = std::make_unique<Gantry>(grbl);
    feeder = std::make_unique<Feeder>(grbl);
    led1   = std::make_unique<LED>(grbl);
    led2   = std::make_unique<LED>(grbl);

    // TODO: Initialize m_gantryCamConfig here with real calibration data

    // Automatically tick the state machine at 30 FPS without blocking the UI
    m_tickTimer = new QTimer(this);
    connect(m_tickTimer, &QTimer::timeout, this, &FlowControl::tickStateMachine);
    m_tickTimer->start(33);
}

FlowControl::~FlowControl()
{
    if (m_tickTimer)
    {
        m_tickTimer->stop();
    }
    grbl->disconnect();
}

QString FlowControl::getStateString() const
{
    switch (m_currentState)
    {
        case FlowState::IDLE: return "Idle";
        case FlowState::RUNNING: return "Running";
        case FlowState::JOB_FAILED: return "Job Failed";
        case FlowState::BOARD_DETECT_SAFE_START_STATE: return "Starting Board Detect";
        case FlowState::CALIBRATION_HOME: return "Homing (Calibration)";
        case FlowState::CALIBRATION_HOMING: return "Homing In Progress...";
        case FlowState::MOVE_Z_SAFE_CALIBRATION: return "Moving Z to Safe Height";
        case FlowState::MOVE_TO_FIDUCIAL_1: return "Aligning Fiducial 1";
        case FlowState::DETECT_FIDUCIAL_1: return "Detecting Fiducial 1";
        case FlowState::MOVE_TO_FIDUCIAL_2: return "Aligning Fiducial 2";
        case FlowState::DETECT_FIDUCIAL_2: return "Detecting Fiducial 2";
        case FlowState::MOVE_TO_FIDUCIAL_3: return "Aligning Fiducial 3";
        case FlowState::DETECT_FIDUCIAL_3: return "Detecting Fiducial 3";
        case FlowState::MOVE_TO_FIDUCIAL_4: return "Aligning Fiducial 4";
        case FlowState::DETECT_FIDUCIAL_4: return "Detecting Fiducial 4";
        case FlowState::CALCULATE_BOARD_TRANSFORM: return "Calculating Transform Matrix";
        case FlowState::FEEDER_SAFE_START_STATE: return "Feeder Safe Start";
        case FlowState::GET_NEXT_COMPONENT: return "Getting Next Component";
        case FlowState::WAIT_FOR_USER_CUTTAPE_RELOAD: return "Waiting for Tape Reload";
        case FlowState::ADVANCE_FEEDER: return "Advancing Feeder";
        case FlowState::PNP_HOME: return "Homing (PnP)";
        case FlowState::PNP_HOMING: return "Homing In Progress...";
        case FlowState::PICKUP_SAFE_START_STATE: return "Pickup Safe Start";
        case FlowState::MOVE_Z_SAFE_PICK: return "Moving Z to Safe Pick";
        case FlowState::MOVE_XY_TO_FEEDER: return "Moving to Feeder";
        case FlowState::LOWER_Z_TO_PICK: return "Lowering to Pick";
        case FlowState::ENABLE_VACUUM: return "Enabling Vacuum";
        case FlowState::DWELL_FOR_VACUUM: return "Dwelling (Vacuum)";
        case FlowState::RAISE_Z_FROM_PICK: return "Raising from Pick";
        case FlowState::PLACE_DETECT_SAFE_START_STATE: return "Place Safe Start";
        case FlowState::MOVE_XY_TO_UPWARD_CAMERA: return "Moving to Upward Camera";
        case FlowState::DETECT_COMPONENT_POSE: return "Detecting Pose";
        case FlowState::MOVING_TO_CORRECTED_POSITION: return "Moving to Placement";
        case FlowState::ROTATE_HEAD_A_AXIS: return "Rotating Head";
        case FlowState::LOWER_Z_TO_DROP: return "Lowering to Drop";
        case FlowState::DISABLE_VACUUM: return "Disabling Vacuum";
        case FlowState::DWELL_FOR_RELEASE: return "Dwelling (Release)";
        case FlowState::RAISE_Z_FROM_DROP: return "Raising from Drop";
        case FlowState::PAUSE: return "Paused";
        case FlowState::AWAIT_OPERATOR_RESUME: return "Awaiting Resume...";
        case FlowState::MACHINE_IS_STUPID: return "Error / Aborted";
        default: return "Unknown State";
    }
}

void FlowControl::setState(FlowState state)
{
    m_previousState = m_currentState;
    m_currentState  = state;

    // Emit the change to all UI elements
    emit stateChanged(getStateString());

    // Dynamically trigger the UI to open the jog controls and show the lock button
    if (state == FlowState::MOVE_TO_FIDUCIAL_1)
        emit requestUserFiducialLock(1);
    else if (state == FlowState::MOVE_TO_FIDUCIAL_2)
        emit requestUserFiducialLock(2);
    else if (state == FlowState::MOVE_TO_FIDUCIAL_3)
        emit requestUserFiducialLock(3);
    else if (state == FlowState::MOVE_TO_FIDUCIAL_4)
        emit requestUserFiducialLock(4);
}

// --- MACHINE CONTROL SLOTS ---

void FlowControl::startJob()
{
    setState(FlowState::BOARD_DETECT_SAFE_START_STATE);
    emit sendLogMessage("Backend: Job Started. Beginning Calibration Sequence.");
}

void FlowControl::pauseJob()
{
    setState(FlowState::PAUSE);
    emit sendLogMessage("Backend: Job Paused.");
}

void FlowControl::resumeJob()
{
    m_userResumesMachine = true;
    emit sendLogMessage("Backend: Job Resumed.");
}

void FlowControl::stopJob()
{
    setState(FlowState::IDLE);
    if (head)
        head->vacuumOff();
    emit sendLogMessage("Backend: Job Stopped/Aborted.");
}

void FlowControl::jogMachine(const QString& axis, double distance)
{
    // Typical GRBL Relative Jog command string. e.g., $J=G91 X1.0 F1000
    QString jogCmd = QString("$J=G91 %1%2 F1000\n").arg(axis).arg(distance);

    // NOTE: Uncomment / replace this depending on your GRBL wrapper method names!
    // grbl->send(jogCmd.toStdString());

    emit sendLogMessage(QString("Jogged %1 by %2 mm").arg(axis).arg(distance));
}

// -------------------------------------

void FlowControl::updateLiveVisionFeed(const QString& cameraName)
{
    if (m_gantryCam && m_gantryCam->RequestFrameCopy(m_cvCurrentFrame).get())
    {
        if (!m_cvCurrentFrame.empty())
        {
            auto fiducials = FiducialDetector::DetectFiducials(m_cvCurrentFrame);
            for (const auto& fid : fiducials)
            {
                cv::drawMarker(m_cvCurrentFrame, fid, cv::Scalar(0, 255, 0), cv::MARKER_CROSS, 20, 2);
                cv::circle(m_cvCurrentFrame, fid, 15, cv::Scalar(0, 255, 0), 2);
            }

            cv::Mat rgb;
            cv::cvtColor(m_cvCurrentFrame, rgb, cv::COLOR_BGR2RGB);
            QImage img(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);

            emit requestCameraFrameUpdate(img.copy(), cameraName);
        }
    }
}

void FlowControl::processFiducialDetection(FlowState nextState)
{
    if (m_gantryCam && m_gantryCam->RequestFrameCopy(m_cvCurrentFrame).get())
    {
        auto fiducials = FiducialDetector::DetectFiducials(m_cvCurrentFrame);

        if (!fiducials.empty())
        {
            grbl_position_t pos = gantry->getGlobalPosition();
            cv::Point3d camPos(pos.x, pos.y, pos.z);
            cv::Vec3d camRPY(0.0, 0.0, 0.0);    // Assuming downward facing

            auto offset = PixelTo3D::ComputePixelTo3DOffset(m_cvCurrentFrame, m_gantryCamConfig, camPos, camRPY, fiducials[0]);

            if (offset.has_value())
            {
                m_fiducialWorldCoords.push_back({offset->x, offset->y, 0.0});
                emit sendLogMessage(QString("Success: Fiducial recorded at World X: %1, Y: %2").arg(offset->x).arg(offset->y));
            }
            else
            {
                emit sendLogMessage("Error: Detected fiducial but failed to compute 3D offset.");
            }
        }
        else
        {
            emit sendLogMessage("Error: No fiducials detected in frame!");
        }
    }
    setState(nextState);
}

void FlowControl::tickStateMachine()
{
    // 1. Check for position updates and push to UI if changed
    if (gantry)
    {
        grbl_position_t pos = gantry->getGlobalPosition();
        if (pos.x != m_lastReportedX || pos.y != m_lastReportedY || pos.z != m_lastReportedZ)
        {
            m_lastReportedX = pos.x;
            m_lastReportedY = pos.y;
            m_lastReportedZ = pos.z;
            emit positionUpdated(pos.x, pos.y, pos.z);
        }
    }

    // 2. Process Machine States
    switch (m_currentState)
    {
        case FlowState::IDLE: updateLiveVisionFeed("Gantry Camera (Idle / Ready)"); break;

        case FlowState::RUNNING: break;

        case FlowState::JOB_FAILED:
            head->vacuumOff();
            setState(FlowState::IDLE);
            break;

        // ==========================================
        // VISION CALIBRATION SEQUENCE
        // ==========================================
        case FlowState::BOARD_DETECT_SAFE_START_STATE:
            m_fiducialWorldCoords.clear();
            setState(FlowState::CALIBRATION_HOME);
            break;

        case FlowState::CALIBRATION_HOME:
            gantry->home();
            setState(FlowState::CALIBRATION_HOMING);
            break;

        case FlowState::CALIBRATION_HOMING:
            if (!grbl->isBusy())
                setState(FlowState::MOVE_Z_SAFE_CALIBRATION);
            break;

        case FlowState::MOVE_Z_SAFE_CALIBRATION:
            if (!grbl->isBusy())
            {
                gantry->setHeadHeight(SAFE_Z_HEIGHT);
                m_userConfirmsPosition = false;
                setState(FlowState::MOVE_TO_FIDUCIAL_1);
            }
            break;

        case FlowState::MOVE_TO_FIDUCIAL_1:
            if (m_userConfirmsPosition)
            {
                setState(FlowState::DETECT_FIDUCIAL_1);
                m_userConfirmsPosition = false;
            }
            else
            {
                updateLiveVisionFeed("Aligning Fiducial 1...");
            }
            break;

        case FlowState::DETECT_FIDUCIAL_1: processFiducialDetection(FlowState::MOVE_TO_FIDUCIAL_2); break;

        case FlowState::MOVE_TO_FIDUCIAL_2:
            if (m_userConfirmsPosition)
            {
                setState(FlowState::DETECT_FIDUCIAL_2);
                m_userConfirmsPosition = false;
            }
            else
            {
                updateLiveVisionFeed("Aligning Fiducial 2...");
            }
            break;

        case FlowState::DETECT_FIDUCIAL_2: processFiducialDetection(FlowState::MOVE_TO_FIDUCIAL_3); break;

        case FlowState::MOVE_TO_FIDUCIAL_3:
            if (m_userConfirmsPosition)
            {
                setState(FlowState::DETECT_FIDUCIAL_3);
                m_userConfirmsPosition = false;
            }
            else
            {
                updateLiveVisionFeed("Aligning Fiducial 3...");
            }
            break;

        case FlowState::DETECT_FIDUCIAL_3: processFiducialDetection(FlowState::MOVE_TO_FIDUCIAL_4); break;

        case FlowState::MOVE_TO_FIDUCIAL_4:
            if (m_userConfirmsPosition)
            {
                setState(FlowState::DETECT_FIDUCIAL_4);
                m_userConfirmsPosition = false;
            }
            else
            {
                updateLiveVisionFeed("Aligning Fiducial 4...");
            }
            break;

        case FlowState::DETECT_FIDUCIAL_4: processFiducialDetection(FlowState::CALCULATE_BOARD_TRANSFORM); break;

        case FlowState::CALCULATE_BOARD_TRANSFORM:
            if (components)
            {
                components->calculateBoardOffset(m_fiducialWorldCoords);
                components->printCoords({0, 0, 0});
                emit sendLogMessage("Successfully calculated Board Transform Matrix.");
            }
            else
            {
                emit sendLogMessage("Error: Components not loaded! Cannot calculate transform.");
            }
            setState(FlowState::FEEDER_SAFE_START_STATE);
            break;

        // ==========================================
        // PICK AND PLACE JOB SEQUENCE
        // ==========================================
        case FlowState::FEEDER_SAFE_START_STATE: setState(FlowState::GET_NEXT_COMPONENT); break;

        case FlowState::GET_NEXT_COMPONENT:
            if (!m_userConfirmsFeederReady)
            {
                setState(FlowState::WAIT_FOR_USER_CUTTAPE_RELOAD);
            }
            else
            {
                setState(FlowState::ADVANCE_FEEDER);
            }
            break;

        case FlowState::WAIT_FOR_USER_CUTTAPE_RELOAD:
            if (m_userConfirmsFeederReady)
                setState(FlowState::PNP_HOME);
            break;

        case FlowState::ADVANCE_FEEDER:
            feeder->increment(current_component.tape_pitch);
            setState(FlowState::PNP_HOME);
            break;

        case FlowState::PNP_HOME:
            gantry->home();
            setState(FlowState::PNP_HOMING);
            break;

        case FlowState::PNP_HOMING:
            if (!grbl->isBusy())
                setState(FlowState::PICKUP_SAFE_START_STATE);
            break;

        case FlowState::PICKUP_SAFE_START_STATE: setState(FlowState::MOVE_Z_SAFE_PICK); break;

        case FlowState::MOVE_Z_SAFE_PICK:
            if (!grbl->isBusy())
            {
                gantry->setHeadHeight(SAFE_Z_HEIGHT);
                setState(FlowState::MOVE_XY_TO_FEEDER);
            }
            break;

        case FlowState::MOVE_XY_TO_FEEDER:
            if (!grbl->isBusy())
            {
                gantry->setGlobalPosition(current_component.feeder_x, current_component.feeder_y);
                setState(FlowState::LOWER_Z_TO_PICK);
            }
            break;

        case FlowState::LOWER_Z_TO_PICK:
            if (!grbl->isBusy())
            {
                gantry->setHeadHeight(current_component.pickup_z);
                setState(FlowState::ENABLE_VACUUM);
            }
            break;

        case FlowState::ENABLE_VACUUM:
            head->vacuumOn();
            m_dwellStartTime = std::chrono::steady_clock::now();
            setState(FlowState::DWELL_FOR_VACUUM);
            break;

        case FlowState::DWELL_FOR_VACUUM:
        {
            auto now     = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_dwellStartTime).count();
            if (elapsed > DWELL_TIME_MS)
                setState(FlowState::RAISE_Z_FROM_PICK);
        }
        break;

        case FlowState::RAISE_Z_FROM_PICK:
            if (!grbl->isBusy())
            {
                gantry->setHeadHeight(SAFE_Z_HEIGHT);
                setState(FlowState::PLACE_DETECT_SAFE_START_STATE);
            }
            break;

        case FlowState::PLACE_DETECT_SAFE_START_STATE: setState(FlowState::MOVE_XY_TO_UPWARD_CAMERA); break;

        case FlowState::MOVE_XY_TO_UPWARD_CAMERA:
            if (!grbl->isBusy())
            {
                // gantry->setGlobalPosition(UPWARD_CAM_X, UPWARD_CAM_Y);
                setState(FlowState::DETECT_COMPONENT_POSE);
            }
            break;

        case FlowState::DETECT_COMPONENT_POSE:
            if (m_upwardCam && m_upwardCam->RequestFrameCopy(m_cvCurrentFrame).get())
            {
                // Detect pose via upward camera
                auto pose = ComponentDetector::DetectComponentPose(m_cvCurrentFrame);

                if (pose.bFound)
                {
                    // Convert pixel offset to mm offset and apply to target place coords
                    // Convert angle offset and apply
                    m_correctedPlaceAngle = pose.dRotationDegrees;

                    // Note: You will need a pixel-to-mm mapping here for m_correctedPlaceX/Y
                    m_correctedPlaceX = current_component.place_x;
                    m_correctedPlaceY = current_component.place_y;
                }
            }

            gantry->setGlobalPosition(m_correctedPlaceX, m_correctedPlaceY);
            setState(FlowState::MOVING_TO_CORRECTED_POSITION);
            break;

        case FlowState::MOVING_TO_CORRECTED_POSITION:
            if (!grbl->isBusy())
                setState(FlowState::ROTATE_HEAD_A_AXIS);
            break;

        case FlowState::ROTATE_HEAD_A_AXIS:
            if (!grbl->isBusy())
            {
                // head->increment(m_correctedPlaceAngle);
                setState(FlowState::LOWER_Z_TO_DROP);
            }
            break;

        case FlowState::LOWER_Z_TO_DROP:
            if (!grbl->isBusy())
            {
                gantry->setHeadHeight(current_component.place_z);
                setState(FlowState::DISABLE_VACUUM);
            }
            break;

        case FlowState::DISABLE_VACUUM:
            head->vacuumOff();
            m_dwellStartTime = std::chrono::steady_clock::now();
            setState(FlowState::DWELL_FOR_RELEASE);
            break;

        case FlowState::DWELL_FOR_RELEASE:
        {
            auto now     = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_dwellStartTime).count();
            if (elapsed > DWELL_TIME_MS)
                setState(FlowState::RAISE_Z_FROM_DROP);
        }
        break;

        case FlowState::RAISE_Z_FROM_DROP:
            if (!grbl->isBusy())
            {
                gantry->setHeadHeight(SAFE_Z_HEIGHT);
                setState(FlowState::GET_NEXT_COMPONENT);
            }
            break;

        // ==========================================
        // ERROR HANDLING & INTERRUPTS
        // ==========================================
        case FlowState::PAUSE:
            if (m_userResumesMachine)
            {
                m_userResumesMachine = false;
                setState(FlowState::AWAIT_OPERATOR_RESUME);
            }
            break;

        case FlowState::AWAIT_OPERATOR_RESUME: break;

        case FlowState::MACHINE_IS_STUPID: head->vacuumOff(); break;
    }
}

FlowState FlowControl::advanceComponent()
{
    FlowState next_state = FlowState::IDLE;

    if (!components)
    {
        emit sendLogMessage("Error: Components not loaded! Cannot advance component.");
        return next_state;
    }

    components_status_t status = components->incrementCurrentComponent();

    if (status == SAME_CUTTAPE)
    {
        // Feed next component
        // Tell Feeder to step forward
        next_state = FlowState::PICKUP_SAFE_START_STATE;
    }
    else if (status == CHANGE_CUTTAPE)
        next_state = FlowState::FEEDER_SAFE_START_STATE;
    else if (status == FINAL_CUTTAPE)
        next_state = FlowState::IDLE;

    return next_state;
}

void FlowControl::updateComponents(const char* posFile)
{
    components = std::make_unique<Components>(posFile);
}
