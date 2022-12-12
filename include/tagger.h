#pragma once

#include "parameters.h"

// ASIC definitions
#define MIME_TYPE "image/jpeg"
#define PICTURE_TYPE 0x03
#define DESCRIPTION "Picture"

void tagging(album_details*, int*);
int compute_total_size(album_details*, int);
void write_tags(album_details*, FILE*, const char*, int);
void get_image_size();
void integer_to_four_char(u_int, unsigned char*, int);
