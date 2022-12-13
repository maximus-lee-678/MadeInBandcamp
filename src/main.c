#include "../include/parameters.h"
#include "../include/curl_operations.h"
#include "../include/read_file.h"
#include "../include/tagger.h"
#include "../include/presentation.h"

u_int track_operation(char*);
u_int album_operation(char*);
u_int everything_operation(char*);

int main(void)
{

	if (welcome()) {
		fprintf(stderr, "[x] Terminated due to Desktop detection\n");
		goodbye();
		return -2;
	}

	link_struct* entered_links = receive_links();

	if (_mkdir(TEMP_DIRECTORY) && errno != EEXIST) {
		fprintf(stderr, "[x] File creation issue: Code %d\n", errno);
		goodbye();
		return -3;
	}

	for (int i = 0; i < entered_links->link_count; i++) {
		u_int status;
		if (strstr(entered_links->links[i], "/album/"))
			status = album_operation(entered_links->links[i]);
		else if (strstr(entered_links->links[i], "/track/"))
			status = track_operation(entered_links->links[i]);
		else
			status = everything_operation(entered_links->links[i]);

		fprintf(stdout, DIVIDER_25);
		if(!status)
			fprintf(stdout, "[i] Download complete for <%s>\n", entered_links->links[i]);
		else
			fprintf(stdout, "[!] Download finished with errors for <%s>\n", entered_links->links[i]);
	}

	curl_global_cleanup();

	for (int i = 0; i < entered_links->malloc_count; i++)
	{
		free(entered_links->links[i]);
	}
	free(entered_links);

	remove(WEBPAGE_DUMP);
	remove(IMAGE_DUMP);
	_rmdir(TEMP_DIRECTORY);

	goodbye();

	return 0;
}

u_int album_operation(char* website_link) {
	u_int fail_code = 0;

	fail_code = get_webpage(website_link);
	if (fail_code) {
		fail_code += 100;
		fprintf(stderr, "[x] Download failed: Code %d\n", fail_code);
		return fail_code;
	}

	album_details* album = (album_details*)malloc(sizeof(album_details));
	mallocChecker(album);

	strcpy(album->operation_type, "album");

	fail_code = get_album_details(album);
	if (fail_code) {
		fail_code += 200;
		fprintf(stderr, "[x] Download failed: Code %d\n", fail_code);
		return fail_code;
	}

	fix_up_fields(album);

	fail_code = get_image(album);
	if (fail_code) {
		fail_code += 300;
		fprintf(stderr, "[x] Download failed: Code %d\n", fail_code);

		for (int i = 0; i < album->song_count; i++)
		{
			free(album->song_titles[i]);
			free(album->song_links[i]);
		}
		free(album->song_titles);
		free(album->song_links);
		free(album);

		return fail_code;
	}

	tagging(album, &fail_code);
	if (fail_code) {
		fail_code += 400;
		fprintf(stderr, "[x] Download failed: Code %d\n", fail_code);

		for (int i = 0; i < album->song_count; i++)
		{
			free(album->song_titles[i]);
			free(album->song_links[i]);
		}
		free(album->song_titles);
		free(album->song_links);
		free(album);

		return fail_code;
	}

	fail_code = get_songs(album);
	if (fail_code) {
		fail_code += 500;
		fprintf(stderr, "[x] Download failed: Code %d\n", 500 + fail_code);

		for (int i = 0; i < album->song_count; i++)
		{
			free(album->song_titles[i]);
			free(album->song_links[i]);
		}
		free(album->song_titles);
		free(album->song_links);
		free(album);

		return fail_code;
	}

	for (int i = 0; i < album->song_count; i++)
	{
		free(album->song_titles[i]);
		free(album->song_links[i]);
	}
	free(album->song_titles);
	free(album->song_links);
	free(album);

	return fail_code;
}

u_int track_operation(char* website_link) {
	u_int fail_code = 0;

	fail_code = get_webpage(website_link);
	if (fail_code) {
		fail_code += 100;
		fprintf(stderr, "[x] Download failed: Code %d\n", fail_code);
		return fail_code;
	}

	album_details* album = (album_details*)malloc(sizeof(album_details));
	mallocChecker(album);

	strcpy(album->operation_type, "track");

	fail_code = get_track_details(album);
	if (fail_code) {
		fail_code += 200;
		fprintf(stderr, "[x] Download failed: Code %d\n", fail_code);
		return fail_code;
	}

	fix_up_fields(album);

	fail_code = get_image(album);
	if (fail_code) {
		fail_code += 300;
		fprintf(stderr, "[x] Download failed: Code %d\n", fail_code);

		free(album->song_titles[0]);
		free(album->song_links[0]);

		free(album->song_titles);
		free(album->song_links);
		free(album);

		return fail_code;
	}

	tagging(album, &fail_code);
	if (fail_code) {
		fail_code += 400;
		fprintf(stderr, "[x] Download failed: Code %d\n", fail_code);

		free(album->song_titles[0]);
		free(album->song_links[0]);

		free(album->song_titles);
		free(album->song_links);
		free(album);

		return fail_code;
	}

	fail_code = get_songs(album);
	if (fail_code) {
		fail_code += 500;
		fprintf(stderr, "[x] Download failed: Code %d\n", fail_code);

		free(album->song_titles[0]);
		free(album->song_links[0]);

		free(album->song_titles);
		free(album->song_links);
		free(album);

		return fail_code;
	}

	free(album->song_titles[0]);
	free(album->song_links[0]);

	free(album->song_titles);
	free(album->song_links);
	free(album);

	return fail_code;
}

u_int everything_operation(char* website_link) {
	u_int fail_code = 0;

	fail_code = get_webpage(website_link);
	if (fail_code) {
		fail_code += 100;
		fprintf(stderr, "[x] Download failed: Code %d\n", fail_code);
		return fail_code;
	}
	link_struct* all_links = get_everything(website_link);
	if (all_links == NULL) {
		fail_code += 600;
		fprintf(stderr, "[x] Download failed: Code %d\n", fail_code);
		return fail_code;
	}

	fprintf(stdout, "[i] Downloading %d albums/tracks from <%s>\n[i] You get comfortable kid, we may be here a while.\n", all_links->link_count, website_link);

	for (int i = 0; i < all_links->link_count; i++) {
		if (strstr(all_links->links[i], "/album/"))
			album_operation(all_links->links[i]);
		else if (strstr(all_links->links[i], "/track/"))
			track_operation(all_links->links[i]);

		fprintf(stdout, DIVIDER_25);
		fprintf(stdout, "[i] Download complete for <%s> (%d/%d)\n", all_links->links[i], i + 1, all_links->link_count);
	}

	// Cheeky line extension
	fprintf(stdout, "%.25s", DIVIDER_25);

	for (int i = 0; i < all_links->malloc_count; i++)
	{
		free(all_links->links[i]);
	}
	free(all_links);

	return fail_code;
}