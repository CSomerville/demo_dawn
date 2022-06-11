#include <stdlib.h>
#include <string.h>

#include "data.h"

void* reallocate(void* ptr, size_t old_size, size_t new_size) {
    if (new_size == 0) {
        free(ptr);
        return NULL;
    }
    void* result = realloc(ptr, new_size);
    if (result == NULL) exit(1);
    return result;
}

DDString* copy_string(const char* chars, int length) {
    DDString* dd_str = DD_ALLOCATE(DDString, sizeof(DDString));
    char* allocated = DD_ALLOCATE(char, length + 1);
    memcpy(allocated, chars, length);
    allocated[length] = '\0';
    dd_str->length = length;
    dd_str->chars = allocated;
    return dd_str;
}
