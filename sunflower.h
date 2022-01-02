#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Object
#include <Object/osf_mem.h>

// Parser
#include <Parser/psf_gen_ast.h>
#include <Parser/psf_gen_ast_preprocessor.h>
#include <Parser/psf_gen_inst_ast.h>

// Instruction Parser
#include <InstructionParser/ipsf_init.h>

#include <gc.h>

/**
 * @brief Initialize Environment for Sunflower
 */
void SF_InitEnv(void);

/**
 * @brief Destroy Sunflower stacks
 */
void SF_DestroyEnv(void);