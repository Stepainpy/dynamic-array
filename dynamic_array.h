/*
Overview:
    Implement dynamic array in C.
    Basic code for "append"s get from nob.h^1 by tsoding

Requirements for function call:
1. use macros DA_DECLARE_STRUCT/DA_DEFINE_STRUCT in global space
2. for use necessary functions need in global space put DA_DECLARE_XXX for
function declaration and DA_DEFINE_XXX for definition function body
3. call in code:
- from named macros:   da_xxx(type)(...)
- from named function: da_fn_xxx_type(...)

!!! Attention: passed type must be is one word without whatever extention

API:
- constants:
    DA_DEFAULT_INIT_CAP - default capacity for just created 'da'^2,
                          maybe set by user before include this file

- structures:
    DA_DEFINE_CUSTOM_FIELDS_STRUCT -
                        create definition for 'da' struct
                        with passed type, name and custom
                        fields definition from variadic arguments
    DA_DECLARE_STRUCT - create declaration for 'da' struct
                        with passed type and name
    DA_DEFINE_STRUCT  - create definition for 'da' struct
                        with passed type and name
    DA_FOREACH        - For-loop macros, as range-based for-loop in C++
    DA_DECLARE_ALL    - expand to all declaration macros
    DA_DEFINE_ALL     - expand to all definition macros

- functions:
    da_append        - append value to end of 'da'
    da_append_many   - append values from another array to end of 'da'
    da_clear         - set all items and count as zero and save capacity,
                       use `NULL` if not need dtor
    da_free          - free memory by items and set all fields as zero,
                       use `NULL` if not need dtor
    da_remove        - call destroy function for item and shift other
                       items, use `NULL` if not need dtor
    da_remove_many   - call destroy function for items in range
                       [`i`, `j`) and shift other items, use `NULL`
                       if not need dtor
    da_reserve       - reserve places for items
    da_shrink_to_fit - reset capacity equal count

Example:
'''
#include "dynamic_array.h"
#include <stdio.h>

typedef const char* cstr_t; // otherwise macro-magic cannot work
DA_DEFINE_STRUCT(cstr_t, args_t)
DA_DEFINE_APPEND_MANY(cstr_t)
DA_DEFINE_FREE(cstr_t)

int main(int argc, char** argv) {
    args_t args = {0};
    da_append_many(cstr_t)(&args, (cstr_t*)argv, argc);
    DA_FOREACH(cstr_t, arg, &args) {
        puts(*arg);
    }
    da_free(cstr_t)(&args, NULL);
    return 0;
}
'''

Footnotes:
    [1]: https://github.com/tsoding/nob.h
    [2]: 'da' - object of type dynamic array
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

#define DA_FUNC_NAME(name, type) da_fn_ ## name ## _ ## type
#define DA_STRUCT_NAME(type)     da_struct_          ## type

/* Short syntax for-loop, for implementation */
#define DA_FORLOOP(var, init, end) \
for (size_t var = init; var < end; ++var)

/* For loop macros, as range-for in c++ */
#define DA_FOREACH(type, item_ptr_name, da)    \
for (type* item_ptr_name = (da)->items;        \
    item_ptr_name < (da)->items + (da)->count; \
    ++item_ptr_name)

/* declare and define structure for dynamic array */
#define DA_DECLARE_STRUCT(type, name) \
typedef struct DA_STRUCT_NAME(type) name;
#define DA_DEFINE_CUSTOM_FIELDS_STRUCT(type, name, ...) \
DA_DECLARE_STRUCT(type, name) \
struct DA_STRUCT_NAME(type) { \
    type*  items;    \
    size_t count;    \
    size_t capacity; \
    __VA_ARGS__      \
};
#define DA_DEFINE_STRUCT(type, name) \
DA_DEFINE_CUSTOM_FIELDS_STRUCT(type, name, )

/**
 * @brief add `value` to end `da`
 * @param da pointer to dynamic array
 * @param value value for append
 */
