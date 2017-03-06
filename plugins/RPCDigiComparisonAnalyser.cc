#include "DPGAnalysis/RPC/plugins/RPCDigiComparisonAnalyser.h"

#include <memory>
#include <algorithm>

#include <set>
#include <cmath>

#include "TAxis.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Provenance/interface/LuminosityBlockRange.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/RPCGeometry/interface/RPCGeometry.h"
#include "Geometry/RPCGeometry/interface/RPCChamber.h"
#include "Geometry/RPCGeometry/interface/RPCRoll.h"

#include "DPGAnalysis/RPC/interface/RPCMaskDetId.h"
#include "DPGAnalysis/RPC/interface/RPCMaskParser.h"
#include "DPGAnalysis/Tools/interface/associator.h"

RPCDigiDistance::RPCDigiDistance(float _weight_strip, float _weight_bx, float _weight_roll)
    : weight_strip_(_weight_strip)
    , weight_bx_(_weight_bx)
    , weight_roll_(_weight_roll)
{}

float RPCDigiDistance::operator()(std::pair<RPCDetId, RPCDigi> const & _lhs, std::pair<RPCDetId, RPCDigi> const & _rhs) const
{
    RPCMaskDetId _lhs_id(_lhs.first), _rhs_id(_rhs.first);
    return (std::fabs(_lhs.second.strip() - _rhs.second.strip()) * weight_strip_
            + std::fabs(_lhs.second.bx() - _rhs.second.bx()) * weight_bx_
            + std::fabs(_lhs_id - _rhs_id) * weight_roll_);
}

void RPCDigiDistance::setStripWeight(float _weight)
{
    weight_strip_ = _weight;
}

void RPCDigiDistance::setBXWeight(float _weight)
{
    weight_strip_ = _weight;
}

void RPCDigiDistance::setRollWeight(float _weight)
{
    weight_roll_ = _weight;
}

void RPCDigiComparisonAnalyserData::initialise(RPCDigiComparisonAnalyserGlobalCache const & _global_cache
                                               , RPCDigiComparisonAnalyserRunCache const & _run_cache)
{
    group_strip_strip_count_->clear();
    group_bx_bx_count_->clear();
    group_roll_roll_count_->clear();
    group_lumisec_roll_lhsonly_->clear();
    group_lumisec_roll_rhsonly_->clear();

    std::map<RPCMaskDetId, TAxis> const & _group_rolls = _run_cache.group_rolls_;
    for (std::map<RPCMaskDetId, TAxis>::const_iterator _it = _group_rolls.begin()
             ; _it != _group_rolls.end() ; ++_it) {
        std::string _title(_it->first.getName());
        std::string _name(_title);
        std::replace(_name.begin(), _name.end(), '/', '_');
        std::replace(_name.begin(), _name.end(), '+', 'P');
        std::replace(_name.begin(), _name.end(), '-', 'N');

        group_strip_strip_count_->insert(std::pair<::uint32_t, MergeTH2D>(_it->first.getId()
                                                                          , MergeTH2D(_name.c_str(), _title.c_str()
                                                                                      , _global_cache.lhs_strip_axis_
                                                                                      , _global_cache.rhs_strip_axis_)));
        group_bx_bx_count_->insert(std::pair<::uint32_t, MergeTH2D>(_it->first.getId()
                                                                    , MergeTH2D(_name.c_str(), _title.c_str()
                                                                                , _global_cache.lhs_bx_axis_
                                                                                , _global_cache.rhs_bx_axis_)));

        if (_global_cache.group_plots_) {
            TAxis _lhs_roll_axis(_it->second), _rhs_roll_axis(_it->second);
            _lhs_roll_axis.SetNameTitle("RPCRoll", (std::string("RPC Roll ") + _global_cache.lhs_name_).c_str());
            _rhs_roll_axis.SetNameTitle("RPCRoll", (std::string("RPC Roll ") + _global_cache.rhs_name_).c_str());

            group_roll_roll_count_->insert(std::pair<::uint32_t, MergeTH2D>(_it->first.getId()
                                                                            , MergeTH2D(_name.c_str(), _title.c_str()
                                                                                        , _lhs_roll_axis, _rhs_roll_axis)));

            std::string _lhsonly_title(_title + ": " + _global_cache.lhs_name_ + " only");
            group_lumisec_roll_lhsonly_->insert(std::pair<::uint32_t, MergeTH2D>(_it->first.getId()
                                                                                 , MergeTH2D(_name.c_str(), _lhsonly_title.c_str()
                                                                                             , _run_cache.lumisec_axis_
                                                                                             , _lhs_roll_axis)));
            std::string _rhsonly_title(_title + ": " + _global_cache.rhs_name_ + " only");
            group_lumisec_roll_rhsonly_->insert(std::pair<::uint32_t, MergeTH2D>(_it->first.getId()
                                                                                 , MergeTH2D(_name.c_str(), _rhsonly_title.c_str()
                                                                                             , _run_cache.lumisec_axis_
                                                                                             , _rhs_roll_axis)));
        }
    }

    lumisec_processed_ = MergeTH1D("LSProcessed", "Processed Lumi Sections", _run_cache.lumisec_axis_);
}

