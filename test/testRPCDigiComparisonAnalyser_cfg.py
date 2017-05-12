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

# FrontierConditions
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
# https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideFrontierConditions#Express_reconstruction_Global_Ta
process.GlobalTag.globaltag = "90X_dataRun2_Express_v4"

# Drop all event content
process.load("DPGAnalysis.Tools.EventFilter_cfi")

# Unpack + pack + unpack
process.load("EventFilter.RPCRawToDigi.RPCTwinMuxRawToDigi_sqlite_cff")
process.load("EventFilter.RPCRawToDigi.rpcUnpackingModule_cfi")
process.load("EventFilter.RPCRawToDigi.RPCTwinMuxDigiToRaw_sqlite_cff")
process.load("CondTools.RPC.RPCLinkMap_sqlite_cff")
process.RPCTwinMuxRawToDigi.bxMin = cms.int32(-2)
process.RPCTwinMuxRawToDigi.bxMax = cms.int32(2)
process.RPCTwinMuxRawToDigi.calculateCRC = cms.bool(True)

process.RPCTwinMuxDigiToRaw.inputTag = cms.InputTag("RPCTwinMuxRawToDigi")
process.RPCTwinMuxDigiToRaw.bxMin = cms.int32(-2)
process.RPCTwinMuxDigiToRaw.bxMax = cms.int32(2)

process.RPCTwinMuxRawToDigiPacked = process.RPCTwinMuxRawToDigi.clone()
process.RPCTwinMuxRawToDigiPacked.inputTag = cms.InputTag("RPCTwinMuxDigiToRaw")

# Link Counters
process.load("DPGAnalysis.RPC.RPCAMCLinkCountersAnalyser_cff")
process.RPCAMCLinkCountersAnalyserPacked = process.RPCAMCLinkCountersAnalyser.clone()
process.RPCAMCLinkCountersAnalyserPacked.RPCAMCLinkCounters = cms.InputTag("RPCTwinMuxRawToDigiPacked")

# Compare RPCDigis
process.load("DPGAnalysis.RPC.RPCDigiComparisonAnalyser_cff")
process.RPCDigiComparisonAnalyser.lhsDigiCollection = cms.InputTag("rpcUnpackingModule")
process.RPCDigiComparisonAnalyser.lhsDigiCollectionName = cms.string("Legacy")
process.RPCDigiComparisonAnalyser.rhsDigiCollection = cms.InputTag("RPCTwinMuxRawToDigi")
process.RPCDigiComparisonAnalyser.rhsDigiCollectionName = cms.string("TwinMux")
process.RPCDigiComparisonAnalyser.rollSelection = cms.vstring("RB") # "RB/W-1/S9", "RB/S9", ...
process.RPCDigiComparisonAnalyser.bxMin = cms.int32(-2)
process.RPCDigiComparisonAnalyser.bxMax = cms.int32(2)
process.RPCDigiComparisonAnalyser.lumiList = lumilist.getVLuminosityBlockRange()

process.RPCDigiComparisonRollAnalyser = process.RPCDigiComparisonAnalyser.clone()
process.RPCDigiComparisonRollAnalyser.groupPlots = cms.bool(False)
process.RPCDigiComparisonRollAnalyser.distanceRollWeight = cms.double(1.) # don't allow mismatches here

process.RPCDigiComparisonAnalyserPacked = process.RPCDigiComparisonAnalyser.clone()
process.RPCDigiComparisonAnalyserPacked.lhsDigiCollection = cms.InputTag("RPCTwinMuxRawToDigi")
process.RPCDigiComparisonAnalyserPacked.lhsDigiCollectionName = cms.string("TwinMux")
process.RPCDigiComparisonAnalyserPacked.rhsDigiCollection = cms.InputTag("RPCTwinMuxRawToDigiPacked")
process.RPCDigiComparisonAnalyserPacked.rhsDigiCollectionName = cms.string("Packed")
process.RPCDigiComparisonAnalyser.bxMin = cms.int32(-2)
process.RPCDigiComparisonAnalyser.bxMax = cms.int32(2)

process.RPCDigiComparisonRollAnalyserPacked = process.RPCDigiComparisonAnalyserPacked.clone()
process.RPCDigiComparisonRollAnalyserPacked.groupPlots = cms.bool(False)
process.RPCDigiComparisonRollAnalyserPacked.distanceRollWeight = cms.double(1.) # don't allow mismatches here

# Do the same taking EOD into account
process.RPCTwinMuxDigiToRawEOD = process.RPCTwinMuxDigiToRaw.clone()
process.RPCTwinMuxDigiToRawEOD.ignoreEOD = cms.bool(False)
process.RPCTwinMuxDigiToRawEOD.inputTag = cms.InputTag("RPCTwinMuxRawToDigi")

