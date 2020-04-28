#include "../src/SortedList.h"
#include "../src/Node.h"
#include "omp.h"
#include <vector>

#include <random>

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

int main(int argc, char* argv[]) {


    if (argc != 3) {
        printf("usage: ./test [nWorkload] [WorkloadType]\n");
        printf("WorkloadType: PURE_READ=0 PURE_WRITE=1 MIXED=2\n");
        return 0; 
    }
    int nWorkload = atoi(argv[1]);
    int WorkloadType = atoi(argv[2]);

    SortedList<Node, int> sl;
    // pre insert some nodes
    printf("pre insert %d nodes\n", PREINSERT_SIZE);
    auto v = genRandInt(PREINSERT_SIZE,RANGE_MIN,RANGE_MAX);
    for (int i : v) sl.insert(i);
    printf("txsl size: %d\n", sl.size);

    // _debug = true;
    auto v2 = genRandInt(nWorkload,RANGE_MIN,RANGE_MAX);
    // test
    double start = omp_get_wtime();
    int totalAborts = 0;
    int abortCount = 0;

    for (int i=0; i<nWorkload; ++i) {
        abortCount = 0;
        switch (WorkloadType)
        {
        case 0:  
            sl.find(v2[i]);   
            break;
        case 1:
            if (i%2) sl.insert(v2[i]); 
            else sl.remove(v[i]);  
            break;
        case 2: 
            if (i%4==1) sl.insert(v2[i]); 
            else if (i%4==3) sl.remove(v[i]); 
            else sl.find(v2[i]);
            break;
        }       
        totalAborts += abortCount;
    }

    double elapsedTime = omp_get_wtime() - start;
    printf("elapsed time: %lf seconds\n", elapsedTime);
    printf("throughput: %lf ops/seconds\n", 1.0*nWorkload/elapsedTime);
    printf("txsl size: %d\n", sl.size);
}