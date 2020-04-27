CPPFLAGS = -std=c++11 -fopenmp -O3
DEPS = src/Node.h src/Utils.h 
TXDEPS = src/TX.h src/TX.cpp $(DEPS)

nontx: src/TX.cpp $(DEPS)
	g++ src/TX.cpp test/testSortedList.cpp -o $@ $(CPPFLAGS)

txsingle: test/testSingle.cpp src/GVC.h $(TXDEPS) 
	g++ src/TX.cpp test/testSingle.cpp -o $@ $(CPPFLAGS)

txmulti: test/testMulti.cpp src/GVC.h $(TXDEPS)
	g++ src/TX.cpp test/testMulti.cpp -o $@ $(CPPFLAGS)

q: test/testTxQueue.cpp src/Queue.h $(DEPS)
	g++ src/TX.cpp test/testTxQueue.cpp -o $@ $(CPPFLAGS)