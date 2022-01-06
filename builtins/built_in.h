#pragma once

#include <Object/osf_mem.h>
#include <Parser/psf_gen_ast.h>
#include <Parser/psf_gen_inst_ast.h>
#include <Parser/psf_gen_ast_preprocessor.h>
#include <InstructionParser/ipsf_init.h>

expr_t *NativeFunctionWrite(mod_t *);
expr_t *NativeFunctionInput(mod_t *);
expr_t *NativeFunctionInt(mod_t *);
expr_t *NativeFunctionEval(mod_t *);

// Int methods
expr_t *Native_Proto_Int__str__(mod_t *);

// Array methods
expr_t *Native_Proto_Array_push(mod_t *);

/**
 * @brief Add basic native functions like write, input
 * @param mod Module
 */
void SFBuiltIn_AddDefaultFunctions(mod_t *);

/**
 * @brief Add prototypes for built-in data types.
 * Like int, string
 */
void SFAdd_Protos_for_built_in_types(void);

const char *read_file(const char *);