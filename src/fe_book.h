#ifndef fe_book_h
#define fe_book_h

#include "hpdf.h"
#include "dd_twine.h"

typedef struct {
	double x1;
	double x2;
	double y1;
	double y2;
	double font_size;
	DDTwine text;
} FEBookLine;

DD_DEF_ARRAY(FEBookLine, FEBookLine);

typedef struct {
	double height;
	double width;
	FEBookLine *last_line;
	DDArrFEBookLine contents;
} FEBookVirtualPage;

DD_DEF_ARRAY(FEBookVirtualPage, FEBookVirtualPage);

typedef struct {
	HPDF_Doc pdf;
	HPDF_Font main_font;
	int current_font_size;
	double margin_top;
	double margin_bottom;
	double margin_outside;
	double margin_inside;
	HPDF_Page current_page;
	const char *outpath;
	FEBookVirtualPage *current_v_page;
	DDArrFEBookVirtualPage virtual_pages;
} FEBook;

int fe_book_init(FEBook *fe_book, const char *outpath);
void fe_book_add_real_page(FEBook *fe_book);
void fe_book_add_page(FEBook *fe_book);
void fe_book_add_text(FEBook *fe_book, DDTwine *text);
void fe_book_write(FEBook *fe_book);
void fe_book_free(FEBook *fe_book);


#endif