#define da_append(type) DA_FUNC_NAME(append, type)
#define DA_DECLARE_APPEND(type)  \
void da_append(type)(            \
struct DA_STRUCT_NAME(type)* da, \
type value)
#define DA_DEFINE_APPEND(type)                     \
DA_DECLARE_APPEND(type) {                          \
    if (da->count >= da->capacity) {               \
        da->capacity += da->capacity == 0          \
            ? DA_DEFAULT_INIT_CAP                  \
            : (da->capacity + 1) / 2;              \
        da->items = realloc(da->items,             \
            da->capacity * sizeof(*da->items));    \
        assert(da->items != NULL && "Not memory"); \
    }                                              \
    da->items[da->count++] = value;                \
}

/**
 * @brief add items from `values` to end `da`
 * @param da pointer to dynamic array
 * @param values pointer to array of values
 * @param values_count count items in `values`
 */
#define da_append_many(type) DA_FUNC_NAME(append_many, type)
#define DA_DECLARE_APPEND_MANY(type)  \
void da_append_many(type)(            \
struct DA_STRUCT_NAME(type)* da,      \
const type* values,                   \
size_t values_count)
#define DA_DEFINE_APPEND_MANY(type)                     \
DA_DECLARE_APPEND_MANY(type) {                          \
    if (da->count + values_count > da->capacity) {      \
        if (da->capacity == 0)                          \
            da->capacity = DA_DEFAULT_INIT_CAP;         \
        while (da->count + values_count > da->capacity) \
            da->capacity += (da->capacity + 1) / 2;     \
        da->items = realloc(da->items,                  \
            da->capacity * sizeof(*da->items));         \
        assert(da->items != NULL && "Not memory");      \
    }                                                   \
    memcpy(da->items + da->count, values,               \
        values_count * sizeof(*da->items));             \
    da->count += values_count;                          \
}

/**
 * @brief set all items at zero,
 * set `count` = 0 and save capacity
 * @param da pointer to dynamic array
 * @param dtor pointer to destroy function
 *             with signature `void (type*)`
 */
#define da_clear(type) DA_FUNC_NAME(clear, type)
#define DA_DECLARE_CLEAR(type)   \
void da_clear(type)(             \
struct DA_STRUCT_NAME(type)* da, \
void (*dtor)(type*))
#define DA_DEFINE_CLEAR(type)      \
DA_DECLARE_CLEAR(type) {           \
    if (dtor != NULL)              \
        DA_FOREACH(type, item, da) \
            dtor(item);            \
    memset(da->items, 0, da->count \
        * sizeof(*da->items));     \
    da->count = 0;                 \
}

/**
 * @brief free memory for `da` and set fields at zero
 * @param da pointer to dynamic array
 * @param dtor pointer to destroy function
 *             with signature `void (type*)`
 */
#define da_free(type) DA_FUNC_NAME(free, type)
#define DA_DECLARE_FREE(type)    \
void da_free(type)(              \
struct DA_STRUCT_NAME(type)* da, \
void (*dtor)(type*))
#define DA_DEFINE_FREE(type)       \
DA_DECLARE_FREE(type) {            \
    if (dtor != NULL)              \
        DA_FOREACH(type, item, da) \
            dtor(item);            \
    free(da->items);               \
    da->items = NULL;              \
    da->count = 0;                 \
    da->capacity = 0;              \
}

/**
 * @brief destroy object at `index` and
 * shift other items in [`index`+1, `da.count`)
 * @param da pointer to dynamic array
 * @param index valid index in range [0, `da.count`)
 * @param dtor pointer to destroy function
 *             with signature `void (type*)`
 */
#define da_remove(type) DA_FUNC_NAME(remove, type)
#define DA_DECLARE_REMOVE(type)    \
void da_remove(type)(              \
struct DA_STRUCT_NAME(type)* da,   \
size_t index, void (*dtor)(type*))
#define DA_DEFINE_REMOVE(type)        \
DA_DECLARE_REMOVE(type) {             \
    assert(index < da->count          \
        && "Out of range");           \
    if (dtor != NULL)                 \
        dtor(&da->items[index]);      \
    memmove(&da->items[index],        \
        &da->items[index] + 1,        \
        sizeof(*da->items) *          \
            (da->count - index - 1)); \
    --(da->count);                    \
}

