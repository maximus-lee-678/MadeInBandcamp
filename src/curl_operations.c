#include "../include/curl_operations.h"

int get_webpage(char* webpage_link) {
	CURL* curl = curl_easy_init();

	if (!curl) {
		fprintf(stderr, "[!] Failed Initializing Curl. (get_webpage)\n");
		curl_easy_cleanup(curl);
		return 1;
	}

	CURLcode result;
	curl_easy_setopt(curl, CURLOPT_URL, webpage_link);	// Set webpage URL

	FILE* initial_file;								// Create file
	initial_file = fopen(WEBPAGE_DUMP, "w");
	if (initial_file == NULL) {
		fprintf(stderr, "[!] Could not create file %s.\n", WEBPAGE_DUMP);
		curl_easy_cleanup(curl);
		return 2;
	}
	else {
		fclose(initial_file);
	}

	fprintf(stdout, "[i] Downloading webpage from <%s>\n", webpage_link);

	int bytes_downloaded = 0;
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, store_webpage);			// Callback function for storing webpage
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&bytes_downloaded);	// Callback argument
	result = curl_easy_perform(curl);

	if (result != CURLE_OK) {
		fprintf(stderr, "[!] Could Not Fetch Webpage %s\n[!] Error : %s\n", webpage_link, curl_easy_strerror(result));
		curl_easy_cleanup(curl);
		return 3;
	}
	else {
		fprintf(stdout, "[>] Webpage Size: %.2f KB\n", (float)bytes_downloaded / 1024);
	}

	curl_easy_cleanup(curl);

	return 0;
}

int store_webpage(char* buffer, int itemsize, int n_items, void* userp) {
	int bytes = itemsize * n_items;

	// Increments total size
	*(int*)userp += bytes;

	FILE* create_file;

	create_file = fopen(WEBPAGE_DUMP, "a");
	if (create_file == NULL) {
		fprintf(stderr, "[!] Could not open file %s.\n", WEBPAGE_DUMP);
		return -1;
	}
	else {
		for (int i = 0; i < bytes; i++) {
			fprintf(create_file, "%c", buffer[i]);
		}
		fclose(create_file);
	}

	return bytes;
}

int get_image(album_details* album) {
	CURL* curl = curl_easy_init();

	if (!curl) {
		fprintf(stderr, "[!] Failed Initializing Curl. (get_image)\n");
		curl_easy_cleanup(curl);
		return 1;
	}

	CURLcode result;
	curl_easy_setopt(curl, CURLOPT_URL, album->album_art_link);	// Set webpage URL

	FILE* initial_file;								// Create file
	initial_file = fopen(IMAGE_DUMP, "wb");
	if (initial_file == NULL) {
		fprintf(stderr, "[!] Could not create file %s.\n", IMAGE_DUMP);
		curl_easy_cleanup(curl);
		return 2;
	}
	else {
		fclose(initial_file);
	}

	fprintf(stdout, "[i] Downloading image from <%s>\n", album->album_art_link);

	int bytes_downloaded = 0;
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, store_image);			// Callback function for storing webpage
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&bytes_downloaded);	// Callback argument
	result = curl_easy_perform(curl);

	if (result != CURLE_OK) {
		fprintf(stderr, "[!] Could Not Fetch Image %s\n[!] Error : %s\n", album->album_art_link, curl_easy_strerror(result));
		curl_easy_cleanup(curl);
		return 3;
	}
	else {
		fprintf(stdout, "[>] Image Size: %.2f KB\n", (float)bytes_downloaded / 1024);
	}

	curl_easy_cleanup(curl);

	return 0;
}

int store_image(char* buffer, int itemsize, int n_items, void* userp) {
	int bytes = itemsize * n_items;

	// Increments total size
	*(int*)userp += bytes;

	FILE* create_file;

	create_file = fopen(IMAGE_DUMP, "ab");
	if (create_file == NULL) {
		fprintf(stderr, "[!] Could not open file %s.\n", IMAGE_DUMP);
		return -1;
	}
	else {
		for (int i = 0; i < bytes; i++) {
			fprintf(create_file, "%c", buffer[i]);
		}
		fclose(create_file);
	}

	return bytes;
}

