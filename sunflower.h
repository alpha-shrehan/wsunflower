#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Parser/str.h>

// Object
#include <Object/osf_mem.h>
#include <Object/osf_except.h>
#include <Object/osf_cmd.h>

// Parser
#include <Parser/trie.h>
#include <Parser/psf_gen_ast.h>
#include <Parser/psf_gen_ast_preprocessor.h>
#include <Parser/psf_gen_inst_ast.h>

// Instruction Parser
#include <InstructionParser/ipsf_init.h>

// Built-ins
#include <builtins/built_in.h>

#include <gc.h>

/**
 * @brief Initialize Environment for Sunflower
 */
void SF_InitEnv(void);

/**
 * @brief Destroy Sunflower stacks
 */
void SF_DestroyEnv(void);