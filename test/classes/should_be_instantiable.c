#include "../test_helper.h"
#include "setup.h"

int test(struct IridiumContext * context) {
    setup(context);

    // Using the default approach
    object MyClass = send(CLASS(Class), "new", IR_STRING("MyClass"));
    object myobj = send(MyClass, "new");
    assertEqual(myobj -> class, MyClass);

    // With an explicit superclass
    MyClass = send(CLASS(Class), "new", IR_STRING("MyClass"), CLASS(Object));
    myobj = send(MyClass, "new");
    assertEqual(myobj -> class, MyClass);
    return 0;
}