std::unique_ptr<RPCDigiComparisonAnalyserGlobalCache> RPCDigiComparisonAnalyser::initializeGlobalCache(edm::ParameterSet const & _config)
{
    RPCDigiComparisonAnalyserGlobalCache * _cache = new RPCDigiComparisonAnalyserGlobalCache();

    // lhs_name_, rhs_name_
    _cache->lhs_name_ = (_config.getParameter<std::string>("lhsDigiCollectionName"));
    _cache->rhs_name_ = (_config.getParameter<std::string>("rhsDigiCollectionName"));
    if (_cache->lhs_name_.empty()) {
        _cache->lhs_name_ = _config.getParameter<edm::InputTag>("lhsDigiCollection").label();
    }
    if (_cache->rhs_name_.empty()) {
        _cache->rhs_name_ = _config.getParameter<edm::InputTag>("rhsDigiCollection").label();
    }

    _cache->group_plots_ = (_config.getParameter<bool>("groupPlots"));

    _cache->distance_function_.setStripWeight(_config.getParameter<double>("distanceStripWeight"));
    _cache->distance_function_.setBXWeight   (_config.getParameter<double>("distanceBXWeight"));
    _cache->distance_function_.setRollWeight (_config.getParameter<double>("distanceRollWeight"));
    _cache->distance_max_ = _config.getParameter<double>("distanceMax");

    // bx_min_, bx_max_, lhs_bx_axis_, rhs_bx_axis_
    _cache->bx_min_ = _config.getParameter<int>("bxMin");
    _cache->bx_max_ = _config.getParameter<int>("bxMax");
    _cache->lhs_bx_axis_ = TAxis(_cache->bx_max_ - _cache->bx_min_ + 2, -1.5 + _cache->bx_min_, .5 + _cache->bx_max_);
    _cache->lhs_bx_axis_.SetNameTitle("BX", (std::string("Bunch Crossing ") + _cache->lhs_name_).c_str());
    _cache->rhs_bx_axis_ = _cache->lhs_bx_axis_;
    _cache->rhs_bx_axis_.SetNameTitle("BX", (std::string("Bunch Crossing ") + _cache->rhs_name_).c_str());

    // lhs_strip_axis_, rhs_strip_axis_
    _cache->lhs_strip_axis_ = TAxis(129, -.5, 128.5);
    _cache->lhs_strip_axis_.SetNameTitle("Strip", (std::string("Strip ") + _cache->lhs_name_).c_str());
    _cache->rhs_strip_axis_ = _cache->lhs_strip_axis_;
    _cache->rhs_strip_axis_.SetNameTitle("Strip", (std::string("Strip ") + _cache->rhs_name_).c_str());

    // roll_selection_
    std::vector<std::string> _roll_selection(_config.getParameter<std::vector<std::string> >("rollSelection"));
    for (std::vector<std::string>::const_iterator _roll = _roll_selection.begin()
             ; _roll != _roll_selection.end() ; ++_roll) {
        _cache->roll_selection_.push_back(RPCMaskParser::parse(*_roll));
        edm::LogInfo("RPCDigiComparisonAnalyser") << "Selected roll " << _cache->roll_selection_.back();
    }

    // run_lumirange_
    std::vector<edm::LuminosityBlockRange> _lumiranges
        = _config.getUntrackedParameter<std::vector<edm::LuminosityBlockRange> >("lumiList");
    std::map<unsigned long long, std::pair<unsigned long long, unsigned long long> >::iterator _run_lumirange;
    for (std::vector<edm::LuminosityBlockRange>::const_iterator _lumirange = _lumiranges.begin()
             ; _lumirange != _lumiranges.end() ; ++_lumirange) {
        if (_lumirange->startRun() == _lumirange->endRun()) {
            unsigned long long _run(_lumirange->startRun());
            std::pair<unsigned long long, unsigned long long> _lumirange_pair(_lumirange->startLumi(), _lumirange->endLumi());
            if ((_run_lumirange = _cache->run_lumirange_.find(_run)) == _cache->run_lumirange_.end()) {
                _cache->run_lumirange_.insert(std::make_pair(_run, _lumirange_pair));
            } else {
                if (_lumirange_pair.first < _run_lumirange->second.first) {
                    _run_lumirange->second.first = _lumirange_pair.first;
                }
                if (_lumirange_pair.second > _run_lumirange->second.second) {
                    _run_lumirange->second.second = _lumirange_pair.second;
                }
            }
        }
    }

    return std::unique_ptr<RPCDigiComparisonAnalyserGlobalCache>(_cache);
}

