#include "DPGAnalysis/RPC/plugins/RPCCPPFDigiOccupancyPlotter.h"

#include <map>

#include "TCanvas.h"
#include "TPolyLine.h"
#include "TText.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Common/interface/Handle.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DPGAnalysis/RPC/interface/RPCMaskDetId.h"

#include "DPGAnalysis/Tools/interface/DivergingPalette.h"
#include "DPGAnalysis/Tools/interface/SequentialPalette.h"

RPCCPPFDigiOccupancyPlotter::RPCCPPFDigiOccupancyPlotter(edm::ParameterSet const & _config)
    : first_run_(true)
    , output_pdf_(_config.getParameter<std::string>("pdfOutput") + ".pdf")
    , create_pdf_(_config.getParameter<bool>("createPDF"))
    , show_run_(_config.getParameter<bool>("showRun"))
    , set_logz_(_config.getParameter<bool>("setLogz"))
    , use_diverging_palette_(_config.getParameter<bool>("useDivergingPalette"))
{
    title_note_ << _config.getParameter<std::string>("titleNote");

    std::string _analyser(_config.getParameter<std::string>("rpcDigiComparisonAnalyser"));
    group_theta_phi_count_token_
        = consumes<MergeMap<::uint32_t, MergeTH2D>, edm::InRun>(edm::InputTag(_analyser, "GroupThetaPhiCount"));
    group_ring_theta_count_token_
        = consumes<MergeMap<::uint32_t, MergeTH2D>, edm::InRun>(edm::InputTag(_analyser, "GroupRingThetaCount"));
    group_sector_phi_count_token_
        = consumes<MergeMap<::uint32_t, MergeTH2D>, edm::InRun>(edm::InputTag(_analyser, "GroupSectorPhiCount"));
}

void RPCCPPFDigiOccupancyPlotter::fillDescriptions(edm::ConfigurationDescriptions & _descs)
{
    edm::ParameterSetDescription _desc;

    _desc.add<std::string>("rpcDigiComparisonAnalyser", "RPCCPPFDigiOccupancyAnalyser");

    _desc.add<std::string>("pdfOutput", "RPCCPPFDigiOccupancyAnalyser");
    _desc.add<bool>("createPDF", false);
    _desc.add<std::string>("titleNote" ,"CMS Preliminary");
    _desc.add<bool>("showRun", true);
    _desc.add<bool>("setLogz", true);
    _desc.add<bool>("useDivergingPalette", true);

    _descs.add("RPCCPPFDigiOccupancyPlotter", _desc);
}

void RPCCPPFDigiOccupancyPlotter::beginJob()
{}

void RPCCPPFDigiOccupancyPlotter::beginRun(edm::Run const &, edm::EventSetup const &)
{}

void RPCCPPFDigiOccupancyPlotter::analyze(edm::Event const &, edm::EventSetup const &)
{}

void RPCCPPFDigiOccupancyPlotter::endRun(edm::Run const & _run, edm::EventSetup const &)
{
    edm::Handle<MergeMap<::uint32_t, MergeTH2D> > _group_theta_phi_count
        , _group_ring_theta_count
        , _group_sector_phi_count;

    _run.getByToken(group_theta_phi_count_token_, _group_theta_phi_count);
    _run.getByToken(group_ring_theta_count_token_, _group_ring_theta_count);
    _run.getByToken(group_sector_phi_count_token_, _group_sector_phi_count);

    if (first_run_) {
        group_theta_phi_count_ = *_group_theta_phi_count;
        group_ring_theta_count_ = *_group_ring_theta_count;
        group_sector_phi_count_ = *_group_sector_phi_count;

        if (show_run_) {
            title_note_ << (title_note_.str().empty() ? "" : " - ")
                        << "run " << _run.run();
        }

        first_run_ = false;
    } else {
        group_theta_phi_count_.mergeProduct(*_group_theta_phi_count);
        group_ring_theta_count_.mergeProduct(*_group_ring_theta_count);
        group_sector_phi_count_.mergeProduct(*_group_sector_phi_count);

        if (show_run_) {
            title_note_ << ", " << _run.run();
        }
    }
}

