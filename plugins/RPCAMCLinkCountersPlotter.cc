#include "DPGAnalysis/RPC/plugins/RPCAMCLinkCountersPlotter.h"

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

#include "CondFormats/DataRecord/interface/RPCTwinMuxLinkMapRcd.h"
#include "CondFormats/RPCObjects/interface/RPCAMCLink.h"
#include "CondFormats/RPCObjects/interface/RPCAMCLinkMap.h"

#include "DPGAnalysis/Tools/interface/DivergingPalette.h"
#include "DPGAnalysis/Tools/interface/SequentialPalette.h"

RPCAMCLinkCountersPlotter::RPCAMCLinkCountersPlotter(edm::ParameterSet const & _config)
    : first_run_(true)
    , output_pdf_(_config.getParameter<std::string>("pdfOutput") + ".pdf")
    , create_pdf_(_config.getParameter<bool>("createPDF"))
    , show_run_(_config.getParameter<bool>("showRun"))
    , set_logz_(_config.getParameter<bool>("setLogz"))
    , use_diverging_palette_(_config.getParameter<bool>("useDivergingPalette"))
{
    title_note_ << _config.getParameter<std::string>("titleNote");

    std::string _analyser(_config.getParameter<std::string>("rpcAMCLinkCountersAnalyser"));
    fed_type_count_token_
        = consumes<MergeTH2D, edm::InRun>(edm::InputTag(_analyser, "FEDTypeCount"));
    fed_amc_type_count_token_
        = consumes<MergeMap<::uint32_t, MergeTH2D>, edm::InRun>(edm::InputTag(_analyser, "FEDAMCTypeCount"));
    fed_amc_input_count_token_
        = consumes<MergeMap<::uint32_t, MergeTH2D>, edm::InRun>(edm::InputTag(_analyser, "FEDAMCInputCount"));
    amc_input_type_count_token_
        = consumes<MergeMap<::uint32_t, MergeTH2D>, edm::InRun>(edm::InputTag(_analyser, "AMCInputTypeCount"));
}

void RPCAMCLinkCountersPlotter::fillDescriptions(edm::ConfigurationDescriptions & _descs)
{
    edm::ParameterSetDescription _desc;

    _desc.add<std::string>("rpcAMCLinkCountersAnalyser", "RPCAMCLinkCountersAnalyser");

    _desc.add<std::string>("pdfOutput", "RPCAMCLinkCountersAnalyser");
    _desc.add<bool>("createPDF", false);
    _desc.add<std::string>("titleNote" ,"CMS Preliminary");
    _desc.add<bool>("showRun", true);
    _desc.add<bool>("setLogz", true);
    _desc.add<bool>("useDivergingPalette", false);

    _descs.add("RPCAMCLinkCountersPlotter", _desc);
}

void RPCAMCLinkCountersPlotter::beginJob()
{}

void RPCAMCLinkCountersPlotter::beginRun(edm::Run const &, edm::EventSetup const &)
{}

void RPCAMCLinkCountersPlotter::analyze(edm::Event const &, edm::EventSetup const &)
{}

void RPCAMCLinkCountersPlotter::endRun(edm::Run const & _run, edm::EventSetup const &)
{
    edm::Handle<MergeTH2D> _fed_type_count;
    edm::Handle<MergeMap<::uint32_t, MergeTH2D> > _fed_amc_type_count
        , _fed_amc_input_count
        , _amc_input_type_count;

    _run.getByToken(fed_type_count_token_, _fed_type_count);
    _run.getByToken(fed_amc_type_count_token_, _fed_amc_type_count);
    _run.getByToken(fed_amc_input_count_token_, _fed_amc_input_count);
    _run.getByToken(amc_input_type_count_token_, _amc_input_type_count);

    if (first_run_) {
        fed_type_count_ = *_fed_type_count;
        fed_amc_type_count_ = *_fed_amc_type_count;
        fed_amc_input_count_ = *_fed_amc_input_count;
        amc_input_type_count_ = *_amc_input_type_count;

        if (show_run_) {
            title_note_ << (title_note_.str().empty() ? "" : " - ")
                        << "run " << _run.run();
        }

        first_run_ = false;
    } else {
        fed_type_count_.mergeProduct(*_fed_type_count);
        fed_amc_type_count_.mergeProduct(*_fed_amc_type_count);
        fed_amc_input_count_.mergeProduct(*_fed_amc_input_count);
        amc_input_type_count_.mergeProduct(*_amc_input_type_count);

        if (show_run_) {
            title_note_ << ", " << _run.run();
        }
    }
}

