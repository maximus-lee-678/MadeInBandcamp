#include "../include/read_file.h"

int get_album_details(album_details* album) {
	int fail_code = 0;
	FILE* fp = fopen(WEBPAGE_DUMP, "r");

	fail_code = verify_validity(fp, album);
	if (fail_code) {
		fclose(fp);
		return fail_code;
	}
	fseek(fp, 0, SEEK_SET);	// Reset file pointer
	fail_code = get_title_fields(fp, album);
	if (fail_code) {
		fclose(fp);
		return fail_code;
	}
	fseek(fp, 0, SEEK_SET);
	fail_code = get_year(fp, album);
	if (fail_code) {
		fclose(fp);
		return fail_code;
	}
	fseek(fp, 0, SEEK_SET);
	fail_code = get_album_art_link(fp, album);
	if (fail_code) {
		fclose(fp);
		return fail_code;
	}
	fseek(fp, 0, SEEK_SET);
	fail_code = get_number_songs(fp, album);
	if (fail_code) {
		fclose(fp);
		return fail_code;
	}
	fseek(fp, 0, SEEK_SET);
	fail_code = get_song_titles(fp, album);
	if (fail_code) {
		fclose(fp);
		return fail_code;
	}
	fseek(fp, 0, SEEK_SET);
	fail_code = get_song_links(fp, album);
	if (fail_code) {
		fclose(fp);
		return fail_code;
	}

	fclose(fp);

	// Generate file names
	album->file_names = (char**)malloc(album->song_count * sizeof(char*));
	malloc_checker(album->file_names);
	for (int i = 0; i < album->song_count; i++)
	{
		album->file_names[i] = (char*)malloc(UNIVERSAL_LENGTH * sizeof(char));
		malloc_checker(album->file_names[i]);
	}

	for (int i = 0; i < album->song_count; i++) {
		sprintf(album->file_names[i], "%s - %s - %02d %s%s", album->artist, album->album, i + 1, album->song_titles[i], SONG_EXTENSION);
	}

	return 0;
}

int get_track_details(album_details* album) {
	int fail_code = 0;
	FILE* fp = fopen(WEBPAGE_DUMP, "r");

	fail_code = verify_validity(fp, album);
	if (fail_code) {
		fclose(fp);
		return fail_code;
	}
	fseek(fp, 0, SEEK_SET);	// Reset file pointer
	fail_code = get_title_fields(fp, album);
	if (fail_code) {
		fclose(fp);
		return fail_code;
	}
	fseek(fp, 0, SEEK_SET);
	fail_code = get_year(fp, album);
	if (fail_code) {
		fclose(fp);
		return fail_code;
	}
	fseek(fp, 0, SEEK_SET);
	fail_code = get_album_art_link(fp, album);
	if (fail_code) {
		fclose(fp);
		return fail_code;
	}
	fseek(fp, 0, SEEK_SET);
	fail_code = get_song_links(fp, album);
	if (fail_code) {
		fclose(fp);
		return fail_code;
	}

	fclose(fp);

	// Generate file names
	album->file_names = (char**)malloc(album->song_count * sizeof(char*));
	malloc_checker(album->file_names);
	album->file_names[0] = (char*)malloc(UNIVERSAL_LENGTH * sizeof(char));
	malloc_checker(album->file_names[0]);

	sprintf(album->file_names[0], "%s - %s%s", album->artist, album->song_titles[0], SONG_EXTENSION);

	return 0;
}

