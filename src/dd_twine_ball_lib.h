#include "dd_twine_ball.tab.h"

void dd_twine_ball_read(DDTwineBallData *data, const char *fpath);
void dd_twine_ball_parse(DDTwineBallData data, FILE *f);
void dd_twine_ball_data_destroy(DDTwineBallData *data);