/**
 * @brief destroy objects in range [`i`, `j`)
 * and shift other items in [`j`, `da.count`)
 * @param da pointer to dynamic array
 * @param i begin index in range [`i`, `j`)
 * @param j end index in range [`i`, `j`)
 * @param dtor pointer to destroy function
 *             with signature `void (type*)`
 */
#define da_remove_many(type) DA_FUNC_NAME(remove_many, type)
#define DA_DECLARE_REMOVE_MANY(type) \
void da_remove_many(type)(           \
struct DA_STRUCT_NAME(type)* da,     \
size_t i, size_t j,                  \
void (*dtor)(type*))
#define DA_DEFINE_REMOVE_MANY(type) \
DA_DECLARE_REMOVE_MANY(type) {      \
    assert(i < da->count            \
        && j <= da->count           \
        && "Out of range");         \
    if (dtor != NULL)               \
        DA_FORLOOP(k, i, j)         \
            dtor(&da->items[k]);    \
    memmove(&da->items[i],          \
        &da->items[j],              \
        sizeof(*da->items) *        \
            (da->count - j));       \
    da->count -= j - i;             \
}

/**
 * @brief reserve memory for need `new_cap`
 * @param da pointer to dynamic array
 * @param new_cap new capacity for storage
 */
#define da_reserve(type) DA_FUNC_NAME(reserve, type)
#define DA_DECLARE_RESERVE(type) \
void da_reserve(type)(           \
struct DA_STRUCT_NAME(type)* da, \
size_t new_cap)
#define DA_DEFINE_RESERVE(type)                \
DA_DECLARE_RESERVE(type) {                     \
    if (da->capacity >= new_cap) return;       \
    da->capacity = new_cap;                    \
    da->items = realloc(da->items,             \
        da->capacity * sizeof(*da->items));    \
    assert(da->items != NULL && "Not memory"); \
}

/**
 * @brief reset capacity equal count
 * @param da pointer to dynamic array
 */
#define da_shrink_to_fit(type) DA_FUNC_NAME(shrink_to_fit, type)
#define DA_DECLARE_SHRINK_TO_FIT(type) \
void da_shrink_to_fit(type)(           \
struct DA_STRUCT_NAME(type)* da)
#define DA_DEFINE_SHRINK_TO_FIT(type)          \
DA_DECLARE_SHRINK_TO_FIT(type) {               \
    da->capacity = da->count;                  \
    da->items = realloc(da->items,             \
        da->capacity * sizeof(*da->items));    \
    assert(da->items != NULL && "Not memory"); \
}

#define DA_DECLARE_ALL(type, name) \
DA_DECLARE_STRUCT(type, name) \
DA_DECLARE_APPEND(type);      \
DA_DECLARE_APPEND_MANY(type); \
DA_DECLARE_CLEAR(type);       \
DA_DECLARE_FREE(type);        \
DA_DECLARE_REMOVE(type);      \
DA_DECLARE_REMOVE_MANY(type); \
DA_DECLARE_RESERVE(type);     \
DA_DECLARE_SHRINK_TO_FIT(type);

#define DA_DEFINE_ALL(type, name) \
DA_DEFINE_STRUCT(type, name) \
DA_DEFINE_APPEND(type)       \
DA_DEFINE_APPEND_MANY(type)  \
DA_DEFINE_CLEAR(type)        \
DA_DEFINE_FREE(type)         \
DA_DEFINE_REMOVE(type)       \
DA_DEFINE_REMOVE_MANY(type)  \
DA_DEFINE_RESERVE(type)      \
DA_DEFINE_SHRINK_TO_FIT(type)

#endif // DYNAMIC_ARRAY_H