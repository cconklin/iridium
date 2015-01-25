void * _l, * _r;
#define assert(s) if (! (s)) { printf("Assertion Failed (line %d): assert(%s)\n", __LINE__, #s); exit(1); }
#define assertEqual(l, r) if ((_l = l) != (_r = r)) { printf("Assertion Failed (line %d): assertEqual(%s, %s)\n    %llu != %llu (%p != %p)\n", __LINE__, #l, #r, (unsigned long long int) _l, (unsigned long long int) _r, _l, _r); exit(1); }
#define assertNotEqual(l, r) if ((_l = l) == (_r = r)) { printf("Assertion Failed (line %d): assertNotEqual(%s, %s)\n    %llu != %llu (%p == %p)\nLine %d\n", __LINE__, #l, #r, (unsigned long long int) _l, (unsigned long long int) _r, _l, _r); exit(1); }
#define assertNotReaches() { printf("Assertion Failed (line %d): Arrived at unreachable point\n", __LINE__); exit(1); }