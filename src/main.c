#include "../include/parameters.h"
#include "../include/curl_operations.h"
#include "../include/read_file.h"
#include "../include/tagger.h"
#include "../include/presentation.h"

// https://get.bandcamp.help/hc/en-us/articles/360007902173-I-heard-you-can-steal-music-on-Bandcamp-What-are-you-doing-about-this

int main(void)
{
	int fail_code = 0;

	welcome();

	int number_websites = 0;
	char** website_links = receive_links(&number_websites);

	_mkdir(TEMP_DIRECTORY);

	for (int i = 0; i < number_websites; i++) {
		fprintf(stdout, "\n");

		fail_code = get_webpage(website_links[i]);
		if (fail_code) {
			fprintf(stderr, "[x] Download failed: Code %d\n", 100 + fail_code);
			continue;
		}

		album_details* album = get_album_details(&fail_code);
		if (fail_code) {
			fprintf(stderr, "[x] Download failed: Code %d\n", 200 + fail_code);
			continue;
		}

		fix_up_fields(album);

		fail_code = get_image(album);
		if (fail_code) {
			fprintf(stderr, "[x] Download failed: Code %d\n", 300 + fail_code);
			for (int i = 0; i < album->song_count; i++)
			{
				free(album->song_titles[i]);
				free(album->song_links[i]);
			}
			free(album->song_titles);
			free(album->song_links);
			free(album);
			continue;
		}

		tagging(album, &fail_code);
		if (fail_code) {
			fprintf(stderr, "[x] Download failed: Code %d\n", 400 + fail_code);
			for (int i = 0; i < album->song_count; i++)
			{
				free(album->song_titles[i]);
				free(album->song_links[i]);
			}
			free(album->song_titles);
			free(album->song_links);
			free(album);
			continue;
		}

		fail_code = get_songs(album);
		if (fail_code) {
			fprintf(stderr, "[x] Download failed: Code %d\n", 500 + fail_code);
			for (int i = 0; i < album->song_count; i++)
			{
				free(album->song_titles[i]);
				free(album->song_links[i]);
			}
			free(album->song_titles);
			free(album->song_links);
			free(album);
			continue;
		}

		for (int i = 0; i < album->song_count; i++)
		{
			free(album->song_titles[i]);
			free(album->song_links[i]);
		}
		free(album->song_titles);
		free(album->song_links);
		free(album);
	}

	curl_global_cleanup();

	for (int i = 0; i < number_websites; i++)
	{
		free(website_links[i]);
	}
	free(website_links);

	remove(WEBPAGE_DUMP);
	remove(IMAGE_DUMP);
	_rmdir(TEMP_DIRECTORY);

	goodbye();

	return 0;
}