// 1. Verify if link leads to album
// If invalid, a <h2> element is present
u_int verify_validity(FILE* fp, album_details* album) {
	char chunk[UNIVERSAL_LENGTH];
	size_t len = sizeof(chunk);		// Store the chunks of text into a line buffer
	char* line = malloc(len);
	malloc_checker(line);
	line[0] = '\0';					// Zeroise the string
	char* examiner = NULL;
	int flag = 0;

	while (fgets(chunk, sizeof(chunk), fp) != NULL) {
		// Resize the line buffer if necessary, x2 each time
		size_t len_used = strlen(line);
		size_t chunk_used = strlen(chunk);

		if (len - len_used < chunk_used) {
			len *= 2;
			line = realloc(line, len);
			malloc_checker(line);
		}

		// Copy the chunk to the end of the line buffer
		strncpy(line + len_used, chunk, len - len_used);
		len_used += chunk_used;

		// Check if line contains '\n', if yes process the line of text
		if (line[len_used - 1] == '\n') {
			examiner = strstr(line, "<h2>");

			if (examiner) {
				flag = 1;
				break;
			}

			// "Empty" the line buffer
			line[0] = '\0';
		}
	}

	if (flag) {
		fprintf(stderr, "[!] Invalid Bandcamp Link!\n");
		free(album);
		return 1;
	}

	free(line);

	return 0;
}

// 2. Get (Album Title / Song Name) and Composer
// Can be found in the <title> element AFTER containing
// In album, first field is Album Title, in track, first field is song name
u_int get_title_fields(FILE* fp, album_details* album) {
	char chunk[UNIVERSAL_LENGTH];
	size_t len = sizeof(chunk);		// Store the chunks of text into a line buffer
	char* line = malloc(len);
	malloc_checker(line);
	line[0] = '\0';					// Zeroise the string
	char* examiner = NULL;
	int flag = 0;

	while (fgets(chunk, sizeof(chunk), fp) != NULL) {
		// Resize the line buffer if necessary, x2 each time
		size_t len_used = strlen(line);
		size_t chunk_used = strlen(chunk);

		if (len - len_used < chunk_used) {
			len *= 2;
			line = realloc(line, len);
			malloc_checker(line);
		}

		// Copy the chunk to the end of the line buffer
		strncpy(line + len_used, chunk, len - len_used);
		len_used += chunk_used;

		// Check if line contains '\n', if yes process the line of text
		if (line[len_used - 1] == '\n') {
			examiner = strstr(line, "<title>");

			if (examiner) {
				flag = 1;
				break;
			}

			// "Empty" the line buffer
			line[0] = '\0';
		}
	}

	if (!flag) {
		fprintf(stderr, "[!] Could Not Locate Album Title and Composer!\n");
		free(album);
		return 2;
	}

	size_t str_length = 0;
	if (!strcmp(album->operation_type, "album")) {
		examiner += strlen("<title>");								// Seek to start of album title (<title>example -> example)
		str_length = (int)(strstr(examiner, "|") - examiner);		// Measure length (example</title> -> measure until before |)
		str_length -= 1;											// String contains " ", -1 to truncate
		strncpy(album->album, examiner, str_length);				// Copy album name
		album->album[str_length] = '\0';							// Provide a terminator

		examiner = strstr(examiner, "|");							// Seek to start of album artist
		examiner += 2;												// Ignore 2 characters of "| "
		if (strstr(examiner, "|") != NULL) {
			str_length = (int)(strstr(examiner, "|") - examiner);	// Measure length (example | unwanted title -> measure until before |)
			str_length -= 1;										// String contains " ", -1 to truncate
		}
		else {
			str_length = (int)(strstr(examiner, "<") - examiner);	// Measure length (example</title> -> measure until before <)
		}
		strncpy(album->artist, examiner, str_length);				// Copy artist
		album->artist[str_length] = '\0';							// Provide a terminator
		strncpy(album->album_artist, examiner, str_length);			// Copy album artist
		album->album_artist[str_length] = '\0';						// Provide a terminator
	}
	else {
		album->song_count = 1;

		album->song_titles = (char**)malloc(album->song_count * sizeof(char*));
		malloc_checker(album->song_titles);
		album->song_titles[0] = (char*)malloc(UNIVERSAL_LENGTH * sizeof(char));
		malloc_checker(album->song_titles[0]);

		examiner += strlen("<title>");								// Seek to start of album title (<title>example -> example)
		str_length = (int)(strstr(examiner, "|") - examiner);		// Measure length (example</title> -> measure until before |)
		str_length -= 1;											// String contains " ", -1 to truncate
		strncpy(album->song_titles[0], examiner, str_length);		// Copy track name
		album->song_titles[0][str_length] = '\0';					// Provide a terminator

		examiner = strstr(examiner, "|");							// Seek to start of album artist
		examiner += 2;												// Ignore 2 characters of "| "
		if (strstr(examiner, "|") != NULL) {
			str_length = (int)(strstr(examiner, "|") - examiner);	// Measure length (example | unwanted title -> measure until before |)
			str_length -= 1;										// String contains " ", -1 to truncate
		}
		else {
			str_length = (int)(strstr(examiner, "<") - examiner);	// Measure length (example</title> -> measure until before <)
		}
		strncpy(album->artist, examiner, str_length);				// Copy artist
		album->artist[str_length] = '\0';							// Provide a terminator
	}

	free(line);

	return 0;
}

