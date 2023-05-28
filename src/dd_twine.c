#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "dd_twine.h"

void dd_twine_init(DDTwine *tw) {
	tw->length = 0;
	tw->chars = NULL;
}

void dd_twine_from_chars_fixed(DDTwine *tw, const char *chars, int len) {
	char *mem = DD_ALLOCATE(char, len + 1);
	memcpy(mem, chars, len);
	mem[len] = '\0';
	tw->length = len;
	tw->chars = mem;
}

void dd_twine_from_chars_dyn(DDTwine *tw, const char *chars) {
	int i = 0;
	while (chars[i] != '\0')
		i++;
	dd_twine_from_chars_fixed(tw, chars, i);
}

unsigned int dd_twine_len(DDTwine *tw) {
	return tw->length;
}

char *dd_twine_chars(DDTwine *tw) {
	return tw->chars;
}

char dd_twine_char_at(DDTwine *twa, unsigned int i) {
	return twa->chars[i];
}

void dd_twine_set_char_at(DDTwine *twa, char c, unsigned int i) {
	twa->chars[i] = c;
}

bool dd_twine_eq(DDTwine *twa, DDTwine *twb) {
	unsigned int i;
	if (dd_twine_len(twa) != dd_twine_len(twb))
		return false;
	for (i = 0; i < dd_twine_len(twa); i++)
		if (dd_twine_char_at(twa, i) != dd_twine_char_at(twb, i))
			return false;
	return true;
}

void dd_twine_to_upper_mut(DDTwine *tw) {
	unsigned int i;
	for (i = 0; i < dd_twine_len(tw); i++)
		dd_twine_set_char_at(tw, toupper(dd_twine_char_at(tw, i)), i);
}

void dd_twine_concat(DDTwine *target, DDTwine *twa, DDTwine *twb) {
	unsigned int i, j;
	unsigned int len = dd_twine_len(twa) + dd_twine_len(twb);
	target->chars = DD_ALLOCATE(char, len + 1);
	target->length = len;
	for (i = 0; i < dd_twine_len(twa); i++) {
		dd_twine_set_char_at(target, dd_twine_char_at(twa, i), i);
	}

	j = dd_twine_len(twa);
	for (i = 0; i < dd_twine_len(twb); i++) {
		dd_twine_set_char_at(target, dd_twine_char_at(twb, i), j);
		j++;
	}
	dd_twine_set_char_at(target, '\0', len);
}

void dd_twine_copy(DDTwine *twa, DDTwine *twb) {
	dd_twine_from_chars_fixed(twa, twb->chars, dd_twine_len(twb));
}

void dd_twine_concat_mut(DDTwine *twa, DDTwine *twb) {
	unsigned int i, j;
	unsigned int len = dd_twine_len(twa) + dd_twine_len(twb);
	twa->chars = (char *)reallocate(twa->chars,
			sizeof(char) * dd_twine_len(twa) + 1,
			sizeof(char) * (len + 1));
	i = dd_twine_len(twa);
	for (j = 0; i < len; j++) {
		dd_twine_set_char_at(twa, dd_twine_char_at(twb, j), i);
		i++;
	}
	dd_twine_set_char_at(twa, '\0', len);
	twa->length = len;
}

static bool is_non_word(char c) {
	return !(isalnum(c) || c == '\'');
}

void dd_twine_word_bounds(DDArrDDTwineWB *wb_arr, DDTwine *twa) {
	unsigned int i;
	DDTwineWB bounds;
	bounds.start = 0;

	i = 0;
	while (i < dd_twine_len(twa)) {
		while (is_non_word(dd_twine_char_at(twa, i)))
			i++;
		if (i > dd_twine_len(twa))
			break;
		bounds.start = i;
		while (!is_non_word(dd_twine_char_at(twa, i)) &&
				i < dd_twine_len(twa))
			i++;
		bounds.end = i;
		DD_ADD_ARRAY(wb_arr, bounds);
	}
}

void dd_twine_join(DDTwine *tw, DDArrDDTwine *tw_arr, DDTwine *between) {
	int i;
	for (i = 0; i < tw_arr->size; i++) {
		dd_twine_concat_mut(tw, &tw_arr->elems[i]);
		if (i+1 < tw_arr->size) {
			dd_twine_concat_mut(tw, between);
		}
	}
}

int dd_arr_dd_twine_index_of(DDArrDDTwine *tws, DDTwine *tw) {
	int i;
	for (i = 0; i < tws->size; i++) {
		if (dd_twine_eq(&tws->elems[i], tw))
		   return i;	
	}
	return -1;
}

void dd_twine_destroy(DDTwine *tw) {
    reallocate(tw->chars, sizeof(char) * (tw->length + 1), 0);
}

void dd_arr_dd_twine_destroy(DDArrDDTwine *tws) {
	int i;
	for (i = 0; i < tws->size; i++) {
		dd_twine_destroy(&tws->elems[i]);
	}
	DD_FREE_ARRAY(tws);
}
