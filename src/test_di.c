#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "dd_data.h"
#include "di_lib.h"

void di_entry_compare(void) {
	printf("testing di_entry_compare... ");
	const char *entry_1 = "AARDVARK  AA1 R D V AA2 R K\n";
	const char *entry_2 = "SALAMANDER  S AE2 L AH0 M AE1 N D ER0\n";
	const char *entry_3 = "!EXCLAMATION-POINT  EH2 K S K L AH0 M EY1 SH AH0 N P OY2 N T\n";

	DDString *test_1 = copy_string("AANCOR", 6);
	DDString *test_2 = copy_string("AARDVARKS", 9);
	DDString *test_3 = copy_string("AARDVARK", 8);
	DDString *test_4 = copy_string("ADORE", 5);
	DDString *test_5 = copy_string("AARD", 4);
	DDString *test_6 = copy_string("SALAMANDER", 10); 
	DDString *test_7 = copy_string("!EXCLAMATION-POINT", 18); 

	assert(entry_comp(test_1, entry_1) == 1);
	assert(entry_comp(test_2, entry_1) == -1);
	assert(entry_comp(test_3, entry_1) == 0);
	assert(entry_comp(test_4, entry_1) == -1);
	assert(entry_comp(test_5, entry_1) == 1);
	assert(entry_comp(test_6, entry_2) == 0);
	assert(entry_comp(test_1, entry_3) == -1);
	assert(entry_comp(test_7, entry_3) == 0);

	free_string(test_1);
	free_string(test_2);
	free_string(test_3);
	free_string(test_4);
	free_string(test_5);
	free_string(test_6);
	free_string(test_7);

	printf("success\n");
}

void di_entry_string(void) {
	printf("testing di_entry_string... ");
	DDString str;
	DIDictEntry entry;
	DISyllable syllable;

	init_dd_string(&str);
	give_to_dd_string(&str, "ACT", 3);
	init_di_dict_entry(&entry);
	entry.word = str;
	entry.variant = 0;

	init_dd_string(&str);
	init_di_syllable(&syllable);
	give_to_dd_string(&str, "AE", 2);
	syllable.pron = str;
	syllable.stress = 1;
	DD_ADD_ARRAY(&entry.pronunciation, syllable);

	init_dd_string(&str);
	init_di_syllable(&syllable);
	give_to_dd_string(&str, "K", 1);
	syllable.pron = str;
	syllable.stress = 0;
	DD_ADD_ARRAY(&entry.pronunciation, syllable);

	init_dd_string(&str);
	init_di_syllable(&syllable);
	give_to_dd_string(&str, "T", 1);
	syllable.pron = str;
	syllable.stress = 0;
	DD_ADD_ARRAY(&entry.pronunciation, syllable);

	init_dd_string(&str);
	entry_to_string(&entry, &str);

	assert(!strcmp(str.chars, "ACT  AE1 K T\n")); 

	free_dd_chars(&str);
	free_di_dict_entry(&entry);
	printf("success\n");
}

void di_string_entry(void) {
	printf("testing di_string_entry... ");
	DDString orig;
	DIDictEntry entry;

	init_dd_string(&orig);
	give_to_dd_string(&orig, "ACT  AE1 K T\n", 13);
	init_di_dict_entry(&entry);

	string_to_di_entry(&orig, &entry);

	assert(!strcmp(entry.word.chars, "ACT"));
	assert(entry.variant == 0);
	assert(!strcmp(entry.pronunciation.elems[0].pron.chars, "AE"));
	assert(entry.pronunciation.elems[0].stress == 1);
	assert(!strcmp(entry.pronunciation.elems[1].pron.chars, "K"));
	assert(entry.pronunciation.elems[1].stress == -1);
	assert(!strcmp(entry.pronunciation.elems[2].pron.chars, "T"));
	assert(entry.pronunciation.elems[2].stress == -1);

	free_dd_chars(&orig);
	free_di_dict_entry(&entry);

	printf("success\n");
}

void di_find_entry(void) {
	printf("testing di_find_entry...");
	DIDictEntry match;
	bool result;
	init_di_dict_entry(&match);
	DDString *test_1 = copy_string("ZYZYZY", 6);
	DDString *test_2 = copy_string("SALAMANDER", 10); 

	result = find_entry(&match, test_1, "./static/cmudict/raw.txt");
	assert(result == false);
	result = find_entry(&match, test_2, "./static/cmudict/raw.txt");
	assert(result == true);
	assert(!strcmp(match.word.chars, "SALAMANDER"));
	assert(!strcmp(match.pronunciation.elems[0].pron.chars, "S"));
	assert(match.pronunciation.elems[0].stress == -1);
	assert(!strcmp(match.pronunciation.elems[1].pron.chars, "AE"));
	assert(match.pronunciation.elems[1].stress == 2);
	assert(!strcmp(match.pronunciation.elems[2].pron.chars, "L"));
	assert(match.pronunciation.elems[2].stress == -1);
	assert(!strcmp(match.pronunciation.elems[3].pron.chars, "AH"));
	assert(match.pronunciation.elems[3].stress == 0);
	assert(!strcmp(match.pronunciation.elems[4].pron.chars, "M"));
	assert(match.pronunciation.elems[4].stress == -1);
	assert(!strcmp(match.pronunciation.elems[5].pron.chars, "AE"));
	assert(match.pronunciation.elems[5].stress == 1);
	assert(!strcmp(match.pronunciation.elems[6].pron.chars, "N"));
	assert(match.pronunciation.elems[6].stress == -1);
	assert(!strcmp(match.pronunciation.elems[7].pron.chars, "D"));
	assert(match.pronunciation.elems[7].stress == -1);
	assert(!strcmp(match.pronunciation.elems[8].pron.chars, "ER"));
	assert(match.pronunciation.elems[8].stress == 0);

	free_string(test_1);
	free_string(test_2);
	free_di_dict_entry(&match);
	printf("success\n");
}

void di_entries_string(void) {
	printf("testing di_find_entries...");
	int i;
	DDString str;
	DDArrDIIndexedEntry entries;
	init_dd_string(&str);
	DD_INIT_ARRAY(&entries);

	give_to_dd_string(&str, "porpoise, jut", 13);
	di_entries_for_string(&str, &entries);
	assert(!strcmp(entries.elems[0].entry.word.chars, "PORPOISE"));
	assert(entries.elems[0].index == 0);
	assert(entries.elems[0].length == 8);

	assert(!strcmp(entries.elems[1].entry.word.chars, "JUT"));
	assert(entries.elems[1].index == 10);
	assert(entries.elems[1].length == 3);

	for (i = 0; i < entries.size; i++) {
		free_di_dict_entry(&entries.elems[i].entry);
	}
	DD_FREE_ARRAY(&entries);
	
	free_dd_chars(&str);
	printf("success\n");
}

int main(void) {
	di_entry_compare();
	di_entry_string();
	di_string_entry();
	di_find_entry();
	di_entries_string();
}
