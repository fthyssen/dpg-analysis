#include "DPGAnalysis/RPC/plugins/RPCAMCLinkCountersAnalyser.h"

#include <memory>
#include <algorithm>

#include <set>
#include <cmath>

#include "TAxis.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Common/interface/Handle.h"

// #include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "CondFormats/DataRecord/interface/RPCTwinMuxLinkMapRcd.h"
#include "CondFormats/RPCObjects/interface/RPCAMCLink.h"
#include "CondFormats/RPCObjects/interface/RPCAMCLinkMap.h"

void RPCAMCLinkCountersAnalyserData::initialise(RPCAMCLinkCountersAnalyserRunCache const & _run_cache)
{
    fed_amc_type_count_->clear();
    fed_amc_input_count_->clear();
    amc_input_type_count_->clear();

    fed_type_count_ = MergeTH2D("FEDCounters", "FED Counters", _run_cache.fed_axis_, _run_cache.fed_type_axis_);

    std::map<RPCAMCLink, TAxis> const & _fed_amcs = _run_cache.fed_amc_axis_;
    for (std::map<RPCAMCLink, TAxis>::const_iterator _it = _fed_amcs.begin()
             ; _it != _fed_amcs.end() ; ++_it) {
        std::ostringstream _name_ss;
        _name_ss << "FED_" << _it->first.getFED();
        std::string _name(_name_ss.str());
        std::string _title(_name);
        std::replace(_title.begin(), _title.end(), '_', ' ');
        _name += "_AMC";
        _title += " AMC Counters";

        fed_amc_type_count_->insert(std::pair<::uint32_t, MergeTH2D>(_it->first.getId()
                                                                     , MergeTH2D(_name.c_str(), _title.c_str()
                                                                                 , _it->second, _run_cache.amc_type_axis_)));
    }

    std::map<RPCAMCLink, TAxis> const & _fed_inputs = _run_cache.fed_input_axis_;
    for (std::map<RPCAMCLink, TAxis>::const_iterator _it = _fed_inputs.begin()
             ; _it != _fed_inputs.end() ; ++_it) {
        std::ostringstream _name_ss;
        _name_ss << "FED_" << _it->first.getFED();
        std::string _name(_name_ss.str());
        std::string _title(_name);
        std::replace(_title.begin(), _title.end(), '_', ' ');
        _name += "_Input";
        _title += " Input Counters";

        fed_amc_input_count_->insert(std::pair<::uint32_t, MergeTH2D>(_it->first.getId()
                                                                      , MergeTH2D(_name.c_str(), _title.c_str()
                                                                                  , _fed_amcs.at(_it->first), _it->second)));
    }

    std::map<RPCAMCLink, TAxis> const & _amc_inputs = _run_cache.amc_input_axis_;
    for (std::map<RPCAMCLink, TAxis>::const_iterator _it = _amc_inputs.begin()
             ; _it != _amc_inputs.end() ; ++_it) {
        std::ostringstream _name_ss;
        _name_ss << "FED_" << _it->first.getFED() << '/' << "AMC_" << _it->first.getAMCNumber();
        std::string _name(_name_ss.str());
        std::string _title(_name);
        std::replace(_title.begin(), _title.end(), '_', ' ');
        std::replace(_title.begin(), _title.end(), '/', ' ');
        _title += " Input Counters";

        amc_input_type_count_->insert(std::pair<::uint32_t, MergeTH2D>(_it->first.getId()
                                                                       , MergeTH2D(_name.c_str(), _title.c_str()
                                                                                   , _it->second, _run_cache.input_type_axis_)));
    }
}

RPCAMCLinkCountersAnalyser::RPCAMCLinkCountersAnalyser(edm::ParameterSet const & _config)
{
    counters_token_ = consumes<RPCAMCLinkCounters>(_config.getParameter<edm::InputTag>("RPCAMCLinkCounters"));

    produces<MergeTH2D, edm::InRun>("FEDTypeCount");
    produces<MergeMap<::uint32_t, MergeTH2D>, edm::InRun>("FEDAMCTypeCount");
    produces<MergeMap<::uint32_t, MergeTH2D>, edm::InRun>("FEDAMCInputCount");
    produces<MergeMap<::uint32_t, MergeTH2D>, edm::InRun>("AMCInputTypeCount");
}

