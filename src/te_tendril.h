#include "dd_data.h"
#include "te_scanner.h"

typedef struct {
    TEToken current;
    TEToken previous;
} TEParser;

typedef struct {
    DDArrDDString keys;
    DDArrDDArrDDString values;
} TETendrilLegend;

typedef struct {
    DDString name;
    TETendrilLegend legend;
} TETendril;

DD_DEF_ARRAY(TETendril, TETendril);

void parse_tendrils(TEScanner *scanner, DDArrTETendril *tendrils);
