#ifndef di_lib
#define di_lib

#include "dd_data.h"
#include <ctype.h>
#include <stdbool.h>

typedef struct {
	DDString pron;
	int stress;
} DISyllable;

DD_DEF_ARRAY(DISyllable, DISyllable);

typedef struct {
	DDString word;
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
int entry_comp(DDString *word, const char *line);
bool find_entry(DIDictEntry *match, DDString *word, const char *path);
void di_dict_entry_to_str(DIDictEntry *entry, DDString *str);
void entry_to_string(DIDictEntry *entry, DDString *str);
void string_to_di_entry(DDString *str, DIDictEntry *entry);
void di_entries_for_string(DDString *str, DDArrDIIndexedEntry *entries);
void free_di_dict_entry(DIDictEntry *di_dict_entry);

#endif
