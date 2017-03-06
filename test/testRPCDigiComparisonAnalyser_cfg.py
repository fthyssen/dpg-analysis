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

process = cms.Process("testRPCDigiComparisonAnalyser")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1000
# USER_CXXFLAGS="-g -D=EDM_ML_DEBUG" scram b
# process.MessageLogger = cms.Service(
#     "MessageLogger"
#     , destinations = cms.untracked.vstring(
#         "critical"
#     )
#     , critical = cms.untracked.PSet(
#         threshold = cms.untracked.string("DEBUG")
#     )
#     , debugModules = cms.untracked.vstring(
#         "RPCTwinMuxRawToDigi"
#     )
# )

process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = "80X_dataRun2_Express_v15"

process.load("DPGAnalysis.Tools.EventFilter_cfi")

process.load("EventFilter.RPCRawToDigi.RPCTwinMuxRawToDigi_sqlite_cff")
process.load("EventFilter.RPCRawToDigi.rpcPacker_cfi")
process.rpcpacker.InputLabel = cms.InputTag("RPCTwinMuxRawToDigi")
process.load("EventFilter.RPCRawToDigi.rpcUnpackingModule_cfi")
process.rpcUnpackingModulePacked = process.rpcUnpackingModule.clone()
process.rpcUnpackingModulePacked.InputLabel = cms.InputTag("rpcpacker")
process.load("EventFilter.RPCRawToDigi.RPCTwinMuxDigiToRaw_sqlite_cff")
process.load("CondTools.RPC.RPCLinkMap_sqlite_cff")
process.RPCTwinMuxRawToDigi.bxMin = cms.int32(-2)
process.RPCTwinMuxRawToDigi.bxMax = cms.int32(2)
process.RPCTwinMuxRawToDigi.calculateCRC = cms.bool(True)

process.RPCTwinMuxDigiToRaw.inputTag = cms.InputTag('RPCTwinMuxRawToDigi')
#process.RPCTwinMuxDigiToRaw.inputTag = cms.InputTag('rpcUnpackingModule')
process.RPCTwinMuxDigiToRaw.bxMin = cms.int32(-2)
process.RPCTwinMuxDigiToRaw.bxMax = cms.int32(2)

process.RPCTwinMuxRawToDigiPacked = process.RPCTwinMuxRawToDigi.clone()
process.RPCTwinMuxRawToDigiPacked.inputTag = cms.InputTag('RPCTwinMuxDigiToRaw')

process.load("EventFilter.L1TXRawToDigi.twinMuxStage2Digis_cfi")
process.twinMuxStage2DigisPacked = process.twinMuxStage2Digis.clone()
process.twinMuxStage2DigisPacked.DTTM7_FED_Source = cms.InputTag("RPCTwinMuxDigiToRaw")

process.load("DPGAnalysis.RPC.RPCDigiComparisonAnalyser_cff")
process.RPCDigiComparisonAnalyser.lhsDigiCollection = cms.InputTag('rpcUnpackingModule')
process.RPCDigiComparisonAnalyser.lhsDigiCollectionName = cms.string('Legacy')
process.RPCDigiComparisonAnalyser.rhsDigiCollection = cms.InputTag('RPCTwinMuxRawToDigi')
process.RPCDigiComparisonAnalyser.rhsDigiCollectionName = cms.string('TwinMux')
process.RPCDigiComparisonAnalyser.rollSelection = cms.vstring('RB') # 'RB/W-1/S9', 'RB/S9', ...
process.RPCDigiComparisonAnalyser.bxMin = cms.int32(-2)
process.RPCDigiComparisonAnalyser.bxMax = cms.int32(2)
process.RPCDigiComparisonAnalyser.lumiList = lumilist.getVLuminosityBlockRange()

process.RPCDigiComparisonRollAnalyser = process.RPCDigiComparisonAnalyser.clone()
process.RPCDigiComparisonRollAnalyser.groupPlots = cms.bool(False)
process.RPCDigiComparisonRollAnalyser.distanceRollWeight = cms.double(1.) # don't allow mismatches here

process.RPCDigiComparisonAnalyserPacked = process.RPCDigiComparisonAnalyser.clone()
process.RPCDigiComparisonAnalyserPacked.lhsDigiCollection = cms.InputTag('RPCTwinMuxRawToDigi')
process.RPCDigiComparisonAnalyserPacked.lhsDigiCollectionName = cms.string('TwinMux')
process.RPCDigiComparisonAnalyserPacked.rhsDigiCollection = cms.InputTag('RPCTwinMuxRawToDigiPacked')
process.RPCDigiComparisonAnalyserPacked.rhsDigiCollectionName = cms.string('Packed')
process.RPCDigiComparisonAnalyser.bxMin = cms.int32(-2)
process.RPCDigiComparisonAnalyser.bxMax = cms.int32(2)

process.RPCDigiComparisonRollAnalyserPacked = process.RPCDigiComparisonAnalyserPacked.clone()
process.RPCDigiComparisonRollAnalyserPacked.groupPlots = cms.bool(False)
process.RPCDigiComparisonRollAnalyserPacked.distanceRollWeight = cms.double(1.) # don't allow mismatches here

process.load("DPGAnalysis.RPC.RPCAMCLinkCountersAnalyser_cff")
process.RPCAMCLinkCountersAnalyserPacked = process.RPCAMCLinkCountersAnalyser.clone()
process.RPCAMCLinkCountersAnalyserPacked.RPCAMCLinkCounters = cms.InputTag('RPCTwinMuxRawToDigiPacked')

process.options = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )

# Source
process.source = cms.Source("PoolSource"
                            , fileNames = cms.untracked.vstring(options.inputFiles)
                            , lumisToProcess = lumilist.getVLuminosityBlockRange()
                            # , inputCommands =  cms.untracked.vstring("keep *"
                            # , "drop GlobalObjectMapRecord_*_*_*")
)

# process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(100) )
# process.maxLuminosityBlocks = cms.untracked.PSet(input = cms.untracked.int32(20))

process.p = cms.Path((process.rpcUnpackingModule
                      + process.RPCTwinMuxRawToDigi
                      + process.twinMuxStage2Digis)
                     * (process.RPCDigiComparisonAnalyser
                        + process.RPCDigiComparisonRollAnalyser
                        + process.RPCAMCLinkCountersAnalyser
                        + (process.RPCTwinMuxDigiToRaw
                           * process.RPCTwinMuxRawToDigiPacked
                           * (process.RPCDigiComparisonAnalyserPacked
                              + process.RPCDigiComparisonRollAnalyserPacked
                              + process.RPCAMCLinkCountersAnalyserPacked)
                           * process.twinMuxStage2DigisPacked
                        )
                        + process.rpcpacker * process.rpcUnpackingModulePacked
                     )
                     * process.EventFilter
)

# Output
process.out = cms.OutputModule("PoolOutputModule"
                               , outputCommands = cms.untracked.vstring("drop *"
                                                                        , "keep *_*_*_testRPCDigiComparisonAnalyser")
                               , fileName = cms.untracked.string(options.outputFile)
                               #, fileName = cms.untracked.string("testRPCDigiComparisonAnalyser.root")
                               , SelectEvents = cms.untracked.PSet(SelectEvents = cms.vstring("p"))
)

process.e = cms.EndPath(process.out)
