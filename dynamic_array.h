/*
Overview:
    Implement dynamic array in C.
    Basic code for "append"s get from nob.h^1 by tsoding

API:
- constants:
    DA_DEFAULT_INIT_CAP - default capacity for just created 'da'^2,
                          maybe set by user before include this file
    DA_NULL_DTOR        - null pointer with type `void (*)(void*)`

- structures:
    DA_CREATE_STRUCT - create definition for 'da' struct
                       with passed type and name
    DA_FORLOOP       - support macros for shorter for-loop

- functions^3:
    da_append        - append value to end of 'da'
    da_append_many   - append values from another array to end of 'da'
    da_clear         - set all items and count as zero and save capacity
    da_free          - free memory by items and set all fields as zero
    da_remove        - call destroy function for item and shift other
                       items, use `DA_NULL_DTOR` if not need dtor
    da_remove_many   - call destroy function for items in range
                       [`i`, `j`) and shift other items,
                       use `DA_NULL_DTOR` if not need dtor
    da_reserve       - reserve places for items
    da_shrink_to_fit - reset capacity equal count

Example:
'''
#include "dynamic_array.h"
#include <stdio.h>

DA_CREATE_STRUCT(const char*, args_t)

int main(int argc, char** argv) {
    args_t args = {0};
    da_append_many(&args, argv, argc);
    for (size_t i = 0; i < args.count; i++)
        puts(args.items[i]);
    da_free(&args, DA_NULL_DTOR);
    return 0;
}
'''

Footnotes:
    [1]: https://github.com/tsoding/nob.h
    [2]: 'da' - object of type dynamic array
    [3]: 'functions' i.e. do {...} while (0) macros
*/

#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef DA_DEFAULT_INIT_CAP
#define DA_DEFAULT_INIT_CAP 64
#endif

#define DA_NULL_DTOR ((void(*)(void*))NULL)

/* define structure for dynamic array */
#define DA_CREATE_STRUCT(type, name) \
typedef struct {     \
    type*  items;    \
    size_t count;    \
    size_t capacity; \
} name;

/* Short syntax for-loop */
#define DA_FORLOOP(var, init, end) \
for (size_t var = init; var < end; ++var)

/**
 * @brief add `value` to end `da`
 * @param da pointer to dynamic array
 * @param value value for append
 */
#define da_append(da, value) do {                    \
    if ((da)->count >= (da)->capacity) {             \
        (da)->capacity += (da)->capacity == 0        \
            ? DA_DEFAULT_INIT_CAP                    \
            : ((da)->capacity + 1) / 2;              \
        (da)->items = realloc((da)->items,           \
            (da)->capacity * sizeof(*(da)->items));  \
        assert((da)->items != NULL && "Not memory"); \
    }                                                \
    (da)->items[(da)->count++] = (value);            \
} while (0)

/**
 * @brief add items from `values` to end `da`
 * @param da pointer to dynamic array
 * @param values pointer to array of values
 * @param values_count count items in `values`
 */
#define da_append_many(da, values, values_count) do {         \
    if ((da)->count + (values_count) > (da)->capacity) {      \
        if ((da)->capacity == 0)                              \
            (da)->capacity = DA_DEFAULT_INIT_CAP;             \
        while ((da)->count + (values_count) > (da)->capacity) \
            (da)->capacity += ((da)->capacity + 1) / 2;       \
        (da)->items = realloc((da)->items,                    \
            (da)->capacity * sizeof(*(da)->items));           \
        assert((da)->items != NULL && "Not memory");          \
    }                                                         \
    memcpy((da)->items + (da)->count, (values),               \
        (values_count) * sizeof(*(da)->items));               \
    (da)->count += (values_count);                            \
} while (0)

/**
 * @brief set all items at zero,
 * set `count` = 0 and save capacity
 * @param da pointer to dynamic array
 * @param dtor pointer to destroy function
 *             with signature `void (type*)`
 */
#define da_clear(da, dtor) do {        \
    if ((dtor) != NULL)                \
        DA_FORLOOP(i, 0, (da)->count)  \
            (dtor)(&(da)->items[i]);   \
    memset((da)->items, 0, (da)->count \
        * sizeof(*(da)->items));       \
    (da)->count = 0;                   \
} while (0)

/**
 * @brief free memory for `da` and set fields at zero
 * @param da pointer to dynamic array
 * @param dtor pointer to destroy function
 *             with signature `void (type*)`
 */
#define da_free(da, dtor) do {        \
    if ((dtor) != NULL)               \
        DA_FORLOOP(i, 0, (da)->count) \
            (dtor)(&(da)->items[i]);  \
    free((da)->items);                \
    (da)->items = NULL;               \
    (da)->count = 0;                  \
    (da)->capacity = 0;               \
} while (0)

/**
 * @brief destroy object at `index` and
 * shift other items in [`index`+1, `da.count`)
 * @param da pointer to dynamic array
 * @param index valid index in range [0, `da.count`)
 * @param dtor pointer to destroy function
 *             with signature `void (type*)`
 */
#define da_remove(da, index, dtor) do {   \
    assert((index) < (da)->count          \
        && "Out of range");               \
    if ((dtor) != NULL)                   \
        (dtor)(&(da)->items[(index)]);    \
    memmove(&(da)->items[(index)],        \
        &(da)->items[(index)] + 1,        \
        sizeof(*(da)->items) *            \
            ((da)->count - (index) - 1)); \
    --((da)->count);                      \
} while (0)

/**
 * @brief destroy objects in range [`i`, `j`)
 * and shift other items in [`j`, `da.count`)
 * @param da pointer to dynamic array
 * @param i begin index in range [`i`, `j`)
 * @param j end index in range [`i`, `j`)
 * @param dtor pointer to destroy function
 *             with signature `void (type*)`
 */
#define da_remove_many(da, i, j, dtor) do { \
    assert((i) < (da)->count                \
        && (j) <= (da)->count               \
        && "Out of range");                 \
    if ((dtor) != NULL)                     \
        DA_FORLOOP(k, (i), (j))             \
            (dtor)(&(da)->items[k]);        \
    memmove(&(da)->items[(i)],              \
        &(da)->items[(j)],                  \
        sizeof(*(da)->items) *              \
            ((da)->count - (j)));           \
    (da)->count -= (j) - (i);               \
} while (0)

/**
 * @brief reserve memory for need `new_cap`
 * @param da pointer to dynamic array
 * @param new_cap new capacity for storage
 */
#define da_reserve(da, new_cap) do {                 \
    if ((da)->capacity < (new_cap)) {                \
        (da)->capacity = (new_cap);                  \
        (da)->items = realloc((da)->items,           \
            (da)->capacity * sizeof(*(da)->items));  \
        assert((da)->items != NULL && "Not memory"); \
    }                                                \
} while (0)

/**
 * @brief reset capacity equal count
 * @param da pointer to dynamic array
 */
#define da_shrink_to_fit(da) do {                \
    (da)->capacity = (da)->count;                \
    (da)->items = realloc((da)->items,           \
        (da)->capacity * sizeof(*(da)->items));  \
    assert((da)->items != NULL && "Not memory"); \
} while (0)

#endif // DYNAMIC_ARRAY_H