process.RPCTwinMuxRawToDigiPackedEOD = process.RPCTwinMuxRawToDigi.clone()
process.RPCTwinMuxRawToDigiPackedEOD.inputTag = cms.InputTag("RPCTwinMuxDigiToRawEOD")

process.RPCDigiComparisonAnalyserPackedEOD = process.RPCDigiComparisonAnalyser.clone()
process.RPCDigiComparisonAnalyserPackedEOD.lhsDigiCollection = cms.InputTag("RPCTwinMuxRawToDigi")
process.RPCDigiComparisonAnalyserPackedEOD.lhsDigiCollectionName = cms.string("TwinMux")
process.RPCDigiComparisonAnalyserPackedEOD.rhsDigiCollection = cms.InputTag("RPCTwinMuxRawToDigiPackedEOD")
process.RPCDigiComparisonAnalyserPackedEOD.rhsDigiCollectionName = cms.string("PackedEOD")

process.RPCDigiComparisonRollAnalyserPackedEOD = process.RPCDigiComparisonRollAnalyser.clone()
process.RPCDigiComparisonRollAnalyserPackedEOD.lhsDigiCollection = cms.InputTag("RPCTwinMuxRawToDigi")
process.RPCDigiComparisonRollAnalyserPackedEOD.lhsDigiCollectionName = cms.string("TwinMux")
process.RPCDigiComparisonRollAnalyserPackedEOD.rhsDigiCollection = cms.InputTag("RPCTwinMuxRawToDigiPackedEOD")
process.RPCDigiComparisonRollAnalyserPackedEOD.rhsDigiCollectionName = cms.string("PackedEOD")

process.RPCAMCLinkCountersAnalyserPackedEOD = process.RPCAMCLinkCountersAnalyser.clone()
process.RPCAMCLinkCountersAnalyserPackedEOD.RPCAMCLinkCounters = cms.InputTag("RPCTwinMuxRawToDigiPackedEOD")

# Do the same for random digis
process.load("DPGAnalysis.RPC.RPCRandomDigiProducer_cff")
process.RPCRandomDigiProducer.bxMin = cms.int32(-2)
process.RPCRandomDigiProducer.bxMax = cms.int32(2)
process.RPCRandomDigiProducer.hitsMax = cms.uint32(2)
process.RPCRandomDigiProducer.clsMax = cms.uint32(96)
process.RPCRandomDigiProducer.rollSelection = cms.vstring("RB")

process.RPCTwinMuxDigiToRawRandom = process.RPCTwinMuxDigiToRaw.clone()
process.RPCTwinMuxDigiToRawRandom.inputTag = cms.InputTag("RPCRandomDigiProducer")

process.RPCTwinMuxRawToDigiRandom = process.RPCTwinMuxRawToDigi.clone()
process.RPCTwinMuxRawToDigiRandom.inputTag = cms.InputTag("RPCTwinMuxDigiToRawRandom")

process.RPCDigiComparisonAnalyserRandom = process.RPCDigiComparisonAnalyser.clone()
process.RPCDigiComparisonAnalyserRandom.lhsDigiCollection = cms.InputTag("RPCRandomDigiProducer")
process.RPCDigiComparisonAnalyserRandom.lhsDigiCollectionName = cms.string("Random")
process.RPCDigiComparisonAnalyserRandom.rhsDigiCollection = cms.InputTag("RPCTwinMuxRawToDigiRandom")
process.RPCDigiComparisonAnalyserRandom.rhsDigiCollectionName = cms.string("Packed")

process.RPCDigiComparisonRollAnalyserRandom = process.RPCDigiComparisonRollAnalyser.clone()
process.RPCDigiComparisonRollAnalyserRandom.lhsDigiCollection = cms.InputTag("RPCRandomDigiProducer")
process.RPCDigiComparisonRollAnalyserRandom.lhsDigiCollectionName = cms.string("Random")
process.RPCDigiComparisonRollAnalyserRandom.rhsDigiCollection = cms.InputTag("RPCTwinMuxRawToDigiRandom")
process.RPCDigiComparisonRollAnalyserRandom.rhsDigiCollectionName = cms.string("Packed")

process.RPCAMCLinkCountersAnalyserRandom = process.RPCAMCLinkCountersAnalyser.clone()
process.RPCAMCLinkCountersAnalyserRandom.RPCAMCLinkCounters = cms.InputTag("RPCTwinMuxRawToDigiRandom")

