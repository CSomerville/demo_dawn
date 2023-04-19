#include <stdio.h>
#define YYSTYPE MONSTRESTYPE
#include "fe_monstre.tab.h"
#include "fe_monstre.lex.h"
#include "dd_data.h"

int main(void) {
	int i;
	struct accum acc;
	DD_INIT_ARRAY(&acc.acc);
	yyscan_t scanner;

	if ((i = monstrelex_init(&scanner)) != 0) {
		exit(i);
	}

	int e = monstreparse(&acc, scanner);
	printf("Code = %d\n", e);
	if (e == 0) {
		for (i = 0; i < acc.acc.size; i++) {
			printf("%d: %ld\n", i, acc.acc.elems[i]);
		}
	}
	monstrelex_destroy(scanner);
	return 0;
}