RPCDigiComparisonAnalyser::RPCDigiComparisonAnalyser(edm::ParameterSet const & _config, RPCDigiComparisonAnalyserGlobalCache const *)
{
    lhs_digis_token_ = consumes<RPCDigiCollection>(_config.getParameter<edm::InputTag>("lhsDigiCollection"));
    rhs_digis_token_ = consumes<RPCDigiCollection>(_config.getParameter<edm::InputTag>("rhsDigiCollection"));

    produces<MergeMap<::uint32_t, MergeTH2D>, edm::InRun>("GroupStripStripCount");
    produces<MergeMap<::uint32_t, MergeTH2D>, edm::InRun>("GroupBXBXCount");

    produces<MergeMap<::uint32_t, MergeTH2D>, edm::InRun>("GroupRollRollCount");
    produces<MergeMap<::uint32_t, MergeTH2D>, edm::InRun>("GroupLSRollLHSOnly");
    produces<MergeMap<::uint32_t, MergeTH2D>, edm::InRun>("GroupLSRollRHSOnly");
    produces<MergeTH1D, edm::InRun>("LSProcessed");
}

void RPCDigiComparisonAnalyser::fillDescriptions(edm::ConfigurationDescriptions & _descs)
{
    edm::ParameterSetDescription _desc;

    _desc.add<edm::InputTag>("lhsDigiCollection", edm::InputTag("rpcunpacker", ""));
    _desc.add<edm::InputTag>("rhsDigiCollection", edm::InputTag("RPCDCCRawToDigi", ""));

    _desc.add<std::string>("lhsDigiCollectionName", std::string(""));
    _desc.add<std::string>("rhsDigiCollectionName", std::string(""));

    _desc.add<bool>("groupPlots", true);

    _desc.add<double>("distanceStripWeight", .005); // any distance
    _desc.add<double>("distanceBXWeight"   , .1);   // any distance
    _desc.add<double>("distanceRollWeight" , 1. / RPCMaskDetId::mask_sector_); // same region-station-layer-ring
    _desc.add<double>("distanceMax"        , 1.);

    _desc.add<int>("bxMin", -2);
    _desc.add<int>("bxMax", 2);

    _desc.add<std::vector<std::string> >("rollSelection", std::vector<std::string>());

    _desc.addUntracked<std::vector<edm::LuminosityBlockRange> >("lumiList", std::vector<edm::LuminosityBlockRange>());

    _descs.add("RPCDigiComparisonAnalyser", _desc);
}

