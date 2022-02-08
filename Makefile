# For those restless programmers (me) who want to compile and run tests in one step

OS 		?= UNIX
CONFIG 	?= Debug

all: compile run

compile:
	cmake --build ./build -DCMAKE_BUILD_TYPE=$(CONFIG) --target all -j 6

run:
ifeq ($(OS),Windows_NT)
	.\build\tests\test_exe tests/test.sf
else
	./build/tests/test_exe tests/test.sf
endif

debug:
ifeq ($(OS),Windows_NT)
	gdb --args .\build\tests\test_exe tests/test.sf
else
	gdb --args ./build/tests/test_exe tests/test.sf
endif

rht:
ifeq ($(OS),Windows_NT)
	.\build\tests\hashtest\hashtest_exe
else
	./build/tests/hashtest/hashtest_exe
endif