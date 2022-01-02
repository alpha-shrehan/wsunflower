#include "osf_mem.h"

#define __DOSF_PRINT_HEAP_TRACE__ 0
// #define SF_DEBUG

OSF_MemTree *OSF_MainMemTree = NULL;
unsigned long OSF_LastHeapSize = 0;
unsigned long OSF_FreeCalled = 0;
unsigned long OSF_MallocCalled = 0;
unsigned long OSF_ReallocCalled = 0;

void __OSF_MemInit(const char *__fname, int __line)
{
    OSF_MainMemTree = (OSF_MemTree *)malloc(sizeof(OSF_MemTree));
    OSF_MainMemTree->root = (OSF_MemBlock *)malloc(sizeof(OSF_MemBlock));
    OSF_MainMemTree->size = 0;

    OSF_DetermineHeapSize();
}

void *__OSF_Malloc(size_t size, const char *__fname, int __line)
{
    OSF_MallocCalled++;
    void *ptr = malloc(size);
    if (ptr == NULL)
    {
        OSF_Error("OSF_Malloc: Out of memory", __fname, __line);
    }

    int mem_al_exists = -1;

    for (size_t i = 0; i < OSF_MainMemTree->size; i++)
    {
        if (OSF_MainMemTree->root[i].addr == ptr)
        {
            mem_al_exists = i;
            break;
        }
    }

    if (mem_al_exists != -1)
    {
        // Re-use memory instead of allocating new
        OSF_MainMemTree->root[mem_al_exists].addr = ptr;
        OSF_MainMemTree->root[mem_al_exists].size = size;
        OSF_MainMemTree->root[mem_al_exists].is_free = 0;
        OSF_MainMemTree->root[mem_al_exists].is_old = 0;
        OSF_MainMemTree->root[mem_al_exists].create_type = MEM_BLOCK_CREATE_TYPE_MALLOC;
    }
    else
    {
        if (OSF_MainMemTree->size)
            OSF_MainMemTree->root = (OSF_MemBlock *)realloc(OSF_MainMemTree->root, sizeof(OSF_MemBlock) * (OSF_MainMemTree->size + 1));

        OSF_MainMemTree->root[OSF_MainMemTree->size].addr = ptr;
        OSF_MainMemTree->root[OSF_MainMemTree->size].size = size;
        OSF_MainMemTree->root[OSF_MainMemTree->size].is_free = 0;
        OSF_MainMemTree->root[OSF_MainMemTree->size].is_old = 0;
        OSF_MainMemTree->root[OSF_MainMemTree->size].create_type = MEM_BLOCK_CREATE_TYPE_MALLOC;

        OSF_MainMemTree->size++;
    }

    OSF_DetermineHeapSize();
#if __DOSF_PRINT_HEAP_TRACE__ == 1
    fprintf(stdout, "[malloc] ");
    OSF_PrintHeapSize();
    fprintf(stdout, " in function: %s at line %d\n", __fname, __line);
#endif

    return ptr;
}

