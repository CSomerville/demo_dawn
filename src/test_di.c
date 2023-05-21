#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "dd_data.h"
#include "dd_twine.h"
#include "di_lib.h"

void di_entry_compare(void) {
	printf("testing di_entry_compare... ");
	const char *entry_1 = "AARDVARK  AA1 R D V AA2 R K\n";
	const char *entry_2 = "SALAMANDER  S AE2 L AH0 M AE1 N D ER0\n";
	const char *entry_3 = "!EXCLAMATION-POINT  EH2 K S K L AH0 M EY1 SH AH0 N P OY2 N T\n";

	DDTwine test_1;
	DDTwine test_2;
	DDTwine test_3;
	DDTwine test_4;
	DDTwine test_5;
	DDTwine test_6; 
	DDTwine test_7; 

	dd_twine_init(&test_1);
	dd_twine_init(&test_2);
	dd_twine_init(&test_3);
	dd_twine_init(&test_4);
	dd_twine_init(&test_5);
	dd_twine_init(&test_6);
	dd_twine_init(&test_7);
	dd_twine_from_chars_fixed(&test_1, "AANCOR", 6);
	dd_twine_from_chars_fixed(&test_2, "AARDVARKS", 9);
	dd_twine_from_chars_fixed(&test_3, "AARDVARK", 8);
	dd_twine_from_chars_fixed(&test_4, "ADORE", 5);
	dd_twine_from_chars_fixed(&test_5, "AARD", 4);
	dd_twine_from_chars_fixed(&test_6, "SALAMANDER", 10); 
	dd_twine_from_chars_fixed(&test_7, "!EXCLAMATION-POINT", 18); 

	assert(entry_comp(&test_1, entry_1) == 1);
	assert(entry_comp(&test_2, entry_1) == -1);
	assert(entry_comp(&test_3, entry_1) == 0);
	assert(entry_comp(&test_4, entry_1) == -1);
	assert(entry_comp(&test_5, entry_1) == 1);
	assert(entry_comp(&test_6, entry_2) == 0);
	assert(entry_comp(&test_1, entry_3) == -1);
	assert(entry_comp(&test_7, entry_3) == 0);

	dd_twine_destroy(&test_1);
	dd_twine_destroy(&test_2);
	dd_twine_destroy(&test_3);
	dd_twine_destroy(&test_4);
	dd_twine_destroy(&test_5);
	dd_twine_destroy(&test_6);
	dd_twine_destroy(&test_7);

	printf("success\n");
}

void di_entry_string(void) {
	printf("testing di_entry_string... ");
	DDTwine str;
	DIDictEntry entry;
	DISyllable syllable;

	dd_twine_init(&str);
	dd_twine_from_chars_fixed(&str, "ACT", 3);
	init_di_dict_entry(&entry);
	entry.word = str;
	entry.variant = 0;

	dd_twine_init(&str);
	init_di_syllable(&syllable);
	dd_twine_from_chars_fixed(&str, "AE", 2);
	syllable.pron = str;
	syllable.stress = 1;
	DD_ADD_ARRAY(&entry.pronunciation, syllable);

	dd_twine_init(&str);
	init_di_syllable(&syllable);
	dd_twine_from_chars_fixed(&str, "K", 1);
	syllable.pron = str;
	syllable.stress = 0;
	DD_ADD_ARRAY(&entry.pronunciation, syllable);

	dd_twine_init(&str);
	init_di_syllable(&syllable);
	dd_twine_from_chars_fixed(&str, "T", 1);
	syllable.pron = str;
	syllable.stress = 0;
	DD_ADD_ARRAY(&entry.pronunciation, syllable);

	dd_twine_init(&str);
	entry_to_string(&entry, &str);

	assert(!strcmp(dd_twine_chars(&str), "ACT  AE1 K T\n")); 

	dd_twine_destroy(&str);
	free_di_dict_entry(&entry);
	printf("success\n");
}

