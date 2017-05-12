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
# https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideFrontierConditions#Express_reconstruction_Global_Ta
process.GlobalTag.globaltag = "90X_dataRun2_Express_v4"

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

process.RPCDigiComparisonPlotterPackedEOD = process.RPCDigiComparisonPlotter.clone()
process.RPCDigiComparisonPlotterPackedEOD.rpcDigiComparisonAnalyser = cms.string('RPCDigiComparisonAnalyserPackedEOD')
process.RPCDigiComparisonPlotterPackedEOD.pdfOutput = cms.string('RPCDigiComparisonAnalyserPackedEOD')

process.RPCDigiComparisonRollPlotterPackedEOD = process.RPCDigiComparisonPlotterPackedEOD.clone()
process.RPCDigiComparisonRollPlotterPackedEOD.rpcDigiComparisonAnalyser = cms.string('RPCDigiComparisonRollAnalyserPackedEOD')
process.RPCDigiComparisonRollPlotterPackedEOD.pdfOutput = cms.string('RPCDigiComparisonRollAnalyserPackedEOD')

process.RPCDigiComparisonPlotterRandom = process.RPCDigiComparisonPlotter.clone()
process.RPCDigiComparisonPlotterRandom.rpcDigiComparisonAnalyser = cms.string('RPCDigiComparisonAnalyserRandom')
process.RPCDigiComparisonPlotterRandom.pdfOutput = cms.string('RPCDigiComparisonAnalyserRandom')

process.RPCDigiComparisonRollPlotterRandom = process.RPCDigiComparisonPlotterRandom.clone()
process.RPCDigiComparisonRollPlotterRandom.rpcDigiComparisonAnalyser = cms.string('RPCDigiComparisonRollAnalyserRandom')
process.RPCDigiComparisonRollPlotterRandom.pdfOutput = cms.string('RPCDigiComparisonRollAnalyserRandom')

process.RPCDigiComparisonPlotterRandomEOD = process.RPCDigiComparisonPlotter.clone()
process.RPCDigiComparisonPlotterRandomEOD.rpcDigiComparisonAnalyser = cms.string('RPCDigiComparisonAnalyserRandomEOD')
process.RPCDigiComparisonPlotterRandomEOD.pdfOutput = cms.string('RPCDigiComparisonAnalyserRandomEOD')

process.RPCDigiComparisonRollPlotterRandomEOD = process.RPCDigiComparisonPlotterRandomEOD.clone()
process.RPCDigiComparisonRollPlotterRandomEOD.rpcDigiComparisonAnalyser = cms.string('RPCDigiComparisonRollAnalyserRandomEOD')
process.RPCDigiComparisonRollPlotterRandomEOD.pdfOutput = cms.string('RPCDigiComparisonRollAnalyserRandomEOD')

process.load("DPGAnalysis.RPC.RPCAMCLinkCountersPlotter_cfi")
process.RPCAMCLinkCountersPlotter.createPDF = cms.bool(True)
process.RPCAMCLinkCountersPlotter.setLogz = cms.bool(True)
process.RPCAMCLinkCountersPlotter.useDivergingPalette = cms.bool(True)

process.RPCAMCLinkCountersPlotterPacked = process.RPCAMCLinkCountersPlotter.clone()
process.RPCAMCLinkCountersPlotterPacked.rpcAMCLinkCountersAnalyser = cms.string('RPCAMCLinkCountersAnalyserPacked')
process.RPCAMCLinkCountersPlotterPacked.pdfOutput = cms.string('RPCAMCLinkCountersAnalyserPacked')

process.RPCAMCLinkCountersPlotterPackedEOD = process.RPCAMCLinkCountersPlotter.clone()
process.RPCAMCLinkCountersPlotterPackedEOD.rpcAMCLinkCountersAnalyser = cms.string('RPCAMCLinkCountersAnalyserPackedEOD')
process.RPCAMCLinkCountersPlotterPackedEOD.pdfOutput = cms.string('RPCAMCLinkCountersAnalyserPackedEOD')

process.RPCAMCLinkCountersPlotterRandom = process.RPCAMCLinkCountersPlotter.clone()
process.RPCAMCLinkCountersPlotterRandom.rpcAMCLinkCountersAnalyser = cms.string('RPCAMCLinkCountersAnalyserRandom')
process.RPCAMCLinkCountersPlotterRandom.pdfOutput = cms.string('RPCAMCLinkCountersAnalyserRandom')

process.RPCAMCLinkCountersPlotterRandomEOD = process.RPCAMCLinkCountersPlotter.clone()
process.RPCAMCLinkCountersPlotterRandomEOD.rpcAMCLinkCountersAnalyser = cms.string('RPCAMCLinkCountersAnalyserRandomEOD')
process.RPCAMCLinkCountersPlotterRandomEOD.pdfOutput = cms.string('RPCAMCLinkCountersAnalyserRandomEOD')

process.options = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )

# Source
process.source = cms.Source("PoolSource"
                            , fileNames = cms.untracked.vstring(options.inputFiles)
                            # , lumisToProcess = lumilist.getVLuminosityBlockRange()
)

process.TFileService = cms.Service("TFileService", fileName = cms.string("testRPCDigiComparisonPlotter.root") )

process.p = cms.Path(process.RPCDigiComparisonPlotter
                     #* process.RPCDigiComparisonRollPlotter
                     * process.RPCAMCLinkCountersPlotter
                     * process.RPCDigiComparisonPlotterPacked
                     #* process.RPCDigiComparisonRollPlotterPacked
                     * process.RPCAMCLinkCountersPlotterPacked
                     * process.RPCDigiComparisonPlotterPackedEOD
                     #* process.RPCDigiComparisonRollPlotterPackedEOD
                     * process.RPCAMCLinkCountersPlotterPackedEOD
                     * process.RPCDigiComparisonPlotterRandom
                     #* process.RPCDigiComparisonRollPlotterRandom
                     * process.RPCAMCLinkCountersPlotterRandom
                     * process.RPCDigiComparisonPlotterRandomEOD
                     #* process.RPCDigiComparisonRollPlotterRandomEOD
                     * process.RPCAMCLinkCountersPlotterRandomEOD
)
