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
	dd_twine_from_chars_fixed(tw, chars, i - 1);
}

unsigned int dd_twine_len(DDTwine *tw) {
	return tw->length;
}

char dd_twine_char_at(DDTwine *twa, unsigned int i) {
	return twa->chars[i];
}

void dd_twine_set_char_at(DDTwine *twa, char c, unsigned int i) {
	twa->chars[i] = c;
}

bool dd_twine_eq(DDTwine *twa, DDTwine *twb) {
	int i;
	if (twa->len != twb->len)
		return false;
	for (i = 0; i < twa->len; i++)
		if (dd_twine_char_at(twa, i) != dd_twine_char_at(twb, i))
			return false;
	return true;
}

void dd_twine_concat(DDTwine *target, DDTwine *twa, DDTwine *twb) {
	int i;
	unsigned int len = dd_twine_len(twa) + dd_twine_len(twb) + 1;
	dd_twine_from_chars_fixed(target, twa->chars, len);
	j = dd_twine_len(twa);
	for (i = 0; i < dd_twine_len(twb); i++) {
		dd_twine_set_char_at(target, dd_twine_char_at(twb, i), j);
		j++;
	}
}

void dd_twine_destroy(DDTwine *tw) {
    reallocate(tw->chars, sizeof(char) * (tw->length + 1), 0);
}
