#/bin/sh

for nThreads in 1 2 4 8 16
do
    for WorkloadType in 0 1 2
    do
        for TxSize in 1 4 16 64
        do
            echo "nThreads=$nThreads nWorkload=10000 WorkloadType=$WorkloadType TxSize=$TxSize"
            ./txmulti $nThreads 10000 $WorkloadType $TxSize > result/multi/nt${nThreads}wl${WorkloadType}sz${TxSize}.txt
        done
    done
done