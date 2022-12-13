#include "../include/presentation.h"

u_int welcome() {
	const char* openers[UNIVERSAL_LENGTH] = {
		"This may be beyond your comprehension but I will part with this last remark. Time will accelerate.",
		"I did not obtain this power to end your lives, nor does this ability promise supreme strength.",
		"This power's purpose is to guide the human race towards true happiness.",
		"My new stand, my new ability. I shall give it a name. Made in Heaven!"
	};

	srand(time(NULL));
	int random = rand() % 4;

	fprintf(stdout, DIVIDER_50);
	fprintf(stdout, "[~] %s\n", openers[random]);

	char pwd[FILENAME_MAX];
	_getcwd(pwd, FILENAME_MAX);
	if (strstr(pwd, "\\Desktop\\")) {
		fprintf(stderr, "[!] This program does not function on the Desktop directory, I've not a clue why. Move it somewhere else!\n");
		return 1;
	}
	else {
		fprintf(stdout, "[~] Accepted Inputs:\n");
		fprintf(stdout, "[~] Album link (https://<?>.bandcamp.com/album/<?>)\n");
		fprintf(stdout, "[~] Track link (https://<?>.bandcamp.com/track/<?>)\n");
		fprintf(stdout, "[~] Composer link (https://<?>.bandcamp.com)\n");
		fprintf(stdout, "[~] Enter \"%s\" to begin downloads\n", EXIT_STRING);
		fprintf(stdout, DIVIDER_50);
		fprintf(stdout, "[i] Accepting Inputs\n");
		return 0;
	}
}

link_struct* receive_links() {
	link_struct* entered_links = (link_struct*)malloc(sizeof(link_struct));
	mallocChecker(entered_links);
	*entered_links = (link_struct){ .link_count = 0, .malloc_count = 4 };
	entered_links->links = (char**)malloc(entered_links->malloc_count * sizeof(char*));
	mallocChecker(entered_links->links);
	for (int i = 0; i < entered_links->malloc_count; i++)
	{
		entered_links->links[i] = (char*)malloc(UNIVERSAL_LENGTH * sizeof(char));
		mallocChecker(entered_links->links[i]);
	}

	char buffer[UNIVERSAL_LENGTH];				// buffer to hold strings

	while (1) {
		fprintf(stdout, "[<] ");

		fgets(buffer, 512, stdin);

		if (strchr(buffer, '\n')) {			// check exist newline
			*strchr(buffer, '\n') = '\0';	// replace newline with null terminator
		}
		else {
			scanf("%*[^\n]");	// clear up to newline
			scanf("%*c");		// clear newline
		}

		// break if matches exit string
		if (strcmp(buffer, EXIT_STRING) == 0)
			break;

		// invalid link
		if (!strstr(buffer, "bandcamp.com")) {
			fprintf(stderr, "[!] Invalid Link!\n");
			continue;
		}

		// strip any trailing slash
		if (buffer[strlen(buffer) - 1] == '/') {
			buffer[strlen(buffer) - 1] = '\0';
		}

		// array no space, double it
		if (entered_links->link_count + 1 > entered_links->malloc_count) {
			entered_links->links = realloc(entered_links->links, sizeof(char*) * (2 * entered_links->malloc_count));
			mallocChecker(entered_links->links);
			for (int i = entered_links->malloc_count; i < 2 * entered_links->malloc_count; i++)
			{
				entered_links->links[i] = (char*)malloc(UNIVERSAL_LENGTH * sizeof(char));
				mallocChecker(entered_links->links[i]);
			}

			entered_links->malloc_count *= 2;
		}

		strcpy(entered_links->links[(entered_links->link_count)++], buffer);
	}

	return entered_links;
}

void goodbye() {
	fprintf(stdout, DIVIDER_50);
	fprintf(stdout, "[~] Press any key to close this window . . .");
	_getch();
}