// 3. Get release year
// Can be found 1 line below line containing [name="description"] AFTER containing
u_int get_year(FILE* fp, album_details* album) {
	char chunk[UNIVERSAL_LENGTH];
	size_t len = sizeof(chunk);		// Store the chunks of text into a line buffer
	char* line = malloc(len);
	malloc_checker(line);
	line[0] = '\0';					// Zeroise the string
	char* examiner = NULL;
	int flag = 0;

	int lines_to_descend = 1, line_found = 0;

	while (fgets(chunk, sizeof(chunk), fp) != NULL) {
		// Resize the line buffer if necessary, x2 each time
		size_t len_used = strlen(line);
		size_t chunk_used = strlen(chunk);

		if (len - len_used < chunk_used) {
			len *= 2;
			line = realloc(line, len);
			malloc_checker(line);
		}

		// Copy the chunk to the end of the line buffer
		strncpy(line + len_used, chunk, len - len_used);
		len_used += chunk_used;

		// Check if line contains '\n', if yes process the line of text
		if (line[len_used - 1] == '\n') {
			// If line was found previously, countdown to target line. If lines_to_descend == 0, exit loop
			if (!lines_to_descend) {
				examiner = line;
				flag = 1;
				break;
			}

			examiner = strstr(line, "name=\"description\"");

			if (examiner || line_found) {
				line_found = 1;
				lines_to_descend--;
			}

			// "Empty" the line buffer
			line[0] = '\0';
		}
	}

	if (!flag) {
		fprintf(stderr, "[!] Could Not Locate Album Release Year!\n");
		free(album);
		return 3;
	}

	examiner = strstr(examiner, "released");		// Seek to start of release date
	examiner++;
	examiner = strstr(examiner, " ");				// Skip 3 spaces to get to year
	examiner++;
	if (examiner[0] == '\n') {						// Checks for existence of release year
		fprintf(stdout, "[i] No Album Release Year found in page, defaulting to NA!\n");
		strncpy(album->year, "-NA-", 4);		// No year found, specify NA
	}
	else {
		examiner = strstr(examiner, " ");				// Skip 3 spaces to get to year
		examiner++;
		examiner = strstr(examiner, " ");				// Skip 3 spaces to get to year
		examiner++;
		strncpy(album->year, examiner, 4);		// Copy year (4 long)
	}

	album->year[4] = '\0';				// Provide a terminator

	free(line);

	return 0;
}

