CPPFLAGS = -std=c++11 -fopenmp -O3
DEPS = src/Node.h src/Utils.h 

nontxtest: test/testSortedList.cpp $(DEPS)
	g++ test/testSortedList.cpp -o nontxtest $(CPPFLAGS)

txtest: src/TxSortedList.h src/TxSortedList.cpp test/testSingleton.cpp src/GVC.h $(DEPS)
	g++ src/TxSortedList.cpp test/testSingleton.cpp -o txtest $(CPPFLAGS)