#include "pnpRunner.h"
#include "Logging.h"

PnPRunner::PnPRunner(std::string comPort) : m_sComPort(comPort)
{
    m_pPnPMachine = std::make_unique<PnP>(m_sComPort.c_str());
}

PnPRunner::~PnPRunner()
{

}

void PnPRunner::Stop()
{
    Thread::RequestStop();
}

void PnPRunner::ThreadedContinuousCode()
{
    m_pPnPMachine->tickStateMachine();
    LOG_DEBUG(logging::g_qSharedLogger, "Executing PnP State Machine current state: "); //m_pPnPMachine->getState()
}

PnP PnPRunner::PooledLinearCode()
{
    // This method is not used in this class, but it must be defined since it's a pure virtual function in the Thread interface.
    return 0;//*(m_pPnPMachine.get());
}

void PnPRunner::sendGCode(const std::string &gcode)
{
    LOG_DEBUG(logging::g_qSharedLogger, "Sending GCode: ", gcode);
    m_pPnPMachine->grbl.sendCommand(gcode);
}

std::string PnPRunner::getState()
{
    qDebug() << "Called getState";

    if (m_pPnPMachine->getState() == STOP) return "STOP";
    else if (m_pPnPMachine->getState() == IDLE) return "IDLE";
    else if (m_pPnPMachine->getState() == PICK) return "PICK";
    else if (m_pPnPMachine->getState() == ORIENT) return "ORIENT";
    else if (m_pPnPMachine->getState() == PLACE) return "PLACE";
    else if (m_pPnPMachine->getState() == PAUSE) return "PAUSE";
    else if (m_pPnPMachine->getState() == ERROR) return "ERROR";
    else if (m_pPnPMachine->getState() == RELOAD) return "RELOAD";
    else if (m_pPnPMachine->getState() == MANUAL) return "MANUAL";
    else return "UNKNOWN";
}