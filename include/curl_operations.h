#pragma once

#include "parameters.h"

#define MAX_PARALLEL 10

int get_webpage(char*);
int store_webpage(char*, int, int, void*);
int get_image(album_details*);
int store_image(char*, int, int, void*);
int get_songs(album_details*);
void song_download_transfer(CURLM*, int, album_details*, int*);
size_t song_write(char*, int, int, void*);
u_int url_to_song_name(album_details*, char*);