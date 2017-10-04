#ifndef DPGAnalysis_RPC_RPCAMCLinkCountersAnalyser_h
#define DPGAnalysis_RPC_RPCAMCLinkCountersAnalyser_h

#include <memory>
#include <cstdint>
#include <vector>
#include <string>

#include "FWCore/Framework/interface/stream/EDFilter.h"
#include "FWCore/Utilities/interface/EDGetToken.h"

#include "DataFormats/RPCDigi/interface/RPCAMCLinkCounters.h"
#include "CondFormats/RPCObjects/interface/RPCAMCLink.h"

#include "DPGAnalysis/Tools/interface/MergeMap.h"
#include "DPGAnalysis/Tools/interface/MergeRootHistogram.h"

namespace edm {
class ParameterSet;
class ConfigurationDescriptions;
class Run;
class LuminosityBlock;
class Event;
class EventSetup;
} // namespace edm

class RPCAMCLinkCountersAnalyserGlobalCache
{ // data coming from ParameterSet
public:
    std::string link_map_rcd_;
};

class RPCAMCLinkCountersAnalyserRunCache
{ // data coming from EventSetup and Run
public:
    TAxis fed_axis_;
    std::map<RPCAMCLink, TAxis> fed_amc_axis_;
    std::map<RPCAMCLink, TAxis> fed_input_axis_;
    std::map<RPCAMCLink, TAxis> amc_input_axis_;
    TAxis fed_type_axis_, amc_type_axis_, input_type_axis_;
};

class RPCAMCLinkCountersAnalyserData
{ // data for run in stream (Analyser) and global (RunSummaryCache)
public:
    void initialise(RPCAMCLinkCountersAnalyserRunCache const & _run_cache);

    MergeTH2D fed_type_count_;
    MergeMap<std::uint32_t, MergeTH2D> fed_amc_type_count_;
    MergeMap<std::uint32_t, MergeTH2D> fed_amc_input_count_;
    MergeMap<std::uint32_t, MergeTH2D> amc_input_type_count_;
};

class RPCAMCLinkCountersAnalyser
    : public edm::stream::EDFilter<edm::GlobalCache<RPCAMCLinkCountersAnalyserGlobalCache>
                                   , edm::RunCache<RPCAMCLinkCountersAnalyserRunCache>
                                   , edm::RunSummaryCache<RPCAMCLinkCountersAnalyserData>
                                   , edm::EndRunProducer>
{
public:
    static std::unique_ptr<RPCAMCLinkCountersAnalyserGlobalCache> initializeGlobalCache(edm::ParameterSet const & _config);
    explicit RPCAMCLinkCountersAnalyser(edm::ParameterSet const & _config, RPCAMCLinkCountersAnalyserGlobalCache const *);

    static void fillDescriptions(edm::ConfigurationDescriptions & _descs);

    static std::shared_ptr<RPCAMCLinkCountersAnalyserRunCache> globalBeginRun(edm::Run const & _run, edm::EventSetup const & _setup
                                                                              , GlobalCache const * _global_cache);
    static std::shared_ptr<RPCAMCLinkCountersAnalyserData> globalBeginRunSummary(edm::Run const & _run, edm::EventSetup const & _setup
                                                                                 , RunContext const * _context);
    void beginRun(edm::Run const & _run, edm::EventSetup const & _setup);
    bool filter(edm::Event & _event, edm::EventSetup const & _setup);
    void endRunSummary(edm::Run const & _run, edm::EventSetup const & _setup
                       , RPCAMCLinkCountersAnalyserData * _run_data) const;
    static void globalEndRun(edm::Run const &, edm::EventSetup const &, RunContext const *) {}
    static void globalEndRunSummary(edm::Run const & iRun, edm::EventSetup const &
                                    , RunContext const * _context
                                    , RPCAMCLinkCountersAnalyserData const * _run_data) {}
    static void globalEndRunProduce(edm::Run & _run, edm::EventSetup const & _setup
                                    , RunContext const * _context
                                    , RPCAMCLinkCountersAnalyserData const * _run_data);
    static void globalEndJob(RPCAMCLinkCountersAnalyserGlobalCache *) {}
protected:
    // tokens
    std::vector<edm::EDGetTokenT<RPCAMCLinkCounters> > counters_token_;
    // stream-specific histograms
    RPCAMCLinkCountersAnalyserData stream_data_;
    bool filter_;
};

#endif // DPGAnalysis_RPC_RPCAMCLinkCountersAnalyser_h
