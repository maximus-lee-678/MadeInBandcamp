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

	fprintf(stdout, "[~] %s\n", openers[random]);

	char pwd[FILENAME_MAX];
	_getcwd(pwd, FILENAME_MAX);
	if (strstr(pwd, "\\Desktop\\")) {
		fprintf(stderr, "[!] This program does not function on the Desktop directory, I've not a clue why. Move it somewhere else!\n");
		return 1;
	}
	else {
		fprintf(stdout, "[~] Enter a Bandcamp album link (*** to finish)\n");
		return 0;
	}
}

char** receive_links(int* number_websites) {
	u_int arr_size = 4;
	char buffer[UNIVERSAL_LENGTH];				// buffer to hold strings

	char** website_links = (char**)malloc(arr_size * sizeof(char*));
	mallocChecker(website_links);
	for (int i = 0; i < arr_size; i++)
	{
		website_links[i] = (char*)malloc(UNIVERSAL_LENGTH * sizeof(char));
		mallocChecker(website_links[i]);
	}

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

		// break if "***"
		if (strcmp(buffer, "***") == 0)
			break;

		// invalid link
		if (!strstr(buffer, "bandcamp.com")) {
			fprintf(stderr, "[!] Invalid Link!\n");
			continue;
		}

		// array no space, double it
		if (*number_websites + 1 > arr_size) {
			website_links = realloc(website_links, sizeof(char*) * (2 * arr_size));
			mallocChecker(website_links);
			for (int i = arr_size; i < 2 * arr_size; i++)
			{
				website_links[i] = (char*)malloc(UNIVERSAL_LENGTH * sizeof(char));
				mallocChecker(website_links[i]);
			}

			arr_size *= 2;
		}

		strcpy(website_links[(*number_websites)++], buffer);
	}

	return website_links;
}

void goodbye() {
	fprintf(stdout, "[~] Press any key to close this window . . .");
	_getch();
}