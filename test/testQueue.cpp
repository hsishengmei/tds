#include "../src/Queue.h"
#include "../src/Node.h"
#include "omp.h"
#include <vector>

#include <random>

#define RANGE_MIN 0
#define RANGE_MAX 2147483647
#define PREINSERT_SIZE 100000

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

int main(int argc, char* argv[]) {


    if (argc != 4) {
        printf("usage: ./test [nThreads] [nWorkload] [WorkloadType]\n");
        printf("WorkloadType: PURE_READ=0 PURE_INSERT=1 PURE_REMOVE=2 MIXED_WRITE=3 MIXED_ALL=4\n");
        return 0; 
    }
    int nThreads = atoi(argv[1]);
    int nWorkload = atoi(argv[2]);
    int WorkloadType = atoi(argv[3]);

    Queue q;
    // pre insert some nodes
    printf("pre insert %d nodes\n", PREINSERT_SIZE);
    {
        auto v = genRandInt(PREINSERT_SIZE,RANGE_MIN,RANGE_MAX);
        for (int i : v) q.push(i);
    }
    printf("q size: %d\n", q.size);

    // _debug = true;
    auto v2 = genRandInt(nWorkload,RANGE_MIN,RANGE_MAX);
    // test
    double start = omp_get_wtime();

    for (int i=0; i<nWorkload; ++i) {
        switch (WorkloadType)
        {
        case 0:  
            q.front();   
            break;
        case 1:
            q.push(v2[i]);   
            break;
        case 2:
            q.pop();   
            break;
        }       
    }

    double elapsedTime = omp_get_wtime() - start;
    printf("elapsed time: %lf seconds\n", elapsedTime);
    printf("nOps: %d\n", nWorkload);
    printf("throughput: %lf ops/seconds\n", 1.0*nWorkload/elapsedTime);
    printf("q size: %d\n", q.size);
}