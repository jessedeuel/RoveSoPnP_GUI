#include "flowControl.h"

FlowControl::FlowControl(std::shared_ptr<GRBL> grbl_instance, std::shared_ptr<BasicCam> gantryCam, std::shared_ptr<BasicCam> upwardCam) :
    m_gantryCam(gantryCam), m_upwardCam(upwardCam)
{
    grbl   = grbl_instance;
    head   = std::make_unique<Head>(grbl);
    gantry = std::make_unique<Gantry>(grbl);
    feeder = std::make_unique<Feeder>(grbl);
    led1   = std::make_unique<LED>(grbl);
    led2   = std::make_unique<LED>(grbl);

    // TODO: Initialize m_gantryCamConfig here with real calibration data
}

FlowControl::~FlowControl()
{
    grbl->comm.closeComm();
}

void FlowControl::setState(FlowState state)
{
    m_previousState = m_currentState;
    m_currentState  = state;
}

void FlowControl::processFiducialDetection(FlowState nextState)
{
    // Request a frame and wait for it
    if (m_gantryCam->RequestFrameCopy(m_cvCurrentFrame).get())
    {
        // Run detection
        auto fiducials = FiducialDetector::DetectFiducials(m_cvCurrentFrame);

        if (!fiducials.empty())
        {
            gantry_coords_t pos = gantry->getGlobalPosition();
            cv::Point3d camPos(pos.x, pos.y, pos.z);
            cv::Vec3d camRPY(0.0, 0.0, 0.0);    // Assuming downward facing

            // Translate pixel to 3D world space
            auto offset = PixelTo3D::ComputePixelTo3DOffset(m_cvCurrentFrame, m_gantryCamConfig, camPos, camRPY, fiducials[0]);

            if (offset.has_value())
            {
                m_fiducialWorldCoords.push_back(offset.value());
            }
        }
    }
    setState(nextState);
}

void FlowControl::tickStateMachine()
{
    switch (m_currentState)
    {
        case FlowState::IDLE:
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
                m_gantryCam->RequestFrameCopy(m_cvCurrentFrame);
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
                m_gantryCam->RequestFrameCopy(m_cvCurrentFrame);
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
                m_gantryCam->RequestFrameCopy(m_cvCurrentFrame);
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
                m_gantryCam->RequestFrameCopy(m_cvCurrentFrame);
            }
            break;

        case FlowState::DETECT_FIDUCIAL_4: processFiducialDetection(FlowState::CALCULATE_BOARD_TRANSFORM); break;

        case FlowState::CALCULATE_BOARD_TRANSFORM:
            // TODO: Pass m_fiducialWorldCoords to the Board class
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
            if (m_upwardCam->RequestFrameCopy(m_cvCurrentFrame).get())
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
    FlowState next_state       = FlowState::IDLE;
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
