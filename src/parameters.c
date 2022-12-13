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


void str_replace(char* target, const char* needle, const char* replacement)
{
	char buffer[UNIVERSAL_LENGTH] = { 0 };
	char* insert_point = &buffer[0];
	const char* tmp = target;
	size_t needle_len = strlen(needle);
	size_t repl_len = strlen(replacement);

	while (1) {
		const char* p = strstr(tmp, needle);

		// walked past last occurrence of needle; copy remaining part
		if (p == NULL) {
			strcpy(insert_point, tmp);
			break;
		}

		// copy part before needle
		memcpy(insert_point, tmp, p - tmp);
		insert_point += p - tmp;

		// copy replacement string
		memcpy(insert_point, replacement, repl_len);
		insert_point += repl_len;

		// adjust pointers, move on
		tmp = p + needle_len;
	}

	// write altered string back to target
	strcpy(target, buffer);
}
