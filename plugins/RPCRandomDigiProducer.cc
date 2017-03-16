#include "DPGAnalysis/RPC/plugins/RPCRandomDigiProducer.h"

#include <algorithm>
#include <memory>
#include <set>

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "CLHEP/Random/RandFlat.h"

#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/RPCGeometry/interface/RPCGeometry.h"
#include "Geometry/RPCGeometry/interface/RPCRoll.h"

#include "DataFormats/RPCDigi/interface/RPCDigi.h"
#include "DataFormats/RPCDigi/interface/RPCDigiCollection.h"

#include "DPGAnalysis/RPC/interface/RPCMaskParser.h"

RPCRandomDigiProducer::RPCRandomDigiProducer(edm::ParameterSet const & _config)
    : bx_min_(_config.getParameter<int>("bxMin"))
    , bx_max_(_config.getParameter<int>("bxMax"))
    , hits_max_(_config.getParameter<unsigned int>("hitsMax"))
    , cls_max_(_config.getParameter<unsigned int>("clsMax"))
{
    produces<RPCDigiCollection>();

    std::vector<std::string> _roll_selection(_config.getParameter<std::vector<std::string> >("rollSelection"));
    for (std::vector<std::string>::const_iterator _roll = _roll_selection.begin()
             ; _roll != _roll_selection.end() ; ++_roll) {
        roll_selection_.push_back(RPCMaskParser::parse(*_roll));
        edm::LogInfo("RPCRandomDigiProducer") << "Selected roll " << roll_selection_.back();
    }
}

RPCRandomDigiProducer::~RPCRandomDigiProducer()
{}

void RPCRandomDigiProducer::fillDescriptions(edm::ConfigurationDescriptions & _descs)
{
    edm::ParameterSetDescription _desc;
    _desc.add<int>("bxMin", -2);
    _desc.add<int>("bxMax", 2);
    _desc.add<unsigned int>("hitsMax", 100);
    _desc.add<unsigned int>("clsMax", 96);
    _desc.add<std::vector<std::string> >("rollSelection", std::vector<std::string>());

    _descs.add("RPCRandomDigiProducer", _desc);
}

void RPCRandomDigiProducer::beginRun(edm::Run const & _run, edm::EventSetup const & _setup)
{
    roll_nstrips_.clear();
    nstrips_.clear();

    edm::ESHandle<RPCGeometry> _es_rpc_geom;
    _setup.get<MuonGeometryRecord>().get(_es_rpc_geom);

    unsigned int _nstrips(0);
    std::vector<RPCRoll const *> const & _rolls = (_es_rpc_geom->rolls());
    for (std::vector<RPCRoll const *>::const_iterator _roll = _rolls.begin()
             ; _roll != _rolls.end() ; ++_roll) {
        RPCMaskDetId _maskid((*_roll)->id());

        bool _keep(roll_selection_.empty());
        for (std::vector<RPCMaskDetId>::const_iterator _mask = roll_selection_.begin()
                 ; !_keep && _mask != roll_selection_.end() ; ++_mask) {
            _keep = _maskid.matches(*_mask);
        }
        if (_keep) {
            roll_nstrips_.push_back(std::pair<RPCDetId, unsigned int>((*_roll)->id(), (*_roll)->nstrips()));
            _nstrips += (*_roll)->nstrips();
            nstrips_.push_back(_nstrips);
        }
    }
}

void RPCRandomDigiProducer::produce(edm::Event & _event, edm::EventSetup const & _setup)
{
    std::set<std::pair<RPCDetId, RPCDigi> > _digis;

    CLHEP::HepRandomEngine & _engine(rand_->getEngine(_event.getLuminosityBlock().index()));

    for (int _bx = bx_min_ ; _bx <= bx_max_ ; ++_bx) {
        unsigned int _hits(CLHEP::RandFlat::shootInt(&_engine, hits_max_ + 1));
        for (unsigned int _hit = 0 ; _hit < _hits ; ++_hit) {
            int _cls_m2(CLHEP::RandFlat::shootInt(&_engine, cls_max_ * 2) + 2); // /2=cls, %2=up/down for even cls
            int _strip(CLHEP::RandFlat::shootInt(&_engine, nstrips_.back()) + 1);
            int _roll(std::upper_bound(nstrips_.begin(), nstrips_.end(), _strip - 1) - nstrips_.begin());
            _strip -= ((_roll > 0 ? nstrips_.at(_roll - 1) : 0) - 1);

            std::pair<RPCDetId, unsigned int> const & _roll_nstrips(roll_nstrips_.at(_roll));

            int _cls(_cls_m2 / 2);
            int _direction(_cls_m2%2);
            int _nstrips(_roll_nstrips.second);
            int _first_strip(_cls%2
                             ? std::max(_strip - _cls / 2 + _direction, 1)
                             : std::max(_strip - _cls / 2, 1));
            int _last_strip(_cls%2
                            ? std::min(_strip + _cls / 2 + _direction - 1, _nstrips)
                            : std::min(_strip + _cls / 2, _nstrips));

            for (_strip = _first_strip ; _strip <= _last_strip ; ++_strip) {
                _digis.insert(std::pair<RPCDetId, RPCDigi>(_roll_nstrips.first, RPCDigi(_strip, _bx)));
            }
        }
    }

    std::unique_ptr<RPCDigiCollection> _rpc_digi_collection(new RPCDigiCollection());
    RPCDetId _rpc_det_id;
    std::vector<RPCDigi> _local_digis;
    for (std::set<std::pair<RPCDetId, RPCDigi> >::const_iterator _rpc_digi = _digis.begin()
             ; _rpc_digi != _digis.end() ; ++_rpc_digi) {
        LogDebug("RPCRandomDigiProducer") << "RPCDigi " << _rpc_digi->first.rawId()
                                          << ", " << _rpc_digi->second.strip() << ", " << _rpc_digi->second.bx();
        if (_rpc_digi->first != _rpc_det_id) {
            if (!_local_digis.empty()) {
                _rpc_digi_collection->put(RPCDigiCollection::Range(_local_digis.begin(), _local_digis.end()), _rpc_det_id);
                _local_digis.clear();
            }
            _rpc_det_id = _rpc_digi->first;
        }
        _local_digis.push_back(_rpc_digi->second);
    }
    if (!_local_digis.empty()) {
        _rpc_digi_collection->put(RPCDigiCollection::Range(_local_digis.begin(), _local_digis.end()), _rpc_det_id);
    }

    _event.put(std::move(_rpc_digi_collection));
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(RPCRandomDigiProducer);
