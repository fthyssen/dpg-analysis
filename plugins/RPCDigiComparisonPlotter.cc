#include "DPGAnalysis/RPC/plugins/RPCDigiComparisonPlotter.h"

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

RPCDigiComparisonPlotter::RPCDigiComparisonPlotter(edm::ParameterSet const & _config)
    : first_run_(true)
    , output_pdf_(_config.getParameter<std::string>("pdfOutput") + ".pdf")
    , create_pdf_(_config.getParameter<bool>("createPDF"))
    , show_run_(_config.getParameter<bool>("showRun"))
    , set_logz_(_config.getParameter<bool>("setLogz"))
    , use_diverging_palette_(_config.getParameter<bool>("useDivergingPalette"))
    , lhs_name_(_config.getParameter<std::string>("lhsDigiCollectionName"))
    , rhs_name_(_config.getParameter<std::string>("rhsDigiCollectionName"))
{
    title_note_ << _config.getParameter<std::string>("titleNote");

    std::string _analyser(_config.getParameter<std::string>("rpcDigiComparisonAnalyser"));
    group_roll_roll_count_token_
        = consumes<MergeMap<::uint32_t, MergeTH2D>, edm::InRun>(edm::InputTag(_analyser, "GroupRollRollCount"));
    group_strip_strip_count_token_
        = consumes<MergeMap<::uint32_t, MergeTH2D>, edm::InRun>(edm::InputTag(_analyser, "GroupStripStripCount"));
    group_bx_bx_count_token_
        = consumes<MergeMap<::uint32_t, MergeTH2D>, edm::InRun>(edm::InputTag(_analyser, "GroupBXBXCount"));
    group_lumisec_roll_lhsonly_token_
        = consumes<MergeMap<::uint32_t, MergeTH2D>, edm::InRun>(edm::InputTag(_analyser, "GroupLSRollLHSOnly"));
    group_lumisec_roll_rhsonly_token_
        = consumes<MergeMap<::uint32_t, MergeTH2D>, edm::InRun>(edm::InputTag(_analyser, "GroupLSRollRHSOnly"));
    lumisec_processed_token_
        = consumes<MergeTH1D, edm::InRun>(edm::InputTag(_analyser, "LSProcessed"));
}

void RPCDigiComparisonPlotter::fillDescriptions(edm::ConfigurationDescriptions & _descs)
{
    edm::ParameterSetDescription _desc;

    _desc.add<std::string>("rpcDigiComparisonAnalyser", "RPCDigiComparisonAnalyser");

    _desc.add<std::string>("pdfOutput", "RPCDigiComparisonAnalyser");
    _desc.add<bool>("createPDF", false);
    _desc.add<std::string>("titleNote" ,"CMS Preliminary");
    _desc.add<bool>("showRun", true);
    _desc.add<bool>("setLogz", true);
    _desc.add<bool>("useDivergingPalette", true);
    _desc.add<std::string>("lhsDigiCollectionName", std::string(""));
    _desc.add<std::string>("rhsDigiCollectionName", std::string(""));

    _descs.add("RPCDigiComparisonPlotter", _desc);
}

void RPCDigiComparisonPlotter::beginJob()
{}

void RPCDigiComparisonPlotter::beginRun(edm::Run const &, edm::EventSetup const &)
{}

void RPCDigiComparisonPlotter::analyze(edm::Event const &, edm::EventSetup const &)
{}