void RPCAMCLinkCountersPlotter::endJob()
{
    TH2D * _fed_type_count;
    std::map<::uint32_t, TH2D *> _fed_amc_type_count
        , _fed_amc_input_count
        , _amc_input_type_count;

    unsigned int _n_events(fed_type_count_->GetBinContent(1, 1));

    {
        TH2D * _th2d = fileservice_->make<TH2D>(*fed_type_count_);
        _th2d->UseCurrentStyle();
        int _nxbins = _th2d->GetXaxis()->GetNbins();
        _th2d->GetXaxis()->Set(_nxbins, 0., 1.);
        _th2d->GetXaxis()->SetNdivisions(-_nxbins);
        int _nybins = _th2d->GetYaxis()->GetNbins();
        _th2d->GetYaxis()->Set(_nybins, 0., 1.);
        _th2d->GetYaxis()->SetNdivisions(-_nybins);
        _fed_type_count = _th2d;
    }

    TFileDirectory _fed_amc_folder(fileservice_->mkdir("FED_AMC"));
    for (MergeMap<::uint32_t, MergeTH2D>::map_type::const_iterator _group = fed_amc_type_count_->begin()
             ; _group != fed_amc_type_count_->end() ; ++_group) {
        TH2D * _th2d = _fed_amc_folder.make<TH2D>(*_group->second);
        _th2d->UseCurrentStyle();
        int _nxbins = _th2d->GetXaxis()->GetNbins();
        _th2d->GetXaxis()->Set(_nxbins, 0., 1.);
        _th2d->GetXaxis()->SetNdivisions(-_nxbins);
        int _nybins = _th2d->GetYaxis()->GetNbins();
        _th2d->GetYaxis()->Set(_nybins, 0., 1.);
        _th2d->GetYaxis()->SetNdivisions(-_nybins);
        _fed_amc_type_count[_group->first] = _th2d;
    }

    TFileDirectory _fed_input_folder(fileservice_->mkdir("FED_Input"));
    for (MergeMap<::uint32_t, MergeTH2D>::map_type::const_iterator _group = fed_amc_input_count_->begin()
             ; _group != fed_amc_input_count_->end() ; ++_group) {
        TH2D * _th2d = _fed_input_folder.make<TH2D>(*_group->second);
        _th2d->UseCurrentStyle();
        int _nxbins = _th2d->GetXaxis()->GetNbins();
        _th2d->GetXaxis()->Set(_nxbins, 0., 1.);
        _th2d->GetXaxis()->SetNdivisions(-_nxbins);
        int _nybins = _th2d->GetYaxis()->GetNbins();
        _th2d->GetYaxis()->Set(_nybins, 0., 1.);
        _th2d->GetYaxis()->SetNdivisions(-_nybins);
        _fed_amc_input_count[_group->first] = _th2d;
    }

    TFileDirectory _amc_folder(fileservice_->mkdir("AMC"));
    for (MergeMap<::uint32_t, MergeTH2D>::map_type::const_iterator _group = amc_input_type_count_->begin()
             ; _group != amc_input_type_count_->end() ; ++_group) {
        TH2D * _th2d = _amc_folder.make<TH2D>(*_group->second);
        _th2d->UseCurrentStyle();
        int _nxbins = _th2d->GetXaxis()->GetNbins();
        _th2d->GetXaxis()->Set(_nxbins, 0., 1.);
        _th2d->GetXaxis()->SetNdivisions(-_nxbins);
        int _nybins = _th2d->GetYaxis()->GetNbins();
        _th2d->GetYaxis()->Set(_nybins, 0., 1.);
        _th2d->GetYaxis()->SetNdivisions(-_nybins);
        _amc_input_type_count[_group->first] = _th2d;
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

    { // _fed_type_count
        TCanvas _canvas(_fed_type_count->GetName());

        _canvas.ResetBit(kMustCleanup);
        _canvas.ResetBit(kCanDelete);
        _canvas.SetTicks(1, 1);
        _canvas.SetLeftMargin(.2);
        _canvas.SetRightMargin(.15);
        // _canvas.SetBottomMargin(.15);
        if (set_logz_) {
            _canvas.SetLogz();
        }
        TH1 * _temp_th2d = _fed_type_count->DrawCopy("colztext");
        _temp_th2d->SetTitle("");
        _temp_th2d->SetStats(false);
        // _temp_th2d->GetXaxis()->SetTitleOffset(2.);
        // _temp_th2d->GetXaxis()->SetLabelSize(.02);
        // _temp_th2d->GetXaxis()->LabelsOption("d"); // "d" draw labels down (start of label left adjusted)
        _temp_th2d->GetYaxis()->SetTitle("");
        // _temp_th2d->GetYaxis()->SetTitleOffset(2.);
        // _temp_th2d->GetYaxis()->SetLabelSize(.02);
        _temp_th2d->GetZaxis()->SetTitle("Count");
        if (set_logz_) {
            _temp_th2d->SetMinimum(.9);
        }
        _temp_th2d->SetMaximum(_n_events);
        _title.SetText(0.2, 0.91, _fed_type_count->GetTitle());
        _title.Draw();
        _title_note.Draw();
        _canvas.Print(output_pdf_.c_str());
    }

    // _fed_amc_type_count
    for (std::map<::uint32_t, TH2D *>::const_iterator _group = _fed_amc_type_count.begin()
             ; _group != _fed_amc_type_count.end() ; ++_group) {
        TCanvas _canvas(_group->second->GetName());

        _canvas.ResetBit(kMustCleanup);
        _canvas.ResetBit(kCanDelete);
        _canvas.SetTicks(1, 1);
        _canvas.SetLeftMargin(.2);
        _canvas.SetRightMargin(.15);
        // _canvas.SetBottomMargin(.15);
        if (set_logz_) {
            _canvas.SetLogz();
        }
        TH1 * _temp_th2d = _group->second->DrawCopy("colztext");
        _temp_th2d->SetTitle("");
        _temp_th2d->SetStats(false);
        // _temp_th2d->GetXaxis()->SetTitleOffset(2.);
        // _temp_th2d->GetXaxis()->SetLabelSize(.02);
        // _temp_th2d->GetXaxis()->LabelsOption("d"); // "d" draw labels down (start of label left adjusted)
        _temp_th2d->GetYaxis()->SetTitle("");
        // _temp_th2d->GetYaxis()->SetTitleOffset(2.);
        // _temp_th2d->GetYaxis()->SetLabelSize(.02);
        _temp_th2d->GetZaxis()->SetTitle("Count");
        if (set_logz_) {
            _temp_th2d->SetMinimum(.9);
        }
        _temp_th2d->SetMaximum(_n_events);
        _title.SetText(0.2, 0.91, _group->second->GetTitle());
        _title.Draw();
        _title_note.Draw();
        _canvas.Print(output_pdf_.c_str());
    }

    // _fed_amc_input_count
    for (std::map<::uint32_t, TH2D *>::const_iterator _group = _fed_amc_input_count.begin()
             ; _group != _fed_amc_input_count.end() ; ++_group) {
        TCanvas _canvas(_group->second->GetName());

        _canvas.ResetBit(kMustCleanup);
        _canvas.ResetBit(kCanDelete);
        _canvas.SetTicks(1, 1);
        // _canvas.SetLeftMargin(.2);
        _canvas.SetRightMargin(.15);
        // _canvas.SetBottomMargin(.15);
        if (set_logz_) {
            _canvas.SetLogz();
        }
        TH1 * _temp_th2d = _group->second->DrawCopy("colztext");
        _temp_th2d->SetTitle("");
        _temp_th2d->SetStats(false);
        // _temp_th2d->GetXaxis()->SetTitleOffset(2.);
        // _temp_th2d->GetXaxis()->SetLabelSize(.02);
        // _temp_th2d->GetXaxis()->LabelsOption("d"); // "d" draw labels down (start of label left adjusted)
        // _temp_th2d->GetYaxis()->SetTitle("");
        // _temp_th2d->GetYaxis()->SetTitleOffset(2.);
        // _temp_th2d->GetYaxis()->SetLabelSize(.02);
        _temp_th2d->GetZaxis()->SetTitle("Count");
        if (set_logz_) {
            _temp_th2d->SetMinimum(.9);
        }
        _temp_th2d->SetMaximum(_n_events);
        _title.SetText(0.1, 0.91, _group->second->GetTitle());
        _title.Draw();
        _title_note.Draw();
        _canvas.Print(output_pdf_.c_str());
    }

    // _amc_input_type_count
    for (std::map<::uint32_t, TH2D *>::const_iterator _group = _amc_input_type_count.begin()
             ; _group != _amc_input_type_count.end() ; ++_group) {
        TCanvas _canvas(_group->second->GetName());

        _canvas.ResetBit(kMustCleanup);
        _canvas.ResetBit(kCanDelete);
        _canvas.SetTicks(1, 1);
        _canvas.SetLeftMargin(.2);
        _canvas.SetRightMargin(.15);
        // _canvas.SetBottomMargin(.15);
        if (set_logz_) {
            _canvas.SetLogz();
        }
        TH1 * _temp_th2d = _group->second->DrawCopy("colztext");
        _temp_th2d->SetTitle("");
        _temp_th2d->SetStats(false);
        // _temp_th2d->GetXaxis()->SetTitleOffset(2.);
        // _temp_th2d->GetXaxis()->SetLabelSize(.02);
        // _temp_th2d->GetXaxis()->LabelsOption("d"); // "d" draw labels down (start of label left adjusted)
        _temp_th2d->GetYaxis()->SetTitle("");
        // _temp_th2d->GetYaxis()->SetTitleOffset(2.);
        // _temp_th2d->GetYaxis()->SetLabelSize(.02);
        _temp_th2d->GetZaxis()->SetTitle("Count");
        if (set_logz_) {
            _temp_th2d->SetMinimum(.9);
        }
        _temp_th2d->SetMaximum(_n_events);
        _title.SetText(0.2, 0.91, _group->second->GetTitle());
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
DEFINE_FWK_MODULE(RPCAMCLinkCountersPlotter);
