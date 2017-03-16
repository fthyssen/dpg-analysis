#ifndef DPGAnalysis_RPC_RPCRandomDigiProducer_h
#define DPGAnalysis_RPC_RPCRandomDigiProducer_h

#include <vector>
#include <utility>

#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/RandomNumberGenerator.h"

#include "DataFormats/MuonDetId/interface/RPCDetId.h"
#include "DPGAnalysis/RPC/interface/RPCMaskDetId.h"

namespace edm {
class ConfigurationDescriptions;
class Event;
class EventSetup;
class ParameterSet;
class Run;
} // namespace edm

class RPCRandomDigiProducer
    : public edm::stream::EDProducer<>
{
public:
    RPCRandomDigiProducer(edm::ParameterSet const & _config);
    ~RPCRandomDigiProducer();

    static void fillDescriptions(edm::ConfigurationDescriptions & _descs);

    void beginRun(edm::Run const & _run, edm::EventSetup const & _setup) override;
    void produce(edm::Event & _event, edm::EventSetup const & _setup) override;

protected:
    int bx_min_, bx_max_;
    unsigned int hits_max_, cls_max_;
    std::vector<RPCMaskDetId> roll_selection_;
    std::vector<std::pair<RPCDetId, unsigned int> > roll_nstrips_;
    std::vector<unsigned int> nstrips_;

    edm::Service<edm::RandomNumberGenerator> rand_;
};

#endif // DPGAnalysis_RPC_RPCRandomDigiProducer_h
