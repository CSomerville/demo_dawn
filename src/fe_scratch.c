#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "dd_utils.h"
#include "te_scanner.h"
#include "te_tendril.h"
#include "dd_data.h"
#include "dd_graph.h"

/* run bfs to determine connected components.
 * check content coverage.
 * fn to load source
 */

int main(void) {
	srand(time(NULL));
	char* source = read_file("./static/festival/goners.te");
	int current;
	TEScanner scanner;
	DDArrTETendril tendrils;

	init_te_scanner(&scanner, source);
	DD_INIT_ARRAY(&tendrils);

	parse_tendrils(&scanner, &tendrils);
	/*print_tendril_legend(&tendrils.elems[0].legend);*/

}