void RPCAMCLinkCountersAnalyser::fillDescriptions(edm::ConfigurationDescriptions & _descs)
{
    edm::ParameterSetDescription _desc;

    _desc.add<edm::InputTag>("RPCAMCLinkCounters", edm::InputTag("RPCTwinMuxRawToDigi", ""));

    _descs.add("RPCAMCLinkCountersAnalyser", _desc);
}

std::shared_ptr<RPCAMCLinkCountersAnalyserRunCache> RPCAMCLinkCountersAnalyser::globalBeginRun(edm::Run const & _run, edm::EventSetup const & _setup
                                                                                               , GlobalCache const *)
{
    // Get the list of links
    std::set<int> _feds;
    std::map<RPCAMCLink, std::set<int> > _fed_amcs;
    std::map<RPCAMCLink, std::set<int> > _amc_inputs;
    std::map<RPCAMCLink, std::set<int> > _fed_inputs;

    edm::ESHandle<RPCAMCLinkMap> _es_rpc_tm_link_map;
    _setup.get<RPCTwinMuxLinkMapRcd>().get(_es_rpc_tm_link_map);

    for (RPCAMCLinkMap::map_type::const_iterator _tm_link = _es_rpc_tm_link_map->getMap().begin()
             ; _tm_link != _es_rpc_tm_link_map->getMap().end() ; ++_tm_link) {
        RPCAMCLink const & _input(_tm_link->first);
        RPCAMCLink _amc(_input);
        _amc.setAMCInput();
        RPCAMCLink _fed(_amc);
        _fed.setAMCNumber();

        _feds.insert(_input.getFED());
        _fed_amcs[_fed].insert(_input.getAMCNumber());
        _amc_inputs[_amc].insert(_input.getAMCInput());
        _fed_inputs[_fed].insert(_input.getAMCInput());
    }

    // Fill the axes
    std::shared_ptr<RPCAMCLinkCountersAnalyserRunCache> _run_cache(new RPCAMCLinkCountersAnalyserRunCache());

    std::vector<double> _d_feds;
    _d_feds.reserve(_feds.size() + 1);
    for (int const & _fed : _feds) {
        _d_feds.push_back(_fed - .5);
    }
    _d_feds.push_back(_d_feds.back() + 1.);
    _run_cache->fed_axis_ = TAxis(_d_feds.size() - 1, &(_d_feds.front()));
    _run_cache->fed_axis_.CenterLabels(true);
    int _bin(1);
    for (std::set<int>::const_iterator _fed = _feds.begin()
             ; _fed != _feds.end() ; ++_fed, ++_bin) {
        std::ostringstream _oss;
        _oss << *_fed;
        _run_cache->fed_axis_.SetBinLabel(_bin, _oss.str().c_str());
    }
    _run_cache->fed_axis_.SetNameTitle("FED", "FED");

    for (std::map<RPCAMCLink, std::set<int> >::const_iterator _it = _fed_amcs.begin()
             ; _it != _fed_amcs.end() ; ++_it) {

        std::vector<double> _d_amcs;
        _d_amcs.reserve(_it->second.size() + 1);
        for (int const & _amc : _it->second) {
            _d_amcs.push_back(_amc - .5);
        }
        _d_amcs.push_back(_d_amcs.back() + 1.);

        TAxis & _axis = (_run_cache->fed_amc_axis_[_it->first] = TAxis(_d_amcs.size() - 1, &(_d_amcs.front())));
        _axis.CenterLabels(true);
        int _bin(1);
        for (std::set<int>::const_iterator _amc = _it->second.begin()
                 ; _amc != _it->second.end() ; ++_amc, ++_bin) {
            std::ostringstream _oss;
            _oss << *_amc;
            _axis.SetBinLabel(_bin, _oss.str().c_str());
        }
        _axis.SetNameTitle("AMC", "AMC");
    }

    for (std::map<RPCAMCLink, std::set<int> >::const_iterator _it = _amc_inputs.begin()
             ; _it != _amc_inputs.end() ; ++_it) {

        std::vector<double> _d_inputs;
        _d_inputs.reserve(_it->second.size() + 1);
        for (int const & _input : _it->second) {
            _d_inputs.push_back(_input - .5);
        }
        _d_inputs.push_back(_d_inputs.back() + 1.);

        TAxis & _axis = (_run_cache->amc_input_axis_[_it->first] = TAxis(_d_inputs.size() - 1, &(_d_inputs.front())));
        _axis.CenterLabels(true);
        int _bin(1);
        for (std::set<int>::const_iterator _input = _it->second.begin()
                 ; _input != _it->second.end() ; ++_input, ++_bin) {
            std::ostringstream _oss;
            _oss << *_input;
            _axis.SetBinLabel(_bin, _oss.str().c_str());
        }
        _axis.SetNameTitle("AMCInput", "Input");
    }

    for (std::map<RPCAMCLink, std::set<int> >::const_iterator _it = _fed_inputs.begin()
             ; _it != _fed_inputs.end() ; ++_it) {

        std::vector<double> _d_inputs;
        _d_inputs.reserve(_it->second.size() + 1);
        for (int const & _input : _it->second) {
            _d_inputs.push_back(_input - .5);
        }
        _d_inputs.push_back(_d_inputs.back() + 1.);

        TAxis & _axis = (_run_cache->fed_input_axis_[_it->first] = TAxis(_d_inputs.size() - 1, &(_d_inputs.front())));
        _axis.CenterLabels(true);
        int _bin(1);
        for (std::set<int>::const_iterator _input = _it->second.begin()
                 ; _input != _it->second.end() ; ++_input, ++_bin) {
            std::ostringstream _oss;
            _oss << *_input;
            _axis.SetBinLabel(_bin, _oss.str().c_str());
        }
        _axis.SetNameTitle("FEDInput", "Input");
    }

    {
        _run_cache->fed_type_axis_ = TAxis(RPCAMCLinkCounters::fed_max_ - RPCAMCLinkCounters::fed_min_ + 1
                                           , -.5 + RPCAMCLinkCounters::fed_min_, .5 + RPCAMCLinkCounters::fed_max_);
        _run_cache->fed_type_axis_.SetNameTitle("FEDTypes", "FED Counter");
        _run_cache->fed_type_axis_.CenterLabels(true);
        int _bin(1);
        for (unsigned int _type = RPCAMCLinkCounters::fed_min_ ; _type <= RPCAMCLinkCounters::fed_max_ ; ++_type, ++_bin) {
            _run_cache->fed_type_axis_.SetBinLabel(_bin, RPCAMCLinkCounters::getTypeName(_type).c_str());
        }
    }

    {
        _run_cache->amc_type_axis_ = TAxis(RPCAMCLinkCounters::amc_max_ - RPCAMCLinkCounters::amc_min_ + 1
                                           , -.5 + RPCAMCLinkCounters::amc_min_, .5 + RPCAMCLinkCounters::amc_max_);
        _run_cache->amc_type_axis_.SetNameTitle("AMCTypes", "AMC Counter");
        _run_cache->amc_type_axis_.CenterLabels(true);
        int _bin(1);
        for (unsigned int _type = RPCAMCLinkCounters::amc_min_ ; _type <= RPCAMCLinkCounters::amc_max_ ; ++_type, ++_bin) {
            _run_cache->amc_type_axis_.SetBinLabel(_bin, RPCAMCLinkCounters::getTypeName(_type).c_str());
        }
    }

    {
        _run_cache->input_type_axis_ = TAxis(RPCAMCLinkCounters::input_max_ - RPCAMCLinkCounters::input_min_ + 1
                                             , -.5 + RPCAMCLinkCounters::input_min_, .5 + RPCAMCLinkCounters::input_max_);
        _run_cache->input_type_axis_.SetNameTitle("InputTypes", "AMC Input Counter");
        _run_cache->input_type_axis_.CenterLabels(true);
        int _bin(1);
        for (unsigned int _type = RPCAMCLinkCounters::input_min_ ; _type <= RPCAMCLinkCounters::input_max_ ; ++_type, ++_bin) {
            _run_cache->input_type_axis_.SetBinLabel(_bin, RPCAMCLinkCounters::getTypeName(_type).c_str());
        }
    }

    return _run_cache;
}

