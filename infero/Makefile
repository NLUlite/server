.SUFFIXES:
.SUFFIXES: .cpp .o

#CPP= g++ -g --ansi --pedantic #-pg
#CC= gcc -g #-pg
#CPP= g++ -O4 -funroll-loops --ansi --pedantic
#CC= gcc -O4 -funroll-loops
#CPP= clang -O3 -funroll-loops --ansi --pedantic
#CC= clang -O3  -funroll-loops 


INFEROBJ=  PTElInst.o  fopl.o ran.o gaussian.o FuzzyPred.o Clause.o Policy.o LogicPath.o Inference.o Memory.o
LIBS= # -lboost_thread

lib: $(INFEROBJ)
	ar rvs libInfero.a $^

fopl.o: ../fopl/fopl.cpp ../fopl/fopl.hpp
	$(CPP) -c $< -I /usr/local/include

PTElInst.o: ../instantiations/PTElInst.cpp
	$(CPP) -c $<	-I /usr/local/include

ran.o: ran.c
	$(CC) -c $<	

gaussian.o: gaussian.c
	$(CC) -c $<	


.cpp.o:; $(CPP) -c $< -I /usr/local/include

.PHONY: clean ~

clean: 
	@rm *.o
	@echo "All's clean!"
~:
	@rm *~
	@echo "~ cleaned"
