#include <assert.h>
void * _l, * _r;
#define assertEqual(l, r) if ((_l = l) != (_r = r)) { printf("Assertion Failed: assertEqual(%s, %s)\n    %p != %p\n# %s:%d\n", #l, #r, _l, _r, __FILE__, __LINE__); exit(1); }
#define assertNotEqual(l, r) if ((_l = l) == (_r = r)) { printf("Assertion Failed: assertNotEqual(%s, %s)\n    %p == %p\n# %s:%d\n", #l, #r, _l, _r, __FILE__, __LINE__); exit(1); }