// 4. Get album art link
// Can be found in line containing [rel="image_src"] AFTER containing
u_int get_album_art_link(FILE* fp, album_details* album) {
	char chunk[UNIVERSAL_LENGTH];
	size_t len = sizeof(chunk);		// Store the chunks of text into a line buffer
	char* line = malloc(len);
	malloc_checker(line);
	line[0] = '\0';					// Zeroise the string
	char* examiner = NULL;
	int flag = 0;

	while (fgets(chunk, sizeof(chunk), fp) != NULL) {
		// Resize the line buffer if necessary, x2 each time
		size_t len_used = strlen(line);
		size_t chunk_used = strlen(chunk);

		if (len - len_used < chunk_used) {
			len *= 2;
			line = realloc(line, len);
			malloc_checker(line);
		}

		// Copy the chunk to the end of the line buffer
		strncpy(line + len_used, chunk, len - len_used);
		len_used += chunk_used;

		// Check if line contains '\n', if yes process the line of text
		if (line[len_used - 1] == '\n') {
			examiner = strstr(line, "rel=\"image_src\"");

			if (examiner) {
				flag = 1;
				break;
			}

			// "Empty" the line buffer
			line[0] = '\0';
		}
	}

	if (!flag) {
		fprintf(stderr, "[!] Could Not Locate Album Art!\n");
		free(album);
		return 4;
	}

	size_t str_length = 0;
	examiner = strstr(examiner, "https");							// Seek to start of link
	str_length = (int)(strstr(examiner, "\"") - examiner);		// Measure length (measure until before ")
	strncpy(album->album_art_link, examiner, str_length);	// Copy artist
	album->album_art_link[str_length] = '\0';			// Provide a terminator

	free(line);

	return 0;
}

// 5. Count number of songs
// Count number of occurrences of [https://t4.bcbits.com]
u_int get_number_songs(FILE* fp, album_details* album) {
	char chunk[UNIVERSAL_LENGTH];
	size_t len = sizeof(chunk);		// Store the chunks of text into a line buffer
	char* line = malloc(len);
	malloc_checker(line);
	line[0] = '\0';					// Zeroise the string
	char* examiner = NULL;
	int flag = 0;

	size_t song_count = 0;

	while (fgets(chunk, sizeof(chunk), fp) != NULL) {
		// Resize the line buffer if necessary, x2 each time
		size_t len_used = strlen(line);
		size_t chunk_used = strlen(chunk);

		if (len - len_used < chunk_used) {
			len *= 2;
			line = realloc(line, len);
			malloc_checker(line);
		}

		// Copy the chunk to the end of the line buffer
		strncpy(line + len_used, chunk, len - len_used);
		len_used += chunk_used;

		// Check if line contains '\n', if yes process the line of text
		if (line[len_used - 1] == '\n') {
			// Search for indicator that line contains song links
			examiner = strstr(line, "data-site");

			// Line found, move to processing
			if (examiner) {
				flag = 1;
				break;
			}

			// "Empty" the line buffer
			line[0] = '\0';
		}
	}

	if (!flag) {
		fprintf(stderr, "[!] Could Not Locate Any Songs!\n");
		free(album);
		return 5;
	}

	while (1) {
		examiner = strstr(examiner, "https://t4.bcbits.com");			// Seek to start of a link
		if (examiner) {
			song_count++;
			examiner++;
		}
		else {
			break;
		}
	}

	album->song_count = song_count;

	free(line);

	return 0;
}

// 6. Get song titles
// When a line with "1." is found, store total number of lines worth of songs
u_int get_song_titles(FILE* fp, album_details* album) {
	char chunk[UNIVERSAL_LENGTH];
	size_t len = sizeof(chunk);		// Store the chunks of text into a line buffer
	char* line = malloc(len);
	malloc_checker(line);
	line[0] = '\0';					// Zeroise the string
	char* examiner = NULL;

	album->song_titles = (char**)malloc(album->song_count * sizeof(char*));
	malloc_checker(album->song_titles);
	for (int i = 0; i < album->song_count; i++)
	{
		album->song_titles[i] = (char*)malloc(UNIVERSAL_LENGTH * sizeof(char));
		malloc_checker(album->song_titles[i]);
	}

	char song_no[8] = "1.";
	size_t song_counter = 0;
	size_t str_length = 0;
	while (fgets(chunk, sizeof(chunk), fp) != NULL) {
		// Copy the chunk to the end of the line buffer
		size_t len_used = strlen(line);
		size_t chunk_used = strlen(chunk);

		if (len - len_used < chunk_used) {
			len *= 2;
			line = realloc(line, len);
			malloc_checker(line);
		}

		// Copy the chunk to the end of the line buffer
		strncpy(line + len_used, chunk, len - len_used);
		len_used += chunk_used;

		// Check if line contains '\n', if yes process the line of text
		if (line[len_used - 1] == '\n') {
			// Search for track numbers in line
			examiner = strstr(line, song_no);

			// Track number found
			if (examiner && song_counter < album->song_count) {
				examiner = strstr(examiner, " ");									// Locate spacing, move to 1 after
				examiner++;
				str_length = (int)(strstr(examiner, "\n") - examiner);
				strncpy(album->song_titles[song_counter], examiner, str_length);	// Copy artist
				album->song_titles[song_counter][str_length] = '\0';				// Provide a terminator
				song_counter++;
				snprintf(song_no, 8, "%d.", song_counter + 1);						// Update track number string
			}
			else if (song_counter != 0) {
				// "Empty" the line buffer
				line[0] = '\0';
				break;
			}

			// "Empty" the line buffer
			line[0] = '\0';
		}
	}

	if (song_counter != album->song_count) {
		fprintf(stderr, "[!] Could Not Locate All Song Names!\n");
		for (int i = 0; i < album->song_count; i++)
		{
			free(album->song_titles[i]);
		}
		free(album->song_titles);
		free(album);
		return 6;
	}

	free(line);

	return 0;
}