void RPCDigiComparisonPlotter::endRun(edm::Run const & _run, edm::EventSetup const &)
{
    edm::Handle<MergeMap<::uint32_t, MergeTH2D> > _group_roll_roll_count
        , _group_strip_strip_count
        , _group_bx_bx_count
        , _group_lumisec_roll_lhsonly
        , _group_lumisec_roll_rhsonly;
    edm::Handle<MergeTH1D> _lumisec_processed;

    _run.getByToken(group_roll_roll_count_token_, _group_roll_roll_count);
    _run.getByToken(group_strip_strip_count_token_, _group_strip_strip_count);
    _run.getByToken(group_bx_bx_count_token_, _group_bx_bx_count);
    _run.getByToken(group_lumisec_roll_lhsonly_token_, _group_lumisec_roll_lhsonly);
    _run.getByToken(group_lumisec_roll_rhsonly_token_, _group_lumisec_roll_rhsonly);
    _run.getByToken(lumisec_processed_token_, _lumisec_processed);

    if (first_run_) {
        group_roll_roll_count_ = *_group_roll_roll_count;
        group_strip_strip_count_ = *_group_strip_strip_count;
        group_bx_bx_count_ = *_group_bx_bx_count;
        group_lumisec_roll_lhsonly_ = *_group_lumisec_roll_lhsonly;
        group_lumisec_roll_rhsonly_ = *_group_lumisec_roll_rhsonly;
        lumisec_processed_ = *_lumisec_processed;

        if (show_run_) {
            title_note_ << (title_note_.str().empty() ? "" : " - ")
                        << "run " << _run.run();
        }

        first_run_ = false;
    } else {
        group_roll_roll_count_.mergeProduct(*_group_roll_roll_count);
        group_strip_strip_count_.mergeProduct(*_group_strip_strip_count);
        group_bx_bx_count_.mergeProduct(*_group_bx_bx_count);
        group_lumisec_roll_lhsonly_.mergeProduct(*_group_lumisec_roll_lhsonly);
        group_lumisec_roll_rhsonly_.mergeProduct(*_group_lumisec_roll_rhsonly);
        lumisec_processed_.mergeProduct(*_lumisec_processed);

        if (show_run_) {
            title_note_ << ", " << _run.run();
        }
    }
}

