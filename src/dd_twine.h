#ifndef dd_twine_h
#define dd_twine_h

#include "dd_data.h"

typedef struct {
	unsigned int length;
	char *chars;
} DDTwine;

DD_DEF_ARRAY(DDTwine, DDTwine);

typedef struct {
	int start;
	int end;
} DDTwineWB;

DD_DEF_ARRAY(DDTwineWB, DDTwineWB);

void dd_twine_init(DDTwine *tw);
void dd_twine_from_chars_fixed(DDTwine *tw, const char *chars, int len);
void dd_twine_from_chars_dyn(DDTwine *tw, const char *chars);
void dd_twine_from_dd_str(DDTwine *tw, DDString *str);

unsigned int dd_twine_len(DDTwine *tw);
char *dd_twine_chars(DDTwine *tw);
char dd_twine_char_at(DDTwine *tw, unsigned int i);
void dd_twine_set_char_at(DDTwine *tw, char c, unsigned int i);
bool dd_twine_eq(DDTwine *twa, DDTwine *twb);
void dd_twine_to_upper_mut(DDTwine *tw);
void dd_twine_concat(DDTwine *target, DDTwine *twa, DDTwine *twb);
void dd_twine_concat_mut(DDTwine *twa, DDTwine *twb);
void dd_twine_copy(DDTwine *twa, DDTwine *twb);
void dd_twine_split(DDArrDDTwine *tw_arr, DDTwine *twb, DDTwine *twc);
void dd_twine_word_bounds(DDArrDDTwineWB *wb_arr, DDTwine *twa);
void dd_twine_join(DDTwine *tw, DDArrDDTwine *tw_arr, DDTwine *between);
int dd_arr_dd_twine_index_of(DDArrDDTwine *tws, DDTwine *tw);

void dd_twine_destroy(DDTwine *tw);
void dd_arr_dd_twine_destroy(DDArrDDTwine *tws);

#endif
