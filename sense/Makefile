.SUFFIXES:
.SUFFIXES: .cpp .o

#CPP= clang++ -O3 -funroll-loops --ansi --pedantic 
#CC= clang -O3  -funroll-loops 


SENSEOBJ= main.o Sense.o fixed_class.o fopl.o PTElInst.o metric.o metric_singleton.o DrsPersonae.o DrtPred.o drt.o Rules.o complements.o Engine.o FuzzyPred.o ../aux/aux.a ../match/Match.a $(BOOST)libboost_system.a
LIBOBJ= Sense.o 
LIBS= #-lboost_thread

sense: $(SENSEOBJ)
	$(CPP) -o $@ $^ $(LIBS)

lib: $(LIBOBJ)
	ar rvs Sense.a $^

PTElInst.o: ../instantiations/PTElInst.cpp
	$(CPP) -c $<	-I /usr/local/include

fixed_class.o: ../fixed_allocator/fixed_class.cpp #../fixed_allocator/fixed_allocator.cpp
	$(CPP) $^ -c -I /usr/local/include

fopl.o: ../fopl/fopl.cpp ../fopl/fopl.hpp
	$(CPP) -c $< -I /usr/local/include

FuzzyPred.o: ../infero/FuzzyPred.cpp
	$(CPP) -c $< -I /usr/local/include

metric_singleton.o: ../drt/metric_singleton.cpp
	$(CPP) -c $<	-I /usr/local/include

DrsPersonae.o: ../drt/DrsPersonae.cpp
	$(CPP) -c $<	-I /usr/local/include

DrtPred.o: ../drt/DrtPred.cpp
	$(CPP) -c $<	-I /usr/local/include

drt.o: ../drt/drt.cpp
	$(CPP) -c $<	-I /usr/local/include

Rules.o: ../drt/Rules.cpp
	$(CPP) -c $<	-I /usr/local/include

Engine.o: ../engine/Engine.cpp
	$(CPP) -c $<	-I /usr/local/include

complements.o: ../drt/complements.cpp
	$(CPP) -c $<	-I /usr/local/include

metric.o: ../drt/metric.cpp
	$(CPP) -c $<	-I /usr/local/include

.cpp.o:; $(CPP) -c $< -I /usr/local/include


.PHONY: clean ~

clean: 
	@rm sense *.o *.a
	@echo "All's clean!"
~:
	@rm *~
	@echo "~ cleaned"