void di_string_entry(void) {
	printf("testing di_string_entry... ");
	DDTwine orig;
	DIDictEntry entry;
	int result;

	dd_twine_init(&orig);
	dd_twine_from_chars_fixed(&orig, "ACT  AE1 K T\n", 13);
	init_di_dict_entry(&entry);

	result = string_to_di_entry(&orig, &entry);

	assert(result == 0);
	assert(!strcmp(entry.word.chars, "ACT"));
	assert(entry.variant == 0);
	assert(!strcmp(entry.pronunciation.elems[0].pron.chars, "AE"));
	assert(entry.pronunciation.elems[0].stress == 1);
	assert(!strcmp(entry.pronunciation.elems[1].pron.chars, "K"));
	assert(entry.pronunciation.elems[1].stress == -1);
	assert(!strcmp(entry.pronunciation.elems[2].pron.chars, "T"));
	assert(entry.pronunciation.elems[2].stress == -1);

	dd_twine_destroy(&orig);
	free_di_dict_entry(&entry);

	dd_twine_init(&orig);
	dd_twine_from_chars_fixed(&orig, "Act AE1 K T\n", 13);
	init_di_dict_entry(&entry);

	result = string_to_di_entry(&orig, &entry);
	assert(result == 1);

	dd_twine_destroy(&orig);

	printf("success\n");
}

void di_find_entry(void) {
	printf("testing di_find_entry...");
	DIDictEntry match;
	bool result;
	DDTwine test_1, test_2;
	init_di_dict_entry(&match);

	dd_twine_init(&test_1);
	dd_twine_init(&test_2);

	dd_twine_from_chars_fixed(&test_1, "ZYZYZY", 6);
	dd_twine_from_chars_fixed(&test_2, "SALAMANDER", 10); 

	result = find_entry(&match, &test_1, "./static/cmudict/raw.txt");
	assert(result == false);
	result = find_entry(&match, &test_2, "./static/cmudict/raw.txt");
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

	dd_twine_destroy(&test_1);
	dd_twine_destroy(&test_2);
	free_di_dict_entry(&match);
	printf("success\n");
}

void di_entries_string(void) {
	printf("testing di_find_entries...");
	int i;
	DDTwine str;
	DDArrDIIndexedEntry entries;
	dd_twine_init(&str);
	DD_INIT_ARRAY(&entries);

	dd_twine_from_chars_fixed(&str, "porpoise, jut", 13);
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
	
	dd_twine_destroy(&str);
	printf("success\n");
}

void test_di_add_entry(void) {
	printf("testing di_add_entry...");
	FILE *tmp_fp;
	DIDictEntry ajangle, match;
	DDTwine contents, new_word;

	init_di_dict_entry(&match);
	init_di_dict_entry(&ajangle);
	dd_twine_init(&contents);
	dd_twine_init(&new_word);

	tmp_fp = fopen("./static/tmp/test_di.txt", "a");
	if (tmp_fp == NULL)
		exit(1);

	dd_twine_from_chars_dyn(&new_word, "AJANGLE  AH0 JH AA1 NG G AH0 L");
	string_to_di_entry(&new_word, &ajangle);
	dd_twine_destroy(&new_word);

	dd_twine_init(&new_word);
	dd_twine_from_chars_fixed(&new_word, "AJANGLE", 7);

	dd_twine_from_chars_dyn(&contents,
			"AJA  AY1 AH0\n"
			"AJAJ  AH0 JH AA1 JH\n"
			"AJAJ'S  AH0 JH AA1 JH IH0 Z\n"
			"AJAMI  EY2 JH AA1 M IY0\n"
			"AJAR  AH0 JH AA1 R\n"
			"AJAX  EY1 JH AE2 K S\n"
			"AJAX'S  EY1 JH AE2 K S AH0 Z\n");
	fprintf(tmp_fp, "%s", contents.chars);
	fclose(tmp_fp);

	di_add_entry(&ajangle, "./static/tmp/test_di.txt");
	assert(find_entry(&match, &new_word, "./static/tmp/test_di.txt"));

	unlink("./static/tmp/test_di.txt");
	dd_twine_destroy(&contents);
	dd_twine_destroy(&new_word);
	free_di_dict_entry(&match);
	free_di_dict_entry(&ajangle);
	printf("success\n");
}

int main(void) {
	di_entry_compare();
	di_entry_string();
	di_string_entry();
	di_find_entry();
	di_entries_string();
	test_di_add_entry();
}
