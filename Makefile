# For those restless programmers who want to compile and run tests in one step

all: compile run

compile:
	cmake --build c:/Users/USER/Desktop/Sunflower/build --config Debug --target all -j 6

run:
	.\build\tests\test_exe

debug:
	gdb .\build\tests\test_exe