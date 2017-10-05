import FWCore.ParameterSet.Config as cms

from FWCore.PythonUtilities.LumiList import LumiList
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing("analysis")
options.register("runList"
                 , []
                 , VarParsing.multiplicity.list
                 , VarParsing.varType.int
                 , "Run selection")
options.register("lumiList"
                 , "/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions17/13TeV/DCSOnly/json_DCSONLY.txt"
                 , VarParsing.multiplicity.singleton
                 , VarParsing.varType.string
                 , "JSON file")
options.parseArguments()

lumilist = LumiList(filename = options.lumiList)
if len(options.runList) :
    runlist = LumiList(runs = options.runList)
    lumilist = lumilist & runlist
    if not len(lumilist) :
        raise RuntimeError, "The resulting LumiList is empty"

process = cms.Process("testRPCUnpackersPlotter")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
# https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideFrontierConditions#Express_reconstruction_Global_Ta
process.GlobalTag.globaltag = "92X_dataRun2_Express_v7"

process.load("DPGAnalysis.RPC.RPCCPPFDigiOccupancyPlotter_cfi")
process.RPCCPPFDigiOccupancyPlotter.createPDF = cms.bool(True)
process.RPCCPPFDigiOccupancyPlotter.setLogz = cms.bool(True)
process.RPCCPPFDigiOccupancyPlotter.useDivergingPalette = cms.bool(True)

process.load("DPGAnalysis.RPC.RPCAMCLinkCountersPlotter_cfi")
process.RPCAMCLinkCountersPlotter.createPDF = cms.bool(True)
process.RPCAMCLinkCountersPlotter.setLogz = cms.bool(True)
process.RPCAMCLinkCountersPlotter.useDivergingPalette = cms.bool(True)

process.RPCCPPFLinkCountersPlotter = process.RPCAMCLinkCountersPlotter.clone()
process.RPCCPPFLinkCountersPlotter.rpcAMCLinkCountersAnalyser = cms.string("RPCCPPFLinkCountersAnalyser")
process.RPCCPPFLinkCountersPlotter.pdfOutput = cms.string('RPCCPPFLinkCountersAnalyser')

process.RPCTwinMuxLinkCountersPlotter = process.RPCAMCLinkCountersPlotter.clone()
process.RPCTwinMuxLinkCountersPlotter.rpcAMCLinkCountersAnalyser = cms.string("RPCTwinMuxLinkCountersAnalyser")
process.RPCTwinMuxLinkCountersPlotter.pdfOutput = cms.string('RPCTwinMuxLinkCountersAnalyser')

process.load("DPGAnalysis.RPC.RPCDigiComparisonPlotter_cfi")
process.RPCDigiComparisonPlotter.createPDF = cms.bool(True)
process.RPCDigiComparisonPlotter.setLogz = cms.bool(True)
process.RPCDigiComparisonPlotter.useDivergingPalette = cms.bool(True)

process.RPCCPPFDigiComparisonPlotter = process.RPCDigiComparisonPlotter.clone()
process.RPCCPPFDigiComparisonPlotter.rpcDigiComparisonAnalyser = cms.string('RPCCPPFDigiComparisonAnalyser')
process.RPCCPPFDigiComparisonPlotter.pdfOutput = cms.string('RPCCPPFDigiComparisonAnalyser')

process.RPCCPPFDigiComparisonRollPlotter = process.RPCCPPFDigiComparisonPlotter.clone()
process.RPCCPPFDigiComparisonRollPlotter.rpcDigiComparisonAnalyser = cms.string('RPCCPPFDigiComparisonRollAnalyser')
process.RPCCPPFDigiComparisonRollPlotter.pdfOutput = cms.string('RPCCPPFDigiComparisonRollAnalyser')

process.RPCTwinMuxDigiComparisonPlotter = process.RPCDigiComparisonPlotter.clone()
process.RPCTwinMuxDigiComparisonPlotter.rpcDigiComparisonAnalyser = cms.string('RPCTwinMuxDigiComparisonAnalyser')
process.RPCTwinMuxDigiComparisonPlotter.pdfOutput = cms.string('RPCTwinMuxDigiComparisonAnalyser')

process.RPCTwinMuxDigiComparisonRollPlotter = process.RPCTwinMuxDigiComparisonPlotter.clone()
process.RPCTwinMuxDigiComparisonRollPlotter.rpcDigiComparisonAnalyser = cms.string('RPCTwinMuxDigiComparisonRollAnalyser')
process.RPCTwinMuxDigiComparisonRollPlotter.pdfOutput = cms.string('RPCTwinMuxDigiComparisonRollAnalyser')

process.options = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )

# Source
process.source = cms.Source("PoolSource"
                            , fileNames = cms.untracked.vstring(options.inputFiles)
                            # , lumisToProcess = lumilist.getVLuminosityBlockRange()
)

process.TFileService = cms.Service("TFileService", fileName = cms.string("testRPCUnpackersPlotter.root") )

process.p = cms.Path(process.RPCCPPFDigiOccupancyPlotter
                     + process.RPCCPPFLinkCountersPlotter + process.RPCTwinMuxLinkCountersPlotter
                     + process.RPCCPPFDigiComparisonPlotter + process.RPCCPPFDigiComparisonRollPlotter
                     + process.RPCTwinMuxDigiComparisonPlotter + process.RPCTwinMuxDigiComparisonRollPlotter
)
