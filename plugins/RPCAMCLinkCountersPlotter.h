#ifndef DPGAnalysis_RPC_RPCAMCLinkCountersPlotter_h
#define DPGAnalysis_RPC_RPCAMCLinkCountersPlotter_h

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

class RPCAMCLinkCountersPlotter
    : public edm::one::EDAnalyzer<edm::one::WatchRuns>
{
public:
    explicit RPCAMCLinkCountersPlotter(edm::ParameterSet const & _config);

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
    // Tokens
    edm::EDGetTokenT<MergeTH2D> fed_type_count_token_;
    edm::EDGetTokenT<MergeMap<::uint32_t, MergeTH2D> > fed_amc_type_count_token_
        , fed_amc_input_count_token_
        , amc_input_type_count_token_;

    // Histograms
    MergeTH2D fed_type_count_;
    MergeMap<::uint32_t, MergeTH2D> fed_amc_type_count_
        , fed_amc_input_count_
        , amc_input_type_count_;
};

#endif // DPGAnalysis_RPC_RPCAMCLinkCountersPlotter_h
