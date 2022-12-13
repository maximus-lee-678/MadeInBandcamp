#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <direct.h>
#include <conio.h>
#include <errno.h>

#define DIVIDER_25 "=========================\n"
#define DIVIDER_50 "==================================================\n"

#define WEBPAGE_DUMP "./temp/webpage.txt"
#define IMAGE_DUMP "./temp/cover.jpg"
#define TEMP_DIRECTORY "./temp/"
#define SONG_DIRECTORY "./songs/"
#define SONG_EXTENSION ".mp3"

#define UNIVERSAL_LENGTH 512

typedef struct {
	char operation_type[6];
	char artist[UNIVERSAL_LENGTH];
	char album_artist[UNIVERSAL_LENGTH];
	char album[UNIVERSAL_LENGTH];
	char year[5];
	char album_art_link[UNIVERSAL_LENGTH];
	int song_count;
	char** song_titles;
	char** song_links;
	char** file_names;
}album_details;

typedef struct {
	int link_count;
	int malloc_count;
	char** links;
}link_struct;

void mallocChecker(void*);
