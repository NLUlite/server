.SUFFIXES:
.SUFFIXES: .cpp .o

#CPP= g++ -O4 -funroll-loops --ansi --pedantic
#CC= gcc -O4 -funroll-loops 
#CPP= g++ -g --ansi --pedantic
#CC= gcc -g 
#CPP= clang++ -O3 -funroll-loops --pedantic
#CC= clang -O3  -funroll-loops



DRTOBJ= WisdomServer.o main.o ran.o\
	../wisdom/Wisdom.a ../arbiter/Arbiter.a ../knowledge/Knowledge.a ../writer/Writer.a ../drt/drt.a ../nl/parser.a ../infero_vector/infero_vector.a ../infero/libInfero.a  ../match/Match.a\
	../pragmatics/Pragmatics.a ../context/Context.a ../aux/aux.a\
	$(BOOST)libboost_thread.a $(BOOST)libboost_system.a $(BOOST)libboost_program_options.a  ../sense/Sense.a
LIBS=   # -lboost_system -lboost_thread

server:	$(DRTOBJ)
	$(CPP) -pthread -o $@ $^ $(LIBS)

lib: $(LIBOBJ)
	ar rvs drt.a $^

ran.o: ran.c
	$(CC) -c $<	

.cpp.o:; $(CPP) -c $< -I /usr/local/include

.PHONY: clean ~

clean: 
	@rm server *.o
	@echo "All's clean!"
~:
	@rm *~
	@echo "~ cleaned"
