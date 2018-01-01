#pragma once

// #include <gc.h>
#include <stdlib.h>

// Fewer problems when using calloc than malloc -- not initializing memory somewhere!
#define GC_MALLOC(n) calloc(n, 1)
#define GC_REALLOC(p, n) realloc(p, n)
#define GC_INIT()