# Do the same for random digis taking EOD into account
process.RPCTwinMuxDigiToRawRandomEOD = process.RPCTwinMuxDigiToRaw.clone()
process.RPCTwinMuxDigiToRawRandomEOD.ignoreEOD = cms.bool(False)
process.RPCTwinMuxDigiToRawRandomEOD.inputTag = cms.InputTag("RPCRandomDigiProducer")

process.RPCTwinMuxRawToDigiRandomEOD = process.RPCTwinMuxRawToDigi.clone()
process.RPCTwinMuxRawToDigiRandomEOD.inputTag = cms.InputTag("RPCTwinMuxDigiToRawRandomEOD")

process.RPCDigiComparisonAnalyserRandomEOD = process.RPCDigiComparisonAnalyser.clone()
process.RPCDigiComparisonAnalyserRandomEOD.lhsDigiCollection = cms.InputTag("RPCRandomDigiProducer")
process.RPCDigiComparisonAnalyserRandomEOD.lhsDigiCollectionName = cms.string("Random")
process.RPCDigiComparisonAnalyserRandomEOD.rhsDigiCollection = cms.InputTag("RPCTwinMuxRawToDigiRandomEOD")
process.RPCDigiComparisonAnalyserRandomEOD.rhsDigiCollectionName = cms.string("Packed")

process.RPCDigiComparisonRollAnalyserRandomEOD = process.RPCDigiComparisonRollAnalyser.clone()
process.RPCDigiComparisonRollAnalyserRandomEOD.lhsDigiCollection = cms.InputTag("RPCRandomDigiProducer")
process.RPCDigiComparisonRollAnalyserRandomEOD.lhsDigiCollectionName = cms.string("Random")
process.RPCDigiComparisonRollAnalyserRandomEOD.rhsDigiCollection = cms.InputTag("RPCTwinMuxRawToDigiRandomEOD")
process.RPCDigiComparisonRollAnalyserRandomEOD.rhsDigiCollectionName = cms.string("Packed")

process.RPCAMCLinkCountersAnalyserRandomEOD = process.RPCAMCLinkCountersAnalyser.clone()
process.RPCAMCLinkCountersAnalyserRandomEOD.RPCAMCLinkCounters = cms.InputTag("RPCTwinMuxRawToDigiRandomEOD")

process.options = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )

# Source
process.source = cms.Source("PoolSource"
                            , fileNames = cms.untracked.vstring(options.inputFiles)
                            # , lumisToProcess = lumilist.getVLuminosityBlockRange()
                            # , inputCommands =  cms.untracked.vstring("keep *"
                            # , "drop GlobalObjectMapRecord_*_*_*")
)

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(10000) )
# process.maxLuminosityBlocks = cms.untracked.PSet(input = cms.untracked.int32(10))

process.p = cms.Path( ( ( ( process.rpcUnpackingModule + process.RPCTwinMuxRawToDigi ) # first unpack
                          * ( process.RPCDigiComparisonAnalyser + process.RPCDigiComparisonRollAnalyser + process.RPCAMCLinkCountersAnalyser # then analyse this
                              + ( process.RPCTwinMuxDigiToRaw * process.RPCTwinMuxRawToDigiPacked # and repack and unpack again
                                  * ( process.RPCDigiComparisonAnalyserPacked + process.RPCDigiComparisonRollAnalyserPacked + process.RPCAMCLinkCountersAnalyserPacked ) ) # then analyse that too
                              + ( process.RPCTwinMuxDigiToRawEOD * process.RPCTwinMuxRawToDigiPackedEOD # and repack and unpack again with EOD
                                  * ( process.RPCDigiComparisonAnalyserPackedEOD + process.RPCDigiComparisonRollAnalyserPackedEOD + process.RPCAMCLinkCountersAnalyserPackedEOD) ) # then analyse that
                          ) )
                        + ( process.RPCRandomDigiProducer
                            * ( ( process.RPCTwinMuxDigiToRawRandomEOD * process.RPCTwinMuxRawToDigiRandomEOD
                                  * ( process.RPCDigiComparisonAnalyserRandomEOD + process.RPCDigiComparisonRollAnalyserRandomEOD + process.RPCAMCLinkCountersAnalyserRandomEOD ) )
                                + ( process.RPCTwinMuxDigiToRawRandom * process.RPCTwinMuxRawToDigiRandom
                                    * ( process.RPCDigiComparisonAnalyserRandom + process.RPCDigiComparisonRollAnalyserRandom + process.RPCAMCLinkCountersAnalyserRandom ) )
                            ) ) )
                      * process.EventFilter # finally, throw event-data away
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
