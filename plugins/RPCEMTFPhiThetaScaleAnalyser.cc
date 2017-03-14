#include "DPGAnalysis/RPC/plugins/RPCEMTFPhiThetaScaleAnalyser.h"

#include <cmath>
#include <fstream>
#include <memory>
#include <sstream>

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "DataFormats/MuonDetId/interface/RPCDetId.h"

#include "Geometry/RPCGeometry/interface/RPCGeometry.h"
#include "Geometry/RPCGeometry/interface/RPCRoll.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"

#include "DataFormats/GeometryVector/interface/LocalPoint.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "DataFormats/GeometryVector/interface/Phi.h"
#include "DataFormats/GeometryVector/interface/Pi.h"

#include "CondFormats/DataRecord/interface/RPCCPPFLinkMapRcd.h"
#include "CondFormats/DataRecord/interface/RPCInverseCPPFLinkMapRcd.h"
#include "CondFormats/DataRecord/interface/RPCInverseLBLinkMapRcd.h"
#include "CondFormats/RPCObjects/interface/RPCAMCLink.h"
#include "CondFormats/RPCObjects/interface/RPCAMCLinkMap.h"
#include "CondFormats/RPCObjects/interface/RPCFebConnector.h"
#include "CondFormats/RPCObjects/interface/RPCInverseAMCLinkMap.h"
#include "CondFormats/RPCObjects/interface/RPCInverseLBLinkMap.h"
#include "CondFormats/RPCObjects/interface/RPCLBLink.h"
#include "CondFormats/RPCObjects/interface/RPCLBLinkMap.h"

// http://cmsdoc.cern.ch/cms/MUON/cscTrigger/html/CERN/in2007_024.pdf
// https://indico.cern.ch/event/614182/contributions/2476325/attachments/1413132/2162370/2017_02_15_EMTF_pT_with_RPC.pdf
// https://indico.cern.ch/event/614182/contributions/2478667/attachments/1413097/2162465/CPPF-EMTF-format_florida.pdf
/*
  Assumptions:
  - Endcap sectors connected for trigger-sectors (1, 2, ..., 6) are (2..8, 8..14, ..., 32..2)
  - Reference phi in degrees = 15 + (trigger-sector - 1) * 60 - 22
  - Theta is measured from beampipe in the respective region, minus 8.5 degrees (from 8.5 to 45, 5 bit)
*/

RPCEMTFPhiThetaScaleAnalyser::RPCEMTFPhiThetaScaleAnalyser(edm::ParameterSet const & _config)
{}

void RPCEMTFPhiThetaScaleAnalyser::fillDescriptions(edm::ConfigurationDescriptions & _descs)
{
    edm::ParameterSetDescription _desc;
    _descs.add("RPCEMTFPhiThetaScaleAnalyser", _desc);
}