std::shared_ptr<RPCDigiComparisonAnalyserRunCache> RPCDigiComparisonAnalyser::globalBeginRun(edm::Run const & _run, edm::EventSetup const & _setup
                                                                                             , GlobalCache const * _cache)
{
    // Get the list of groups and their rolls
    std::map<RPCMaskDetId, std::vector<RPCMaskDetId> > _group_rolls;

    edm::ESHandle<RPCGeometry> _es_rpc_geom;
    _setup.get<MuonGeometryRecord>().get(_es_rpc_geom);
    std::vector<RPCRoll const *> const & _rolls = (_es_rpc_geom->rolls());
    for (std::vector<RPCRoll const *>::const_iterator _roll = _rolls.begin()
             ; _roll != _rolls.end() ; ++_roll) {
        RPCMaskDetId _maskid((*_roll)->id());

        bool _keep(_cache->roll_selection_.empty());
        for (std::vector<RPCMaskDetId>::const_iterator _mask = _cache->roll_selection_.begin()
                 ; !_keep && _mask != _cache->roll_selection_.end() ; ++_mask) {
            _keep = _maskid.matches(*_mask);
        }
        if (!_keep) {
            continue;
        }

        RPCMaskDetId _group(_maskid);
        if (_cache->group_plots_) {
            _group.setSector().setSubSector().setSubSubSector().setRoll().setGap();
        }
        _group_rolls[_group].push_back(_maskid);
    }

    // fill the axes
    std::shared_ptr<RPCDigiComparisonAnalyserRunCache> _run_cache(new RPCDigiComparisonAnalyserRunCache());

    for (std::map<RPCMaskDetId, std::vector<RPCMaskDetId> >::iterator _it = _group_rolls.begin()
             ; _it != _group_rolls.end() ; ++_it) {
        _it->second.push_back(RPCMaskDetId(-1, 1, 1, -2, 1, 0, 0, 1, 1)); // the smallest id
        std::sort(_it->second.begin(), _it->second.end());
        RPCMaskDetId _next(_it->second.back());
        if (_next.getRoll() < RPCMaskDetId::max_roll_) {
            _next.setRoll(_next.getRoll() + 1);
        } else {
            _next.setRoll();
        }
        _it->second.push_back(_next);
        std::vector<double> _ranges(_it->second.begin(), _it->second.end());
        _ranges.push_back(RPCMaskDetId(1, 4, 2, 3, 12, 2, 2, 5, 2).getId());
        _ranges.insert(_ranges.begin(), -1.);

        TAxis _roll_axis(_ranges.size() - 1, &(_ranges.front()));
        _roll_axis.SetNameTitle("RPCRoll", "RPC Roll");
        _roll_axis.CenterLabels(true);

        _roll_axis.SetBinLabel(1, "unmatched");
        _roll_axis.SetBinLabel(2, "mismatched elsewhere");
        _roll_axis.SetBinLabel(_ranges.size() - 1, "mismatched elsewhere");
        int _bin(3);
        std::vector<RPCMaskDetId>::const_iterator _end = _it->second.end() - 1;
        for (std::vector<RPCMaskDetId>::const_iterator _id = _it->second.begin() + 1
                 ; _id != _end ; ++_id, ++_bin) {
            _roll_axis.SetBinLabel(_bin, _id->getName().c_str());
        }

        _run_cache->group_rolls_.insert(std::pair<RPCMaskDetId, TAxis>(_it->first, _roll_axis));
    }

    // lumi sections axis
    std::pair<unsigned long long, unsigned long long> _lumirange(0, 10000);
    std::map<unsigned long long, std::pair<unsigned long long, unsigned long long> >::const_iterator _lumirange_it;
    if ((_lumirange_it = _cache->run_lumirange_.find(_run.run())) != _cache->run_lumirange_.end()) {
        _lumirange = _lumirange_it->second;
    }
    _run_cache->lumisec_axis_ = TAxis((_lumirange.second - _lumirange.first) + 1, -.5 + _lumirange.first, .5 + _lumirange.second);
    _run_cache->lumisec_axis_.SetNameTitle("LumiSection", "LumiSection");

    return _run_cache;
}

std::shared_ptr<RPCDigiComparisonAnalyserData> RPCDigiComparisonAnalyser::globalBeginRunSummary(edm::Run const & _run, edm::EventSetup const & _setup
                                                                                                , RunContext const * _context)
{
    std::shared_ptr<RPCDigiComparisonAnalyserData> _data(new RPCDigiComparisonAnalyserData());
    _data->initialise(*(_context->global()), *(_context->run()));
    return _data;
}

void RPCDigiComparisonAnalyser::beginRun(edm::Run const & _run, edm::EventSetup const & _setup)
{
    stream_data_.initialise(*(globalCache()), *(runCache()));
}

void RPCDigiComparisonAnalyser::beginLuminosityBlock(edm::LuminosityBlock const & _lumi, edm::EventSetup const & _setup)
{
    stream_data_.lumisec_processed_->Fill(_lumi.luminosityBlock());
}