std::shared_ptr<RPCAMCLinkCountersAnalyserData> RPCAMCLinkCountersAnalyser::globalBeginRunSummary(edm::Run const & _run, edm::EventSetup const & _setup
                                                                                                  , RunContext const * _context)
{
    std::shared_ptr<RPCAMCLinkCountersAnalyserData> _data(new RPCAMCLinkCountersAnalyserData());
    _data->initialise(*(_context->run()));
    return _data;
}

void RPCAMCLinkCountersAnalyser::beginRun(edm::Run const & _run, edm::EventSetup const & _setup)
{
    stream_data_.initialise(*(runCache()));
}

void RPCAMCLinkCountersAnalyser::produce(edm::Event & _event, edm::EventSetup const & _setup)
{
    edm::Handle<RPCAMCLinkCounters> _counters_handle;
    _event.getByToken(counters_token_, _counters_handle);

    // FED Types
    std::pair<RPCAMCLinkCounters::const_iterator, RPCAMCLinkCounters::const_iterator> _fed_counters
        = _counters_handle->getCounters(RPCAMCLinkCounters::fed_min_, RPCAMCLinkCounters::fed_max_);

    for (RPCAMCLinkCounters::const_iterator _it = _fed_counters.first
             ; _it != _fed_counters.second ; ++_it) {
        RPCAMCLink _input(_it->first.second);
        stream_data_.fed_type_count_->Fill(_input.getFED(), _it->first.first, _it->second);
    }

    // AMC Types
    std::pair<RPCAMCLinkCounters::const_iterator, RPCAMCLinkCounters::const_iterator> _amc_counters
        = _counters_handle->getCounters(RPCAMCLinkCounters::amc_min_, RPCAMCLinkCounters::amc_max_);

    for (RPCAMCLinkCounters::const_iterator _it = _amc_counters.first
             ; _it != _amc_counters.second ; ++_it) {
        RPCAMCLink _input(_it->first.second);
        RPCAMCLink _fed(_input);
        _fed.setAMCNumber();
        stream_data_.fed_amc_type_count_->at(_fed.getId())->Fill(_input.getAMCNumber(), _it->first.first, _it->second);
    }

    // Input Types
    std::pair<RPCAMCLinkCounters::const_iterator, RPCAMCLinkCounters::const_iterator> _input_counters
        = _counters_handle->getCounters(RPCAMCLinkCounters::input_min_, RPCAMCLinkCounters::input_max_);

    for (RPCAMCLinkCounters::const_iterator _it = _input_counters.first
             ; _it != _input_counters.second ; ++_it) {
        RPCAMCLink _input(_it->first.second);
        RPCAMCLink _amc(_input);
        _amc.setAMCInput();
        RPCAMCLink _fed(_amc);
        _fed.setAMCNumber();
        stream_data_.amc_input_type_count_->at(_amc.getId())->Fill(_input.getAMCInput(), _it->first.first, _it->second);
        if (_it->first.first != RPCAMCLinkCounters::input_data_ && _it->first.first != RPCAMCLinkCounters::input_eod_) {
            stream_data_.fed_amc_input_count_->at(_fed.getId())->Fill(_input.getAMCNumber(), _input.getAMCInput(), _it->second);
        }
    }
}

