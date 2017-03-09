#ifndef DPGAnalysis_RPC_RPCEMTFPhiThetaScaleAnalyser_h
#define DPGAnalysis_RPC_RPCEMTFPhiThetaScaleAnalyser_h

#include "FWCore/Framework/interface/stream/EDAnalyzer.h"

namespace edm {
class ConfigurationDescriptions;
class Event;
class EventSetup;
class ParameterSet;
class Run;
} // namespace edm

class RPCEMTFPhiThetaScaleAnalyser : public edm::stream::EDAnalyzer<>
{
public:
    explicit RPCEMTFPhiThetaScaleAnalyser(edm::ParameterSet const & _config);

    static void fillDescriptions(edm::ConfigurationDescriptions & _descs);

    void analyze(edm::Event const & _event, edm::EventSetup const & _setup) override;

public:
};

#endif // DPGAnalysis_RPC_RPCEMTFPhiThetaScaleAnalyser_h
