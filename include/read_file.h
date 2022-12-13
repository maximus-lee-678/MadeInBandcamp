#pragma once

#include "parameters.h"

int get_album_details(album_details*);
int get_track_details(album_details*);
u_int verify_validity(FILE*, album_details*);
u_int get_title_fields(FILE*, album_details*);
u_int get_year(FILE*, album_details*);
u_int get_album_art_link(FILE*, album_details*);
u_int get_number_songs(FILE*, album_details*);
u_int get_song_titles(FILE*, album_details*);
u_int get_song_links(FILE*, album_details*);
u_int is_webpage_everything();
link_struct* get_everything(char*);
void fix_up_fields(album_details*);
void ascii_convert(char*, int);