void RPCCPPFDigiOccupancyPlotter::endJob()
{
    std::map<::uint32_t, TH2D *> _group_theta_phi_count
        , _group_ring_theta_count
        , _group_sector_phi_count;

    TFileDirectory _theta_phi_folder(fileservice_->mkdir("ThetaPhi"));
    for (auto _group : *group_theta_phi_count_) {
        TH2D * _th2d = _theta_phi_folder.make<TH2D>(*_group.second);
        _th2d->UseCurrentStyle();
        _group_theta_phi_count[_group.first] = _th2d;
    }

    TFileDirectory _ring_theta_folder(fileservice_->mkdir("RingTheta"));
    for (auto _group : *group_ring_theta_count_) {
        TH2D * _th2d = _ring_theta_folder.make<TH2D>(*_group.second);
        _th2d->UseCurrentStyle();
        _group_ring_theta_count[_group.first] = _th2d;
    }

    TFileDirectory _sector_phi_folder(fileservice_->mkdir("SectorPhi"));
    for (auto _group : *group_sector_phi_count_) {
        TH2D * _th2d = _sector_phi_folder.make<TH2D>(*_group.second);
        _th2d->UseCurrentStyle();
        _group_sector_phi_count[_group.first] = _th2d;
    }

    if (!create_pdf_) {
        return;
    }

    // PDF Output
    TCanvas _dummy_canvas("DummyCanvas");
    _dummy_canvas.ResetBit(kMustCleanup);
    _dummy_canvas.ResetBit(kCanDelete);
    {
        std::string _output_pdf = output_pdf_ + "[";
        _dummy_canvas.Print(_output_pdf.c_str());
    }

    TText _title(0.10, 0.91, "Title");
    _title.ResetBit(kMustCleanup);
    _title.ResetBit(kCanDelete);
    _title.SetNDC();
    _title.SetTextSize(0.04);
    _title.SetTextAlign(11);
    TText _title_note(0.85, 0.91, title_note_.str().c_str());
    _title_note.ResetBit(kMustCleanup);
    _title_note.ResetBit(kCanDelete);
    _title_note.SetNDC();
    _title_note.SetTextSize(0.02);
    _title_note.SetTextAlign(31);

    DivergingPalette _diverging_palette;
    SequentialPalette _sequential_palette;
    if (use_diverging_palette_) {
        _diverging_palette.applyPalette();
    } else {
        _sequential_palette.applyPalette();
    }

    for (auto _group : _group_theta_phi_count) {
        TCanvas _canvas(_group.second->GetName());
        _canvas.ResetBit(kMustCleanup);
        _canvas.ResetBit(kCanDelete);
        _canvas.SetTicks(1, 1);
        _canvas.SetLeftMargin(.10);
        _canvas.SetRightMargin(.15);
        _canvas.SetBottomMargin(.10);
        if (set_logz_) {
            _canvas.SetLogz();
        }
        TH1 * _temp_th2d = _group.second->DrawCopy("colz");
        _temp_th2d->SetTitle("");
        _temp_th2d->SetStats(false);
        _temp_th2d->GetZaxis()->SetTitle("Digis");
        _title.SetText(0.10, 0.91, _group.second->GetTitle());
        _title.Draw();
        _title_note.Draw();
        _canvas.Print(output_pdf_.c_str());
    }

    for (auto _group : _group_ring_theta_count) {
        TCanvas _canvas(_group.second->GetName());
        _canvas.ResetBit(kMustCleanup);
        _canvas.ResetBit(kCanDelete);
        _canvas.SetTicks(1, 1);
        _canvas.SetLeftMargin(.10);
        _canvas.SetRightMargin(.15);
        _canvas.SetBottomMargin(.10);
        if (set_logz_) {
            _canvas.SetLogz();
        }
        TH1 * _temp_th2d = _group.second->DrawCopy("colz");
        _temp_th2d->SetTitle("");
        _temp_th2d->SetStats(false);
        _temp_th2d->GetZaxis()->SetTitle("Digis");
        _title.SetText(0.10, 0.91, _group.second->GetTitle());
        _title.Draw();
        _title_note.Draw();
        _canvas.Print(output_pdf_.c_str());
    }

    for (auto _group : _group_sector_phi_count) {
        TCanvas _canvas(_group.second->GetName());
        _canvas.ResetBit(kMustCleanup);
        _canvas.ResetBit(kCanDelete);
        _canvas.SetTicks(1, 1);
        _canvas.SetLeftMargin(.10);
        _canvas.SetRightMargin(.15);
        _canvas.SetBottomMargin(.10);
        if (set_logz_) {
            _canvas.SetLogz();
        }
        TH1 * _temp_th2d = _group.second->DrawCopy("colz");
        _temp_th2d->SetTitle("");
        _temp_th2d->SetStats(false);
        _temp_th2d->GetZaxis()->SetTitle("Digis");
        _title.SetText(0.10, 0.91, _group.second->GetTitle());
        _title.Draw();
        _title_note.Draw();
        _canvas.Print(output_pdf_.c_str());
    }

    {
        std::string _output_pdf = output_pdf_ + "]";
        _dummy_canvas.Print(_output_pdf.c_str());
    }
}

//define this as a module
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(RPCCPPFDigiOccupancyPlotter);
