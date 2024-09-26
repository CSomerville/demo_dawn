#ifndef fe_rain_h
#define fe_rain_h

#include "dd_twine_ball.tab.h"
#include "dd_twine.h"

typedef enum {
	FE_RAIN_NULL,
	FE_RAIN_FOR_DOLLARS,
	FE_RAIN_FOR_THE_DOLLARS,
	FE_RAIN_ABOUT_CURRENCY,
	FE_RAIN_HAS_SONG,
	FE_RAIN_SONG_GOING_SUBJECT,
	FE_RAIN_SONG_GOING_OBJECT,
	FE_RAIN_TO_BUY,
	FE_RAIN_SONG_MEDIUM,
	FE_RAIN_TEMPORAL_CONNECTOR,

	FE_RAIN_1_NULL,
	FE_RAIN_1_MAKE_REMEMBER,
	FE_RAIN_1_FOR_DOLLARS,
	FE_RAIN_1_HOW_DOLLARS,
	FE_RAIN_1_WHO_SINGS,
	FE_RAIN_1_SONG_GOING,
	FE_RAIN_1_SPEECH_ENDING,
} FERainSavePoint;

typedef struct {
	DDTwineBallData rain_data;
	FERainSavePoint save_point;
	int level;
	int level_0_ctr;
} FERain;

void fe_rain_init(FERain *fe_rain);
void fe_rain_destroy(FERain *fe_rain);
void fe_rain_advance(DDTwine *out_str, FERain *fe_rain);

#endif