// 7. Get song links
// Can be found in line containing [rel="image_src"] EITHER BEFORE OR AFTER field
u_int get_song_links(FILE* fp, album_details* album) {
	char chunk[UNIVERSAL_LENGTH];
	size_t len = sizeof(chunk);		// Store the chunks of text into a line buffer
	char* line = malloc(len);
	malloc_checker(line);
	line[0] = '\0';					// Zeroise the string
	char* examiner = NULL;
	int flag = 0;

	while (fgets(chunk, sizeof(chunk), fp) != NULL) {
		// Resize the line buffer if necessary, x2 each time
		size_t len_used = strlen(line);
		size_t chunk_used = strlen(chunk);

		if (len - len_used < chunk_used) {
			len *= 2;
			line = realloc(line, len);
			malloc_checker(line);
		}

		// Copy the chunk to the end of the line buffer
		strncpy(line + len_used, chunk, len - len_used);
		len_used += chunk_used;

		// Check if line contains '\n', if yes process the line of text
		if (line[len_used - 1] == '\n') {
			// Search for indicator that line contains song links
			examiner = strstr(line, "data-site");

			// Line found, move to processing
			if (examiner) {
				flag = 1;
				break;
			}

			// "Empty" the line buffer
			line[0] = '\0';
		}
	}

	if (!flag) {
		fprintf(stderr, "[!] Could Not Locate All Song Links!\n");
		for (int i = 0; i < album->song_count; i++)
		{
			free(album->song_titles[i]);
		}
		free(album->song_titles);
		free(album);
		return 7;
	}

	size_t str_length = 0;
	examiner = line;
	album->song_links = (char**)malloc(album->song_count * sizeof(char*));
	malloc_checker(album->song_links);
	for (int i = 0; i < album->song_count; i++)
	{
		album->song_links[i] = (char*)malloc(UNIVERSAL_LENGTH * sizeof(char));
		malloc_checker(album->song_links[i]);

		examiner = strstr(examiner, "https://t4.bcbits.com");			// Seek to start of a link
		if (!examiner) {												// If NULL is returned, it usually means a single track's link could not be found.
			fprintf(stderr, "[!] Single track is paywalled!\n");
			for (int i = 0; i < album->song_count; i++)
			{
				free(album->song_titles[i]);
			}
			free(album->song_titles);
			free(album);
			return 8;															// This occurs naturally if the song is paywalled.
		}

		str_length = (int)(strstr(examiner, ";}") - examiner);		// Measure length of link
		strncpy(album->song_links[i], examiner, str_length);	// Copy link
		album->song_links[i][str_length] = '\0';			// Provide a terminator
		examiner++;												// Move forward once to get next string in next loop
	}

	free(line);

	return 0;
}

