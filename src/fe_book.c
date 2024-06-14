#include <setjmp.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "hpdf.h"
#include "dd_twine.h"
#include "fe_book.h"
#include "dd_utils.h"

jmp_buf env;

error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no,
			void *user_data) {
	printf("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
				                (HPDF_UINT)detail_no);
	longjmp(env, 1);
}

int fe_book_init(FEBook *fe_book, const char *outpath) {
	fe_book->pdf = HPDF_New(error_handler, NULL);
	const char *font_name;
    if (!fe_book->pdf) {
		printf("error: cannot create PdfDoc object\n");
		return 1;
	}

	if (setjmp(env)) {
		HPDF_Free(fe_book->pdf);
		return 1;
	}
	font_name = HPDF_LoadTTFontFromFile(fe_book->pdf,
			"./static/fonts/cormorant/Cormorant-VariableFont_wght.ttf",
			HPDF_TRUE);
	
	fe_book->main_font = HPDF_GetFont(fe_book->pdf, font_name, NULL);
	fe_book->current_font_size = 12;

	fe_book->margin_top = 0.1;
	fe_book->margin_bottom = 0.1;
	fe_book->margin_outside = 0.15;
	fe_book->margin_inside = 0.075;

	fe_book->outpath = outpath;
	fe_book->current_v_page = NULL;
	DD_INIT_ARRAY(&fe_book->virtual_pages);

	/* add one page for virtual pages to work off of */
	fe_book_add_real_page(fe_book);
}

void fe_book_add_real_page(FEBook *fe_book) {
	fe_book->current_page = HPDF_AddPage(fe_book->pdf);
	HPDF_Page_SetSize(fe_book->current_page, HPDF_PAGE_SIZE_LETTER,
			HPDF_PAGE_LANDSCAPE);
	HPDF_Page_SetFontAndSize(fe_book->current_page, fe_book->main_font,
			fe_book->current_font_size);
}

void fe_book_add_page(FEBook *fe_book) {
	FEBookVirtualPage new_page;
	double real_height, real_width, virtual_height, virtual_width,
		   half_width;
	real_height = HPDF_Page_GetHeight(fe_book->current_page);
	real_width = HPDF_Page_GetWidth(fe_book->current_page);
	virtual_height = real_height - (real_height * fe_book->margin_top) -
		(real_height * fe_book->margin_bottom);
	half_width = real_width / 2;
	virtual_width = half_width - 
		(half_width * fe_book->margin_outside) -
		(half_width * fe_book->margin_inside);
	new_page.height = virtual_height;
	new_page.width = virtual_width;
	new_page.last_line = NULL;
	DD_INIT_ARRAY(&new_page.contents);
	DD_ADD_ARRAY(&fe_book->virtual_pages, new_page);
	fe_book->current_v_page = &fe_book->virtual_pages
		.elems[fe_book->virtual_pages.size-1];
}

static void lineate(DDArrFEBookLine *lines, FEBook *fe_book,
		FEBookBoundingRect *rect, DDTwine *text) {
	printf("entering lineate:%s\n", text->chars);
	int idx = 0;
	int start = 0;
	int ctr = 0;
	int line_height = fe_book->current_font_size + 3;
	double width, allowed_width;
	char *str, *last_str = NULL;
	FEBookLine line;
	allowed_width = fe_book->current_v_page->width - rect->x1;
	while (idx < dd_twine_len(text)) {
		if (isspace(dd_twine_chars(text)[idx]) || idx + 1 == dd_twine_len(text)) {
			str = strndup(&dd_twine_chars(text)[start], idx - start);
			width = HPDF_Page_TextWidth(fe_book->current_page, str);
			if (width < allowed_width) {
				last_str = str;
				while (isspace(dd_twine_chars(text)[idx]))
					idx++;
			} else {
			dd_twine_from_chars_dyn(&line.text, last_str);
			last_str = NULL;
			idx = dd_twine_len(&line.text) + start;
			line.x1 = rect->x1;
			line.x2 = HPDF_Page_TextWidth(fe_book->current_page,
					dd_twine_chars(&line.text));
			line.y1 = rect->y1 - (line_height * ctr);
			line.y2 = line.y1 - line_height;
			DD_ADD_ARRAY(lines, line);
			ctr++;
			while (isspace(dd_twine_chars(text)[idx]))
				idx++;
			start = idx;
		}
	} else {
		idx++;
	}
}
	dd_twine_from_chars_dyn(&line.text, last_str);
	line.x1 = rect->x1;
	line.x2 = HPDF_Page_TextWidth(fe_book->current_page,
			dd_twine_chars(&line.text));
	line.y1 = rect->y1 - (line_height * ctr);
	line.y2 = line.y1 - line_height;
	DD_ADD_ARRAY(lines, line);
	free(str);
}

