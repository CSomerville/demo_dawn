#include <setjmp.h>
#include <stdio.h>
#include "hpdf.h"
#include "dd_twine.h"
#include "fe_book.h"

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

static void add_text_straight_left(FEBook *fe_book, DDTwine *text) {
	FEBookLine line;
	double text_width;
	int line_height;

	dd_twine_copy(&line.text, text);
	text_width = HPDF_Page_TextWidth(fe_book->current_page,
			dd_twine_chars(&line.text));
	line.font_size = fe_book->current_font_size;
	
	line_height = fe_book->current_font_size + 5;
	/* what follows to be replaced */
	if (fe_book->current_v_page->last_line == NULL) {
		line.x1 = 0;
		line.x2 = text_width;
		line.y1 = fe_book->current_v_page->height;
		line.y2 = line.y1 - line.font_size;
	} else if (fe_book->current_v_page->last_line->y1 - line_height < 0) {
		fe_book_add_page(fe_book);
		line.x1 = 0;
		line.x2 = text_width;
		line.y1 = fe_book->current_v_page->height;
		line.y2 = line.y1 - line.font_size;
	} else {
		line.x1 = 0;
		line.x2 = text_width;
		line.y1 = fe_book->current_v_page->last_line->y1 - line_height;
		line.y2 = line.y1 - line.font_size;
	}

	DD_ADD_ARRAY(&fe_book->current_v_page->contents, line);
	fe_book->current_v_page->last_line = &fe_book->current_v_page
		->contents.elems[fe_book->current_v_page->contents.size-1];
}

void fe_book_add_text(FEBook *fe_book, DDTwine *text) {
	add_text_straight_left(fe_book, text);
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

