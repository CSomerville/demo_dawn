#include <stdio.h>
#include <time.h>
#include "fe_lib.h"

int main() {
	srand(time(NULL));

	FEstival festival;
	init_festival(&festival);
	inaugurate_festival(&festival);

	printf("%s\n", festival.raw->chars);

	destroy_festival(&festival);
}
