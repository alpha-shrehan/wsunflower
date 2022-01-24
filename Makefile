# For those restless programmers (me) who want to compile and run tests in one step

all: compile run

compile:
	cmake --build ./build --config Debug --target all -j 6

run:
	ifeq ($(OS),Windows_NT)
		.\build\tests\test_exe
	else
		./build/tests/test_exe
	endif

debug:
	ifeq ($(OS),Windows_NT)
		gdb .\build\tests\test_exe
	else
		gdb ./build/tests/test_exe