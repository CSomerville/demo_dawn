#include <stdio.h>
#include <getopt.h>
#include "dd_data.h"
#include "di_lib.h"

typedef struct DIConfig {
	DDTwine path;
	DDTwine lookup;
	DDTwine add;
	DDTwine read_str;
} DIConfig;

static void di_config_init(DIConfig *conf) {
	dd_twine_init(&conf->path);
	dd_twine_init(&conf->lookup);
	dd_twine_init(&conf->add);
	dd_twine_init(&conf->read_str);
}

static void di_config_destroy(DIConfig *conf) {
	dd_twine_destroy(&conf->path);
	dd_twine_destroy(&conf->lookup);
	dd_twine_destroy(&conf->add);
	dd_twine_destroy(&conf->read_str);
}

static void print_lookup(DIConfig *conf) {
	DDTwine word, tmp;
	DIDictEntry match;
	bool matched;

	dd_twine_init(&word);
	dd_twine_init(&tmp);
	init_di_dict_entry(&match);
	dd_twine_from_chars_dyn(&word, dd_twine_chars(&conf->lookup));
	dd_twine_to_upper_mut(&word);
	matched = find_entry(&match, &word, dd_twine_chars(&conf->path));
	if (!matched) {
		printf("No match for word: \"%s\"\n",
				dd_twine_chars(&conf->lookup));
	} else {
		entry_to_string(&match, &tmp);
		printf("%s", tmp.chars);
		dd_twine_destroy(&tmp);
	}
	dd_twine_destroy(&word);
	free_di_dict_entry(&match);
}

static void add_pron(DIConfig *conf) {
	DIDictEntry new_entry;
	init_di_dict_entry(&new_entry);
	string_to_di_entry(&conf->add, &new_entry);
	di_add_entry(&new_entry, dd_twine_chars(&conf->path));
	free_di_dict_entry(&new_entry);
}

static void read_string(DIConfig *conf) {
	int i; 
	DDArrDIIndexedEntry entries;
	DDTwine tmp;

	DD_INIT_ARRAY(&entries);
	di_entries_for_string(&conf->read_str, &entries,
			dd_twine_chars(&conf->path));
	for (i = 0; i < entries.size; i++) {
		dd_twine_init(&tmp);
		entry_to_string(&entries.elems[i].entry, &tmp);
		printf("%s", dd_twine_chars(&tmp));
	}
	for (i = 0; i < entries.size; i++) {
		free_di_dict_entry(&entries.elems[i].entry);
	}
	DD_FREE_ARRAY(&entries);
}

static void di_config_handle(DIConfig *conf) {
	if (!dd_twine_len(&conf->path)) {
		fprintf(stderr, "exec_di: --path is required\n");
		exit (1);
	} else if (dd_twine_len(&conf->lookup)) {
		print_lookup(conf);
	} else if (dd_twine_len(&conf->add)) {
		add_pron(conf);
	} else if (dd_twine_len(&conf->read_str)) {
		read_string(conf);
	}
}

int main(int argc, char **argv) {
	int c;
	DIConfig conf;

	di_config_init(&conf);

	while (1) {
		static struct option long_options[] =
		{
			{"lookup", required_argument, 0, 'a'},
			{"path", required_argument, 0, 'b'},
			{"add", required_argument, 0, 'c'},
			{"read", required_argument, 0, 'd'},
			{0, 0, 0, 0}
		};

		int option_index = 0;

		c = getopt_long(argc, argv, "", long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
			case 'a':
				dd_twine_from_chars_dyn(&conf.lookup, optarg);
				break;
			case 'b':
				dd_twine_from_chars_dyn(&conf.path, optarg);
				break;
			case 'c':
				dd_twine_from_chars_dyn(&conf.add, optarg);
				break;
			case 'd':
				dd_twine_from_chars_dyn(&conf.read_str, optarg);
				break;
		}
	}

	di_config_handle(&conf);
	di_config_destroy(&conf);
}