void RPCDigiComparisonPlotter::endJob()
{
    std::map<::uint32_t, TH2D *> _group_roll_roll_count
        , _group_strip_strip_count
        , _group_bx_bx_count
        , _group_lumisec_roll_lhsonly
        , _group_lumisec_roll_rhsonly;
    TH1D * _lumisec_processed;

    TFileDirectory _roll_folder(fileservice_->mkdir("Roll"));
    for (MergeMap<::uint32_t, MergeTH2D>::map_type::const_iterator _group = group_roll_roll_count_->begin()
             ; _group != group_roll_roll_count_->end() ; ++_group) {
        TH2D * _th2d = _roll_folder.make<TH2D>(*_group->second);
        _th2d->UseCurrentStyle();
        int _nbins = _th2d->GetXaxis()->GetNbins();
        _th2d->GetXaxis()->Set(_nbins, 0., 1.);
        _th2d->GetXaxis()->SetNdivisions(-_nbins);
        _th2d->GetYaxis()->Set(_nbins, 0., 1.);
        _th2d->GetYaxis()->SetNdivisions(-_nbins);
        _group_roll_roll_count[_group->first] = _th2d;
    }

    TFileDirectory _strip_folder(fileservice_->mkdir("Strip"));
    for (MergeMap<::uint32_t, MergeTH2D>::map_type::const_iterator _group = group_strip_strip_count_->begin()
             ; _group != group_strip_strip_count_->end() ; ++_group) {
        TH2D * _th2d = _strip_folder.make<TH2D>(*_group->second);
        _group_strip_strip_count[_group->first] = _th2d;
        _th2d->UseCurrentStyle();
    }

    TFileDirectory _bx_folder(fileservice_->mkdir("BX"));
    for (MergeMap<::uint32_t, MergeTH2D>::map_type::const_iterator _group = group_bx_bx_count_->begin()
             ; _group != group_bx_bx_count_->end() ; ++_group) {
        TH2D * _th2d = _bx_folder.make<TH2D>(*_group->second);
        _group_bx_bx_count[_group->first] = _th2d;
        _th2d->UseCurrentStyle();
    }

    TFileDirectory _lhsonly_folder(fileservice_->mkdir("LHSOnly"));
    for (MergeMap<::uint32_t, MergeTH2D>::map_type::const_iterator _group = group_lumisec_roll_lhsonly_->begin()
             ; _group != group_lumisec_roll_lhsonly_->end() ; ++_group) {
        TH2D * _th2d = _lhsonly_folder.make<TH2D>(*_group->second);
        _th2d->UseCurrentStyle();
        int _nbins = _th2d->GetYaxis()->GetNbins();
        _th2d->GetYaxis()->Set(_nbins, 0., 1.);
        _th2d->GetYaxis()->SetNdivisions(-_nbins);
        _group_lumisec_roll_lhsonly[_group->first] = _th2d;
    }

    TFileDirectory _rhsonly_folder(fileservice_->mkdir("RHSOnly"));
    for (MergeMap<::uint32_t, MergeTH2D>::map_type::const_iterator _group = group_lumisec_roll_rhsonly_->begin()
             ; _group != group_lumisec_roll_rhsonly_->end() ; ++_group) {
        TH2D * _th2d = _rhsonly_folder.make<TH2D>(*_group->second);
        _th2d->UseCurrentStyle();
        int _nbins = _th2d->GetYaxis()->GetNbins();
        _th2d->GetYaxis()->Set(_nbins, 0., 1.);
        _th2d->GetYaxis()->SetNdivisions(-_nbins);
        _group_lumisec_roll_rhsonly[_group->first] = _th2d;
    }

    _lumisec_processed = fileservice_->make<TH1D>(*lumisec_processed_);

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

    for (std::map<::uint32_t, TH2D *>::const_iterator _group = _group_roll_roll_count.begin()
             ; _group != _group_roll_roll_count.end() ; ++_group) {
        TCanvas _canvas(_group->second->GetName());
        _canvas.ResetBit(kMustCleanup);
        _canvas.ResetBit(kCanDelete);
        _canvas.SetTicks(1, 1);
        _canvas.SetLeftMargin(.15);
        _canvas.SetRightMargin(.15);
        _canvas.SetBottomMargin(.15);
        if (set_logz_) {
            _canvas.SetLogz();
        }
        TH1 * _temp_th2d = _group->second->DrawCopy("colz");
        _temp_th2d->SetTitle("");
        _temp_th2d->SetStats(false);
        if (!lhs_name_.empty()) {
            _temp_th2d->GetXaxis()->SetTitle((std::string("RPC Roll ") + lhs_name_).c_str());
        }
        if (!rhs_name_.empty()) {
            _temp_th2d->GetYaxis()->SetTitle((std::string("RPC Roll ") + rhs_name_).c_str());
        }
        _temp_th2d->GetXaxis()->SetTitleOffset(2.);
        _temp_th2d->GetXaxis()->SetLabelSize(.02);
        _temp_th2d->GetXaxis()->LabelsOption("d"); // "d" draw labels down (start of label left adjusted)
        _temp_th2d->GetYaxis()->SetTitleOffset(2.);
        _temp_th2d->GetYaxis()->SetLabelSize(.02);
        _temp_th2d->GetZaxis()->SetTitle("Digis");
        { // unmatched
            std::vector<double> _x({_temp_th2d->GetXaxis()->GetXmin(), _temp_th2d->GetXaxis()->GetXmin()
                        , _temp_th2d->GetXaxis()->GetBinLowEdge(2), _temp_th2d->GetXaxis()->GetBinLowEdge(2)
                        , _temp_th2d->GetXaxis()->GetXmax(), _temp_th2d->GetXaxis()->GetXmax()
                        , _temp_th2d->GetXaxis()->GetXmin()})
                , _y({_temp_th2d->GetYaxis()->GetXmin(), _temp_th2d->GetYaxis()->GetXmax()
                            , _temp_th2d->GetYaxis()->GetXmax(), _temp_th2d->GetYaxis()->GetBinLowEdge(2)
                            , _temp_th2d->GetYaxis()->GetBinLowEdge(2), _temp_th2d->GetYaxis()->GetXmin()
                            , _temp_th2d->GetYaxis()->GetXmin()});
            TPolyLine * _unmatched = new TPolyLine(7, &(_x.front()), &(_y.front()));
            _unmatched->SetFillStyle(3254);
            _unmatched->SetFillColorAlpha(kBlack, .5);
            _unmatched->SetLineColorAlpha(kBlack, .5);
            _unmatched->Draw("f");
            _unmatched->Draw("");
        }
        unsigned int _nbins(_temp_th2d->GetXaxis()->GetNbins());
        { // mismatched fill
            std::vector<double> _x({_temp_th2d->GetXaxis()->GetBinLowEdge(3), _temp_th2d->GetXaxis()->GetBinLowEdge(3)
                        , _temp_th2d->GetXaxis()->GetBinLowEdge(_nbins), _temp_th2d->GetXaxis()->GetBinLowEdge(_nbins)
                        , _temp_th2d->GetXaxis()->GetBinLowEdge(3)
                        , _temp_th2d->GetXaxis()->GetBinLowEdge(2), _temp_th2d->GetXaxis()->GetXmax()
                        , _temp_th2d->GetXaxis()->GetXmax(), _temp_th2d->GetXaxis()->GetBinLowEdge(2)
                        , _temp_th2d->GetXaxis()->GetBinLowEdge(2)})
                , _y({_temp_th2d->GetYaxis()->GetBinLowEdge(3), _temp_th2d->GetYaxis()->GetBinLowEdge(_nbins)
                            , _temp_th2d->GetYaxis()->GetBinLowEdge(_nbins), _temp_th2d->GetYaxis()->GetBinLowEdge(3)
                            , _temp_th2d->GetYaxis()->GetBinLowEdge(3)
                            , _temp_th2d->GetYaxis()->GetBinLowEdge(2), _temp_th2d->GetYaxis()->GetBinLowEdge(2)
                            , _temp_th2d->GetYaxis()->GetXmax(), _temp_th2d->GetYaxis()->GetXmax()
                            , _temp_th2d->GetYaxis()->GetBinLowEdge(2)});
            TPolyLine * _mismatched = new TPolyLine(10, &(_x.front()), &(_y.front()));
            _mismatched->SetFillStyle(3245);
            _mismatched->SetFillColorAlpha(kBlack, .5);
            _mismatched->Draw("f");
        }
        { // mismatched line
            std::vector<double> _x({_temp_th2d->GetXaxis()->GetBinLowEdge(3), _temp_th2d->GetXaxis()->GetBinLowEdge(3)
                        , _temp_th2d->GetXaxis()->GetBinLowEdge(_nbins), _temp_th2d->GetXaxis()->GetBinLowEdge(_nbins)
                        , _temp_th2d->GetXaxis()->GetBinLowEdge(3)})
                , _y({_temp_th2d->GetYaxis()->GetBinLowEdge(3), _temp_th2d->GetYaxis()->GetBinLowEdge(_nbins)
                            , _temp_th2d->GetYaxis()->GetBinLowEdge(_nbins), _temp_th2d->GetYaxis()->GetBinLowEdge(3)
                            , _temp_th2d->GetYaxis()->GetBinLowEdge(3)});
            TPolyLine * _mismatched = new TPolyLine(5, &(_x.front()), &(_y.front()));
            _mismatched->SetLineColorAlpha(kBlack, .5);
            _mismatched->Draw("");
        }
        _title.SetText(0.15, 0.91, _group->second->GetTitle());
        _title.Draw();
        _title_note.Draw();
        _canvas.Print(output_pdf_.c_str());
    }
    for (std::map<::uint32_t, TH2D *>::const_iterator _group = _group_strip_strip_count.begin()
             ; _group != _group_strip_strip_count.end() ; ++_group) {
        TCanvas _canvas(_group->second->GetName());
        _canvas.ResetBit(kMustCleanup);
        _canvas.ResetBit(kCanDelete);
        _canvas.SetTicks(1, 1);
        _canvas.SetRightMargin(.15);
        if (set_logz_) {
            _canvas.SetLogz();
        }
        TH1 * _temp_th2d = _group->second->DrawCopy("colz");
        _temp_th2d->SetTitle("");
        _temp_th2d->SetStats(false);
        if (!lhs_name_.empty()) {
            _temp_th2d->GetXaxis()->SetTitle((std::string("Strip ") + lhs_name_).c_str());
        }
        if (!rhs_name_.empty()) {
            _temp_th2d->GetYaxis()->SetTitle((std::string("Strip ") + rhs_name_).c_str());
        }
        _temp_th2d->GetZaxis()->SetTitle("Digis");
        std::vector<double> _x({_temp_th2d->GetXaxis()->GetXmin(), _temp_th2d->GetXaxis()->GetXmin()
                    , _temp_th2d->GetXaxis()->GetBinLowEdge(2), _temp_th2d->GetXaxis()->GetBinLowEdge(2)
                    , _temp_th2d->GetXaxis()->GetXmax(), _temp_th2d->GetXaxis()->GetXmax()
                    , _temp_th2d->GetXaxis()->GetXmin()})
            , _y({_temp_th2d->GetYaxis()->GetXmin(), _temp_th2d->GetYaxis()->GetXmax()
                        , _temp_th2d->GetYaxis()->GetXmax(), _temp_th2d->GetYaxis()->GetBinLowEdge(2)
                        , _temp_th2d->GetYaxis()->GetBinLowEdge(2), _temp_th2d->GetYaxis()->GetXmin()
                        , _temp_th2d->GetYaxis()->GetXmin()});
        TPolyLine * _unmatched = new TPolyLine(7, &(_x.front()), &(_y.front()));
        _unmatched->SetFillStyle(3254);
        _unmatched->SetFillColorAlpha(kBlack, .5);
        _unmatched->SetLineColorAlpha(kBlack, .5);
        _unmatched->Draw("f");
        _unmatched->Draw("");
        _title.SetText(0.10, 0.91, _group->second->GetTitle());
        _title.Draw();
        _title_note.Draw();
        _canvas.Print(output_pdf_.c_str());
    }

    for (std::map<::uint32_t, TH2D *>::const_iterator _group = _group_bx_bx_count.begin()
             ; _group != _group_bx_bx_count.end() ; ++_group) {
        TCanvas _canvas(_group->second->GetName());
        _canvas.ResetBit(kMustCleanup);
        _canvas.ResetBit(kCanDelete);
        _canvas.SetTicks(1, 1);
        _canvas.SetRightMargin(.15);
        if (set_logz_) {
            _canvas.SetLogz();
        }
        TH1 * _temp_th2d = _group->second->DrawCopy("colztext");
        _temp_th2d->SetTitle("");
        _temp_th2d->SetStats(false);
        if (!lhs_name_.empty()) {
            _temp_th2d->GetXaxis()->SetTitle((std::string("Bunch Crossing ") + lhs_name_).c_str());
        }
        if (!rhs_name_.empty()) {
            _temp_th2d->GetYaxis()->SetTitle((std::string("Bunch Crossing ") + rhs_name_).c_str());
        }
        _temp_th2d->GetZaxis()->SetTitle("Digis");
        std::vector<double> _x({_temp_th2d->GetXaxis()->GetXmin(), _temp_th2d->GetXaxis()->GetXmin()
                    , _temp_th2d->GetXaxis()->GetBinLowEdge(2), _temp_th2d->GetXaxis()->GetBinLowEdge(2)
                    , _temp_th2d->GetXaxis()->GetXmax(), _temp_th2d->GetXaxis()->GetXmax()
                    , _temp_th2d->GetXaxis()->GetXmin()})
            , _y({_temp_th2d->GetYaxis()->GetXmin(), _temp_th2d->GetYaxis()->GetXmax()
                        , _temp_th2d->GetYaxis()->GetXmax(), _temp_th2d->GetYaxis()->GetBinLowEdge(2)
                        , _temp_th2d->GetYaxis()->GetBinLowEdge(2), _temp_th2d->GetYaxis()->GetXmin()
                        , _temp_th2d->GetYaxis()->GetXmin()});
        TPolyLine * _unmatched = new TPolyLine(7, &(_x.front()), &(_y.front()));
        _unmatched->SetFillStyle(3254);
        _unmatched->SetFillColorAlpha(kBlack, .5);
        _unmatched->SetLineColorAlpha(kBlack, .5);
        _unmatched->Draw("f");
        _unmatched->Draw("");
        _title.SetText(0.10, 0.91, _group->second->GetTitle());
        _title.Draw();
        _title_note.Draw();
        _canvas.Print(output_pdf_.c_str());
    }

    unsigned int _first_lumisec(1e9), _last_lumisec(0);
    {
        unsigned int _nbins(_lumisec_processed->GetXaxis()->GetNbins());
        for (unsigned int _bin = 0 ; _bin < _nbins; ++_bin) {
            if (_lumisec_processed->GetBinContent(_bin) > 0) {
                if (_first_lumisec > _bin) {
                    _first_lumisec = _bin;
                }
                _last_lumisec = _bin;
                _lumisec_processed->SetBinContent(_bin, 0.);
            } else {
                _lumisec_processed->SetBinContent(_bin, 1.);
            }
        }
        _lumisec_processed->GetYaxis()->SetRangeUser(0., 1.);
        _lumisec_processed->GetXaxis()->SetRange(_first_lumisec, _last_lumisec);
        _lumisec_processed->SetFillStyle(3254);
        _lumisec_processed->SetFillColorAlpha(kBlack, .5);
        _lumisec_processed->SetLineColorAlpha(kBlack, .5);
    }

    _title.SetTextSize(0.03);

    for (std::map<::uint32_t, TH2D *>::const_iterator _group = _group_lumisec_roll_lhsonly.begin()
             ; _group != _group_lumisec_roll_lhsonly.end() ; ++_group) {
        TCanvas _canvas(_group->second->GetName());
        _canvas.ResetBit(kMustCleanup);
        _canvas.ResetBit(kCanDelete);
        _canvas.SetTicks(1, 1);
        _canvas.SetLeftMargin(.15);
        _canvas.SetRightMargin(.15);
        if (set_logz_) {
            _canvas.SetLogz();
        }
        TH1 * _temp_th2d = _group->second->DrawCopy("colz");
        _temp_th2d->SetTitle("");
        _temp_th2d->SetStats(false);
        _temp_th2d->GetYaxis()->SetTitle("");
        _temp_th2d->GetYaxis()->SetLabelSize(.02);
        _temp_th2d->GetZaxis()->SetTitle("Digis");
        _temp_th2d->GetXaxis()->SetRange(_first_lumisec, _last_lumisec);
        _lumisec_processed->Draw("same");
        if (!lhs_name_.empty()) {
            _title.SetText(0.15, 0.91, (RPCMaskDetId(_group->first).getName() + ": " + lhs_name_ + " only").c_str());
        } else {
            _title.SetText(0.15, 0.91, _group->second->GetTitle());
        }
        _title.Draw();
        _title_note.Draw();
        _canvas.Print(output_pdf_.c_str());
    }

    for (std::map<::uint32_t, TH2D *>::const_iterator _group = _group_lumisec_roll_rhsonly.begin()
             ; _group != _group_lumisec_roll_rhsonly.end() ; ++_group) {
        TCanvas _canvas(_group->second->GetName());
        _canvas.ResetBit(kMustCleanup);
        _canvas.ResetBit(kCanDelete);
        _canvas.SetTicks(1, 1);
        _canvas.SetLeftMargin(.15);
        _canvas.SetRightMargin(.15);
        if (set_logz_) {
            _canvas.SetLogz();
        }
        TH1 * _temp_th2d = _group->second->DrawCopy("colz");
        _temp_th2d->SetTitle("");
        _temp_th2d->SetStats(false);
        _temp_th2d->GetYaxis()->SetTitle("");
        _temp_th2d->GetYaxis()->SetLabelSize(.02);
        _temp_th2d->GetZaxis()->SetTitle("Digis");
        _temp_th2d->GetXaxis()->SetRange(_first_lumisec, _last_lumisec);
        _lumisec_processed->Draw("same");
        if (!rhs_name_.empty()) {
            _title.SetText(0.15, 0.91, (RPCMaskDetId(_group->first).getName() + ": " + rhs_name_ + " only").c_str());
        } else {
            _title.SetText(0.15, 0.91, _group->second->GetTitle());
        }
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
DEFINE_FWK_MODULE(RPCDigiComparisonPlotter);