static void add_line(FEBook *fe_book, FEBookBoundingRect *rect,
		DDTwine *text) {
	FEBookLine line;
	line.x1 = rect->x1;
	line.x2 = rect->x2;
	line.y1 = rect->y1;
	line.y2 = rect->y2;
	dd_twine_copy(&line.text, text);
	line.font_size = fe_book->current_font_size;
	DD_ADD_ARRAY(&fe_book->current_v_page->contents, line);
	fe_book->current_v_page->last_line = &fe_book->current_v_page
		->contents.elems[fe_book->current_v_page->contents.size-1];
}

static void add_text_straight_left(FEBook *fe_book, DDTwine *text) {
	FEBookBoundingRect rect;
	double text_width;
	int line_height;

	text_width = HPDF_Page_TextWidth(fe_book->current_page,
			dd_twine_chars(text));
	
	line_height = fe_book->current_font_size + 5;
	/* what follows to be replaced */
	if (fe_book->current_v_page->last_line == NULL) {
		rect.x1 = 0;
		rect.x2 = text_width;
		rect.y1 = fe_book->current_v_page->height;
		rect.y2 = rect.y1 - fe_book->current_font_size;
	} else if (fe_book->current_v_page->last_line->y1 - line_height < 0) {
		fe_book_add_page(fe_book);
		rect.x1 = 0;
		rect.x2 = text_width;
		rect.y1 = fe_book->current_v_page->height;
		rect.y2 = rect.y1 - fe_book->current_font_size;
	} else {
		rect.x1 = 0;
		rect.x2 = text_width;
		rect.y1 = fe_book->current_v_page->last_line->y1 - line_height;
		rect.y2 = rect.y1 - fe_book->current_font_size;
	}

	add_line(fe_book, &rect, text);
}

/* if left page and none previous
 * 		place top left
 * else
 * 		try to place in range
 *
 * if try to place fails
 * 		if right
 * 			if line is too long for page
 * 				if is in left two-thirds
 *	 				lineate
 * 				else if on left page
 * 					jump to right and lineate
 * 				else
 * 					new page and lineate
 * 			if left in first third
 * 				lineate
 * 			else if left in center third
 * 				clamp right
 * 			else if on left page
 * 				jump to right page
 * 			else
 * 				new page
 * 		if bottom
 * 			new page
 */

#define MEANDER_X 0.3
#define MEANDER_Y 0.075

static void meandering_place_top_left(FEBookBoundingRect *rect,
		FEBook *fe_book, DDTwine *text) {
	rect->x1 = rand_double() * MEANDER_X * fe_book->current_v_page->width;
	rect->x2 = HPDF_Page_TextWidth(fe_book->current_page,
			dd_twine_chars(text));
	rect->y1 = fe_book->current_v_page->height - rand_double() * 
		MEANDER_Y * fe_book->current_v_page->height;
	rect->y2 = rect->y1 - fe_book->current_font_size;
}

static void meandering_place_next(FEBookBoundingRect *rect,
		FEBook *fe_book, DDTwine *text) {
	rect->x1 = fe_book->current_v_page->last_line->x1 - 4 + rand_double() 
		* MEANDER_X * fe_book->current_v_page->width;
	rect->x2 = rect->x1 + HPDF_Page_TextWidth(fe_book->current_page,
			dd_twine_chars(text));
	rect->y1 = fe_book->current_v_page->last_line->y2 + 4 
		- rand_double() * MEANDER_Y * fe_book->current_v_page->height;
	rect->y2 = rect->y1 - fe_book->current_font_size;
}

static void get_placement(FEBookPlacement *placement, 
		FEBookBoundingRect *rect, FEBook *fe_book) {
	placement->fits_page = fe_book->current_v_page->width >
		rect->x2 - rect->x1;

	if (rect->x1 < fe_book->current_v_page->width / 3) {
		placement->left_pos = FIRST_THIRD;
	} else if (rect->x1 < (fe_book->current_v_page->width / 3) * 2) {
		placement->left_pos = SECOND_THIRD;
	} else {
		placement->left_pos = THIRD_THIRD;
	}

	placement->is_right = rect->x2 > fe_book->current_v_page->width;
	placement->is_bottom = rect->y2 < 0;
}


