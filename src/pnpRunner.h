#pragma once

#include <string>
#include <atomic>

#include "../libdeps/RoveSoPnP_Vision/src/interfaces/Thread.hpp"
#include "pnp.hpp"

class PnPRunner : public Thread<PnP>
{
public:
    PnPRunner(std::string comPort = "");
    ~PnPRunner();

    void Stop();
    std::unique_ptr<PnP> &getPnPMachine() { return m_pPnPMachine; }

private:
    std::string m_sComPort;
    std::string m_sCsvFile;
    std::unique_ptr<PnP> m_pPnPMachine;

    void ThreadedContinuousCode() override;
    PnP PooledLinearCode() override;
};