#pragma once

#include <string>

#include "Thread.hpp"
#include "pnp.hpp"

class PnPRunner : public Thread<PnP>
{
public:
    PnPRunner(std::string comPort = "", std::string csvFile = "");
    ~PnPRunner();

private:
    std::string m_sComPort;
    std::string m_sCsvFile;
    std::unique_ptr<PnP> m_pPnPMachine;

    void ThreadedContinuousCode() override;
    PnP PooledLinearCode() override;
};