void RPCDigiComparisonAnalyser::produce(edm::Event & _event, edm::EventSetup const & _setup)
{
    edm::Handle<RPCDigiCollection> _lhs_digis_handle, _rhs_digis_handle;
    _event.getByToken(lhs_digis_token_, _lhs_digis_handle);
    _event.getByToken(rhs_digis_token_, _rhs_digis_handle);

    std::set<std::pair<RPCDetId, RPCDigi> > _lhs_digis, _rhs_digis;

    unsigned int _lumi(_event.luminosityBlock());

    for (RPCDigiCollection::DigiRangeIterator _detid_digis = _lhs_digis_handle->begin()
             ; _detid_digis != _lhs_digis_handle->end() ; ++_detid_digis) {
        RPCDetId _detid((*_detid_digis).first);

        RPCMaskDetId _maskid(_detid);
        bool _keep(globalCache()->roll_selection_.empty());
        for (std::vector<RPCMaskDetId>::const_iterator _mask = globalCache()->roll_selection_.begin()
                 ; !_keep && _mask != globalCache()->roll_selection_.end() ; ++_mask) {
            _keep = _maskid.matches(*_mask);
        }
        if (!_keep) {
            continue;
        }

        RPCDigiCollection::DigiRangeIterator::DigiRangeIterator _digi_end = (*_detid_digis).second.second;
        for (RPCDigiCollection::DigiRangeIterator::DigiRangeIterator _digi = (*_detid_digis).second.first
                 ; _digi != _digi_end ; ++_digi) {
            if (_digi->bx() >= globalCache()->bx_min_ && _digi->bx() <= globalCache()->bx_max_) {
                _lhs_digis.insert(std::pair<RPCDetId, RPCDigi>(_detid, *_digi));
            }
        }
    }

    for (RPCDigiCollection::DigiRangeIterator _detid_digis = _rhs_digis_handle->begin()
             ; _detid_digis != _rhs_digis_handle->end() ; ++_detid_digis) {
        RPCDetId _detid((*_detid_digis).first);

        RPCMaskDetId _maskid(_detid);
        bool _keep(globalCache()->roll_selection_.empty());
        for (std::vector<RPCMaskDetId>::const_iterator _mask = globalCache()->roll_selection_.begin()
                 ; !_keep && _mask != globalCache()->roll_selection_.end() ; ++_mask) {
            _keep = _maskid.matches(*_mask);
        }
        if (!_keep) {
            continue;
        }

        RPCDigiCollection::DigiRangeIterator::DigiRangeIterator _digi_end = (*_detid_digis).second.second;
        for (RPCDigiCollection::DigiRangeIterator::DigiRangeIterator _digi = (*_detid_digis).second.first
                 ; _digi != _digi_end ; ++_digi) {
            if (_digi->bx() >= globalCache()->bx_min_ && _digi->bx() <= globalCache()->bx_max_) {
                _rhs_digis.insert(std::pair<RPCDetId, RPCDigi>(_detid, *_digi));
            }
        }
    }

    typedef association<std::set<std::pair<RPCDetId, RPCDigi> >, std::set<std::pair<RPCDetId, RPCDigi> > >
        association_type;
    associationvector<association_type> _associations
        = associator::associate<associationvector<association_type> >(&_lhs_digis, &_rhs_digis, globalCache()->distance_function_
                                                                      , associator::munkres_
                                                                      , globalCache()->distance_max_);

    bool _report(false);
    for (associationvector<association_type>::const_iterator _association = _associations.begin()
             ; _association != _associations.end() ; ++_association) {
        RPCMaskDetId _id(_association.key_valid() ? _association.key()->first : _association.value()->first);
        RPCMaskDetId _group(_id);
        if (globalCache()->group_plots_) {
            _group.setSector().setSubSector().setSubSubSector().setRoll().setGap();
        }

        if (_association.distance() > 0) {
            _report = true;
        }

        if (!_association.value_valid()) {
            stream_data_.group_strip_strip_count_->at(_group.getId())->Fill(_association.key()->second.strip(), 0.);
            stream_data_.group_bx_bx_count_->at(_group.getId())->Fill(_association.key()->second.bx(), globalCache()->bx_min_ - 1);
            if (globalCache()->group_plots_) {
                stream_data_.group_roll_roll_count_->at(_group.getId())->Fill(.5 + _id.getId(), 0.);
                stream_data_.group_lumisec_roll_lhsonly_->at(_group.getId())->Fill(_lumi, .5 + _id.getId());
            }
            _report = true;
        } else if (!_association.key_valid()) {
            stream_data_.group_strip_strip_count_->at(_group.getId())->Fill(0., _association.value()->second.strip());
            stream_data_.group_bx_bx_count_->at(_group.getId())->Fill(globalCache()->bx_min_ - 1, _association.value()->second.bx());
            if (globalCache()->group_plots_) {
                stream_data_.group_roll_roll_count_->at(_group.getId())->Fill(0., .5 + _id.getId());
                stream_data_.group_lumisec_roll_rhsonly_->at(_group.getId())->Fill(_lumi, .5 + _id.getId());
            }
            _report = true;
        } else {
            RPCMaskDetId _rhs(_association.value()->first);
            if (globalCache()->group_plots_) {
                stream_data_.group_roll_roll_count_->at(_group.getId())->Fill(.5 + _id.getId(), .5 + _rhs.getId());
            }
            stream_data_.group_strip_strip_count_->at(_group.getId())->Fill(_association.key()->second.strip(), _association.value()->second.strip());
            stream_data_.group_bx_bx_count_->at(_group.getId())->Fill(_association.key()->second.bx(), _association.value()->second.bx());
        }
    }
    if (_report) {
        LogDebug("RPCDigiComparisonAnalyser") << "Report difference";
    }
}

