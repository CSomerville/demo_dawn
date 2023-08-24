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
