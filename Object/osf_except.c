#include "osf_except.h"
#include "osf_cmd.h"

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#endif

except_t *exc_log; // Size 1
int exception_occupied;
char *file_data;
char *file_path;
backtrace_t *bck_log;
int bck_lcount;

void OSF_ExceptionInit(void)
{
    exc_log = OSF_Malloc(sizeof(except_t));
    exception_occupied = 0;
    file_data = OSF_Malloc(1);
    *file_data = '\0';
    file_path = OSF_Malloc(1);
    *file_path = '\0';
    bck_log = OSF_Malloc(sizeof(backtrace_t));
    bck_lcount = 0;
}

int OSF_GetExceptionState(void)
{
    return exception_occupied;
}

void OSF_SetExceptionState(int v)
{
    exception_occupied = v;
}

void OSF_SetException(except_t e)
{
    *exc_log = e;
    OSF_SetExceptionState(1);
}

except_t *OSF_GetExceptionLog(void)
{
    return exc_log;
}

void OSF_RaiseExceptionMessage(except_t *e)
{
    char **spl_fd = Sf_Str_Split(OSF_GetFileData(), "\n");
    int bl = *OSF_GetBacklogSize();
    backtrace_t *bb = *OSF_GetBacklog();
    clopt_t *flg_extra_info = OSF_cmd_get_flag_fromType(CMD_FLAG_DETAILED_ERRORS);

#if defined(WIN32) || defined(_WIN32)
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    WORD saved_attrs;

    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    saved_attrs = consoleInfo.wAttributes;

    SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
    printf("Error: ");

    SetConsoleTextAttribute(hConsole, saved_attrs);
#else
    printf("Error: ");
#endif

    printf("In file '%s',\n", OSF_GetFileName());

    switch (e->type)
    {
    case EXCEPT_INDEX_OUT_OF_RANGE:
    {
        printf("       Requested index out of range.\n");

        if (flg_extra_info != NULL)
        {
            printf("\n");
            printf("Additional meta-data returned by the interpreter:\n");
            printf("[Index requested   : %d]\n", e->v.ce0.idx);
            printf("[Range length      : %d]\n", e->v.ce0.targ.len);
            printf("[Exceeded limit by : %d]\n", e->v.ce0.idx - e->v.ce0.targ.len);
        }
    }
    break;
    case EXCEPT_VARIABLE_DOES_NOT_EXIST:
    {
        printf("       Variable '%s' does not exist.\n", e->v.ce1.vname);

        if (flg_extra_info != NULL)
        {
            printf("\n");
            printf("Additional meta-data returned by the interpreter:\n");
            printf("[Variable referenced : '%s']\n", e->v.ce1.vname);

            if (e->v.ce1.m_ref != NULL)
            {
                int closest_score = INT_MAX;
                char *cv = NULL;

                for (size_t i = 0; i < e->v.ce1.m_ref->var_holds_size; i++)
                {
                    int x = strcmp(e->v.ce1.vname, e->v.ce1.m_ref->var_holds[i].name);
                    if (x < closest_score &&
                        strlen(e->v.ce1.vname) ==
                            strlen(e->v.ce1.m_ref->var_holds[i].name))
                    {
                        closest_score = x;
                        cv = e->v.ce1.m_ref->var_holds[i].name;
                    }
                }

                if (cv != NULL)
                    printf("[Closest Match       : '%s']\n", cv);
            }
        }
    }
    break;
    case EXCEPT_SYNTAX_ERROR:
    {
        if (flg_extra_info != NULL)
        {
            if (e->v.ce10.additional_msg != NULL)
            {
                printf("\n");
                printf("Additional meta-data returned by the interpreter:\n");
                printf("[%s]\n", e->v.ce10.additional_msg);
            }
        }
    }
    break;

    default:
        break;
    }

    printf("\n");

    if (bl)
        if (flg_extra_info != NULL)
        {
            printf("-------------\n");
            printf("| Backtrace |\n");
            printf("-------------\n");

            for (int i = bl - 1; i >= 0; i--)
            {
                // TODO: here
                printf("%d | %s\n", bb[i].line - 1, spl_fd[bb[i].line - 1]);
            }
        }
        else
        {
            /* Print last line */
            int std_l = printf("%d | %s\n", bb[bl - 1].line - 1, spl_fd[bb[bl - 1].line - 1]);
            for (size_t j = 0; j < std_l; j++)
                printf("-");
            printf("\n");
        }
    else if (e->line - 1)
        printf("%d | %s\n", e->line - 1, spl_fd[e->line - 1]);
}

void OSF_SetFileData(char *d)
{
    OSF_Free(file_data);
    file_data = OSF_strdup(d);
}

char *OSF_GetFileData(void)
{
    return file_data;
}

void OSF_SetFileName(char *c)
{
    OSF_Free(c);
    file_path = OSF_strdup(c);
}

char *OSF_GetFileName(void)
{
    return file_path;
}

void OSF_AddBackLog(backtrace_t b)
{
    if (!b.line)
        return;

    if (*OSF_GetBacklogSize())
    {
        if ((*OSF_GetBacklog())[(*OSF_GetBacklogSize()) - 1].line == b.line)
            return;
        (*OSF_GetBacklog()) = OSF_Realloc(*OSF_GetBacklog(), (*OSF_GetBacklogSize() + 1) * sizeof(backtrace_t));
    }

    (*OSF_GetBacklog())[(*OSF_GetBacklogSize())++] = b;
}

void OSF_PopBackLog(void)
{
    if (!(*OSF_GetBacklogSize()))
        return;

    int bl_size = *OSF_GetBacklogSize();
    backtrace_t *cpy = OSF_Malloc((bl_size - 1) * sizeof(backtrace_t));

    for (size_t i = 0; i < bl_size - 1; i++)
        cpy[i] = (*OSF_GetBacklog())[i];

    OSF_Free(*OSF_GetBacklog());
    *OSF_GetBacklog() = cpy;
    (*OSF_GetBacklogSize())--;
}

backtrace_t **OSF_GetBacklog(void)
{
    return &bck_log;
}

int *OSF_GetBacklogSize(void)
{
    return &bck_lcount;
}

void OSF_ClearBacklog(void)
{
    OSF_Free(*OSF_GetBacklog());
    *OSF_GetBacklog() = OSF_Malloc(sizeof(backtrace_t));
    *OSF_GetBacklogSize() = 0;
}

backtrace_t OSF_CreateBackLog(int type, int line)
{
    backtrace_t b;
    b.type = type;
    b.line = line;

    return b;
}

void OSF_RaiseException_SyntaxError(int line, char *msg)
{
    OSF_SetException((except_t){
        .type = EXCEPT_SYNTAX_ERROR,
        .line = line,
        .v.ce10.additional_msg = msg});
}