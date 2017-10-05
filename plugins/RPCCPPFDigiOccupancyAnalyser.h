#ifndef DPGAnalysis_RPC_RPCCPPFDigiOccupancyAnalyser_h
#define DPGAnalysis_RPC_RPCCPPFDigiOccupancyAnalyser_h

#include <memory>
#include <vector>
#include <cstdint>

#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Utilities/interface/EDGetToken.h"

#include "DataFormats/RPCDigi/interface/RPCCPPFDigi.h"

#include "DPGAnalysis/Tools/interface/MergeMap.h"
#include "DPGAnalysis/Tools/interface/MergeRootHistogram.h"
#include "DPGAnalysis/RPC/interface/RPCMaskDetId.h"

namespace edm {
class ParameterSet;
class ConfigurationDescriptions;
class Run;
class LuminosityBlock;
class Event;
class EventSetup;
} // namespace edm

class RPCCPPFDigiOccupancyAnalyserGlobalCache
{ // data coming from ParameterSet
public:
    std::vector<RPCMaskDetId> selection_;
};

class RPCCPPFDigiOccupancyAnalyserRunCache
{ // data coming from EventSetup and Run
public:
    std::vector<RPCMaskDetId> groups_;
};

class RPCCPPFDigiOccupancyAnalyserData
{ // data for run in stream (Analyser) and global (RunSummaryCache)
public:
    void initialise(RPCCPPFDigiOccupancyAnalyserGlobalCache const & _global_cache
                    , RPCCPPFDigiOccupancyAnalyserRunCache const & _run_cache);

    MergeMap<std::uint32_t, MergeTH2D> group_theta_phi_count_
        , group_ring_theta_count_
        , group_sector_phi_count_;
};

class RPCCPPFDigiOccupancyAnalyser
    : public edm::stream::EDProducer<edm::GlobalCache<RPCCPPFDigiOccupancyAnalyserGlobalCache>
                                   , edm::RunCache<RPCCPPFDigiOccupancyAnalyserRunCache>
                                   , edm::RunSummaryCache<RPCCPPFDigiOccupancyAnalyserData>
                                   , edm::EndRunProducer>
{
public:
    static std::unique_ptr<RPCCPPFDigiOccupancyAnalyserGlobalCache> initializeGlobalCache(edm::ParameterSet const & _config);
    explicit RPCCPPFDigiOccupancyAnalyser(edm::ParameterSet const & _config, RPCCPPFDigiOccupancyAnalyserGlobalCache const *);

    static void fillDescriptions(edm::ConfigurationDescriptions & _descs);

    static std::shared_ptr<RPCCPPFDigiOccupancyAnalyserRunCache> globalBeginRun(edm::Run const & _run, edm::EventSetup const & _setup
                                                                                , GlobalCache const * _global_cache);
    static std::shared_ptr<RPCCPPFDigiOccupancyAnalyserData> globalBeginRunSummary(edm::Run const & _run, edm::EventSetup const & _setup
                                                                                   , RunContext const * _context);
    void beginRun(edm::Run const & _run, edm::EventSetup const & _setup);
    void produce(edm::Event & _event, edm::EventSetup const & _setup);
    void endRunSummary(edm::Run const & _run, edm::EventSetup const & _setup
                       , RPCCPPFDigiOccupancyAnalyserData * _run_data) const;
    static void globalEndRun(edm::Run const & iRun, edm::EventSetup const &, RunContext const *) {}
    static void globalEndRunSummary(edm::Run const & iRun, edm::EventSetup const &
                                    , RunContext const * _context
                                    , RPCCPPFDigiOccupancyAnalyserData const * _run_data) {}
    static void globalEndRunProduce(edm::Run & _run, edm::EventSetup const & _setup
                                    , RunContext const * _context
                                    , RPCCPPFDigiOccupancyAnalyserData const * _run_data);
    static void globalEndJob(RPCCPPFDigiOccupancyAnalyserGlobalCache *) {}
protected:
    // tokens
    edm::EDGetTokenT<RPCCPPFDigiCollection> digis_token_;
    // stream-specific histograms
    RPCCPPFDigiOccupancyAnalyserData stream_data_;
};

#endif // DPGAnalysis_RPC_RPCCPPFDigiOccupancyAnalyser_h
