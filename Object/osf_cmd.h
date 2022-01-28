#pragma once

#include <Parser/psf_gen_inst_ast.h>

enum CommandLineOptionsEnum
{
    CMD_FLAG_H,                 // -h, --help
    CMD_FLAG_VERSION,           // --version
    CMD_FLAG_VERBOSE,           // -v, --verbose
    CMD_FLAG_DETAILED_ERRORS,   // -E, --err (this includes meta-data, backtrace and interpretor comments)
};

struct _cmd_opts_s
{
    enum CommandLineOptionsEnum type;
    char *val;
    int takes_val;
    
};

typedef struct _cmd_opts_s clopt_t;

/**
 * @brief Initialise environment for command line flags
 *
 */
void OSF_cmd_init(void);

/**
 * @brief Check whether a command line flag is valid
 * @param flag Flag
 * @return int 1 | 0
 */
int OSF_cmd_is_valid_flag(char *);

/**
 * @brief Get flags (pointer to holder)
 * @return clopt_t** 
 */
clopt_t **OSF_cmd_get_flags(void);

/**
 * @brief Get flags size (pointer to holder)
 * @return int* 
 */
int *OSF_cmd_get_flags_size(void);

/**
 * @brief Create flag from attributes
 * @param fType flag type
 * @param fVal flag value
 * @param fTv flag parameter necessity
 * @return clopt_t 
 */
clopt_t OSF_cmd_flag_new(int, char *, int);

/**
 * @brief Register a flag to environment
 * @param cl Flag
 */
void OSF_cmd_set_flag(clopt_t);

/**
 * @brief Get flag from value (string)
 * @param val Value
 * @return clopt_t* 
 */
clopt_t *OSF_cmd_get_flag_fromVal(char *);

/**
 * @brief Get flag from type (int). Better and faster than 
 * OSF_cmd_get_flag_fromVal(...)
 * @param type Type
 * @return clopt_t* 
 */
clopt_t *OSF_cmd_get_flag_fromType(int);