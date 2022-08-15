#include <stdlib.h>
#include <string.h>

#include "dd_data.h"

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

DDString* dd_string_concat(DDString *a, DDString *b) {
	int i;
	DDString *result = DD_ALLOCATE(DDString, sizeof(DDString));
	char *allocated = DD_ALLOCATE(char, a->length + b->length + 1);
	for (i = 0; i < a->length; i++) {
		allocated[i] = a->chars[i];
	}
	for (i = 0; i < b->length; i++) {
		allocated[i+a->length] = b->chars[i];
	}
	allocated[a->length + b->length] = '\0';
	result->length = a->length + b->length;
	result->chars = allocated;
	return result;
}

void free_string(DDString* str) {
    reallocate(str->chars, sizeof(char) * (str->length + 1), 0);
    DD_FREE(DDString, str);
}