void RPCEMTFPhiThetaScaleAnalyser::analyze(edm::Event const &, edm::EventSetup const & _setup)
{
    edm::ESHandle<RPCInverseLBLinkMap> _es_rpc_ilb_map;
    _setup.get<RPCInverseLBLinkMapRcd>().get(_es_rpc_ilb_map);
    RPCInverseLBLinkMap _ilb_map(*_es_rpc_ilb_map);

    edm::ESHandle<RPCInverseAMCLinkMap> _es_rpc_icppf_map;
    _setup.get<RPCInverseCPPFLinkMapRcd>().get(_es_rpc_icppf_map);
    RPCInverseAMCLinkMap _icppf_map(*_es_rpc_icppf_map);

    edm::ESHandle<RPCGeometry> _es_rpc_geom;
    _setup.get<MuonGeometryRecord>().get(_es_rpc_geom);
    std::vector<RPCRoll const *> const & _rpc_rolls(_es_rpc_geom->rolls());

    // Prepare ofstreams, one per CPPF
    std::map<RPCAMCLink, std::shared_ptr<std::ofstream> > _cppf_ofstream;
    {
        edm::ESHandle<RPCAMCLinkMap> _es_rpc_cppf_map;
        _setup.get<RPCCPPFLinkMapRcd>().get(_es_rpc_cppf_map);
        RPCAMCLinkMap _cppf_map(*_es_rpc_cppf_map);

        for (RPCAMCLinkMap::map_type::const_iterator _cppf_input = _cppf_map.getMap().begin()
                 ; _cppf_input != _cppf_map.getMap().end() ; ++_cppf_input) {
            RPCAMCLink _cppf(_cppf_input->first);
            _cppf.setAMCInput();
            if (_cppf_ofstream.find(_cppf) == _cppf_ofstream.end()) {
                std::ostringstream _ofstream_name;
                _ofstream_name << _cppf << ".txt";
                _cppf_ofstream[_cppf] = std::shared_ptr<std::ofstream>(new std::ofstream(_ofstream_name.str()));
            }
        }
    }

    // Loop over the EMTFs manually
    for (int _region = -1 ; _region <= 1 ; _region += 2) {
        for (int _trigger_sector = 1 ; _trigger_sector <= 6 ; ++_trigger_sector) {
            edm::LogWarning("RPCEMTFPhiThetaScaleAnalyser") << "Handling EMTF_" << (_region < 0 ? 'n' : 'p') << _trigger_sector;

            Geom::Phi<float> _phi_ref((15. + (float)(_trigger_sector - 1) * 60. - 22.) / 180. * Geom::pi());
            edm::LogWarning("RPCEMTFPhiThetaScaleAnalyser") << "Reference phi: " << _phi_ref
                                                            << " (" << (float)(_phi_ref) * 180. / Geom::pi() << " degrees"
                                                            << ", " << int((float)(_phi_ref) * 180. / Geom::pi() * 15. + (_phi_ref < 0. ? -.5 : .5)) << " units)";

            // Here I should have the Mappings for CPPF/EMTF to check which links are in use, but since that's not there (yet)
            // use all links in endcap_sector (2..8, 8..14, ..., 32..2), excluding the last endcap_sector
            int _endcap_sector_min((_trigger_sector - 1) * 6 + 2)
                , _endcap_sector_max((_trigger_sector * 6 + 2) % 36);

            for (std::vector<RPCRoll const *>::const_iterator _rpc_roll_it = _rpc_rolls.begin()
                     ; _rpc_roll_it != _rpc_rolls.end() ; ++_rpc_roll_it) {
                RPCRoll const & _rpc_roll(*(*_rpc_roll_it));
                RPCDetId _rpc_id(_rpc_roll.id());
                int _endcap_sector((_rpc_id.sector() - 1) * 6 + _rpc_id.subsector());

                if (_rpc_id.region() != _region
                    || ((_endcap_sector - _endcap_sector_min + 36) % 36 >= (_endcap_sector_max - _endcap_sector_min + 36) % 36)) {
                    continue;
                }

                std::pair<RPCInverseLBLinkMap::map_type::const_iterator
                          , RPCInverseLBLinkMap::map_type::const_iterator> _lookup_range(_ilb_map.getMap().equal_range(_rpc_id.rawId()));

                int _n_strips(_rpc_roll.nstrips());
                for (int _strip = 1 ; _strip <= _n_strips ; ++_strip) {
                    GlobalPoint _strip_gp(_rpc_roll.toGlobal(_rpc_roll.centreOfStrip(_strip)));
                    int _phi = (float)(_strip_gp.phi() - _phi_ref) * 180. / Geom::pi() * 15.
                        + .5; // rounding
                    int _theta = (_region == -1 ? 180. * 32. / 36.5 : 0.)
                        + (float)(_region) * (float)(_strip_gp.theta()) * 180. / Geom::pi() * 32. / 36.5
                        - 8.5 * 32. / 36.5 // offset
                        + .5; // rounding

                    int _channel(-1);
                    RPCLBLink _lb, _link;
                    int _slope(0);
                    for (RPCInverseLBLinkMap::map_type::const_iterator _link_it = _lookup_range.first
                             ; _link_it != _lookup_range.second ; ++_link_it) {
                        if (_link_it->second.second.hasStrip(_strip)) {
                            _lb = _link_it->second.first;
                            _lb.setConnector();
                            _link = _lb;
                            _link.setLinkBoard();
                            _channel = _link_it->second.second.getChannel(_strip) - 1
                                + 16 * _link_it->second.first.getConnector();
                            _slope = _link_it->second.second.getSlope();
                            break;
                        }
                    }

                    if (_channel < 0) {
                        edm::LogError("RPCEMTFPhiThetaScaleAnalyser") << "Could not find rpc_id " << _rpc_id.rawId() << " strip " << _strip;
                        break;
                    }

                    std::pair<RPCInverseAMCLinkMap::map_type::const_iterator, RPCInverseAMCLinkMap::map_type::const_iterator> _cppf_input_range
                        = _icppf_map.getMap().equal_range(_link);

                    for (RPCInverseAMCLinkMap::map_type::const_iterator _cppf_input = _cppf_input_range.first
                             ;_cppf_input != _cppf_input_range.second ; ++_cppf_input) {
                        RPCAMCLink _cppf(_cppf_input->second);
                        _cppf.setAMCInput();
                        *(_cppf_ofstream.at(_cppf)) << _lb
                                                    << ' ' << std::setw(10) << _rpc_id.rawId()
                                                    << " strip: " << std::setw(3) << _strip // << ' '
                                                    << " lbChannel: " << std::setw(3) << _channel
                                                    << " halfChannel: " << std::setw(4) << _channel * 2
                                                    << " phi: " << std::setw(5) << _phi
                                                    << " theta: " << std::setw(4) << _theta
                                                    << std::endl;
                    }

                    if (_channel * 2 + 1 > 190) {
                        continue;
                    }

                    if (_strip + _slope >= 1 && _strip + _slope <= _n_strips) {
                        _strip_gp = _rpc_roll.toGlobal(_rpc_roll.centreOfStrip((float)(_strip + (_slope > 0 ? 0 : -1)))); // float strip .5 = int strip 1
                        _phi = (float)(_strip_gp.phi() - _phi_ref) * 180. / Geom::pi() * 15.
                            + .5; // rounding
                        _theta = (_region == -1 ? 180. * 32. / 36.5 : 0.)
                            + (float)(_region) * (float)(_strip_gp.theta()) * 180. / Geom::pi() * 32. / 36.5
                            - 8.5 * 32. / 36.5 // offset
                            + .5; // rounding
                    }

                    for (RPCInverseAMCLinkMap::map_type::const_iterator _cppf_input = _cppf_input_range.first
                             ;_cppf_input != _cppf_input_range.second ; ++_cppf_input) {
                        RPCAMCLink _cppf(_cppf_input->second);
                        _cppf.setAMCInput();
                        *(_cppf_ofstream.at(_cppf)) << _lb
                                                    << ' ' << std::setw(10) << 0 // _rpc_id.rawId()
                                                    << " strip: " << std::setw(3) << 0 // _strip << (_slope > 0 ? '+' : '-')
                                                    << " lbChannel: " << std::setw(3) << 0 // << _channel << '+'
                                                    << " halfChannel: " << std::setw(4) << _channel * 2 + 1
                                                    << " phi: " << std::setw(5) << _phi
                                                    << " theta: " << std::setw(4) << _theta
                                                    << std::endl;
                    }
                }
            }
        }
    }
}

//define this as a plug-in
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(RPCEMTFPhiThetaScaleAnalyser);
