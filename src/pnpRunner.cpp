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