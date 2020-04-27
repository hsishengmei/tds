#include <iostream>
#include <random>
#include "../src/TxSortedList.h"
#include "../src/Node.h"
#include "omp.h"

#define RANGE_MIN 0
#define RANGE_MAX 2147483647
#define PREINSERT_SIZE 20000

bool _debug = false;

std::vector<int> genRandInt(int sz, int mn, int mx) {
    std::random_device rd;
    std::uniform_int_distribution<int> dist(mn, mx);
    std::vector<int> v;
    v.reserve(sz);
    for (int n = 0; n < sz; ++n) {
        v.push_back(dist(rd));
    }
    return v;
}

int TxInsertSingleton(int i, TxSortedList& txsl) {
    bool done = false;
    int nAborts = 0;
    Transaction tx;
    while (!done) {
        try {
            tx.TxBegin();
            txsl.insert(i, tx);
            tx.TxCommit(txsl);
            done = true;
        }
        catch(const TxAbortException& e) {
            ++nAborts;
        }
    }
    return nAborts;
}

int TxRemoveSingleton(int i, TxSortedList& txsl) {
    bool done = false;
    int nAborts = 0;
    Transaction tx;
    while (!done) {
        try {
            tx.TxBegin();
            txsl.remove(i, tx);
            tx.TxCommit(txsl);
            done = true;
        }
        catch(const TxAbortException& e) {
            ++nAborts;
        }
    }
    return nAborts;
}

int TxReadSingleton(int i, TxSortedList& txsl) {
    bool done = false;
    int nAborts = 0;
    Transaction tx;
    while (!done) {
        try {
            tx.TxBegin();
            txsl.find(i, tx);
            tx.TxCommit(txsl);
            done = true;
        }
        catch(const TxAbortException& e) {
            // printf("abort\n");
            ++nAborts;
        }
    }
    return nAborts;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("usage: ./test [nThreads] [nWorkload] [WorkloadType]\n");
        printf("WorkloadType: PURE_READ=0 PURE_INSERT=1 PURE_REMOVE=2 3READ1WRITE=3 5READ1WRITE=4\n");
        return 0; 
    }
    int nThreads = atoi(argv[1]);
    int nWorkload = atoi(argv[2]);
    int WorkloadType = atoi(argv[3]);

    TxSortedList txsl;
    // pre insert some nodes
    printf("pre insert %d nodes\n", PREINSERT_SIZE);

    auto v = genRandInt(PREINSERT_SIZE,RANGE_MIN,RANGE_MAX);
    Transaction tx;
    tx.TxBegin();
    for (int i : v) txsl.insert(i, tx);
    tx.TxCommit(txsl);

    printf("done, txsl size: %d\n", txsl.size);

    // _debug = true;
    auto v2 = genRandInt(nWorkload,RANGE_MIN,RANGE_MAX);
    // test
    double start = omp_get_wtime();
    int totalAborts = 0;
    int abortCount = 0;
    #pragma omp parallel for num_threads(nThreads) default(none) \
            firstprivate(abortCount, WorkloadType, nWorkload) \
            shared(txsl, v, v2, totalAborts)
    for (int i=0; i<nWorkload; ++i) {
        abortCount = 0;
        switch (WorkloadType)
        {
        case 0:  
            abortCount += TxReadSingleton(v2[i], txsl);   
            break;
        case 1:
            abortCount += TxInsertSingleton(v2[i], txsl);   
            break;
        case 2:
            abortCount += TxRemoveSingleton(v[i], txsl);   
            break;
        case 3:
            switch (i % 4)
            {
            case 0:
                abortCount += TxInsertSingleton(v2[i], txsl);   
                break;
            default:
                abortCount += TxReadSingleton(v2[i], txsl);
                break;
            }
            break;
        case 4:
            switch (i % 6)
            {
            case 0:
                abortCount += TxInsertSingleton(v2[i], txsl);   
                break;
            default:
                abortCount += TxReadSingleton(v2[i], txsl);
                break;
            }
            break;
        }       
        #pragma omp atomic
        totalAborts += abortCount;
    }
    double elapsedTime = omp_get_wtime() - start;
    printf("elapsed time: %lf seconds\n", elapsedTime);
    // printf("nOps: %d\nnAborts: %d\n", nWorkload, totalAborts);
    printf("abort rate: %lf\n", 1.0*totalAborts/nWorkload);
    printf("throughput: %lf ops/seconds\n", 1.0*nWorkload/elapsedTime);
    printf("txsl size: %d\n\n", txsl.size);
}