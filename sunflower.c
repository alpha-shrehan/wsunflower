#include "sunflower.h"

void SF_InitEnv(void)
{
    GC_INIT();
    OSF_MemInit();
    OSF_ExceptionInit();
    OSF_cmd_init();
    _PSG_EnvInit();
}

void SF_DestroyEnv(void)
{
    OSF_MemDestroy();
}