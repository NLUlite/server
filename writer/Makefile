.SUFFIXES:
.SUFFIXES: .cpp .o

#CPP= g++ -g --ansi --pedantic  #-pg
#CC= gcc -g  #-pg
#CPP= g++ -O4 -funroll-loops --ansi --pedantic
#CC= gcc -O4 -funroll-loops
#CPP= clang++ -O3 -funroll-loops --ansi --pedantic -static
#CC= clang -O3  -funroll-loops -static


LIBOBJ= sentence_tree.o	Writer.o 

LIBS= # -lboost_thread

lib: $(LIBOBJ)
	ar rvs Writer.a $^

PTElInst.o: ../instantiations/PTElInst.cpp
	$(CPP) -c $<	-I /usr/local/include

fopl.o: ../fopl/fopl.cpp ../fopl/fopl.hpp
	$(CPP) -c $< -I /usr/local/include

DrtPred.o: ../drt/DrtPred.cpp ../drt/DrtPred.hpp
	$(CPP) -c $< -I /usr/local/include

sentence_tree.o: ../instantiations/sentence_tree.cpp
	$(CPP) -c $<	-I /usr/local/include


metric.o: ../drt/metric.cpp
	$(CPP) -c $<	-I /usr/local/include

metric_singleton.o: ../drt/metric_singleton.cpp
	$(CPP) -c $<	-I /usr/local/include

.cpp.o:; $(CPP) -c $< -I /usr/local/include

.PHONY: clean ~

clean: 
	@rm iv *.o *.a
	@echo "All's clean!"
~:
	@rm *~
	@echo "~ cleaned"
