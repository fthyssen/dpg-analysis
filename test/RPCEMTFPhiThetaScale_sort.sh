#! /bin/sh

for sector in 1 2 3 4; do
    board=$((sector))
    sort -k1,1 -k8,8n -o angleScale_RPC_CPPFp${sector}.txt RPCAMCLink_1386_${board}.txt
    board=$((sector + 4))
    sort -k1,1 -k8,8n -o angleScale_RPC_CPPFn${sector}.txt RPCAMCLink_1386_${board}.txt
done;
