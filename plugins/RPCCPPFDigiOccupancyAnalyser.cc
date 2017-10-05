#include "DPGAnalysis/RPC/plugins/RPCCPPFDigiOccupancyAnalyser.h"

#include <memory>
#include <algorithm>

#include <set>
#include <cmath>
#include <iostream>

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

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/RPCGeometry/interface/RPCGeometry.h"
#include "Geometry/RPCGeometry/interface/RPCRoll.h"

#include "DPGAnalysis/RPC/interface/RPCMaskDetId.h"
#include "DPGAnalysis/RPC/interface/RPCMaskParser.h"

void RPCCPPFDigiOccupancyAnalyserData::initialise(RPCCPPFDigiOccupancyAnalyserGlobalCache const & _global_cache
                                                  , RPCCPPFDigiOccupancyAnalyserRunCache const & _run_cache)
{
    group_theta_phi_count_->clear();
    group_ring_theta_count_->clear();
    group_sector_phi_count_->clear();

    TAxis _theta_axis = TAxis(32, -.5, 31.5);
    _theta_axis.SetNameTitle("Theta", "Binned Theta");
    TAxis _phi_axis = TAxis(2048, -.5, 2047.5);
    _phi_axis.SetNameTitle("Phi", "Binned Phi");
    TAxis _ring_axis = TAxis(2, 1.5, 3.5);
    _ring_axis.SetNameTitle("Ring", "Ring");
    TAxis _sector_axis = TAxis(36, .5, 36.5);
    _sector_axis.SetNameTitle("Sector", "Sector");

    for (auto _group : _run_cache.groups_) {
        std::string _title(_group.getName());
        std::string _name(_title);
        std::replace(_name.begin(), _name.end(), '/', '_');
        std::replace(_name.begin(), _name.end(), '+', 'P');
        std::replace(_name.begin(), _name.end(), '-', 'N');

        group_theta_phi_count_->insert(std::pair<std::uint32_t, MergeTH2D>(_group.getId()
                                                                           , MergeTH2D(_name.c_str(), _title.c_str()
                                                                                       , _theta_axis, _phi_axis)));
        std::string _ring_theta_name(_name + "_ring_theta");
        group_ring_theta_count_->insert(std::pair<std::uint32_t, MergeTH2D>(_group.getId()
                                                                            , MergeTH2D(_ring_theta_name.c_str(), _title.c_str()
                                                                                        , _ring_axis, _theta_axis)));
        std::string _sector_phi_name(_name + "_sector_phi");
        group_sector_phi_count_->insert(std::pair<std::uint32_t, MergeTH2D>(_group.getId()
                                                                            , MergeTH2D(_sector_phi_name.c_str(), _title.c_str()
                                                                                        , _sector_axis, _phi_axis)));
    }
}

std::unique_ptr<RPCCPPFDigiOccupancyAnalyserGlobalCache> RPCCPPFDigiOccupancyAnalyser::initializeGlobalCache(edm::ParameterSet const & _config)
{
    RPCCPPFDigiOccupancyAnalyserGlobalCache * _global_cache = new RPCCPPFDigiOccupancyAnalyserGlobalCache();

    // selection
    std::vector<std::string> _selection(_config.getParameter<std::vector<std::string> >("selection"));
    for (std::vector<std::string>::const_iterator _name = _selection.begin()
             ; _name != _selection.end() ; ++_name) {
        _global_cache->selection_.push_back(RPCMaskParser::parse(*_name));
        edm::LogInfo("RPCCPPFDigiOccupancyAnalyser") << "Selected " << _global_cache->selection_.back();
    }

    return std::unique_ptr<RPCCPPFDigiOccupancyAnalyserGlobalCache>(_global_cache);
}

RPCCPPFDigiOccupancyAnalyser::RPCCPPFDigiOccupancyAnalyser(edm::ParameterSet const & _config, RPCCPPFDigiOccupancyAnalyserGlobalCache const *)
{
    digis_token_ = consumes<RPCCPPFDigiCollection>(_config.getParameter<edm::InputTag>("digiCollection"));

    produces<MergeMap<std::uint32_t, MergeTH2D>, edm::InRun>("GroupThetaPhiCount");
    produces<MergeMap<std::uint32_t, MergeTH2D>, edm::InRun>("GroupRingThetaCount");
    produces<MergeMap<std::uint32_t, MergeTH2D>, edm::InRun>("GroupSectorPhiCount");
}

void RPCCPPFDigiOccupancyAnalyser::fillDescriptions(edm::ConfigurationDescriptions & _descs)
{
    edm::ParameterSetDescription _desc;

    _desc.add<edm::InputTag>("digiCollection", edm::InputTag("RPCCPPFRawToDigi", ""));
    std::vector<std::string> _selection;
    _selection.push_back("RE-");
    _selection.push_back("RE+");
    _desc.add<std::vector<std::string> >("selection", _selection);

    _descs.add("RPCCPPFDigiOccupancyAnalyser", _desc);
}

