#ifndef dd_data_h
#define dd_data_h

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

typedef struct {
    int length;
    char* chars;
} DDString;

void* reallocate(void* ptr, size_t old_size, size_t new_size);
DDString* copy_string(const char* chars, int length);
void free_string(DDString* dd_string);

DD_DEF_ARRAY(DDString, DDString);
DD_DEF_ARRAY(DDArrDDString, DDArrDDString);

#endif
