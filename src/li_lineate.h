#ifndef li_lineate_h
#define li_lineate_h


void li_lineate_trochee(DDArrInt *indices,
		DDArrDIIndexedEntry *dict_entries, DDTwine *raw_txt);
void li_lineate_to_arr(DDArrDDTwine *lines, DDArrInt *indices,
		DDTwine *raw_txt);

#endif