void RPCAMCLinkCountersAnalyser::endRunSummary(edm::Run const & _run, edm::EventSetup const & _setup
                                               , RPCAMCLinkCountersAnalyserData * _run_data) const
{
    _run_data->fed_type_count_.mergeProduct(stream_data_.fed_type_count_);
    _run_data->fed_amc_type_count_.mergeProduct(stream_data_.fed_amc_type_count_);
    _run_data->fed_amc_input_count_.mergeProduct(stream_data_.fed_amc_input_count_);
    _run_data->amc_input_type_count_.mergeProduct(stream_data_.amc_input_type_count_);
}

void RPCAMCLinkCountersAnalyser::globalEndRunProduce(edm::Run & _run, edm::EventSetup const & _setup
                                                     , RunContext const * _context
                                                     , RPCAMCLinkCountersAnalyserData const * _run_data)
{
    std::unique_ptr<MergeTH2D> _fed_type_count(new MergeTH2D(_run_data->fed_type_count_));
    std::unique_ptr<MergeMap<::uint32_t, MergeTH2D> > _fed_amc_type_count(new MergeMap<::uint32_t, MergeTH2D>(_run_data->fed_amc_type_count_));
    std::unique_ptr<MergeMap<::uint32_t, MergeTH2D> > _fed_amc_input_count(new MergeMap<::uint32_t, MergeTH2D>(_run_data->fed_amc_input_count_));
    std::unique_ptr<MergeMap<::uint32_t, MergeTH2D> > _amc_input_type_count(new MergeMap<::uint32_t, MergeTH2D>(_run_data->amc_input_type_count_));

    _run.put(std::move(_fed_type_count), "FEDTypeCount");
    _run.put(std::move(_fed_amc_type_count), "FEDAMCTypeCount");
    _run.put(std::move(_fed_amc_input_count), "FEDAMCInputCount");
    _run.put(std::move(_amc_input_type_count), "AMCInputTypeCount");
}

//define this as a module
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(RPCAMCLinkCountersAnalyser);
