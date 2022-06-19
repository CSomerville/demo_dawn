#include <stdio.h>
#include <stdlib.h>

#include "dd_utils.h"
#include "te_scanner.h"
#include "te_tendril.h"
#include "dd_data.h"

int main(void) {
    char* source = read_file("./static/festival/test2.graa");
    int i, j, k;

    TEScanner scanner;
    DDArrTETendril tendrils;

    init_te_scanner(&scanner, source);
    DD_INIT_ARRAY(&tendrils);

    parse_tendrils(&scanner, &tendrils);

    printf("my statespace name: %s\n", tendrils.elems[0].name.chars);

    for (i = 0; i < tendrils.size; i++) {
        for (j = 0; j < tendrils.elems[i].legend.keys.size; j++) {
            printf("key: %s\tvalues: ", tendrils.elems[i].legend.keys.elems[j].chars);
            for (k = 0; k < tendrils.elems[i].legend.values.elems[j].size; k++) {
                printf("%s ", tendrils.elems[i].legend.values.elems[j].elems[k].chars);
            }
            printf("\n");
        }
    }


    free(source);
    DD_FREE_ARRAY(&tendrils);
}
