#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "di_lib.h"
#include "dd_data.h"

void init_di_syllable(DISyllable *di_syllable) {
	dd_twine_init(&di_syllable->pron);
	di_syllable->stress = -1;
}

void init_di_dict_entry(DIDictEntry *di_dict_entry) {
	dd_twine_init(&di_dict_entry->word);
	di_dict_entry->variant = -1;
	DD_INIT_ARRAY(&di_dict_entry->pronunciation);
}

static bool entry_end_of_word(const char *entry) {
	return entry[0] == ' ' ||
	   (entry[0] == '(' && isdigit(entry[1]));
}

int entry_comp(DDTwine *word, const char *line) {
	unsigned int i;
	for (i = 0; i < dd_twine_len(word); i++) {
		if (line[i] == '\0' || dd_twine_char_at(word, i) < line[i]) {
			return 1;
		} else if (entry_end_of_word(&line[i]) ||
				dd_twine_char_at(word, i) > line[i]) {
			return -1;
		}
	}
	if (entry_end_of_word(&line[dd_twine_len(word)])) {
		return 0;
	} else {
		return 1;
	}
}

static void add_to_num_entry(DIDictEntry *entry, const char *str, int len) {
	DIDictEntry tmp_entry;
	DDTwine tmp_tw;
	DISyllable tmp_syl;
	int i;

	dd_twine_init(&tmp_tw);
	init_di_dict_entry(&tmp_entry);

	dd_twine_from_chars_fixed(&tmp_tw, str, len);
	string_to_di_entry(&tmp_tw, &tmp_entry);
	dd_twine_concat_with_char_mut(&entry->word, &tmp_entry.word, ' ');
	for (i = 0; i < tmp_entry.pronunciation.size; i++) {
		init_di_syllable(&tmp_syl);
		dd_twine_copy(&tmp_syl.pron, &tmp_entry.pronunciation.elems[i].pron);
		tmp_syl.stress = tmp_entry.pronunciation.elems[i].stress;
		DD_ADD_ARRAY(&entry->pronunciation, tmp_syl);

	}
	free_di_dict_entry(&tmp_entry);
	dd_twine_destroy(&tmp_tw);
}

/* pron for number */
static void pron_one_digit(DIDictEntry *entry, char c) {
	switch (c) {
		case '0':
			add_to_num_entry(entry, "ZERO  Z IY1 R OW0", 17);
			break;
		case '1':
			add_to_num_entry(entry, "ONE  W AH1 N", 12);
			break;
		case '2':
			add_to_num_entry(entry, "TWO  T UW1", 10);
			break;
		case '3':
			add_to_num_entry(entry, "THREE  TH R IY1", 15);
			break;
		case '4':
			add_to_num_entry(entry, "FOUR  F AO1 R", 13);
			break;
		case '5':
			add_to_num_entry(entry, "FIVE  F AY1 V", 13);
			break;
		case '6':
			add_to_num_entry(entry, "SIX  S IH1 K S", 14);
			break;
		case '7':
			add_to_num_entry(entry, "SEVEN  S EH1 V AH0 N", 20);
			break;
		case '8':
			add_to_num_entry(entry, "EIGHT  EY1 T", 12);
			break;
		case '9':
			add_to_num_entry(entry, "NINE  N AY1 N", 13);
			break;
	}
}

static void pron_tens(DIDictEntry *entry, char c) {
	switch (c) {
		case '0':
			add_to_num_entry(entry, "TEN  T EH1 N", 12);
			break;
		case '1':
			add_to_num_entry(entry, "ELEVEN  IH0 L EH1 V AH0 N", 25);
			break;
		case '2':
			add_to_num_entry(entry, "TWELVE  T W EH1 L V", 19);
			break;
		case '3':
			add_to_num_entry(entry, "THIRTEEN  TH ER1 T IY1 N", 24);
			break;
		case '4':
			add_to_num_entry(entry, "FOURTEEN  F AO1 R T IY1 N", 25);
			break;
		case '5':
			add_to_num_entry(entry, "FIFTEEN  F IH0 F T IY1 N", 24);
			break;
		case '6':
			add_to_num_entry(entry, "SIXTEEN  S IH0 K S T IY1 N", 26);
			break;
		case '7':
			add_to_num_entry(entry, "SEVENTEEN  S EH1 V AH0 N T IY1 N", 32);
			break;
		case '8':
			add_to_num_entry(entry, "EIGHTEEN  EY0 T IY1 N", 21);
			break;
		case '9':
			add_to_num_entry(entry, "NINETEEN  N AY1 N T IY1 N", 25);
			break;
	}
}

