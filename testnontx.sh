#/bin/sh
mkdir result
mkdir result/nontx
for WorkloadType in 0 1 2
do
    echo "nWorkload=10000 WorkloadType=$WorkloadType"
    ./nontx 10000 $WorkloadType > result/nontx/wl${WorkloadType}.txt
done