static void add_meandering_text(FEBook *fe_book, DDTwine *text) {
	FEBookBoundingRect rect;
	FEBookPlacement placement;
	DDArrFEBookLine lines;
	bool is_lineated = false;
	int i;

	DD_INIT_ARRAY(&lines);

	if (fe_book->current_v_page->last_line == NULL) {
		meandering_place_top_left(&rect, fe_book, text);
	} else {
		meandering_place_next(&rect, fe_book, text);
	}

	get_placement(&placement, &rect, fe_book);
	
	if (placement.is_right) {
		if (!placement.fits_page) {
			if (placement.left_pos != THIRD_THIRD) {
				lineate(&lines, fe_book, &rect, text);
				is_lineated = true;
			} else {
				if (fe_book->virtual_pages.size % 2 == 1) {
					rect.y1 = fe_book->current_v_page->last_line->y1;
					rect.y2 = fe_book->current_v_page->last_line->y2;
					fe_book_add_page(fe_book);
					lineate(&lines, fe_book, &rect, text);
					is_lineated = true;
				} else {
					fe_book_add_page(fe_book);
					meandering_place_top_left(&rect, fe_book, text);
					lineate(&lines, fe_book, &rect, text);
					is_lineated = true;
				}
			}
		} else {
			if (placement.left_pos == FIRST_THIRD) {
				lineate(&lines, fe_book, &rect, text);
				is_lineated = true;
			} else if (placement.left_pos == SECOND_THIRD) {
				lineate(&lines, fe_book, &rect, text);
				is_lineated = true;
			} else {
				if (fe_book->virtual_pages.size % 2 == 1) {
					rect.y1 = fe_book->current_v_page->last_line->y1;
					rect.y2 = fe_book->current_v_page->last_line->y2;
					fe_book_add_page(fe_book);
					rect.x1 = rand_double() * MEANDER_X 
						* fe_book->current_v_page->width;
					rect.x2 = HPDF_Page_TextWidth(fe_book->current_page,
						dd_twine_chars(text));
				} else {
					fe_book_add_page(fe_book);
					meandering_place_top_left(&rect, fe_book, text);
				}
			}
		}
	} else if (placement.is_bottom) {
		fe_book_add_page(fe_book);
		meandering_place_top_left(&rect, fe_book, text);
	}
	if (is_lineated) {
		for (i = 0; i < lines.size; i++) {
			DD_ADD_ARRAY(&fe_book->current_v_page->contents, 
					lines.elems[i]);
			fe_book->current_v_page->last_line = &fe_book->current_v_page
				->contents.elems[fe_book->current_v_page->contents.size-1];
		}
	} else {
		add_line(fe_book, &rect, text);
	}
}

void fe_book_add_text(FEBook *fe_book, DDTwine *text) {
	add_meandering_text(fe_book, text);
}

static double computed_margin_bottom(FEBook *fe_book) {
	return HPDF_Page_GetHeight(fe_book->current_page) 
		* fe_book->margin_bottom;
}

static double computed_margin_left(FEBook *fe_book, bool is_left_page) {
	double half_page, real_width;
	real_width = HPDF_Page_GetWidth(fe_book->current_page);
	half_page = real_width / 2;
	if (is_left_page) {
		return half_page * fe_book->margin_outside;
	} else {
		return half_page * fe_book->margin_inside + half_page;
	}
}

void fe_book_write(FEBook *fe_book) {
	int i, j;
	for (i = 0; i < fe_book->virtual_pages.size; i++) {
		if (i != 0 && i % 2 == 0) {
			fe_book_add_real_page(fe_book);
		}
		for (j = 0; j < fe_book->virtual_pages.elems[i].contents.size; j++) {
			HPDF_Page_BeginText(fe_book->current_page);
			HPDF_Page_TextOut(fe_book->current_page, 
					fe_book->virtual_pages.elems[i].contents.elems[j].x1
						+ computed_margin_left(fe_book, i % 2 == 0),
					fe_book->virtual_pages.elems[i].contents.elems[j].y1
						+ computed_margin_bottom(fe_book),
					dd_twine_chars(&fe_book->virtual_pages.elems[i]
						.contents.elems[j].text));
			HPDF_Page_EndText(fe_book->current_page);
		}
	}

	HPDF_SaveToFile(fe_book->pdf, fe_book->outpath);
}

void fe_book_free(FEBook *fe_book) {
	int i, j;
	HPDF_Free(fe_book->pdf);

	for (i = 0; i < fe_book->virtual_pages.size; i++) {
		for (j = 0; j < fe_book->virtual_pages.elems[i].contents.size; j++) {
			dd_twine_destroy(&fe_book->virtual_pages.elems[i].contents
					.elems[j].text);
		}
		DD_FREE_ARRAY(&fe_book->virtual_pages.elems[i].contents);
	}
	DD_FREE_ARRAY(&fe_book->virtual_pages);
}	

