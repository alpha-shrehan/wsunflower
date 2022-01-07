#pragma once

#include <Object/osf_mem.h>
#include <Parser/psf_gen_ast.h>
#include <Parser/psf_gen_inst_ast.h>
#include <Parser/psf_gen_ast_preprocessor.h>
#include <InstructionParser/ipsf_init.h>

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