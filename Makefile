BOOST  = /usr/lib/x86_64-linux-gnu/

CPP = g++ -O3 -funroll-loops --ansi --pedantic -std=c++11
CC  = gcc -O3 -funroll-loops

export CPP
export CC
export BOOST

.SUFFIXES:
.SUFFIXES: .cpp .o

.PHONY: server clean

all: 
	cd fopl && make
	cd aux && make
	cd infero && make
	cd match && make
	cd infero_vector && make lib
	cd knowledge && make
	cd context && make lib
	cd nl && make lib
	cd writer && make lib
	cd pragmatics && make lib
	cd engine && make lib
	cd sense && make lib
	cd arbiter && make lib
	cd drt && make lib
	cd wisdom && make && make lib
	cd server && make

clean:
	-rm */*.a
	-cd fopl && make clean 
	-cd aux && make clean 
	-cd infero && make clean 
	-cd engine && make clean 
	-cd sense && make clean 
	-cd infero_vector && make clean 
	-cd knowledge && make clean
	-cd context && make clean
	-cd nl && make clean 
	-cd match && make clean
	-cd writer && make clean
	-cd pragmatics && make clean
	-cd arbiter && make clean
	-cd drt && make clean 
	-cd wisdom && make clean 
	-cd server && make clean 
