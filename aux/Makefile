.SUFFIXES:
.SUFFIXES: .cpp .o

#CPP= g++ -O4 -funroll-loops --ansi --pedantic
#CC= gcc -O4 -funroll-loops 
#CPP= g++ -g  --ansi --pedantic #-pg
#CC= gcc -g  #-pg
#CPP= clang++ -O3 -funroll-loops --ansi --pedantic
#CC= clang -O3  -funroll-loops 


LIBOBJ= parameters_singleton.o parameters.o
LIBS= #-lboost_thread

lib: $(LIBOBJ)
	ar rvs aux.a $^

parameters_singleton.o: parameters_singleton.cpp
	$(CPP) -c $<	-I /usr/local/include
parameters.o: parameters.cpp
	$(CPP) -c $<	-I /usr/local/include

.cpp.o:; $(CPP) -c $< -I /usr/local/include

.PHONY: clean ~

clean: 
	@rm *.o
	@echo "All's clean!"
~:
	@rm *~
	@echo "~ cleaned"
