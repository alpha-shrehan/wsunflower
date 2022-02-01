## Sunflower
> A programming language for fast coding and micro systems.

Contents:
* [General Information](#general-information)
* [Build Instructions](#build-instructions)
* [Embedding](#embedding)
* [Copyright](#copyright)


#### <u id='generalinformation'>General Information:</u>
This is a programming language implemented in C, targeted for automation and scripts.
Sunflower uses many third party libraries for its functioning. For linking such libraries, it is recommended to link them
* Dynamically for Windows.
* Statically for Unix based OS.

Dynamic libraries for Windows are provided in the ```DLLs/``` folder and static libraries for Unix based OS are provided in the ```StaticLibs/``` folder.
The various libraries used in Sunflower are as follows: <br>
* [BDW-GC](https://github.com/ivmai/bdwgc): Boehm-Demers-Weiser Garbage Collector.
* [libatomic_ops](github.com/ivmai/libatomic_ops.git): Required by GC.

> Note: Sunflower does NOT itself compile dynamically. However, it can be easily linked with dynamic libraries.
>  Always compile it statically (default build option). For Windows, use MSYS2 or Cygwin.
>  Use a compiler tool-chain which supports the creation and linking of ```.a``` files.

#### <u id='buildinstructions'>Build Instructions</u>
Build Setup
```bash
git clone https://www.github.com/alpha-shrehan/wsunflower.git
cd wsunflower
mkdir build
```

With Makefile and ```make```
```bash
make compile
make run
```

> Note: Makefile is only for those systems which support the ```-j```  and ```-D``` option in CMake.

With ```cmake```
```bash
cd build
cmake ..
cmake --build . --target all -DCMAKE_BUILD_TYPE=Debug -j2
```

After a successful build, ```libsunflower.a``` should be present in ```build/``` folder.

#### <u id='embedding'>Embedding</u>
An easy to use Sunflower API is still in development, and will be ready in the future releases. Kindly refer to [HACKING](#hacking) for high level embedding.

#### <u id='hacking'>HACKING</u>

This section covers the usage of Sunflower for personal use.
Given the following directory structure:
```
build
 |----	gcmt-dll.dll  		<--- For Windows
 |----	msys-gc-1.dll  		<--- For Windows
 |----	msys-gcmt-dll.dll	<--- For Windows
bin
 |----	libsunflower.a
src
 |----	test.c
wsunflower
 |----	Sunflower source...
 
CMakeLists.txt
```

test.c
```c
#include  <sunflower.h>

int main(int argc,  char  const  *argv[])
{
	SF_InitEnv();
	OSF_cmd_set_flag(OSF_cmd_flag_new(CMD_FLAG_DETAILED_ERRORS,  NULL,  0));

	psf_byte_array_t  *ast;
	char *fd = "write('Hello, Sunflower!')"; // Sunflower code
	OSF_SetFileData(fd);

	OSF_SetFileName("<stdin>");
	ast = PSF_AST_fromString(fd);
	if  (OSF_GetExceptionState())
	{
		except_t *expr_log = OSF_GetExceptionLog();
		OSF_RaiseExceptionMessage(expr_log);
		exit(EXIT_FAILURE);
	}
	
	PSF_AST_Preprocess_fromByteArray(ast);
	if (OSF_GetExceptionState())
	{
		except_t *expr_log = OSF_GetExceptionLog();
		OSF_RaiseExceptionMessage(expr_log);
		exit(EXIT_FAILURE);
	}
	
	mod_t *mod = SF_CreateModule(MODULE_TYPE_FILE, ast);
	SF_FrameIT_fromAST(mod);
	SFAdd_Protos_for_built_in_types();
	SFBuiltIn_AddDefaultFunctions(mod);
	
	int err;
	OSF_Free(IPSF_ExecIT_fromMod(mod, &err));
	
	if  (err != IPSF_OK)
	{
		if (err == IPSF_NOT_OK_CHECK_EXPR_LOG)
		{
			except_t *expr_log = OSF_GetExceptionLog();
			OSF_RaiseExceptionMessage(expr_log);
		}
		exit(EXIT_FAILURE);
	}
	
	_IPSF_DestClasses(mod);
	SF_Module_safeDelete(mod);
	OSF_Free(mod);
	SF_DestroyEnv();
	
	return  0;
}
```

CMakeLists.txt
```c
cmake_minimum_required(VERSION 3.0.0)
project(sunhack VERSION 0.1.0)

include_directories(wsunflower)
target_link_libraries(sunhack ./bin/libsunflower.a)

add_executable(sunhack "./src/test.c")
```
Build hack
```bash
cd build
cmake .. -G "MinGW Makefiles" # Recommended
cmake --build .
./sunhack # .\sunhack.exe	  <-- For Windows
```

Variables:
```c
/* mod_t *mod = SF_CreateModule(MODULE_TYPE_FILE, NULL); */

IPSF_AddVar_toModule(mod, "var", (expr_t) {
	.type = EXPR_TYPE_CONSTANT,
	.v.constant = {
		.constant_type = CONSTANT_TYPE_INT,
		.Int.value = 10
	}
}); // var = 10

/* Strings in Sunflower must be enclosed within '' */
IPSF_AddVar_toModule(mod, "str_var", (expr_t) {
	.type = EXPR_TYPE_CONSTANT,
	.v.constant = {
		.constant_type = CONSTANT_TYPE_STRING,
		.String.value = "\'Hello, String!\'"
	}
}); // str_var = 'Hello, String!'

```
> Refer to ```Parser/psf_gen_inst_ast.h``` for more information regarding Sunflower  data structures.

Functions:
```c
expr_t *print(mod_t *mod_ref) {
	expr_t *__ret = OSF_Malloc(sizeof(expr_t));
	*__ret = (expr_t) {
		.type = EXPR_TYPE_CONSTANT,
		.v.constant = {
			.constant_type = CONSTANT_TYPE_DTYPE,
			.Dtype.type = DATA_TYPE_NONE // never use DATA_TYPE_VOID
		}
	};
	var_t *arg = IPSF_GetVar_fromMod(mod_ref, "val", NULL);
	printf("%s\n", _IPSF_ObjectRepr(arg->val, 0));
	
	return __ret;
}
```
Function configuration:
```c
fun_t print_fun = (fun_t){
	.name = "print",
	.is_native = 1,
	.arg_acceptance_count = 1,
	.v.Native = {
		.arg_size = 1,
		.args = OSF_Malloc(sizeof(expr_t /* var_t for Non-Native functions */)),
		.f = print},
	.parent = mod};

print_fun.v.Native.args[0] = (expr_t) {
	.type = EXPR_TYPE_VARIABLE,
	.v.variable = {.name = "val"},
	.next = NULL /* Recommended */
};

int func_idx = PSG_AddFunction(print_fun);
```
Fetch a function from global memory table:
```c
fun_t _f_print = (*PSG_GetFunctions())[func_idx];
// fun_t _f_print = (*PSG_GetFunctions())[IPSF_GetVar_fromMod(mod, "print", NULL)->val.v.function_s.index];
```

#### <u id='copyright'>Copyright</u>
Copyright 2022 Shrehan Raj Singh

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.