#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

void init_dd_string(DDString *dd_string) {
	dd_string->length = 0;
	dd_string->chars = NULL;
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

void dd_string_concat_mutate(DDString *a, DDString *b) {
	DDString *tmp_str;
	tmp_str = dd_string_concat(a, b);
	free_dd_chars(a);
	a->length = tmp_str->length;
	a->chars = tmp_str->chars;
	free(tmp_str);
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

static bool is_non_word(char c) {
	return c == ' ' || c == ',' || c == '\n';
}

int get_next_dd_string_word_bounds(DDString *dd_str, int start,
		int *word_start, int *word_end) {
	int i = start;
	if (i >= dd_str->length)
		return 1;
	while (is_non_word(dd_str->chars[i]))
		i++;
	if (i >= dd_str->length)
		return 1;
	*word_start = i;
	while (!is_non_word(dd_str->chars[i]) && i < dd_str->length)
		i++;
	*word_end = i;
	return 0;
}

bool dd_string_equal(DDString *a, DDString *b) {
	if (a->length != b->length)
		return false;
	if (a->chars == NULL || b->chars == NULL)
		return false;
	return (!strcmp(a->chars, b->chars));
}

void dd_copy_dd_string(DDString *target, DDString *dd_str) {
	give_to_dd_string(target, dd_str->chars, dd_str->length);
	target->length = dd_str->length;
}

void dd_repeat_dd_string(DDString *target, DDString *to_repeat, int n) {
	int i, j, idx;
	target->length = to_repeat->length * n;
	target->chars = DD_ALLOCATE(char, to_repeat->length * n + 1);
	for (i = 0; i < n; i++) {
		for (j = 0; j < to_repeat->length; j++) {
			idx = (i * to_repeat->length) + j;
			target->chars[idx] = to_repeat->chars[j];
		}
	}
	target->chars[to_repeat->length * n] = '\0';
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