static void pron_two_digits(DIDictEntry *entry, char c1, char c2) {
	if (c1 == '0' && c2 == '0') {
	} else if (c1 == '0') {
		pron_one_digit(entry, c2);
	} else if (c1 == '1') {
		pron_tens(entry, c2);
	} else {
		switch (c1) {
			case '2':
				add_to_num_entry(entry, "TWENTY  T W EH1 N T IY0", 23);
				break;
			case '3':
				add_to_num_entry(entry, "THIRTY  TH ER1 D IY2", 20);
				break;
			case '4':
				add_to_num_entry(entry, "FORTY  F AO1 R T IY0", 20);
				break;
			case '5':
				add_to_num_entry(entry, "FIFTY  F IH1 F T IY0", 20);
				break;
			case '6':
				add_to_num_entry(entry, "SIXTY  S IH1 K S T IY0", 22);
				break;
			case '7':
				add_to_num_entry(entry, "SEVENTY  S EH1 V AH0 N T IY0", 28);
				break;
			case '8':
				add_to_num_entry(entry, "EIGHTY  EY1 T IY0", 17);
				break;
			case '9':
				add_to_num_entry(entry, "NINETY  N AY1 N T IY0", 21);
				break;
		}
		pron_one_digit(entry, c2);
	}
}

static void pron_add_hundred(DIDictEntry *entry) {
	add_to_num_entry(entry, "HUNDRED  HH AH1 N D R AH0 D", 27);
}

static void pron_add_thousand(DIDictEntry *entry) {
	add_to_num_entry(entry, "THOUSAND  TH AW1 Z AH0 N D", 26);
}

static void pron_add_million(DIDictEntry *entry) {
	add_to_num_entry(entry, "MILLION  M IH1 L Y AH0 N", 24);
}

static void pron_add_point(DIDictEntry *entry) {
	add_to_num_entry(entry, "POINT  P OY1 N T", 16);
}

static void pron_three_digits(DIDictEntry *entry, DDTwine *digits) {
	if (dd_twine_len(digits) == 3 && dd_twine_char_at(digits, 0) != '0') {
		pron_one_digit(entry, dd_twine_char_at(digits, 0));
		pron_add_hundred(entry);
	}
	if (dd_twine_len(digits) >= 2) {
		pron_two_digits(entry, dd_twine_char_at(digits, dd_twine_len(digits) - 2),
			dd_twine_char_at(digits, dd_twine_len(digits) - 1));
	} else if (dd_twine_len(digits) == 1) {
		pron_one_digit(entry, dd_twine_char_at(digits, 0));

	}
}

static bool is_number(DDTwine *str) {
	bool decimal_seen = false;
	int i = 0;
	if (!isdigit(dd_twine_char_at(str, i)))
		return false;
	i++;

	for (; i < str->length; i++) {
		if (dd_twine_char_at(str, i) == '.' && !decimal_seen) {
			decimal_seen = true;
		} else if (!isdigit(dd_twine_char_at(str, i))) {
			return false;
		}	
	}
	return true;
}

static void dict_for_num(DIDictEntry *entry, DDTwine *str) {
	DDTwine tmp;
	int i, j, right_offset;

	i = 0;
	while (i < str->length && dd_twine_char_at(str, i) != '.')
		i++;

	if (i > 9) {
		fprintf(stderr, "Not sure what to do with numbers bigger than 9");
		exit(1);
	}
	if (i > 6) {
		dd_twine_init(&tmp);
		dd_twine_from_chars_fixed(&tmp, str->chars, i - 6);
		pron_three_digits(entry, &tmp);
		pron_add_million(entry);
		dd_twine_destroy(&tmp);
	}
	if (i > 3) {
		dd_twine_init(&tmp);
		j = i < 7 ? 0 : i - 6;
		dd_twine_from_chars_fixed(&tmp, &str->chars[j], i - 3 > 3 ? 3 : i - 3);
		pron_three_digits(entry, &tmp);
		pron_add_thousand(entry);
		dd_twine_destroy(&tmp);
	}
	dd_twine_init(&tmp);
	j = i < 4 ? 0 : i - 3;
	dd_twine_from_chars_fixed(&tmp, &str->chars[j], i > 3 ? 3 : i);
	pron_three_digits(entry, &tmp);
	dd_twine_destroy(&tmp);

	/* do left side */

	if (i < str->length && dd_twine_char_at(str, i) == '.') {
		pron_add_point(entry);
		right_offset = i++;
		while (i < str->length) {
			pron_one_digit(entry, dd_twine_char_at(str, i));
			i++;
		}
	}
}


