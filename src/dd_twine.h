#ifndef dd_twine_h
#define dd_twine_h

#include "dd_data.h"

typedef struct {
	unsigned int length;
	char *chars;
} DDTwine;

DD_DEF_ARRAY(DDTwine, DDTwine);

void dd_twine_init(DDTwine *tw);
void dd_twine_from_chars_fixed(DDTwine *tw, const char *chars, int len);
void dd_twine_from_chars_dyn(DDTwine *tw, const char *chars);

unsigned int dd_twine_len(DDTwine *tw);
char dd_twine_char_at(DDTwine *tw, unsigned int i);
bool dd_twine_eq(DDTwine *twa, DDTwine *twb);
void dd_twine_concat(DDTwine *target, DDTwine *twa, DDTwine *twb);
void dd_twine_concat_mut(DDTwine *twa, DDTwine *twb);
void dd_twine_copy(DDTwine *twa, DDTwine *twb);
void dd_twine_split(DDArrDDTwine *tw_arr, DDTwine *twb, DDTwine *twc);

void dd_twine_destroy(DDTwine *tw);

#endif
