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
                 , "/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions16/13TeV/DCSOnly/json_DCSONLY.txt"
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

process = cms.Process("testRPCDigiComparisonPlotter")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = "80X_dataRun2_Express_v10"

process.load("DPGAnalysis.RPC.RPCDigiComparisonPlotter_cfi")
process.RPCDigiComparisonPlotter.createPDF = cms.bool(True)
process.RPCDigiComparisonPlotter.setLogz = cms.bool(True)
process.RPCDigiComparisonPlotter.useDivergingPalette = cms.bool(True)

process.RPCDigiComparisonRollPlotter = process.RPCDigiComparisonPlotter.clone()
process.RPCDigiComparisonRollPlotter.rpcDigiComparisonAnalyser = cms.string('RPCDigiComparisonRollAnalyser')
process.RPCDigiComparisonRollPlotter.pdfOutput = cms.string('RPCDigiComparisonRollAnalyser')

process.RPCDigiComparisonPlotterPacked = process.RPCDigiComparisonPlotter.clone()
process.RPCDigiComparisonPlotterPacked.rpcDigiComparisonAnalyser = cms.string('RPCDigiComparisonAnalyserPacked')
process.RPCDigiComparisonPlotterPacked.pdfOutput = cms.string('RPCDigiComparisonAnalyserPacked')

process.RPCDigiComparisonRollPlotterPacked = process.RPCDigiComparisonPlotterPacked.clone()
process.RPCDigiComparisonRollPlotterPacked.rpcDigiComparisonAnalyser = cms.string('RPCDigiComparisonRollAnalyserPacked')
process.RPCDigiComparisonRollPlotterPacked.pdfOutput = cms.string('RPCDigiComparisonRollAnalyserPacked')

process.load("DPGAnalysis.RPC.RPCAMCLinkCountersPlotter_cfi")
process.RPCAMCLinkCountersPlotter.createPDF = cms.bool(True)
process.RPCAMCLinkCountersPlotter.setLogz = cms.bool(True)
process.RPCAMCLinkCountersPlotter.useDivergingPalette = cms.bool(True)

process.RPCAMCLinkCountersPlotterPacked = process.RPCAMCLinkCountersPlotter.clone()
process.RPCAMCLinkCountersPlotterPacked.rpcAMCLinkCountersAnalyser = cms.string('RPCAMCLinkCountersAnalyserPacked')
process.RPCAMCLinkCountersPlotterPacked.pdfOutput = cms.string('RPCAMCLinkCountersAnalyserPacked')

process.options = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )

# Source
process.source = cms.Source("PoolSource"
                            , fileNames = cms.untracked.vstring(options.inputFiles)
                            # , lumisToProcess = lumilist.getVLuminosityBlockRange()
)

process.TFileService = cms.Service("TFileService", fileName = cms.string("testRPCDigiComparisonPlotter.root") )

process.p = cms.Path(#process.RPCDigiComparisonPlotter
                     #* process.RPCDigiComparisonRollPlotter
                     #* process.RPCAMCLinkCountersPlotter
                     process.RPCDigiComparisonPlotterPacked
                     * process.RPCDigiComparisonRollPlotterPacked
                     * process.RPCAMCLinkCountersPlotterPacked
)