/* takes the first pronunciation variant and throws away the others,
 * for now */
bool find_entry(DIDictEntry *match, DDTwine *word, const char *path) {
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	bool found = false;
	DDTwine tmp_str;
	int result;
	int num_read = 0;

	fp = fopen(path, "r");
	if (fp == NULL) {
		exit(1);
	}

	while ((read = getline(&line, &len, fp)) != -1) {
		num_read++;
		if (line[0] == ';' && line[1] == ';' && line[2] == ';') {
			continue;
		}
		result = entry_comp(word, line);
		if (result == 0) {
			found = true;
			dd_twine_init(&tmp_str);
			dd_twine_from_chars_fixed(&tmp_str, line, strlen(line) + 1);
			string_to_di_entry(&tmp_str, match);
			dd_twine_destroy(&tmp_str);
			break;
		} else if (result == 1) {
			break;
		}
	}

	if (line)
		free(line);
	fclose(fp);

	return found;
}

bool find_next_entry(DIDictEntry *match, DDTwine *word, FILE *dict) {
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	bool found = false;
	DDTwine tmp_str;
	int result;

	while ((read = getline(&line, &len, dict)) != -1) {
		if (line[0] == ';' && line[1] == ';' && line[2] == ';') {
			continue;
		}
		result = entry_comp(word, line);
		if (result == 0) {
			found = true;
			dd_twine_init(&tmp_str);
			dd_twine_from_chars_fixed(&tmp_str, line, strlen(line) + 1);
			string_to_di_entry(&tmp_str, match);
			dd_twine_destroy(&tmp_str);
			break;
		} else if (result == 1) {
			break;
		}
	}

	if (line)
		free(line);

	return found;
}

void di_add_entry(DIDictEntry *new_entry, const char *path) {
	FILE *fp, *fp_tmp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	bool found = false;
	DDTwine str;
	int result;

	dd_twine_init(&str);
	entry_to_string(new_entry, &str);

	fp = fopen(path, "r");
	if (fp == NULL)
		exit(1);
	fp_tmp = fopen("./static/cmudict/tmp.txt", "a");
	if (fp_tmp == NULL)
		exit(1);

	while ((read = getline(&line, &len, fp)) != -1) {
		if (found || 
				(line[0] == ';' && line[1] == ';' && line[2] == ';')) {
			fprintf(fp_tmp, "%s", line);
			continue;
		}
		result = entry_comp(&new_entry->word, line);
		if (result == -1) {
			fprintf(fp_tmp, "%s", line);
		} else if (result == 0) {
			/* fail... somehow */ 
		} else if (result == 1) {
			found = true;
			fprintf(fp_tmp, "%s", str.chars);
			fprintf(fp_tmp, "%s", line);
		}
	}
	dd_twine_destroy(&str);
	fclose(fp);
	fclose(fp_tmp);
	unlink(path);
	rename("./static/cmudict/tmp.txt", path);
	if (line)
		free(line);
}

