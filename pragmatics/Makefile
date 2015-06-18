.SUFFIXES:
.SUFFIXES: .cpp .o

#CPP= g++ -O4 -funroll-loops --ansi --pedantic
#CC= gcc -O4 -funroll-loops 
#CPP= g++ -g  --ansi --pedantic
#CC= gcc -g 
#CPP= clang++ -O3 -funroll-loops --ansi --pedantic -static
#CC= clang -O3  -funroll-loops -static


LIBOBJ= Presupposition.o

LIBS= #-lboost_thread

lib: $(LIBOBJ)
	ar rvs Pragmatics.a $^

.cpp.o:; $(CPP) -c $< -I /usr/local/include

.PHONY: clean ~

clean: 
	@rm *.o *.a
	@echo "All's clean!"
~:
	@rm *~
	@echo "~ cleaned"
