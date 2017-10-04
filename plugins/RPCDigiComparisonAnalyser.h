#ifndef DPGAnalysis_RPC_RPCDigiComparisonAnalyser_h
#define DPGAnalysis_RPC_RPCDigiComparisonAnalyser_h

#include <memory>
#include <vector>
#include <cstdint>

#include "FWCore/Framework/interface/stream/EDFilter.h"
#include "FWCore/Utilities/interface/EDGetToken.h"

#include "DataFormats/RPCDigi/interface/RPCDigiCollection.h"

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

class RPCDigiDistance {
public:
    RPCDigiDistance(float _weight_strip  = .005
                    , float _weight_bx   = .1
                    , float _weight_roll = 1. / RPCMaskDetId::mask_sector_
                    , int _shift_bx = 0);

    float operator()(std::pair<RPCDetId, RPCDigi> const & _lhs, std::pair<RPCDetId, RPCDigi> const & _rhs) const;

    void setStripWeight(float _weight);
    void setBXWeight   (float _weight);
    void setRollWeight (float _weight);
    void setBXShift    (int _shift);

protected:
    // for the roll distance, RPCMaskDetId is used.
    // This way weight distance_max_./RPCMaskDetId::mask_sector_ will allow swaps within a (region-station-layer-ring)
    // and 1./RPCMaskDetId::mask_region_ allows any swap
    float weight_strip_, weight_bx_, weight_roll_;
    int shift_bx_;
};

class RPCDigiComparisonAnalyserGlobalCache
{ // data coming from ParameterSet
public:
    std::string lhs_name_, rhs_name_;

    bool group_plots_;

    RPCDigiDistance distance_function_;
    float distance_max_;

    int bx_min_, bx_max_;
    TAxis lhs_bx_axis_, rhs_bx_axis_;
    TAxis lhs_strip_axis_, rhs_strip_axis_;
    std::vector<RPCMaskDetId> roll_selection_;
    std::map<unsigned long long, std::pair<unsigned long long, unsigned long long> > run_lumirange_;
};

class RPCDigiComparisonAnalyserRunCache
{ // data coming from EventSetup and Run
public:
    std::map<RPCMaskDetId, TAxis> group_rolls_;
    TAxis lumisec_axis_;
};

class RPCDigiComparisonAnalyserData
{ // data for run in stream (Analyser) and global (RunSummaryCache)
public:
    void initialise(RPCDigiComparisonAnalyserGlobalCache const & _global_cache
                    , RPCDigiComparisonAnalyserRunCache const & _run_cache);

    MergeMap<std::uint32_t, MergeTH2D> group_strip_strip_count_
        , group_bx_bx_count_
        , group_roll_roll_count_
        , group_lumisec_roll_lhsonly_
        , group_lumisec_roll_rhsonly_;
    MergeTH1D lumisec_processed_;
};

class RPCDigiComparisonAnalyser
    : public edm::stream::EDFilter<edm::GlobalCache<RPCDigiComparisonAnalyserGlobalCache>
                                   , edm::RunCache<RPCDigiComparisonAnalyserRunCache>
                                   , edm::RunSummaryCache<RPCDigiComparisonAnalyserData>
                                   , edm::EndRunProducer>
{
public:
    static std::unique_ptr<RPCDigiComparisonAnalyserGlobalCache> initializeGlobalCache(edm::ParameterSet const & _config);
    explicit RPCDigiComparisonAnalyser(edm::ParameterSet const & _config, RPCDigiComparisonAnalyserGlobalCache const *);

    static void fillDescriptions(edm::ConfigurationDescriptions & _descs);

    static std::shared_ptr<RPCDigiComparisonAnalyserRunCache> globalBeginRun(edm::Run const & _run, edm::EventSetup const & _setup
                                                                             , GlobalCache const * _global_cache);
    static std::shared_ptr<RPCDigiComparisonAnalyserData> globalBeginRunSummary(edm::Run const & _run, edm::EventSetup const & _setup
                                                                                , RunContext const * _context);
    void beginRun(edm::Run const & _run, edm::EventSetup const & _setup);
    void beginLuminosityBlock(edm::LuminosityBlock const & _lumi, edm::EventSetup const & _setup);
    bool filter(edm::Event & _event, edm::EventSetup const & _setup);
    void endRunSummary(edm::Run const & _run, edm::EventSetup const & _setup
                       , RPCDigiComparisonAnalyserData * _run_data) const;
    static void globalEndRun(edm::Run const & iRun, edm::EventSetup const &, RunContext const *) {}
    static void globalEndRunSummary(edm::Run const & iRun, edm::EventSetup const &
                                    , RunContext const * _context
                                    , RPCDigiComparisonAnalyserData const * _run_data) {}
    static void globalEndRunProduce(edm::Run & _run, edm::EventSetup const & _setup
                                    , RunContext const * _context
                                    , RPCDigiComparisonAnalyserData const * _run_data);
    static void globalEndJob(RPCDigiComparisonAnalyserGlobalCache *) {}
protected:
    // tokens
    edm::EDGetTokenT<RPCDigiCollection> lhs_digis_token_, rhs_digis_token_;
    // stream-specific histograms
    RPCDigiComparisonAnalyserData stream_data_;
    bool filter_;
};

#endif // DPGAnalysis_RPC_RPCDigiComparisonAnalyser_h
