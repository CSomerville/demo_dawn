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

void give_to_dd_string(DDString *dd_str, const char* chars, int length) {
	if (dd_str->chars != NULL) {
		reallocate(dd_str->chars, sizeof(char) * (dd_str->length + 1), 0);
	}
    char* allocated = DD_ALLOCATE(char, length + 1);
    memcpy(allocated, chars, length);
    allocated[length] = '\0';
    dd_str->length = length;
    dd_str->chars = allocated;
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

void free_dd_chars(DDString *dd_str) {
    reallocate(dd_str->chars, sizeof(char) * (dd_str->length + 1), 0);
}

void free_dd_arr_dd_str(DDArrDDString *dd_arr_dd_str) {
	int i;
	for (i = 0; i < dd_arr_dd_str->size; i++) {
		free_dd_chars(&(dd_arr_dd_str->elems[i]));
	}
	DD_FREE_ARRAY(dd_arr_dd_str);
}

void free_string(DDString* str) {
	free_dd_chars(str);
    DD_FREE(DDString, str);
}
