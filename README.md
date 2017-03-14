## RPC CPPF EMTF phi-theta conversion

CMSSW Analyser to produce the CPPF EMTF phi-theta conversion table for clustered RPC hits.

It depends on several conditions, including the RPCLBLinkMapRcd and RPCCPPFLinkMapRcd, and their inverse.  The latter are in the process of being introduced to CMSSW.


## Use

This code should be used from subdirectory `DPGAnalysis/RPC`.

When developing user code, this branch can be used directly in this folder.  When developing for CMSSW, it should be added as a submodule.
