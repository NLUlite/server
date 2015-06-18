#CPP= g++ -O4
#CPP= clang++ -O3 -funroll-loops --ansi --pedantic -static
#CC= clang -O3  -funroll-loops -static


INCLUDE= ~/myLib/include
LIB= ~/myLib/lib


Tree: main.o #tree.o
	$(CPP) $< -o$@ -L $(LIB) fixed_class.o

main.o: main.cpp mytree.cpp mytree.hpp fixed_class.o
	$(CPP) $< -I $(INCLUDE) -c

tree.o: mytree.cpp fixed_class.o
	$(CPP) $^ -I $(INCLUDE)  -c

fixed_class.o: ../fixed_allocator/fixed_class.cpp ../fixed_allocator/fixed_allocator.cpp
	$(CPP) $^ -c

.PHONY: clean ~

clean:
	@rm *.o Tree
	@echo "All's clean!"
~:
	@rm *~
	@echo "~ cleaned"

