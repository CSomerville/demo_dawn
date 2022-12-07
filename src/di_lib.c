#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "di_lib.h"
#include "dd_data.h"

void init_di_syllable(DISyllable *di_syllable) {
	init_dd_string(&di_syllable->pron);
	di_syllable->stress = -1;
}

void init_di_dict_entry(DIDictEntry *di_dict_entry) {
	init_dd_string(&di_dict_entry->word);
	di_dict_entry->variant = -1;
	DD_INIT_ARRAY(&di_dict_entry->pronunciation);
}

static bool entry_end_of_word(const char *entry) {
	return entry[0] == ' ' ||
	   (entry[0] == '(' && isdigit(entry[1]));
}

int entry_comp(DDString *word, const char *line) {
	int i;
	for (i = 0; i < word->length; i++) {
		if (line[i] == '\0' || word->chars[i] < line[i]) {
			return 1;
		} else if (entry_end_of_word(&line[i]) ||
				word->chars[i] > line[i]) {
			return -1;
		}
	}
	if (entry_end_of_word(&line[word->length])) {
		return 0;
	} else {
		return 1;
	}
}


/* takes the first pronunciation variant and throws away the others,
 * for now */
bool find_entry(DIDictEntry *match, DDString *word, const char *path) {
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	bool found = false;
	DDString tmp_str;
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
			init_dd_string(&tmp_str);
			give_to_dd_string(&tmp_str, line, strlen(line) + 1);
			string_to_di_entry(&tmp_str, match);
			free_dd_chars(&tmp_str);
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

void entry_to_string(DIDictEntry *entry, DDString *str) {
	DDString tmp_str;
	int i;

	give_to_dd_string(str, entry->word.chars, entry->word.length);
	init_dd_string(&tmp_str);
	give_to_dd_string(&tmp_str, "  ", 2);
	dd_string_concat_mutate(str, &tmp_str);
	free_dd_chars(&tmp_str);

	for (i = 0; i < entry->pronunciation.size; i++) {
		dd_string_concat_mutate(str, &entry->pronunciation.elems[i].pron);
		if (entry->pronunciation.elems[i].stress > 0) {
			init_dd_string(&tmp_str);
			char j[12];
			snprintf(j, 12, "%d", entry->pronunciation.elems[i].stress);
			give_to_dd_string(&tmp_str,
					j, 1);
			dd_string_concat_mutate(str, &tmp_str);
			free_dd_chars(&tmp_str);
		}
		if (entry->pronunciation.size - i > 1) {
			init_dd_string(&tmp_str);
			give_to_dd_string(&tmp_str, " ", 1);
			dd_string_concat_mutate(str, &tmp_str);
			free_dd_chars(&tmp_str);
		}
	}

	init_dd_string(&tmp_str);
	give_to_dd_string(&tmp_str, "\n", 1);
	dd_string_concat_mutate(str, &tmp_str);
	free_dd_chars(&tmp_str);
}

int string_to_di_entry(DDString *str, DIDictEntry *entry) {
	int i, j;
	DDString tmp_str;
	DISyllable tmp_syl;

	i = 0;
	while (!entry_end_of_word(&str->chars[i])) {
		if (!isupper(str->chars[i])) 
			return 1;
		i++;
	}
	init_dd_string(&tmp_str);
	give_to_dd_string(&tmp_str, str->chars, i);
	entry->word = tmp_str;

	/* this should be fixed and done for real */
	entry->variant = 0;

	while (str->chars[i] != '\n' && i < str->length - 1) {
		while (!isalpha(str->chars[i]) && str->chars[i] != '\n')
			i++;
		j = i;
		while (isalpha(str->chars[j]))
			j++;
		init_di_syllable(&tmp_syl);
		init_dd_string(&tmp_str);
		give_to_dd_string(&tmp_str, &str->chars[i], j - i);
		tmp_syl.pron = tmp_str;
		if (isdigit(str->chars[j])) {
			tmp_syl.stress = str->chars[j] - '0';
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
void di_entries_for_string(DDString *str, DDArrDIIndexedEntry *entries) {
	DDString tmp_str;
	DIDictEntry tmp_entry;
	DIIndexedEntry tmp_indexed_entry;
	int start, result, word_start, word_end, i;
	start = 0;
	while ((result = get_next_dd_string_word_bounds(str, start,
				&word_start, &word_end)) == 0) {
		init_dd_string(&tmp_str);
		give_to_dd_string(&tmp_str, &str->chars[word_start], 
				word_end - word_start);
		for (i = 0; i < tmp_str.length; i++) {
			tmp_str.chars[i] = toupper(tmp_str.chars[i]);
		}

		init_di_dict_entry(&tmp_entry);
		if (!find_entry(&tmp_entry, &tmp_str, "./static/cmudict/raw.txt")) {
			printf("Could not find dict word: %s\n", tmp_str.chars);
			exit(1);
		}

		free_dd_chars(&tmp_str);

		tmp_indexed_entry.index = word_start;
		tmp_indexed_entry.length = tmp_str.length;
		tmp_indexed_entry.entry = tmp_entry;
		DD_ADD_ARRAY(entries, tmp_indexed_entry);

		start = word_end;
	}
}

void free_di_dict_entry(DIDictEntry *di_dict_entry) {
	int i;
	free_dd_chars(&di_dict_entry->word);
	for (i = 0; i < di_dict_entry->pronunciation.size; i++) {
		free_dd_chars(&di_dict_entry->pronunciation.elems[i].pron);
	}
	DD_FREE_ARRAY(&di_dict_entry->pronunciation);
}
