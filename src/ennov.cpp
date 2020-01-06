#include <stdio.h>

extern "C" void Foo(void)
{
	printf("Hello form the dll is the best hello ever\n");
	printf("Hello form the dll after reloading\n");

	printf("Get the hell out of here\n");
}
