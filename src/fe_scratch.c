#include <stdio.h>
#include <stdlib.h>

#include "dd_utils.h"
#include "fe_scanner.h"
#include "dd_data.h"

int main(void) {
    char* source = read_file("./static/festival/test.graa");
    DDArrTok tokens;
    Scanner scanner;
    Token tok;
    int i;

    init_scanner(&scanner, source);
    DD_INIT_ARRAY(&tokens);

    while (1) {
        tok = scan_token(&scanner);
        DD_ADD_ARRAY(&tokens, tok);
        if (tok.type == TOKEN_EOF) {
            break;
        }
    }

    for (i = 0; i < tokens.size; i++) {
        printf("token type: %d\n", tokens.elems[i].type);
    }

    free(source);
    DD_FREE_ARRAY(&tokens);
}
