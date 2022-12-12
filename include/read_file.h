#pragma once

#include "parameters.h"

album_details* get_album_details(int*);
u_int verify_validity(FILE*, album_details*);
u_int get_albumtitle_composer(FILE*, album_details*);
u_int get_year(FILE*, album_details*);
u_int get_album_art_link(FILE*, album_details*);
u_int get_number_songs(FILE*, album_details*);
u_int get_song_titles(FILE*, album_details*);
u_int get_song_links(FILE*, album_details*);
void fix_up_fields(album_details*);
void str_replace(char*, const char*, const char*);
void ascii_convert(char*, int);
