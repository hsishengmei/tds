#include <iostream>
#include <random>
#include "../src/TX.h"
#include "../src/Node.h"
#include "omp.h"

#define RANGE_MIN 1
#define RANGE_MAX 400000
#define PREINSERT_SIZE 40000

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

int TxReadMulti(std::vector<int> v, TxSortedList& txsl) {
    bool done = false;
    int nAborts = 0;
    Transaction tx;
    while (!done) {
        try {
            tx.TxBegin();
            for (int i=0; i<v.size(); ++i) txsl.find(v[i], tx);
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

int TxWriteMulti(std::vector<int> v, std::vector<int> v2, TxSortedList& txsl) {
    bool done = false;
    int nAborts = 0;
    Transaction tx;
    while (!done) {
        try {
            tx.TxBegin();
            for (int i=0; i<v.size(); ++i) {
                if (i%2 == 1) txsl.insert(v[i], tx);
                else if (i%2 == 3) txsl.remove(v2[i], tx);
                else txsl.find(v[i], tx);
            }
            tx.TxCommit(txsl);
            done = true;
        }
        catch(const TxAbortException& e) {
            ++nAborts;
        }
    }
    return nAborts;
}

int TxMix(std::vector<int> v, std::vector<int> v2, TxSortedList& txsl) {
    bool done = false;
    int nAborts = 0;
    Transaction tx;
    while (!done) {
        try {
            tx.TxBegin();
            for (int i=0; i<v.size(); ++i) {
                if (i%2 == 0) txsl.insert(v[i], tx);
                else txsl.remove(v2[i], tx);
            }
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
    if (argc < 5) {
        printf("usage: ./test [nThreads] [nWorkload] [WorkloadType] [n]\n");
        printf("WorkloadType: PURE_READ=0 PURE_INSERT=1 PURE_REMOVE=2 (n-1)READ(1)WRITE=3 \n");
        return 0; 
    }
    int nThreads = atoi(argv[1]);
    int nWorkload = atoi(argv[2]);
    int WorkloadType = atoi(argv[3]);
    int n = atoi(argv[4]);
    
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
            firstprivate(abortCount, WorkloadType, nWorkload, n) \
            shared(txsl, v, v2, totalAborts)
    for (int i=0; i<nWorkload; i+=n) {
        abortCount = 0;
        std::vector<int> todo; todo.reserve(n);
        for (int j=0; j<n; ++j) todo.push_back(v2[j]);
        std::vector<int> todo2; todo2.reserve(n);
        for (int j=0; j<n; ++j) todo2.push_back(v[j]);
        switch (WorkloadType)
        {
        case 0:  
            abortCount += TxReadMulti(todo, txsl);   
            break;
        case 1:
            abortCount += TxWriteMulti(todo, todo2, txsl);   
            break;
        case 2:
            abortCount += TxMix(todo, todo2, txsl);
            break;
        }       
        #pragma omp critical
        totalAborts += abortCount;
    }
    double elapsedTime = omp_get_wtime() - start;
    printf("elapsed time: %lf seconds\n", elapsedTime);
    // printf("nOps: %d\nnAborts: %d\n", nWorkload, totalAborts);
    printf("abort rate: %lf%%\n", 100.0*totalAborts/nWorkload);
    printf("throughput: %lf ops/seconds\n", 1.0*nWorkload/elapsedTime);
    printf("txsl size: %d\n\n", txsl.size);
}