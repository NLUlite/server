.SUFFIXES:
.SUFFIXES: .cpp .o

#CPP= g++ -g --ansi --pedantic # -pg
#CC= gcc -g # -pg
#CPP= g++ -O4 -funroll-loops --ansi --pedantic
#CC= gcc -O4  -funroll-loops 
#CPP= clang++ -O3 -funroll-loops --ansi --pedantic 
#CC= clang -O3  -funroll-loops 

#CC= /usr/local/musl/bin/musl-gcc -O4  -funroll-loops -L/usr/local/musl/include/


PARSEROBJ=  main.o parser.o parser_info.o WisdomInfo.o parser_aux.o ../infero/libInfero.a tagger_info.o tagger.o tagger_aux.o NumeralCast.o Filter.o metric_singleton.o metric.o ../context/Context.a $(BOOST)libboost_system.a $(BOOST)libboost_thread.a ../aux/aux.a
AROBJ=  parser.o parser_aux.o parser_info.o tagger_info.o tagger.o tagger_aux.o NumeralCast.o Filter.o metric_singleton.o metric.o WisdomInfo.o
LIBS= #-lboost_thread

nl: $(PARSEROBJ)
	$(CPP) -pthread -o $@ $^ $(LIBS)

ran.o: ran.c
	$(CC) -c $<

lib: $(AROBJ)
	ar rvs parser.a $^

metric_singleton.o: ../drt/metric_singleton.cpp
	$(CPP) -c $<	-I /usr/local/include

WisdomInfo.o: ../wisdom/WisdomInfo.cpp
	$(CPP) -c $<	-I /usr/local/include


metric.o: ../drt/metric.cpp
	$(CPP) -c $<	-I /usr/local/include

.cpp.o:; $(CPP) -c $< -I /usr/local/include

.PHONY: clean ~

clean: 
	@rm nl *.o
	@echo "All's clean!"
~:
	@rm *~
	@echo "~ cleaned"