int get_songs(album_details* album) {
	CURLM* m_curl = curl_multi_init();

	if (!m_curl) {
		fprintf(stderr, "[!] Failed Initializing Curl. (get_songs)\n");
		return 1;
	}

	fprintf(stdout, "[i] Downloading %s by %s (%d songs)\n", album->album, album->artist, album->song_count);

	CURLMsg* msg;
	unsigned int transfers = 0;
	int msgs_left = -1;
	int left = 0;

	curl_multi_setopt(m_curl, CURLMOPT_MAXCONNECTS, (long)MAX_PARALLEL);	// Set maximum parallel count

	for (transfers = 0; transfers < MAX_PARALLEL && transfers < (u_int)album->song_count; transfers++)
		song_download_transfer(m_curl, transfers, album, &left);

	do {
		int still_alive = 1;
		curl_multi_perform(m_curl, &still_alive);

		while ((msg = curl_multi_info_read(m_curl, &msgs_left))) {
			if (msg->msg == CURLMSG_DONE) {
				char* url;
				CURL* e = msg->easy_handle;
				curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &url);
				if (msg->data.result) {
					fprintf(stderr, "[!] Could Not Fetch song <%s>!\n[!] Error : Code %d - %s\n", album->song_titles[url_to_song_name(album, url)], 
						msg->data.result, curl_easy_strerror(msg->data.result));
					curl_multi_cleanup(m_curl);
					return 2;
				}

				// msg->data.result for exit code (0), curl_easy_strerror(msg->data.result) for exit code meaning (no error)
				fprintf(stdout, "[>] Song downloaded: <%s>\n", album->song_titles[url_to_song_name(album, url)]);
				curl_multi_remove_handle(m_curl, e);
				curl_easy_cleanup(e);
				left--;
			}
			else {
				fprintf(stderr, "[!] Could Not Fetch songs!\n[!] CURL Error Code: %d\n", msg->msg);
				curl_multi_cleanup(m_curl);
				return 3;
			}

			if (transfers < (u_int) album->song_count)
				song_download_transfer(m_curl, transfers++, album, &left);
		}
		if (left)
			curl_multi_wait(m_curl, NULL, 0, 1000, NULL);

	} while (left);

	curl_multi_cleanup(m_curl);

	return 0;
}

void song_download_transfer(CURLM* m_curl, int transfers, album_details* album, int* left)
{
	CURL* sdt_curl = curl_easy_init();
	curl_easy_setopt(sdt_curl, CURLOPT_WRITEFUNCTION, song_write);
	curl_easy_setopt(sdt_curl, CURLOPT_WRITEDATA, (void*)album->file_names[transfers]);
	curl_easy_setopt(sdt_curl, CURLOPT_URL, album->song_links[transfers]);
	curl_easy_setopt(sdt_curl, CURLOPT_PRIVATE, album->song_links[transfers]);
	curl_multi_add_handle(m_curl, sdt_curl);
	(*left)++;
}

size_t song_write(char* buffer, int itemsize, int n_items, void* userp)
{
	int bytes = itemsize * n_items;

	char song_title[512] = SONG_DIRECTORY;
	strcat(song_title, (char*)userp);

	FILE* open_file;

	open_file = fopen(song_title, "ab");
	if (open_file == NULL) {
		fprintf(stderr, "[!] Could not open %s.\n", song_title);
		return -1;
	}
	else {
		for (int i = 0; i < bytes; i++) {
			fprintf(open_file, "%c", buffer[i]);
		}
		fclose(open_file);
	}

	return bytes;
}

u_int url_to_song_name(album_details* album, char* url) {
	for (int i = 0; i < album->song_count; i++) {
		if (!strcmp(album->song_links[i], url)) {
			return i;
		}
	}

	return 0;
}