std::shared_ptr<RPCCPPFDigiOccupancyAnalyserRunCache> RPCCPPFDigiOccupancyAnalyser::globalBeginRun(edm::Run const & _run, edm::EventSetup const & _setup
                                                                                                   , GlobalCache const * _global_cache)
{
    // Get the list of groups and their rolls
    std::set<RPCMaskDetId> _groups;

    edm::ESHandle<RPCGeometry> _es_rpc_geom;
    _setup.get<MuonGeometryRecord>().get(_es_rpc_geom);
    std::vector<RPCRoll const *> const & _rolls = (_es_rpc_geom->rolls());
    for (std::vector<RPCRoll const *>::const_iterator _roll = _rolls.begin()
             ; _roll != _rolls.end() ; ++_roll) {
        RPCMaskDetId _id((*_roll)->id());

        bool _keep(_global_cache->selection_.empty());
        for (std::vector<RPCMaskDetId>::const_iterator _mask = _global_cache->selection_.begin()
                 ; !_keep && _mask != _global_cache->selection_.end() ; ++_mask) {
            _keep = _id.matches(*_mask);
        }
        if (!_keep) {
            continue;
        }

        RPCMaskDetId _group(_id);
        _group.setSector().setSubSector().setSubSubSector().setRoll().setGap();
        _groups.insert(_group);
    }

    std::shared_ptr<RPCCPPFDigiOccupancyAnalyserRunCache> _run_cache(new RPCCPPFDigiOccupancyAnalyserRunCache());
    _run_cache->groups_.insert(_run_cache->groups_.end(), _groups.begin(), _groups.end());

    return _run_cache;
}

std::shared_ptr<RPCCPPFDigiOccupancyAnalyserData> RPCCPPFDigiOccupancyAnalyser::globalBeginRunSummary(edm::Run const & _run, edm::EventSetup const & _setup
                                                                                                      , RunContext const * _context)
{
    std::shared_ptr<RPCCPPFDigiOccupancyAnalyserData> _data(new RPCCPPFDigiOccupancyAnalyserData());
    _data->initialise(*(_context->global()), *(_context->run()));
    return _data;
}

void RPCCPPFDigiOccupancyAnalyser::beginRun(edm::Run const & _run, edm::EventSetup const & _setup)
{
    stream_data_.initialise(*(globalCache()), *(runCache()));
}

void RPCCPPFDigiOccupancyAnalyser::produce(edm::Event & _event, edm::EventSetup const & _setup)
{
    edm::Handle<RPCCPPFDigiCollection> _digis_handle;
    _event.getByToken(digis_token_, _digis_handle);

    for (auto _rpc_cppf_digi : *_digis_handle) {
        if (_rpc_cppf_digi.isValid()) {
            RPCMaskDetId _id(_rpc_cppf_digi.getRPCDetId());

            bool _keep(globalCache()->selection_.empty());
            for (std::vector<RPCMaskDetId>::const_iterator _mask = globalCache()->selection_.begin()
                     ; !_keep && _mask != globalCache()->selection_.end() ; ++_mask) {
                _keep = _id.matches(*_mask);
            }
            if (!_keep) {
                continue;
            }

            RPCMaskDetId _group(_id);
            _group.setSector().setSubSector().setSubSubSector().setRoll().setGap();

            stream_data_.group_theta_phi_count_->at(_group.getId())->Fill(_rpc_cppf_digi.getTheta(), _rpc_cppf_digi.getPhi());
            stream_data_.group_ring_theta_count_->at(_group.getId())->Fill(_id.getRing(), _rpc_cppf_digi.getTheta());
            stream_data_.group_sector_phi_count_->at(_group.getId())->Fill(_id.getEndcapSector(), _rpc_cppf_digi.getPhi());
        }
    }
}

void RPCCPPFDigiOccupancyAnalyser::endRunSummary(edm::Run const & _run, edm::EventSetup const & _setup
                                                 , RPCCPPFDigiOccupancyAnalyserData * _run_data) const
{
    _run_data->group_theta_phi_count_.mergeProduct(stream_data_.group_theta_phi_count_);
    _run_data->group_ring_theta_count_.mergeProduct(stream_data_.group_ring_theta_count_);
    _run_data->group_sector_phi_count_.mergeProduct(stream_data_.group_sector_phi_count_);
}

void RPCCPPFDigiOccupancyAnalyser::globalEndRunProduce(edm::Run & _run, edm::EventSetup const & _setup
                                                       , RunContext const * _context
                                                       , RPCCPPFDigiOccupancyAnalyserData const * _run_data)
{
    std::unique_ptr<MergeMap<std::uint32_t, MergeTH2D> > _group_theta_phi_count(new MergeMap<std::uint32_t, MergeTH2D>(_run_data->group_theta_phi_count_));
    std::unique_ptr<MergeMap<std::uint32_t, MergeTH2D> > _group_ring_theta_count(new MergeMap<std::uint32_t, MergeTH2D>(_run_data->group_ring_theta_count_));
    std::unique_ptr<MergeMap<std::uint32_t, MergeTH2D> > _group_sector_phi_count(new MergeMap<std::uint32_t, MergeTH2D>(_run_data->group_sector_phi_count_));

    _run.put(std::move(_group_theta_phi_count), "GroupThetaPhiCount");
    _run.put(std::move(_group_ring_theta_count), "GroupRingThetaCount");
    _run.put(std::move(_group_sector_phi_count), "GroupSectorPhiCount");
}

//define this as a module
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(RPCCPPFDigiOccupancyAnalyser);
