#ifndef DPGAnalysis_RPC_RPCDigiComparisonPlotter_h
#define DPGAnalysis_RPC_RPCDigiComparisonPlotter_h

#include <stdint.h>

#include <sstream>

#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Utilities/interface/EDGetToken.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DPGAnalysis/Tools/interface/MergeMap.h"
#include "DPGAnalysis/Tools/interface/MergeRootHistogram.h"

namespace edm {
class ParameterSet;
class ConfigurationDescriptions;
class Run;
class Event;
class EventSetup;
} // namespace edm

class RPCDigiComparisonPlotter
    : public edm::one::EDAnalyzer<edm::one::WatchRuns>
{
public:
    explicit RPCDigiComparisonPlotter(edm::ParameterSet const & _config);

    static void fillDescriptions(edm::ConfigurationDescriptions & _descs);

    void beginJob();
    void beginRun(edm::Run const & _run, edm::EventSetup const & _setup);
    void analyze(edm::Event const & _event, edm::EventSetup const & _setup);
    void endRun(edm::Run const & _run, edm::EventSetup const & _setup);
    void endJob();

protected:
    edm::Service<TFileService> fileservice_;

    bool first_run_;
    std::string output_pdf_;
    bool create_pdf_;
    std::ostringstream title_note_;
    bool show_run_, set_logz_, use_diverging_palette_;
    std::string lhs_name_, rhs_name_;
    // Tokens
    edm::EDGetTokenT<MergeMap<::uint32_t, MergeTH2D> > group_roll_roll_count_token_
        , group_strip_strip_count_token_
        , group_bx_bx_count_token_
        , group_lumisec_roll_lhsonly_token_
        , group_lumisec_roll_rhsonly_token_;
    edm::EDGetTokenT<MergeTH1D> lumisec_processed_token_;

    // Histograms
    MergeMap<::uint32_t, MergeTH2D> group_roll_roll_count_
        , group_strip_strip_count_
        , group_bx_bx_count_
        , group_lumisec_roll_lhsonly_
        , group_lumisec_roll_rhsonly_;
    MergeTH1D lumisec_processed_;
};

#endif // DPGAnalysis_RPC_RPCDigiComparisonPlotter_h