void RPCDigiComparisonAnalyser::endRunSummary(edm::Run const & _run, edm::EventSetup const & _setup
                                              , RPCDigiComparisonAnalyserData * _run_data) const
{
    _run_data->group_strip_strip_count_.mergeProduct(stream_data_.group_strip_strip_count_);
    _run_data->group_bx_bx_count_.mergeProduct(stream_data_.group_bx_bx_count_);
    _run_data->group_roll_roll_count_.mergeProduct(stream_data_.group_roll_roll_count_);
    _run_data->group_lumisec_roll_lhsonly_.mergeProduct(stream_data_.group_lumisec_roll_lhsonly_);
    _run_data->group_lumisec_roll_rhsonly_.mergeProduct(stream_data_.group_lumisec_roll_rhsonly_);
    _run_data->lumisec_processed_.mergeProduct(stream_data_.lumisec_processed_);
}

void RPCDigiComparisonAnalyser::globalEndRunProduce(edm::Run & _run, edm::EventSetup const & _setup
                                                    , RunContext const * _context
                                                    , RPCDigiComparisonAnalyserData const * _run_data)
{
    std::unique_ptr<MergeMap<::uint32_t, MergeTH2D> > _group_strip_strip_count(new MergeMap<::uint32_t, MergeTH2D>(_run_data->group_strip_strip_count_));
    std::unique_ptr<MergeMap<::uint32_t, MergeTH2D> > _group_bx_bx_count(new MergeMap<::uint32_t, MergeTH2D>(_run_data->group_bx_bx_count_));
    std::unique_ptr<MergeMap<::uint32_t, MergeTH2D> > _group_roll_roll_count(new MergeMap<::uint32_t, MergeTH2D>(_run_data->group_roll_roll_count_));
    std::unique_ptr<MergeMap<::uint32_t, MergeTH2D> > _group_lumisec_roll_lhsonly(new MergeMap<::uint32_t, MergeTH2D>(_run_data->group_lumisec_roll_lhsonly_));
    std::unique_ptr<MergeMap<::uint32_t, MergeTH2D> > _group_lumisec_roll_rhsonly(new MergeMap<::uint32_t, MergeTH2D>(_run_data->group_lumisec_roll_rhsonly_));
    std::unique_ptr<MergeTH1D> _lumisec_processed(new MergeTH1D(_run_data->lumisec_processed_));

    _run.put(std::move(_group_strip_strip_count), "GroupStripStripCount");
    _run.put(std::move(_group_bx_bx_count), "GroupBXBXCount");
    _run.put(std::move(_group_roll_roll_count), "GroupRollRollCount");
    _run.put(std::move(_group_lumisec_roll_lhsonly), "GroupLSRollLHSOnly");
    _run.put(std::move(_group_lumisec_roll_rhsonly), "GroupLSRollRHSOnly");
    _run.put(std::move(_lumisec_processed), "LSProcessed");
}

//define this as a module
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(RPCDigiComparisonAnalyser);