void *__OSF_Realloc(void *ptr, size_t size, const char *__fname, int __line)
{
    OSF_ReallocCalled++;
    void *new_ptr = realloc(ptr, size);
    if (new_ptr == NULL)
    {
        OSF_Error("OSF_Realloc: Out of memory", __fname, __line);
    }

    int done = 0,
        removeEntry = -1;

    for (int i = OSF_MainMemTree->size - 1; i >= 0; i--)
    {
        if (OSF_MainMemTree->root[i].addr == ptr)
        {
            if (new_ptr != ptr)
            {
                OSF_MainMemTree->root[i].addr = new_ptr;
                OSF_MainMemTree->root[i].size = size;
                OSF_MainMemTree->root[i].is_free = 0;
                OSF_MainMemTree->root[i].is_old = 0;
                OSF_MainMemTree->root[i].create_type = MEM_BLOCK_CREATE_TYPE_REALLOC;
                done = 1;
                break;
            }
            else
            {
                OSF_MainMemTree->root[i].is_free = 1;
                OSF_MainMemTree->root[i].is_old = 1;
                removeEntry = i;
                break;
            }
        }
    }

    if (!done)
    {
        if (OSF_MainMemTree->size)
            OSF_MainMemTree->root = (OSF_MemBlock *)realloc(OSF_MainMemTree->root, sizeof(OSF_MemBlock) * (OSF_MainMemTree->size + 1));

        OSF_MainMemTree->root[OSF_MainMemTree->size].addr = new_ptr;
        OSF_MainMemTree->root[OSF_MainMemTree->size].size = size;
        OSF_MainMemTree->root[OSF_MainMemTree->size].is_free = 0;
        OSF_MainMemTree->root[OSF_MainMemTree->size].is_old = 0;
        OSF_MainMemTree->root[OSF_MainMemTree->size].create_type = MEM_BLOCK_CREATE_TYPE_REALLOC;

        OSF_MainMemTree->size++;
    }

    if (removeEntry != -1)
    {
        OSF_MemTree *copyOsfMemBlock = (OSF_MemTree *)malloc(sizeof(OSF_MemTree));
        copyOsfMemBlock->root = (OSF_MemBlock *)malloc((OSF_MainMemTree->size - 1) * sizeof(OSF_MemBlock));
        copyOsfMemBlock->size = 0;

        for (size_t i = 0; i < OSF_MainMemTree->size; i++)
        {
            if (i == removeEntry)
                continue;

            copyOsfMemBlock->root[copyOsfMemBlock->size++] = (OSF_MemBlock){
                .addr = OSF_MainMemTree->root[i].addr,
                .create_type = OSF_MainMemTree->root[i].create_type,
                .is_free = OSF_MainMemTree->root[i].is_free,
                .is_old = OSF_MainMemTree->root[i].is_old,
                .size = OSF_MainMemTree->root[i].size};
        }

        free(OSF_MainMemTree->root);
        free(OSF_MainMemTree);

        OSF_MainMemTree = (OSF_MemTree *)malloc(sizeof(OSF_MemTree));
        OSF_MainMemTree->root = (OSF_MemBlock *)malloc(copyOsfMemBlock->size * sizeof(OSF_MemBlock));
        OSF_MainMemTree->size = copyOsfMemBlock->size;

        for (size_t i = 0; i < copyOsfMemBlock->size; i++)
        {
            OSF_MainMemTree->root[i] = (OSF_MemBlock){
                .addr = copyOsfMemBlock->root[i].addr,
                .create_type = copyOsfMemBlock->root[i].create_type,
                .is_free = copyOsfMemBlock->root[i].is_free,
                .is_old = copyOsfMemBlock->root[i].is_old,
                .size = copyOsfMemBlock->root[i].size};
        }

        free(copyOsfMemBlock->root);
        copyOsfMemBlock->root = NULL;
        free(copyOsfMemBlock);
        copyOsfMemBlock = NULL;
    }

    OSF_DetermineHeapSize();

    return new_ptr;
}

void *__OSF_Calloc(size_t num, size_t size, const char *__fname, int __line)
{
    void *ptr = malloc(num * size);
    if (ptr == NULL)
    {
        OSF_Error("OSF_Calloc: Out of memory", __fname, __line);
    }

    if (OSF_MainMemTree->size)
        OSF_MainMemTree->root = (OSF_MemBlock *)realloc(OSF_MainMemTree->root, sizeof(OSF_MemBlock) * (OSF_MainMemTree->size + 1));

    OSF_MainMemTree->root[OSF_MainMemTree->size].addr = ptr;
    OSF_MainMemTree->root[OSF_MainMemTree->size].size = num * size;
    OSF_MainMemTree->root[OSF_MainMemTree->size].is_free = 0;
    OSF_MainMemTree->root[OSF_MainMemTree->size].is_old = 0;
    OSF_MainMemTree->root[OSF_MainMemTree->size].create_type = MEM_BLOCK_CREATE_TYPE_CALLOC;

    OSF_MainMemTree->size++;
    OSF_DetermineHeapSize();

    return ptr;
}

int __OSF_MemorySafeToFree(void *ptr, const char *__fname, int __line)
{
    for (size_t i = 0; i < OSF_MainMemTree->size; i++)
    {
        if (OSF_MainMemTree->root[i].addr == ptr && !(OSF_MainMemTree->root[i].is_old))
            return !(OSF_MainMemTree->root[i].is_free);
    }

    return 0; // Memory was not dynamically allocated, so it CANNOT be free'd
}

