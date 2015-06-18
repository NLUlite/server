.SUFFIXES:
.SUFFIXES: .cpp .o

#CPP= g++ -g --ansi --pedantic  #-pg
#CC= gcc -g  #-pg
#CPP= g++ -O4 -funroll-loops --ansi --pedantic
#CC= gcc -O4 -funroll-loops
#CPP= clang -O3 -funroll-loops --ansi --pedantic
#CC= clang -O3  -funroll-loops 


INFEROBJ=  main.o PTElInst.o fopl.o ran.o gaussian.o DrtPred.o clause_vector.o \
	logic_path_vector.o inference_vector.o metric.o \
	metric_singleton.o path_memory.o solver.o ../match/Match.a
LIBOBJ=  PTElInst.o fopl.o ran.o gaussian.o path_memory.o solver.o DrtPred.o clause_vector.o \
	logic_path_vector.o inference_vector.o metric.o \
	metric_singleton.o path_memory.o solver.o ../match/Match.a

LIBS= # -lboost_thread

iv: $(INFEROBJ)
	$(CPP) -o $@ $^

lib: $(LIBOBJ)
	ar rvs infero_vector.a $^


fopl.o: ../fopl/fopl.cpp ../fopl/fopl.hpp
	$(CPP) -c $< -I /usr/local/include

DrtPred.o: ../drt/DrtPred.cpp ../drt/DrtPred.hpp
	$(CPP) -c $< -I /usr/local/include


PTElInst.o: ../instantiations/PTElInst.cpp
	$(CPP) -c $<	-I /usr/local/include

metric.o: ../drt/metric.cpp
	$(CPP) -c $<	-I /usr/local/include

metric_singleton.o: ../drt/metric_singleton.cpp
	$(CPP) -c $<	-I /usr/local/include


ran.o: ran.c
	$(CC) -c $<	

gaussian.o: gaussian.c
	$(CC) -c $<	


.cpp.o:; $(CPP) -c $< -I /usr/local/include

.PHONY: clean ~

clean: 
	@rm *.o *.a
	@echo "All's clean!"
~:
	@rm *~
	@echo "~ cleaned"
