#include "../include/parameters.h"
#include "../include/presentation.h"

// ensure malloc success, eliminate catastrophic failure.
void mallocChecker(void* ptr)
{
	if (ptr == NULL)
	{
		fprintf(stderr, "[x] Unable to allocate memory!\n[x] Program closed with code %d\n", -1);
		goodbye();
		exit(-1);
	}
}