// Removes strange formatting, converts numbers to ASCII, sanitises file names.
void fix_up_fields(album_details* album) {
	char* to_strip = "amp;";

	if (!strcmp(album->operation_type, "album")) {
		// Strip amp;
		str_replace(album->album, to_strip, "");
		str_replace(album->album_artist, to_strip, "");
		str_replace(album->artist, to_strip, "");
		for (int i = 0; i < album->song_count; i++) {
			str_replace(album->song_titles[i], to_strip, "");
			str_replace(album->file_names[i], to_strip, "");
		}

		// Convert ASCII codes to symbols
		ascii_convert(album->album, 0);
		ascii_convert(album->album_artist, 0);
		ascii_convert(album->artist, 0);
		for (int i = 0; i < album->song_count; i++) {
			ascii_convert(album->song_titles[i], 0);
			ascii_convert(album->file_names[i], 1);
		}
	}
	else {
		// Strip amp;
		str_replace(album->artist, to_strip, "");
		str_replace(album->song_titles[0], to_strip, "");
		str_replace(album->file_names[0], to_strip, "");

		// Convert ASCII codes to symbols
		ascii_convert(album->artist, 0);
		ascii_convert(album->song_titles[0], 0);
		ascii_convert(album->file_names[0], 1);
	}

}

// Sometimes, main pages have an album on them instead of all music.
// This can be determined by looking for [<title>Music |], if this field exists, the page is correct.
// If not, need to go to <link>/music .
u_int is_webpage_everything() {
	FILE* fp = fopen(WEBPAGE_DUMP, "r");

	char chunk[UNIVERSAL_LENGTH];
	size_t len = sizeof(chunk);		// Store the chunks of text into a line buffer
	char* line = malloc(len);
	malloc_checker(line);
	line[0] = '\0';					// Zeroise the string
	char* examiner = NULL;
	int flag = 0;

	while (fgets(chunk, sizeof(chunk), fp) != NULL) {
		// Resize the line buffer if necessary, x2 each time
		size_t len_used = strlen(line);
		size_t chunk_used = strlen(chunk);

		if (len - len_used < chunk_used) {
			len *= 2;
			line = realloc(line, len);
			malloc_checker(line);
		}

		// Copy the chunk to the end of the line buffer
		strncpy(line + len_used, chunk, len - len_used);
		len_used += chunk_used;

		// Check if line contains '\n', if yes process the line of text
		if (line[len_used - 1] == '\n') {
			examiner = strstr(line, "<title>Music |");

			if (examiner) {
				flag = 1;
				break;
			}

			// "Empty" the line buffer
			line[0] = '\0';
		}
	}

	free(line);
	fclose(fp);

	return flag;
}

