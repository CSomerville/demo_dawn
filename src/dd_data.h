#ifndef dd_data_h
#define dd_data_h

#include <stdlib.h>

#define DD_ALLOCATE(type, count) \
    (type*)reallocate(NULL, 0, sizeof(type) * (count))

#define DD_FREE(type, ptr) reallocate(ptr, sizeof(type), 0)

#define DD_GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define DD_GROW_ARRAY(type, ptr, old_cap, new_cap) \
    (type*)reallocate(ptr, sizeof(type) * (old_cap), sizeof(type) * (new_cap))

#define DD_DEF_ARRAY(T, name)                                                       \
    typedef struct {                                                                \
        int size;                                                                   \
        int capacity;                                                               \
        T* elems;                                                                   \
    } DDArr##name;

#define DD_INIT_ARRAY(a)                                                            \
    do {                                                                            \
        (a)->size = 0;                                                              \
        (a)->capacity = 0;                                                          \
        (a)->elems = NULL;                                                          \
    } while (0)

#define DD_INIT_ARRAY_SIZE(a, s)							\
	do {													\
		(a)->size = s;										\
		(a)->capacity = s;									\
		(a)->elems = reallocate((a)->elems, 0,				\
				sizeof(*((a)->elems)) * (a)->capacity);		\
	} while (0)
		

#define DD_ADD_ARRAY(a, k)                                                          \
    do {                                                                            \
        if ((a)->capacity < (a)->size + 1) {                                        \
            int old_cap = (a)->capacity;                                            \
            (a)->capacity = DD_GROW_CAPACITY(old_cap);                              \
            (a)->elems = reallocate((a)->elems, sizeof(*((a)->elems)) * old_cap,    \
                    sizeof(*((a)->elems)) * (a)->capacity);                         \
        }                                                                           \
        (a)->elems[(a)->size] = k;                                                  \
        (a)->size++;                                                                \
    } while (0)

#define DD_FREE_ARRAY(a)                                                            \
    do {                                                                            \
        reallocate((a)->elems, sizeof(*((a)->elems)) * (a)->capacity, 0);           \
        DD_INIT_ARRAY(a);                                                           \
    } while (0)

#define DD_DEF_QUEUE(T, name) 			\
	typedef struct DDQNode##name { 		\
		T val;							\
		struct DDQNode##name *next;		\
	} DDQNode##name; 					\
	typedef struct {					\
		DDQNode##name *first;			\
		DDQNode##name *last;			\
	} DDQ##name;	

#define DD_INIT_QUEUE(q)				\
	do {								\
		(q)->first = NULL;				\
		(q)->last = NULL;				\
	} while (0)

#define DD_QUEUE_EMPTY(q)				\
	((q)->first == NULL)

#define DD_ENQUEUE(name, q, v)				\
	do {									\
		DDQNode##name *qn;					\
		qn = malloc(sizeof(*((q)->first)));	\
		qn->val = v;						\
		qn->next = NULL;					\
		if (DD_QUEUE_EMPTY(q)) {			\
			(q)->first = qn;				\
			(q)->last = qn;					\
		} else {							\
			(q)->last->next = qn;			\
			(q)->last = qn;					\
		}									\
	} while (0)

#define DD_DEQUEUE(name, q, v)				\
	do {									\
		v = (q)->first->val;				\
		DDQNode##name *qn = (q)->first;		\
		(q)->first = qn->next;				\
		free(qn);							\
	} while (0)

typedef struct {
    int length;
    char* chars;
} DDString;

void* reallocate(void* ptr, size_t old_size, size_t new_size);
void init_dd_string(DDString *dd_string);
DDString* copy_string(const char* chars, int length);
void give_to_dd_string(DDString *dd_str, const char* chars, int length);
DDString* dd_string_concat(DDString *a, DDString *b);
void dd_string_concat_mutate(DDString *a, DDString *b);
int get_next_dd_string_word_bounds(DDString *dd_str, int start,
		int *word_start, int *word_end);
void free_string(DDString* dd_string);
void free_dd_chars(DDString *dd_str);

DD_DEF_ARRAY(DDString, DDString)
DD_DEF_ARRAY(DDArrDDString, DDArrDDString)
DD_DEF_ARRAY(int, Int)
DD_DEF_ARRAY(DDArrInt, DDArrInt)
DD_DEF_QUEUE(int, Int)

void free_dd_arr_dd_str(DDArrDDString *dd_arr_dd_str);

#endif
