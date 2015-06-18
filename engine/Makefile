.SUFFIXES:
.SUFFIXES: .cpp .o

#CPP= clang++ -g -funroll-loops --pedantic 
#CPP= clang++ -O3 -funroll-loops --pedantic 
#CC= clang -O3  -funroll-loops 

#CPP= g++ -O3 -funroll-loops --ansi --pedantic
#CC= gcc -O3 -funroll-loops


PMLOBJ= main.o fopl.o PTElInst.o Engine.o FuzzyPred.o ../knowledge/Knowledge.a   ../match/Match.a  ../aux/aux.a ../drt/drt.a ../nl/parser.a ../infero/libInfero.a ../infero_vector/infero_vector.a ../context/Context.a $(BOOST)libboost_thread.a $(BOOST)libboost_system.a 
LIBOBJ= Engine.o fopl.o PTElInst.o ../aux/aux.a ../infero/libInfero.a ../infero_vector/infero_vector.a ../nl/parser.a $(BOOST)libboost_system.a
LIBS= #-lboost_thread

engine: $(PMLOBJ)
	$(CPP) -pthread -o $@ $^ $(LIBS)

lib: $(LIBOBJ)
	ar rvs Engine.a $^

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

complements.o: ../drt/complements.cpp
	$(CPP) -c $<	-I /usr/local/include

metric.o: ../drt/metric.cpp
	$(CPP) -c $<	-I /usr/local/include

tagger.o: ../nl/tagger.cpp
	$(CPP) -c $<	-I /usr/local/include

tagger_info.o: ../nl/tagger_info.cpp
	$(CPP) -c $<	-I /usr/local/include

parser_singleton.o: ../drt/parser_singleton.cpp
	$(CPP) -c $<	-I /usr/local/include

.cpp.o:; $(CPP) -c $< -I /usr/local/include


.PHONY: clean ~

clean: 
	@rm engine *.o *.a
	@echo "All's clean!"
~:
	@rm *~
	@echo "~ cleaned"