// Reads a provided artist link to get all published work links
link_struct* get_everything(char* website_link) {
	link_struct* all_links = (link_struct*)malloc(sizeof(link_struct));
	malloc_checker(all_links);
	*all_links = (link_struct){ .link_count = 0, .malloc_count = 4 };
	all_links->links = (char**)malloc(all_links->malloc_count * sizeof(char*));
	malloc_checker(all_links->links);
	for (int i = 0; i < all_links->malloc_count; i++)
	{
		all_links->links[i] = (char*)malloc(UNIVERSAL_LENGTH * sizeof(char));
		malloc_checker(all_links->links[i]);
	}

	FILE* fp = fopen(WEBPAGE_DUMP, "r");

	char chunk[UNIVERSAL_LENGTH];
	size_t len = sizeof(chunk);		// Store the chunks of text into a line buffer
	char* line = malloc(len);
	malloc_checker(line);
	line[0] = '\0';					// Zeroise the string
	char* examiner = NULL;

	size_t str_length = 0;
	while (fgets(chunk, sizeof(chunk), fp) != NULL) {
		// Resize the line buffer if necessary, x2 each time
		size_t len_used = strlen(line);
		size_t chunk_used = strlen(chunk);

		if (len - len_used < chunk_used) {
			len *= 2;
			line = realloc(line, len);
			malloc_checker(line);
		}

		// Copy the chunk to the end of the line buffer
		strncpy(line + len_used, chunk, len - len_used);
		len_used += chunk_used;

		// Check if line contains '\n', if yes process the line of text
		if (line[len_used - 1] == '\n') {
			examiner = strstr(line, "/track/");

			if (examiner) {

				strcpy(all_links->links[all_links->link_count], website_link);
				str_length = (int)(strstr(examiner, "\"") - examiner);
				strncat(all_links->links[all_links->link_count], examiner, str_length);

				(all_links->link_count)++;
			}

			examiner = strstr(line, "/album/");

			if (examiner) {

				strcpy(all_links->links[all_links->link_count], website_link);
				str_length = (int)(strstr(examiner, "\"") - examiner);
				strncat(all_links->links[all_links->link_count], examiner, str_length);

				(all_links->link_count)++;
			}

			// "Empty" the line buffer
			line[0] = '\0';
		}

		// Link count is always -1 of malloc count (1st +1), if the next insertion is overrunning (2nd +1) expand 
		if (all_links->link_count + 1 + 1 > all_links->malloc_count) {
			all_links->links = realloc(all_links->links, sizeof(char*) * (2 * all_links->malloc_count));
			malloc_checker(all_links->links);
			for (int i = all_links->malloc_count; i < 2 * all_links->malloc_count; i++)
			{
				all_links->links[i] = (char*)malloc(UNIVERSAL_LENGTH * sizeof(char));
				malloc_checker(all_links->links[i]);
			}

			all_links->malloc_count *= 2;
		}
	}

	// If artist does not exist, Bandcamp returns a small page containing:
	// "You are being redirected, please follow <a href="https://bandcamp.com/signup?new_domain=[domain]">this link to: https://bandcamp.com/signup?new_domain=[domain]</a>!	0"
	// This will result in no songs found, meaning invalid artist
	if (!all_links->link_count) {
		fprintf(stderr, "[!] Invalid Bandcamp Artist Link!\n");
		return NULL;
	}

	free(line);
	fclose(fp);

	return all_links;
}

// Special characters are sometimes converted to &#<number>;, convert these to ASCII.
// Also swaps out illegal characters for windows filenames.
void ascii_convert(char* target, int is_file_name) {
	char buffer[UNIVERSAL_LENGTH] = { 0 };
	char* insert_point = &buffer[0];
	char* tmp = target;
	int ascii_length = 0;
	char temp_buffer[8] = { 0 };
	char ascii_value;

	while (1) {
		char* examiner = strstr(tmp, "&#");

		// walked past last occurrence of needle; copy remaining part
		if (examiner == NULL) {
			strcpy(insert_point, tmp);
			break;
		}

		// copy part before needle
		memcpy(insert_point, tmp, examiner - tmp);
		insert_point += examiner - tmp;

		// measure length of &# string, total length of &# (needs +1 as strstr seeks to 1 before the end) - start of string
		ascii_length = (int)(strstr(examiner, ";") + 1 - examiner);

		// convert ascii to character
		strncpy(temp_buffer, examiner + strlen("&#"), ascii_length - strlen("&#;"));
		ascii_value = (char)strtol(temp_buffer, NULL, 10);

		snprintf(temp_buffer, 2, "%c", ascii_value);

		// copy replacement string, 1 character long
		memcpy(insert_point, temp_buffer, 1);
		insert_point += 1;


		// adjust pointers, move on
		tmp = examiner + ascii_length;
	}

	// These characters cannot be present in filenames in Windows
	if (is_file_name) {
		str_replace(buffer, "\\", "I");
		str_replace(buffer, "/", "I");
		str_replace(buffer, ":", ";");
		str_replace(buffer, "*", "x");
		str_replace(buffer, "?", "¿");
		str_replace(buffer, "\"", "I");
		str_replace(buffer, "<", "(");
		str_replace(buffer, ">", ")");
		str_replace(buffer, "|", "I");
	}

	// write altered string back to target
	strcpy(target, buffer);
}