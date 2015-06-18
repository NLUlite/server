.SUFFIXES:
.SUFFIXES: .cpp .o

#CPP= g++ -O4 -funroll-loops --ansi --pedantic
#CC= gcc -O4 -funroll-loops 
#CPP= g++ -g --ansi --pedantic
#CC= gcc -g 
#CPP= clang++ -O3 -funroll-loops --ansi --pedantic -static
#CC= clang -O3  -funroll-loops -static



DRTOBJ= Wisdom.o WisdomInfo.o WikidataInfo.o WikidataSingleton.o main.o ../arbiter/Arbiter.a\
        ../writer/Writer.a ../drt/drt.a ../nl/parser.a ../infero_vector/infero_vector.a ../infero/libInfero.a ../knowledge/Knowledge.a ../match/Match.a \
	../pragmatics/Pragmatics.a ../context/Context.a \
        $(BOOST)libboost_thread.a $(BOOST)libboost_system.a $(BOOST)libboost_program_options.a ../aux/aux.a  ../sense/Sense.a 
LIBOBJ= Wisdom.o WisdomInfo.o WikidataInfo.o WikidataSingleton.o main.o \
        ../writer/Writer.a ../drt/drt.a ../nl/parser.a ../infero_vector/infero_vector.a ../infero/libInfero.a ../knowledge/Knowledge.a ../match/Match.a ../aux/aux.a  ../sense/Sense.a
LIBS= #-lboost_system -lboost_thread

wisdomize:	$(DRTOBJ)
	$(CPP) -pthread -o $@ $^ $(LIBS)

lib: $(LIBOBJ)
	ar rvs Wisdom.a $^

.cpp.o:; $(CPP) -c $< -I /usr/local/include

.PHONY: clean ~

clean: 
	@rm wisdomize *.o
	@echo "All's clean!"
~:
	@rm *~
	@echo "~ cleaned"
