CXX=g++
CFLAGS = -std=c++11 -o0 -g
OBJECTS = Clustering.o IPLoM.o Test.o MurmurHash3.o Utils.o lcs.o logTrie.o

# Change this to wherever you have boost installed
BOOST_DIR = /usr/lib/x86_64-linux-gnu/

BOOST_LIBS = -lboost_system -lboost_filesystem 

default:
	make main

Clustering.o: Clustering.cpp Clustering.h
	${CXX} ${CFLAGS} -c  Clustering.cpp -o Clustering.o

IPLoM.o: IPLoM.cpp IPLoM.h
	${CXX} ${CFLAGS} -c IPLoM.cpp -o IPLoM.o

Test.o: Test.cpp Test.h
	${CXX} ${CFLAGS} -c Test.cpp -o Test.o

MurmurHash3.o: MurmurHash3.cpp MurmurHash3.h
	${CXX} ${CFLAGS} -c MurmurHash3.cpp -o MurmurHash3.o

Utils.o: Utils.cpp Utils.h
	${CXX} ${CFLAGS} -c Utils.cpp -o Utils.o -I ${BOOST_DIR}/include 

lcs.o: lcs.cpp lcs.h
	${CXX} ${CFLAGS} -c lcs.cpp -o lcs.o -lboost_system -L ./libjson -ljson

logTrie.o: logTrie.cpp logTrie.h
	${CXX} ${CFLAGS} -c logTrie.cpp -o logTrie.o

main: main.cpp ${OBJECTS}
	${CXX} ${CFLAGS} main.cpp -o main ${OBJECTS} ${BOOST_LIBS} -L ./libjson -ljson

test: main
	./main
	
clean:
	rm -f *.o main
