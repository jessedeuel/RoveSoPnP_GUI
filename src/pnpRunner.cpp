#include "pnpRunner.h"
#include "Logging.h"

PnPRunner::PnPRunner(std::string comPort) : m_sComPort(comPort), m_eCurrentState(MachineState::DISCONNECTED)
{
    // TODO: FLOW CONTROL - Initialize serial connection to GRBL controller here.
    // If successful:
    // m_eCurrentState = MachineState::IDLE;
    m_pPnPMachine = std::make_unique<PnP>(m_sComPort.c_str());
}

PnPRunner::~PnPRunner() {}

void PnPRunner::Stop() { Thread::RequestStop(); }

MachineState PnPRunner::GetCurrentState() { return m_eCurrentState.load(); }
void PnPRunner::SetState(MachineState newState) { m_eCurrentState.store(newState); }

// --- UI Command Hooks ---
void PnPRunner::CommandHomeMachine()
{
    if (m_eCurrentState == MachineState::IDLE || m_eCurrentState == MachineState::ERROR_STATE)
        SetState(MachineState::HOMING);
}

void PnPRunner::CommandCalibrateVision()
{
    if (m_eCurrentState == MachineState::IDLE)
        SetState(MachineState::VISION_CALIBRATION);
}

void PnPRunner::CommandStartJob(const std::string &jobCsvPath)
{
    if (m_eCurrentState == MachineState::IDLE)
    {
        m_sCsvFile = jobCsvPath;
        SetState(MachineState::RUNNING_JOB);
    }
}

void PnPRunner::CommandPauseJob()
{
    if (m_eCurrentState == MachineState::RUNNING_JOB)
        SetState(MachineState::PAUSED);
}

void PnPRunner::CommandResumeJob()
{
    if (m_eCurrentState == MachineState::PAUSED)
        SetState(MachineState::RUNNING_JOB);
}

void PnPRunner::CommandAbort()
{
    // TODO: FLOW CONTROL - Send GRBL soft-reset / abort command immediately (e.g., '!')
    SetState(MachineState::IDLE);
}

// --- Main Thread Loop ---
void PnPRunner::ThreadedContinuousCode()
{
    // Tick the underlying FlowControl state machine to keep buffers clear and GRBL happy
    m_pPnPMachine->tickStateMachine();

    switch (m_eCurrentState.load())
    {
    case MachineState::DISCONNECTED:
        // Waiting for comms to establish...
        break;

    case MachineState::IDLE:
        // Waiting for user input.
        break;

    case MachineState::HOMING:
        LOG_DEBUG(logging::g_qSharedLogger, "State: HOMING. Executing Homing Routine.");
        // 1. TODO: FLOW CONTROL - Send $H to GRBL for mechanical homing.
        // 2. TODO: FLOW CONTROL - Wait for 'ok'.
        // 3. TODO: VISION - Move to visual home fiducial coordinates.
        // 4. TODO: VISION - Call VisualHoming::FindHomeMarker().
        // 5. TODO: FLOW CONTROL - Apply G92 Work Coordinate Offset based on visual error.
        SetState(MachineState::IDLE); // Once finished
        break;

    case MachineState::VISION_CALIBRATION:
        LOG_DEBUG(logging::g_qSharedLogger, "State: VISION_CALIBRATION. Aligning PCB.");
        // 1. TODO: FLOW CONTROL - Read Board CSV to get theoretical Fiducial 1 & 2 coords.
        // 2. TODO: FLOW CONTROL - Move Top Camera to Fiducial 1.
        // 3. TODO: VISION - Call FiducialDetector::DetectFiducials() -> get pixel offset.
        // 4. TODO: VISION - Call PixelTo3D::ComputePixelTo3DOffset() -> get mm offset.
        // 5. TODO: FLOW CONTROL - Move to Fiducial 2, repeat.
        // 6. TODO: FLOW CONTROL - Calculate Affine Transform / Rotation Matrix for PCB skew.
        SetState(MachineState::IDLE);
        break;

    case MachineState::RUNNING_JOB:
        // Main Pick and Place Loop
        LOG_DEBUG(logging::g_qSharedLogger, "State: RUNNING_JOB. Processing components.");
        // 1. TODO: FLOW CONTROL - Pop next component from queue.
        // 2. TODO: FLOW CONTROL - Move to Feeder/Tape coordinate.
        // 3. TODO: VISION (Optional) - Detect sprocket hole to adjust pickup center.
        // 4. TODO: FLOW CONTROL - Z-down, Vacuum ON, Z-up.
        // 5. TODO: FLOW CONTROL - Move over Bottom Camera.
        // 6. TODO: VISION - Call ComponentDetector::DetectComponentPose().
        // 7. TODO: FLOW CONTROL - Calculate drop offsets based on bottom vision angle/center.
        // 8. TODO: FLOW CONTROL - Move to PCB drop coordinate (apply PCB skew matrix from VisionCalibration).
        // 9. TODO: FLOW CONTROL - Z-down, Vacuum OFF, Z-up.
        // 10. Repeat or set to IDLE if queue is empty.
        break;

    case MachineState::PAUSED:
        // TODO: FLOW CONTROL - Send GRBL Feed Hold ('!') or just stop sending new GCode.
        break;

    case MachineState::ERROR_STATE:
        // TODO: FLOW CONTROL - Handle GRBL alarms, limits triggered, or vacuum drops.
        break;
    }
}

PnP PnPRunner::PooledLinearCode() { return 0; }

void PnPRunner::sendGCode(const std::string &gcode)
{
    LOG_DEBUG(logging::g_qSharedLogger, "Sending GCode: ", gcode);
    m_pPnPMachine->grbl.sendCommand(gcode);
}

std::string PnPRunner::GetCurrentStateString()
{
    switch (m_eCurrentState.load())
    {
    case MachineState::DISCONNECTED:
        return "DISCONNECTED";
    case MachineState::IDLE:
        return "IDLE";
    case MachineState::HOMING:
        return "HOMING";
    case MachineState::VISION_CALIBRATION:
        return "VISION_CALIBRATION";
    case MachineState::RUNNING_JOB:
        return "RUNNING_JOB";
    case MachineState::PAUSED:
        return "PAUSED";
    case MachineState::ERROR_STATE:
        return "ERROR_STATE";
    default:
        return "UNKNOWN";
    }
}