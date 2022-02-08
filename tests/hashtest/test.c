#include "sunflower.h"

int main(int argc, char const *argv[])
{
    SF_InitEnv();

    HashTable *ht = Hash_new_ptr("", NULL, 0, 0);

    Hash_add(ht, "test", (int *)(int []){10});
    Hash_add(ht, "testing", (int *)(int []){20});
    Hash_add(ht, "tests", (int *)(int[]){30});
    Hash_add(ht, "hello", (int *)(int[]){40});
    Hash_add(ht, "hell", (int *)(int[]){50});
    Hash_add(ht, "test", (int *)(int[]){60});

    printf("%d\n", *(int *)Hash_get(ht, "testing"));

    SF_DestroyEnv();
    return 0;
}