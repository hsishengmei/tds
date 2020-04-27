CPPFLAGS = -std=c++11 -fopenmp -O3
DEPS = src/Node.h src/Utils.h 
TXDEPS = src/TxSortedList.h src/TxSortedList.cpp $(DEPS)

nontx: test/testSortedList.cpp $(DEPS)
	g++ test/testSortedList.cpp -o $@ $(CPPFLAGS)

txsingle: test/testSingle.cpp src/GVC.h $(TXDEPS) 
	g++ src/TxSortedList.cpp test/testSingle.cpp -o $@ $(CPPFLAGS)

txmulti: test/testMulti.cpp src/GVC.h $(TXDEPS)
	g++ src/TxSortedList.cpp test/testMulti.cpp -o $@ $(CPPFLAGS)