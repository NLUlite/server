.SUFFIXES:
.SUFFIXES: .cpp .o

#CPP= g++ -O4 -funroll-loops --ansi --pedantic
#CC= gcc -O4 -funroll-loops 
#CPP= g++ -g  --ansi --pedantic #-pg
#CC= gcc -g  #-pg

#CPP= clang++ -O3 -funroll-loops --pedantic -static
#CC= clang -O3  -funroll-loops -static


DRTOBJ= main.o  metric.o phrase_info.o phrase.o constituents.o const_tree.o parser_singleton.o \
	drt.o phrase_versions.o drt_collection.o drs_anaphora_levels.o\
	metric_singleton.o drt_builder.o drt_builder-corrector.o tools.o\
	DrtPred.o DrsPersonae.o complements.o\
	Rules.o  ../engine/Engine.a ../arbiter/Arbiter.a Triggers.o \
	WisdomInfo.o\
	../nl/parser.a ../infero_vector/infero_vector.a ../infero/libInfero.a  ../knowledge/Knowledge.a ../writer/Writer.a ../match/Match.a ../aux/aux.a\
	../pragmatics/Pragmatics.a ../context/Context.a ../sense/Sense.a\
        $(BOOST)libboost_thread.a $(BOOST)libboost_system.a
LIBOBJ= metric.o phrase_info.o phrase.o constituents.o const_tree.o parser_singleton.o \
	drt.o phrase_versions.o drt_collection.o drs_anaphora_levels.o\
	metric_singleton.o drt_builder.o drt_builder-corrector.o tools.o\
	Rules.o ../engine/Engine.a Triggers.o\
	DrtPred.o DrsPersonae.o complements.o ../arbiter/Arbiter.a\
	../nl/parser.a ../infero_vector/infero_vector.a ../infero/libInfero.a  ../knowledge/Knowledge.a ../match/Match.a ../aux/aux.a ../sense/Sense.a\
	../pragmatics/Pragmatics.a ../context/Context.a 
LIBS= #-lboost_thread

drt: 	$(DRTOBJ)
	$(CPP) -pthread -o $@ $^ $(LIBS)

lib: $(LIBOBJ)
	ar rvs drt.a $^

const_tree.o: ../instantiations/const_tree.cpp
	$(CPP) -c $<	-I /usr/local/include

WisdomInfo.o: ../wisdom/WisdomInfo.cpp
	$(CPP) -c $<	-I /usr/local/include

.cpp.o:; $(CPP) -c $< -I /usr/local/include

.PHONY: clean ~

clean: 
	@rm drt *.o *.a
	@echo "All's clean!"
~:
	@rm *~
	@echo "~ cleaned"
