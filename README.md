# Dynamic array in C

Implement dynamic array in C. Documentation in header file.

## Example

``` c
#include "dynamic_array.h"
#include <stdio.h>

typedef const char* cstr_t; // otherwise macro-magic cannot work
DA_DEFINE_STRUCT(cstr_t, args_t)
DA_DEFINE_APPEND_MANY(cstr_t)
DA_DEFINE_FREE(cstr_t)

int main(int argc, char** argv) {
    args_t args = {0};
    da_append_many(cstr_t)(&args, (cstr_t*)argv, argc);
    for (size_t i = 0; i < args.count; i++)
        puts(args.items[i]);
    da_free(cstr_t)(&args, NULL);
    return 0;
}
```