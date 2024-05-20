#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include "fe_lib.h"
#include "dd_twine.h"

void handle_conf(FEstival *festival, int argc, char **argv) {
	int c;

	while (1) {
		static struct option long_options[] =
		{
			{"test", required_argument, 0, 'a'},
			{0, 0, 0, 0}
		};
		int option_index = 0;
		c = getopt_long(argc, argv, "", long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
			case 'a':
				dd_twine_from_chars_dyn(festival->test, optarg);
		}
	}
}

int main(int argc, char **argv) {
	srand(time(NULL));

	FEstival festival;
	init_festival(&festival);
	handle_conf(&festival, argc, argv);
	inaugurate_festival(&festival);

	destroy_festival(&festival);
	return 0;
}
