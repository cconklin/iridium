#include <assert.h>
void * _l, * _r;
#define assertEqual(l, r) if ((_l = l) != (_r = r)) { printf("Assertion Failed: assertEqual(%s, %s)\n    %llu != %llu (%p != %p)\n# %s:%d\n", #l, #r, (unsigned long long int) _l, (unsigned long long int) _r, _l, _r, __FILE__, __LINE__); exit(1); }
#define assertNotEqual(l, r) if ((_l = l) == (_r = r)) { printf("Assertion Failed: assertNotEqual(%s, %s)\n    %llu != %llu (%p == %p)\n# %s:%d\n", #l, #r, (unsigned long long int) _l, (unsigned long long int) _r, _l, _r, __FILE__, __LINE__); exit(1); }
#define assertNotReaches() { printf("Assertion Failed: Arrived at unreachable point\n# %s:%d\n", __FILE__, __LINE__); exit(1); }