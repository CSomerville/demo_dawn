#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "dd_utils.h"
#include "te_scanner.h"
#include "te_tendril.h"
#include "dd_data.h"
#include "dd_graph.h"
#include "fe_lib.h"

/* run bfs to determine connected components.
 * check content coverage.
 * fn to load source
 */

int main(void) {
	srand(time(NULL));
	char* source = read_file("./static/festival/therm.te");
	TEScanner scanner;
	DDArrTETendril tendrils;
	DDArrDDString result;
	int i;

	init_te_scanner(&scanner, source);
	DD_INIT_ARRAY(&tendrils);
	DD_INIT_ARRAY(&result);

	parse_tendrils(&scanner, &tendrils);

	run_n_steps(&tendrils.elems[0], &result, 0, 20);

	for (i = 0; i < result.size; i++) {
		printf("%s ", result.elems[i].chars);
	}
}
