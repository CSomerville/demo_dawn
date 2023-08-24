#ifndef di_lib
#define di_lib

#include "dd_data.h"
#include "dd_twine.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct {
	DDTwine pron;
	int stress;
} DISyllable;

DD_DEF_ARRAY(DISyllable, DISyllable);

typedef struct {
	DDTwine word;
	int variant;
	DDArrDISyllable pronunciation;
} DIDictEntry;

typedef struct {
	int index;
	int length;
	DIDictEntry entry;
} DIIndexedEntry;

DD_DEF_ARRAY(DIIndexedEntry, DIIndexedEntry);

void init_di_syllable(DISyllable *di_syllable);
void init_di_dict_entry(DIDictEntry *di_dict_entry);
int entry_comp(DDTwine *word, const char *line);
bool find_entry(DIDictEntry *match, DDTwine *word, const char *path);
void entry_to_string(DIDictEntry *entry, DDTwine *str);
int string_to_di_entry(DDTwine *str, DIDictEntry *entry);
void di_entries_for_string(DDTwine *str, DDArrDIIndexedEntry *entries,
		const char *path);
void di_entries_from_wbs(DDArrDIIndexedEntry *entries, int start,
		DDArrDDTwineWB *word_bounds, DDTwine *str, FILE *dict);
void di_add_entry(DIDictEntry *new_entry, const char *path);
void free_di_dict_entry(DIDictEntry *di_dict_entry);

#endif
