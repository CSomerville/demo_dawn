#include "fe_rain.h"
#include "dd_twine.h"
#include "dd_twine_ball.tab.h"
#include "dd_twine_ball_lib.h"

static void for_dollars(DDTwine *out_str, FERain *fe_rain) {
	DDTwine tmp;
	dd_twine_init(&tmp);

	dd_twine_ball_select_twine(&tmp, fe_rain->rain_data.root, "for_dollars");
	dd_twine_concat_with_char_mut(out_str, &tmp, ' ');
	dd_twine_destroy(&tmp);

	fe_rain->save_point = FE_RAIN_FOR_DOLLARS;
}

static void for_the_dollars(DDTwine *out_str, FERain *fe_rain) {
	DDTwine tmp;
	dd_twine_init(&tmp);

	dd_twine_ball_select_twine(&tmp, fe_rain->rain_data.root,
			"for_the_dollars");
	dd_twine_concat_with_char_mut(out_str, &tmp, ' ');
	dd_twine_destroy(&tmp);

	fe_rain->save_point = FE_RAIN_FOR_THE_DOLLARS;
}

static void about_currency(DDTwine *out_str, FERain *fe_rain) {
	DDTwine tmp;
	dd_twine_init(&tmp);

	dd_twine_ball_select_twine(&tmp, fe_rain->rain_data.root,
			"about_currency");
	dd_twine_concat_with_char_mut(out_str, &tmp, ' ');
	dd_twine_destroy(&tmp);

	fe_rain->save_point = FE_RAIN_ABOUT_CURRENCY;
}

static void song_going(DDTwine *out_str, FERain *fe_rain,
		int part_of_speech) {
	DDTwine tmp;
	dd_twine_init(&tmp);

	dd_twine_from_chars_dyn(&tmp, "song going");
	dd_twine_concat_with_char_mut(out_str, &tmp, ' ');
	dd_twine_destroy(&tmp);
	dd_twine_init(&tmp);

	dd_twine_ball_select_twine(&tmp, fe_rain->rain_data.root, "lyric");
	dd_twine_concat_with_char_mut(out_str, &tmp, ' ');
	dd_twine_destroy(&tmp);

	if (part_of_speech == 0)
		fe_rain->save_point = FE_RAIN_SONG_GOING_SUBJECT;
	else if (part_of_speech == 1)
		fe_rain->save_point = FE_RAIN_SONG_GOING_OBJECT;
}

static void to_buy(DDTwine *out_str, FERain *fe_rain) {
	DDTwine tmp;
	dd_twine_init(&tmp);

	dd_twine_from_chars_dyn(&tmp, "to grip");
	dd_twine_concat_with_char_mut(out_str, &tmp, ' ');
	dd_twine_destroy(&tmp);

	fe_rain->save_point = FE_RAIN_TO_BUY;
}

static void has_song(DDTwine *out_str, FERain *fe_rain) {
	DDTwine tmp;
	dd_twine_init(&tmp);

	dd_twine_ball_select_twine(&tmp, fe_rain->rain_data.root, "has_song");
	dd_twine_concat_with_char_mut(out_str, &tmp, ' ');
	dd_twine_destroy(&tmp);

	fe_rain->save_point = FE_RAIN_HAS_SONG;
}

/* threads of celeb with the song going
 * flag of the land with the song going
 */

static void song_medium(DDTwine *out_str, FERain *fe_rain, 
		int include_that) {
	DDTwine tmp;
	dd_twine_init(&tmp);

	dd_twine_from_chars_dyn(&tmp, include_that ? "that ll" : "ll");
	dd_twine_concat_with_char_mut(out_str, &tmp, ' ');
	dd_twine_destroy(&tmp);
	dd_twine_init(&tmp);

	dd_twine_ball_select_twine(&tmp, fe_rain->rain_data.root, "medium");
	dd_twine_concat_with_char_mut(out_str, &tmp, ' ');
	dd_twine_destroy(&tmp);

	fe_rain->save_point = FE_RAIN_SONG_MEDIUM;
}

static void temporal_connector(DDTwine *out_str, FERain *fe_rain) {
	DDTwine tmp;
	dd_twine_init(&tmp);

	dd_twine_from_chars_dyn(&tmp, "over");
	dd_twine_concat_with_char_mut(out_str, &tmp, ' ');
	dd_twine_destroy(&tmp);

	fe_rain->save_point = FE_RAIN_TEMPORAL_CONNECTOR;
}

void fe_rain_init(FERain *fe_rain) {
	dd_twine_ball_read(&fe_rain->rain_data, "./static/festival/rain.twb");
	fe_rain->save_point = FE_RAIN_NULL;
}

void fe_rain_destroy(FERain *fe_rain) {
	dd_twine_ball_data_destroy(&fe_rain->rain_data);
}

void fe_rain_advance(DDTwine *out_str, FERain *fe_rain) {
	switch (fe_rain->save_point) {
		case FE_RAIN_NULL:
			if (rand() % 3 < 1)
				for_the_dollars(out_str, fe_rain);
			else
				for_dollars(out_str, fe_rain);
			break;
		case FE_RAIN_FOR_DOLLARS:
			if (rand() % 2 < 1)
				song_going(out_str, fe_rain, 0);
			else
				to_buy(out_str, fe_rain);
			break;
		case FE_RAIN_FOR_THE_DOLLARS:
			about_currency(out_str, fe_rain);
			break;
		case FE_RAIN_ABOUT_CURRENCY:
			if (rand() % 2 < 1)
				song_going(out_str, fe_rain, 0);
			else
				to_buy(out_str, fe_rain);
			break;
		case FE_RAIN_TO_BUY:
			if (rand() % 2 < 1)
				song_going(out_str, fe_rain, 1);
			else
				has_song(out_str, fe_rain);
			break;
		case FE_RAIN_HAS_SONG:
			song_going(out_str, fe_rain, 1);
			break;
		case FE_RAIN_SONG_GOING_SUBJECT:
			song_medium(out_str, fe_rain, 0);
			break;
		case FE_RAIN_SONG_GOING_OBJECT:
			song_medium(out_str, fe_rain, 1);
			break;
		case FE_RAIN_SONG_MEDIUM:
			temporal_connector(out_str, fe_rain);
			break;
		case FE_RAIN_TEMPORAL_CONNECTOR:
			if (rand() % 3 < 1)
				for_the_dollars(out_str, fe_rain);
			else
				for_dollars(out_str, fe_rain);
			break;
	}
}
