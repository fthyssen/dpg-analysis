import FWCore.ParameterSet.Config as cms

import FWCore.ParameterSet.VarParsing as VarParsing
options = VarParsing.VarParsing()
options.register('sinceRun'
                 , 286520 # end of 2016
                 , VarParsing.VarParsing.multiplicity.singleton
                 , VarParsing.VarParsing.varType.int
                 , "IOV Start Run Number")
options.parseArguments()

process = cms.Process("RPCEMTFPhiThetaScaleAnalyser")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.GlobalTag.globaltag = "90X_dataRun2_Prompt_v1"

process.load("DPGAnalysis.RPC.RPCEMTFPhiThetaScaleAnalyser_sqlite_cff")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1)
)

process.source = cms.Source("EmptyIOVSource"
                            , timetype = cms.string('runnumber')
                            , lastValue = cms.uint64(options.sinceRun)
                            , firstValue = cms.uint64(options.sinceRun)
                            , interval = cms.uint64(1))

process.p = cms.Path(process.RPCEMTFPhiThetaScaleAnalyser)
