#CPP= g++ -O3 -funroll-loops --ansi --pedantic
#CPP= g++ -g -funroll-loops --ansi --pedantic

#CPP= clang++ -O3 -funroll-loops --pedantic 
#CC= clang -O3  -funroll-loops

fopl: main.o fopl.o PTElInst.o $(BOOST)libboost_system.a
	$(CPP) -o $@ $^  $(LIBS) 

main.o: main.cpp 
	$(CPP) -c $< -I /usr/local/include

fopl.o: fopl.cpp fopl.hpp
	$(CPP) -c $< -I /usr/local/include

PTElInst.o: ../instantiations/PTElInst.cpp
	$(CPP) -c $< -I /usr/local/include

fixed_class.o: ../fixed_allocator/fixed_class.cpp #../fixed_allocator/fixed_allocator.cpp
	$(CPP) $^ -c -I /usr/local/include

.PHONY: clean ~

clean: 
	@rm fopl *.o
	@echo "All's clean!"
~:
	@rm *~
	@echo "~ cleaned"