void entry_to_string(DIDictEntry *entry, DDTwine *str) {
	DDTwine tmp_str;
	int i;
	char j[12];

	dd_twine_init(&tmp_str);
	
	dd_twine_copy(str, &entry->word);
	dd_twine_from_chars_fixed(&tmp_str, "  ", 2);
	dd_twine_concat_mut(str, &tmp_str);
	dd_twine_destroy(&tmp_str);
	dd_twine_init(&tmp_str);

	for (i = 0; i < entry->pronunciation.size; i++) {
		dd_twine_concat_mut(str, &entry->pronunciation.elems[i].pron);

		if (entry->pronunciation.elems[i].stress >= 0) {
			snprintf(j, 12, "%d", entry->pronunciation.elems[i].stress);
			dd_twine_from_chars_fixed(&tmp_str, j, 1);
			dd_twine_concat_mut(str, &tmp_str);
			dd_twine_destroy(&tmp_str);
			dd_twine_init(&tmp_str);
		}
		if (entry->pronunciation.size - 1 > i) {
			dd_twine_from_chars_fixed(&tmp_str, " ", 1);
			dd_twine_concat_mut(str, &tmp_str);
			dd_twine_destroy(&tmp_str);
			dd_twine_init(&tmp_str);
		}
	}

	dd_twine_from_chars_fixed(&tmp_str, "\n", 1);
	dd_twine_concat_mut(str, &tmp_str);
	dd_twine_destroy(&tmp_str);
	dd_twine_init(&tmp_str);
}
/* good god */
int string_to_di_entry(DDTwine *str, DIDictEntry *entry) {
	unsigned int i, j;
	DDTwine tmp_str;
	DISyllable tmp_syl;

	i = 0;
	while (!entry_end_of_word(&str->chars[i])) {
		if (!isupper(dd_twine_char_at(str, i)) && dd_twine_char_at(str, i) != '\'') {
			return 1;
		}
		i++;
	}
	dd_twine_init(&tmp_str);
	dd_twine_from_chars_fixed(&tmp_str, dd_twine_chars(str), i);
	entry->word = tmp_str;

	/* this should be fixed and done for real */
	entry->variant = 0;

	while (dd_twine_char_at(str, i) != '\n' &&
			i < dd_twine_len(str) - 1) {
		while (!isalpha(dd_twine_char_at(str, i)) &&
			dd_twine_char_at(str, i) != '\n')
			i++;
		j = i;
		while (isalpha(dd_twine_char_at(str, j)))
			j++;
		init_di_syllable(&tmp_syl);
		dd_twine_init(&tmp_str);
		dd_twine_from_chars_fixed(&tmp_str, &str->chars[i], j - i);
		tmp_syl.pron = tmp_str;
		if (isdigit(dd_twine_char_at(str, j))) {
			tmp_syl.stress = dd_twine_char_at(str, j) - '0';
			j++;
		} else {
			tmp_syl.stress = -1;
		}
		DD_ADD_ARRAY(&entry->pronunciation, tmp_syl);
		i = j;
	}
	return 0;
}

/* If a word isn't found it crashes. Philosophically don't love this
 * behavior obviously but at the moment I don't care.
 */
void di_entries_for_string(DDTwine *str, DDArrDIIndexedEntry *entries,
		const char *path) {
	int i;
	DDTwine tmp_str;
	DIDictEntry tmp_entry;
	DIIndexedEntry tmp_indexed_entry;
	DDArrDDTwineWB word_bounds;

	DD_INIT_ARRAY(&word_bounds);
	dd_twine_word_bounds(&word_bounds, str);

	for (i = 0; i < word_bounds.size; i++) {
		init_di_dict_entry(&tmp_entry);
		dd_twine_init(&tmp_str);
		dd_twine_from_chars_fixed(&tmp_str,
				&str->chars[word_bounds.elems[i].start],
				word_bounds.elems[i].end - word_bounds.elems[i].start);
		dd_twine_to_upper_mut(&tmp_str);
		if (!find_entry(&tmp_entry, &tmp_str, path)) {
			printf("Could not find dict word: %s\n", tmp_str.chars);
			exit(1);
		}

		tmp_indexed_entry.index = word_bounds.elems[i].start;
		tmp_indexed_entry.length = dd_twine_len(&tmp_str);
		tmp_indexed_entry.entry = tmp_entry;
		DD_ADD_ARRAY(entries, tmp_indexed_entry);

		dd_twine_destroy(&tmp_str);
	}
	DD_FREE_ARRAY(&word_bounds);
}

/* 1) build array of twines from word bounds
 * 2) sort
 * 3) scan file word by word
 * 4) if next word is a duplicate, reuse pron
 * 5) resort by index
 * 6) add prons in order
 * */

typedef struct {
	DDTwineWB wb;
	DDTwine tw;
} TWWBPair;
DD_DEF_ARRAY(TWWBPair, TWWBPair);

static int comp_1(const void *a, const void *b) {
	TWWBPair a_pair = *((TWWBPair *)a);
	TWWBPair b_pair = *((TWWBPair *)b);
	int blarg = strcmp(a_pair.tw.chars, b_pair.tw.chars);
	return blarg;
}

