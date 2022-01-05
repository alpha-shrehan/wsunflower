#pragma once

#include <stdio.h>  // for stderr
#include <stdlib.h> // for malloc, free, realloc, etc.
#include <string.h> // for memset, memcpy, etc.
#include <gc.h>

enum MemBlockCreateTypes
{
    MEM_BLOCK_CREATE_TYPE_MALLOC,
    MEM_BLOCK_CREATE_TYPE_CALLOC,
    MEM_BLOCK_CREATE_TYPE_REALLOC
};

typedef struct
{
    void *addr;
    size_t size;
    int is_free;
    int is_old;
    enum MemBlockCreateTypes create_type;

} OSF_MemBlock;

typedef struct
{
    OSF_MemBlock *root;
    int size;

} OSF_MemTree;

// #define OSF_USE_BWD_GC_ALLOCS
#define OSF_USE_STANDARD_ALLOCS

#ifdef OSF_USE_STANDARD_ALLOCS
#define OSF_MemInit() __OSF_MemInit(__func__, __LINE__)
#define OSF_Malloc(a) __OSF_Malloc(a, __func__, __LINE__)
#define OSF_Realloc(a, b) __OSF_Realloc(a, b, __func__, __LINE__)
#define OSF_Calloc(a, b) __OSF_Calloc(a, b, __func__, __LINE__)
#define OSF_Free(a) __OSF_Free(a, __func__, __LINE__)
#define OSF_Memcpy(a, b, c) __OSF_Memcpy(a, b, c, __func__, __LINE__)
#define OSF_Memset(a, b, c) __OSF_Memset(a, b, c, __func__, __LINE__)
#define OSF_Memmove(a, b, c) __OSF_Memmove(a, b, c, __func__, __LINE__)
#define OSF_Memcmp(a, b, c) __OSF_Memcmp(a, b, c, __func__, __LINE__)
#define OSF_Error(a, b, c) __OSF_Error(a, b, c, __func__, __LINE__)
#define OSF_MemorySafeToFree(a) __OSF_MemorySafeToFree(a, __func__, __LINE__)
#define OSF_MemDestroy() __OSF_MemDestroy(__func__, __LINE__)
#define OSF_DetermineHeapSize() __OSF_DetermineHeapSize(__func__, __LINE__)
#define OSF_PrintHeapSize() __OSF_PrintHeapSize(__func__, __LINE__)
#elif defined(OSF_USE_BWD_GC_ALLOCS)
#define OSF_MemInit() __OSF_MemInit(__func__, __LINE__)
#define OSF_Malloc(a) GC_MALLOC(a)
#define OSF_Realloc(a, b) GC_REALLOC(a, b)
#define OSF_Calloc(a, b) GC_MALLOC(a * b)
#define OSF_Free(a) GC_FREE(a)
#define OSF_Memcpy(a, b, c) __OSF_Memcpy(a, b, c, __func__, __LINE__)
#define OSF_Memset(a, b, c) __OSF_Memset(a, b, c, __func__, __LINE__)
#define OSF_Memmove(a, b, c) __OSF_Memmove(a, b, c, __func__, __LINE__)
#define OSF_Memcmp(a, b, c) __OSF_Memcmp(a, b, c, __func__, __LINE__)
#define OSF_Error(a, b, c) __OSF_Error(a, b, c, __func__, __LINE__)
#define OSF_MemorySafeToFree(a) __OSF_MemorySafeToFree(a, __func__, __LINE__)
#define OSF_MemDestroy() __OSF_MemDestroy(__func__, __LINE__)
#define OSF_DetermineHeapSize() __OSF_DetermineHeapSize(__func__, __LINE__)
#define OSF_PrintHeapSize() __OSF_PrintHeapSize(__func__, __LINE__)
#else
#define OSF_MemInit() __OSF_MemInit(__func__, __LINE__)
#define OSF_Malloc(a) malloc(a)
#define OSF_Realloc(a, b) realloc(a, b)
#define OSF_Calloc(a, b) calloc(a, b)
#define OSF_Free(a) free(a)
#define OSF_Memcpy(a, b, c) memcpy(a, b, c)
#define OSF_Memset(a, b, c) memset(a, b, c)
#define OSF_Memmove(a, b, c) memmove(a, b, c)
#define OSF_Memcmp(a, b, c) memcmp(a, b, c)
#define OSF_Error(a, b, c) __OSF_Error(a, b, c, __func__, __LINE__)
#define OSF_MemorySafeToFree(a) __OSF_MemorySafeToFree(a, __func__, __LINE__)
#define OSF_MemDestroy() __OSF_MemDestroy(__func__, __LINE__)
#define OSF_DetermineHeapSize() __OSF_DetermineHeapSize(__func__, __LINE__)
#define OSF_PrintHeapSize() __OSF_PrintHeapSize(__func__, __LINE__)
#endif

#define OSF_IntFree(x) __OSF_InternalFree(x)
#define OSF_strdup(x) GC_STRDUP(x)

void __OSF_MemInit(const char *, int);
void *__OSF_Malloc(size_t size, const char *, int) __attribute__((malloc));
void *__OSF_Realloc(void *ptr, size_t size, const char *, int);
void *__OSF_Calloc(size_t num, size_t size, const char *, int);
void __OSF_Free(void *ptr, const char *, int);
void *__OSF_Memcpy(void *dest, const void *src, size_t size, const char *, int);
void *__OSF_Memset(void *dest, int value, size_t size, const char *, int);
void *__OSF_Memmove(void *dest, const void *src, size_t size, const char *, int);
int __OSF_Memcmp(const void *ptr1, const void *ptr2, size_t size, const char *, int);
int __OSF_Error(const char *format, const char *, int, const char *, int);
int __OSF_MemorySafeToFree(void *ptr, const char *, int);
int __OSF_MemDestroy(const char *, int);
void __OSF_DetermineHeapSize(const char *, int);
void __OSF_PrintHeapSize(const char *, int);
OSF_MemTree **__OSF_GetMainMemTree(void);
void __OSF_InternalFree(void *);
char *__OSF_strdup(const char *);