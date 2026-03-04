#pragma once

#include <string>
#include <atomic>
#include <mutex>

#include "../libdeps/RoveSoPnP_Vision/src/interfaces/Thread.hpp"
#include "pnp.hpp"

// High-level state machine for the UI to track what the machine is currently doing
enum class MachineState
{
    DISCONNECTED,
    IDLE,               // Connected, homed, waiting for commands
    HOMING,             // Executing mechanical + visual homing
    VISION_CALIBRATION, // Finding fiducials, calculating PCB offsets
    RUNNING_JOB,        // Actively picking and placing
    PAUSED,             // Job paused by user or non-fatal error
    ERROR_STATE         // Fatal error, requires manual reset
};

class PnPRunner : public Thread<PnP>
{
public:
    PnPRunner(std::string comPort = "");
    ~PnPRunner();

    void Stop();
    std::unique_ptr<PnP> &getPnPMachine() { return m_pPnPMachine; }

    // --- State Machine Controls ---
    MachineState GetCurrentState();
    void SetState(MachineState newState);

    // --- UI Command Hooks ---
    void CommandHomeMachine();
    void CommandCalibrateVision();
    void CommandStartJob(const std::string &jobCsvPath);
    void CommandPauseJob();
    void CommandResumeJob();
    void CommandAbort();

private:
    std::string m_sComPort;
    std::string m_sCsvFile;
    std::unique_ptr<PnP> m_pPnPMachine;

    // Thread-safe state tracking
    std::atomic<MachineState> m_eCurrentState;
    std::mutex m_stateMutex;

    void ThreadedContinuousCode() override;
    PnP PooledLinearCode() override;
};