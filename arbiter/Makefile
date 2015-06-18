.SUFFIXES:
.SUFFIXES: .cpp .o

#CPP= g++ -O4 -funroll-loops --ansi --pedantic
#CC= gcc -O4 -funroll-loops 
#CPP= g++ -g  --ansi --pedantic #-pg
#CC= gcc -g  #-pg
#CPP= clang++ -O3 -funroll-loops --ansi --pedantic -static
#CC= clang -O3  -funroll-loops -static


LIBOBJ= Arbiter.o WikidataSingleton.o WikidataInfo.o\
	../engine/Engine.a\
	../nl/parser.a ../infero_vector/infero_vector.a ../infero/libInfero.a  ../knowledge/Knowledge.a ../match/Match.a ../aux/aux.a ../sense/Sense.a\
	../pragmatics/Pragmatics.a ../context/Context.a 
LIBS= #-lboost_thread

lib: $(LIBOBJ)
	ar rvs Arbiter.a $^

const_tree.o: ../instantiations/const_tree.cpp
	$(CPP) -c $<	-I /usr/local/include

WikidataInfo.o: ../wisdom/WikidataInfo.cpp
	$(CPP) -c $<	-I /usr/local/include

WikidataSingleton.o: ../wisdom/WikidataSingleton.cpp
	$(CPP) -c $<	-I /usr/local/include


.cpp.o:; $(CPP) -c $< -I /usr/local/include

.PHONY: clean ~

clean: 
	@rm drt *.o *.a
	@echo "All's clean!"
~:
	@rm *~
	@echo "~ cleaned"