static int comp_2(const void *a, const void *b) {
	DDTwineWB wb_a = *((DDTwineWB *)a);
	DDTwineWB wb_b = *((DDTwineWB *)b);
	if (wb_a.start > wb_b.start) return 1;
	if (wb_a.start < wb_b.start) return -1;
	return 0;
}

static void deep_copy_dict_entry(DIDictEntry *target, DIDictEntry *entry) {
	DISyllable tmp_syl;
	int i;

	init_di_dict_entry(target);

	dd_twine_copy(&target->word, &entry->word);
	target->variant = entry->variant;

	for (i = 0; i < entry->pronunciation.size; i++) {
		init_di_syllable(&tmp_syl);
		dd_twine_copy(&tmp_syl.pron, &entry->pronunciation.elems[i].pron);
		tmp_syl.stress = entry->pronunciation.elems[i].stress;
		DD_ADD_ARRAY(&target->pronunciation, tmp_syl);
	}
}

void di_entries_from_wbs(DDArrDIIndexedEntry *entries, int start,
		DDArrDDTwineWB *word_bounds, DDTwine *str, FILE *dict) {

	int i, j;
	DDArrTWWBPair pairs;
	TWWBPair tmp_pair;
	DIDictEntry tmp_entry;
	DIIndexedEntry tmp_indexed_entry;
	DIIndexedEntry tmp_indexed_entry_2;
	bool word_found;

	DD_INIT_ARRAY(&pairs);

	for (i = start; i < word_bounds->size; i++) {
		dd_twine_init(&tmp_pair.tw);
		tmp_pair.wb = word_bounds->elems[i]; 
		dd_twine_from_chars_fixed(&tmp_pair.tw, &str->chars[tmp_pair.wb.start],
				tmp_pair.wb.end - tmp_pair.wb.start);
		dd_twine_to_upper_mut(&tmp_pair.tw);
		DD_ADD_ARRAY(&pairs, tmp_pair);
	}

	qsort(pairs.elems, pairs.size, sizeof(TWWBPair), comp_1);

	i = 0;
	while (i < pairs.size) {
		init_di_dict_entry(&tmp_entry);
		if (is_number(&pairs.elems[i].tw)) {
			dict_for_num(&tmp_entry, &pairs.elems[i].tw);
			tmp_indexed_entry.index = pairs.elems[i].wb.start;
			tmp_indexed_entry.length = dd_twine_len(&pairs.elems[i].tw);
			tmp_indexed_entry.entry = tmp_entry;
			DD_ADD_ARRAY(entries, tmp_indexed_entry);
			i++;
			continue;
		}
		word_found = find_next_entry(&tmp_entry, &pairs.elems[i].tw,
				dict);
		if (!word_found) {
			printf("Could not find dict word: %s\n", pairs.elems[i].tw.chars);
			exit(1);
		}

		tmp_indexed_entry.index = pairs.elems[i].wb.start;
		tmp_indexed_entry.length = dd_twine_len(&pairs.elems[i].tw);
		tmp_indexed_entry.entry = tmp_entry;
		DD_ADD_ARRAY(entries, tmp_indexed_entry);

		/* test for duplicates */
		j = i + 1;
		while (j < pairs.size && dd_twine_eq(&pairs.elems[i].tw, &pairs.elems[j].tw)) {
			tmp_indexed_entry_2.index = pairs.elems[j].wb.start;
			tmp_indexed_entry_2.length = dd_twine_len(&pairs.elems[i].tw);
			deep_copy_dict_entry(&tmp_indexed_entry_2.entry, &tmp_entry);
			DD_ADD_ARRAY(entries, tmp_indexed_entry_2);
			j++;
		}
		i = j;
	}

	qsort(entries->elems, entries->size, sizeof(DIIndexedEntry), comp_2);
	rewind(dict);
}

void free_di_dict_entry(DIDictEntry *di_dict_entry) {
	int i;
	dd_twine_destroy(&di_dict_entry->word);
	for (i = 0; i < di_dict_entry->pronunciation.size; i++) {
		dd_twine_destroy(&di_dict_entry->pronunciation.elems[i].pron);
	}
	DD_FREE_ARRAY(&di_dict_entry->pronunciation);
}
