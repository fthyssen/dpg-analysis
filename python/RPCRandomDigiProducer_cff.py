import FWCore.ParameterSet.Config as cms
from Geometry.RPCGeometryBuilder.rpcGeometryDB_cfi import *
from IOMC.RandomEngine.IOMC_cff import RandomNumberGeneratorService

RandomNumberGeneratorService.RPCRandomDigiProducer = cms.PSet(
    initialSeed = cms.untracked.uint32(12345)
)

from DPGAnalysis.RPC.RPCRandomDigiProducer_cfi import RPCRandomDigiProducer