void __OSF_Free(void *ptr, const char *__fname, int __line)
{
    OSF_FreeCalled++;
    if (ptr == NULL)
    {
        // Base case
        return;
    }

    int already_free = 0;
    int sf_del = -1;

    for (int i = OSF_MainMemTree->size - 1; i >= 0; i--)
    {
        if (OSF_MainMemTree->root[i].addr == ptr && !(OSF_MainMemTree->root[i].is_old))
        {
            sf_del = i;
            if (OSF_MainMemTree->root[i].is_free)
                already_free = 1;

            OSF_MainMemTree->root[i].is_free = 1;
            break;
        }
    }

    if (sf_del != -1)
        if (!already_free)
            free(ptr);
        else
            OSF_Error("OSF_Free: Pointer already freed", __fname, __line);
    else
    {
        OSF_Error("OSF_Free: Unsafe to delete memory", __fname, __line);
        return;
    }

    OSF_MemTree *copyOsfMemBlock = (OSF_MemTree *)malloc(sizeof(OSF_MemTree));
    copyOsfMemBlock->root = (OSF_MemBlock *)malloc((OSF_MainMemTree->size - 1) * sizeof(OSF_MemBlock));
    copyOsfMemBlock->size = 0;

    for (size_t i = 0; i < OSF_MainMemTree->size; i++)
    {
        if (i == sf_del)
            continue;

        copyOsfMemBlock->root[copyOsfMemBlock->size++] = (OSF_MemBlock){
            .addr = OSF_MainMemTree->root[i].addr,
            .create_type = OSF_MainMemTree->root[i].create_type,
            .is_free = OSF_MainMemTree->root[i].is_free,
            .is_old = OSF_MainMemTree->root[i].is_old,
            .size = OSF_MainMemTree->root[i].size};
    }

    free(OSF_MainMemTree->root);
    free(OSF_MainMemTree);

    OSF_MainMemTree = (OSF_MemTree *)malloc(sizeof(OSF_MemTree));
    OSF_MainMemTree->root = (OSF_MemBlock *)malloc(copyOsfMemBlock->size * sizeof(OSF_MemBlock));
    OSF_MainMemTree->size = copyOsfMemBlock->size;

    for (size_t i = 0; i < copyOsfMemBlock->size; i++)
    {
        OSF_MainMemTree->root[i] = (OSF_MemBlock){
            .addr = copyOsfMemBlock->root[i].addr,
            .create_type = copyOsfMemBlock->root[i].create_type,
            .is_free = copyOsfMemBlock->root[i].is_free,
            .is_old = copyOsfMemBlock->root[i].is_old,
            .size = copyOsfMemBlock->root[i].size};
    }

    free(copyOsfMemBlock->root);
    copyOsfMemBlock->root = NULL;
    free(copyOsfMemBlock);
    copyOsfMemBlock = NULL;

    OSF_DetermineHeapSize();
#if __DOSF_PRINT_HEAP_TRACE__ == 1
    fprintf(stdout, "[free] ");
    OSF_PrintHeapSize();
    fprintf(stdout, " in function: %s at line %d\n", __fname, __line);
#endif
}

void *__OSF_Memcpy(void *dest, const void *src, size_t size, const char *__fname, int __line)
{
    return memcpy(dest, src, size);
}

void *__OSF_Memset(void *dest, int value, size_t size, const char *__fname, int __line)
{
    return memset(dest, value, size);
}

void *__OSF_Memmove(void *dest, const void *src, size_t size, const char *__fname, int __line)
{
    return memmove(dest, src, size);
}

int __OSF_Memcmp(const void *ptr1, const void *ptr2, size_t size, const char *__fname, int __line)
{
    return memcmp(ptr1, ptr2, size);
}

int __OSF_Error(const char *format, const char *__ffrom, int __lfrom, const char *__fname, int __line)
{
#ifdef SF_DEBUG
    fprintf(stderr, "OSF_Error: %s\nIn function %s at line %d\n", format, __ffrom, __lfrom);
#endif
    return 1;
}

int __OSF_MemDestroy(const char *__fname, int __line)
{
    // for (size_t i = 0; i < OSF_MainMemTree->size; i++)
    // {
    //     if (!(OSF_MainMemTree->root[i].is_free) && !(OSF_MainMemTree->root[i].is_old))
    //         free(OSF_MainMemTree->root[i].addr);
    // }

    OSF_MainMemTree->size = 0;
    free(OSF_MainMemTree->root);
    OSF_MainMemTree->root = NULL;
    free(OSF_MainMemTree);
    OSF_MainMemTree = NULL;

    return 0;
}

void __OSF_DetermineHeapSize(const char *__fname, int __line)
{
    OSF_LastHeapSize = 0;
    for (size_t i = 0; i < OSF_MainMemTree->size; i++)
    {
        OSF_LastHeapSize += OSF_MainMemTree->root[i].size;
    }
}

void __OSF_PrintHeapSize(const char *__fname, int __line)
{
    fprintf(stdout, "Heap size: %lu bytes (%lu KB, %.2f MB)", OSF_LastHeapSize, OSF_LastHeapSize / 1024, (double)OSF_LastHeapSize / (1024 * 1024));
}

OSF_MemTree **__OSF_GetMainMemTree(void)
{
    return &OSF_MainMemTree;
}

void __OSF_InternalFree(void *ptr)
{
    if (ptr == NULL)
        return;
    free(ptr);
}