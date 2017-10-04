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

process = cms.Process("testRPCUnpackers")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

process.MessageLogger.destinations.append("testRPCUnpackers_log")
process.MessageLogger.testRPCUnpackers_log = cms.untracked.PSet(
    threshold = cms.untracked.string("WARNING")
    , FwkReport = cms.untracked.PSet(
        reportEvery = cms.untracked.int32(1)
    )
)

# FrontierConditions
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
# https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideFrontierConditions#Express_reconstruction_Global_Ta
process.GlobalTag.globaltag = "92X_dataRun2_Express_v7"

# Drop all event content
# process.load("DPGAnalysis.Tools.EventFilter_cfi")

# TwinMux
process.load("EventFilter.RPCRawToDigi.RPCTwinMuxRawToDigi_sqlite_cff")

# Link Counters
process.load("DPGAnalysis.RPC.RPCAMCLinkCountersAnalyser_cff")
process.RPCTwinMuxLinkCountersAnalyser = process.RPCAMCLinkCountersAnalyser.clone()
process.RPCTwinMuxLinkCountersAnalyser.RPCAMCLinkCounters = cms.VInputTag("RPCTwinMuxRawToDigi"
                                                                          , "RPCTwinMuxRawToDigi:RPCAMCUnpacker"
)
process.RPCTwinMuxLinkCountersAnalyser.RPCAMCLinkMapRcd = cms.string("RPCTwinMuxLinkMapRcd")
process.RPCTwinMuxLinkCountersAnalyser.Filter = cms.bool(False)

# CPPF
process.load("EventFilter.RPCRawToDigi.RPCCPPFRawToDigi_sqlite_cff")

# Link Counters
process.load("DPGAnalysis.RPC.RPCAMCLinkCountersAnalyser_cff")
process.RPCCPPFLinkCountersAnalyser = process.RPCAMCLinkCountersAnalyser.clone()
process.RPCCPPFLinkCountersAnalyser.RPCAMCLinkCounters = cms.VInputTag("RPCCPPFRawToDigi"
                                                                       , "RPCCPPFRawToDigi:RPCAMCUnpacker"
)
process.RPCCPPFLinkCountersAnalyser.RPCAMCLinkMapRcd = cms.string("RPCCPPFLinkMapRcd")
process.RPCCPPFLinkCountersAnalyser.Filter = cms.bool(False)

# DCC
process.load("EventFilter.RPCRawToDigi.rpcUnpackingModule_cfi")

# Compare CPPF RPCDigis
process.load("DPGAnalysis.RPC.RPCDigiComparisonAnalyser_cff")
process.RPCCPPFDigiComparisonAnalyser = process.RPCDigiComparisonAnalyser.clone()
process.RPCCPPFDigiComparisonAnalyser.Filter = cms.bool(True)
process.RPCCPPFDigiComparisonAnalyser.lhsDigiCollection = cms.InputTag("rpcUnpackingModule")
process.RPCCPPFDigiComparisonAnalyser.lhsDigiCollectionName = cms.string("Legacy")
process.RPCCPPFDigiComparisonAnalyser.rhsDigiCollection = cms.InputTag("RPCCPPFRawToDigi")
process.RPCCPPFDigiComparisonAnalyser.rhsDigiCollectionName = cms.string("CPPF")
process.RPCCPPFDigiComparisonAnalyser.rollSelection = cms.vstring("RE-1/R2", "RE-2/R2", "RE-3/R2", "RE-3/R3", "RE-4/R2", "RE-4/R3"
                                                                  , "RE+1/R2", "RE+2/R2", "RE+3/R2", "RE+3/R3", "RE+4/R2", "RE+4/R3")
process.RPCCPPFDigiComparisonAnalyser.lumiList = lumilist.getVLuminosityBlockRange()

process.RPCCPPFDigiComparisonRollAnalyser = process.RPCCPPFDigiComparisonAnalyser.clone()
process.RPCCPPFDigiComparisonRollAnalyser.Filter = cms.bool(False)
process.RPCCPPFDigiComparisonRollAnalyser.groupPlots = cms.bool(False)
process.RPCCPPFDigiComparisonRollAnalyser.distanceRollWeight = cms.double(10.) # don't allow mismatches here

# Compare TwinMux RPCDigis
process.load("DPGAnalysis.RPC.RPCDigiComparisonAnalyser_cff")
process.RPCTwinMuxDigiComparisonAnalyser = process.RPCDigiComparisonAnalyser.clone()
process.RPCTwinMuxDigiComparisonAnalyser.Filter = cms.bool(True)
process.RPCTwinMuxDigiComparisonAnalyser.lhsDigiCollection = cms.InputTag("rpcUnpackingModule")
process.RPCTwinMuxDigiComparisonAnalyser.lhsDigiCollectionName = cms.string("Legacy")
process.RPCTwinMuxDigiComparisonAnalyser.rhsDigiCollection = cms.InputTag("RPCTwinMuxRawToDigi")
process.RPCTwinMuxDigiComparisonAnalyser.rhsDigiCollectionName = cms.string("TwinMux")
process.RPCTwinMuxDigiComparisonAnalyser.rollSelection = cms.vstring("RB")
process.RPCTwinMuxDigiComparisonAnalyser.lumiList = lumilist.getVLuminosityBlockRange()

process.RPCTwinMuxDigiComparisonRollAnalyser = process.RPCTwinMuxDigiComparisonAnalyser.clone()
process.RPCTwinMuxDigiComparisonRollAnalyser.Filter = cms.bool(False)
process.RPCTwinMuxDigiComparisonRollAnalyser.groupPlots = cms.bool(False)
process.RPCTwinMuxDigiComparisonRollAnalyser.distanceRollWeight = cms.double(10.) # don't allow mismatches here

process.options = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )

# Source
process.source = cms.Source("PoolSource"
                            , fileNames = cms.untracked.vstring(options.inputFiles)
                            , lumisToProcess = lumilist.getVLuminosityBlockRange()
                            # , inputCommands =  cms.untracked.vstring("keep *"
                            # , "drop GlobalObjectMapRecord_*_*_*")
)

#process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(10000) )
process.maxLuminosityBlocks = cms.untracked.PSet(input = cms.untracked.int32(10))

process.p = cms.Path( ( process.RPCCPPFRawToDigi + process.RPCTwinMuxRawToDigi + process.rpcUnpackingModule )
                      * ( ( process.RPCCPPFLinkCountersAnalyser + process.RPCTwinMuxLinkCountersAnalyser )
                          + ( process.RPCCPPFDigiComparisonAnalyser + process.RPCCPPFDigiComparisonRollAnalyser )
                          + ( process.RPCTwinMuxDigiComparisonAnalyser + process.RPCTwinMuxDigiComparisonRollAnalyser )
                      )
                      # * process.EventFilter # and throw event-data away
)

# Output
process.out = cms.OutputModule("PoolOutputModule"
                               , outputCommands = cms.untracked.vstring("drop *"
                                                                        , "keep *_*_*_testRPCUnpackers")
                               # , fileName = cms.untracked.string(options.outputFile)
                               , fileName = cms.untracked.string("testRPCUnpackers.root")
                               , SelectEvents = cms.untracked.PSet(SelectEvents = cms.vstring("p"))
)

process.e = cms.EndPath(process.out)
