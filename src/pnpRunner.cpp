#include "pnpRunner.h"

PnPRunner::PnPRunner(std::string comPort, std::string csvFile) : m_sComPort(comPort), m_sCsvFile(csvFile)
{
    m_pPnPMachine = std::make_unique<PnP>(m_sComPort.c_str(), m_sCsvFile.c_str());
}

PnPRunner::~PnPRunner()
{

}

void PnPRunner::ThreadedContinuousCode()
{
    m_pPnPMachine->tickStateMachine();
}

PnP PnPRunner::PooledLinearCode()
{
    // This method is not used in this class, but it must be defined since it's a pure virtual function in the Thread interface.
    return *m_pPnPMachine;
}