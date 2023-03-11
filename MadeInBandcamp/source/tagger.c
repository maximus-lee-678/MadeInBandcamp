#include "../include/tagger.h"

// https://id3.org/id3v2.3.0
// https://en.wikipedia.org/wiki/ID3

static int image_size = 0;

void tagging(album_details* album, int* fail_code) {
	FILE* create_file;
	char dir_buffer[1024] = { 0 };

	get_image_size();
	if (!image_size) {
		*fail_code = 1;
		return;
	}

	_mkdir(SONG_DIRECTORY);

	for (int i = 0; i < album->song_count; i++) {
		sprintf(dir_buffer, "%s%s", SONG_DIRECTORY, album->file_names[i]);

		create_file = fopen(dir_buffer, "wb");
		if (create_file == NULL) {
			fprintf(stderr, "[!] Could not open file %s.\n", dir_buffer);
			*fail_code = 2;
			return;
		}
		else {
			if (!strcmp(album->operation_type, "album")) {
				write_tags(album, create_file, "ID3", i);
				write_tags(album, create_file, "TIT2", i);
				write_tags(album, create_file, "TPE1", i);
				write_tags(album, create_file, "TPE2", i);
				write_tags(album, create_file, "TALB", i);
				write_tags(album, create_file, "TYER", i);
				write_tags(album, create_file, "TRCK", i);
				write_tags(album, create_file, "APIC", i);
			}
			else {
				write_tags(album, create_file, "ID3", i);
				write_tags(album, create_file, "TIT2", i);
				write_tags(album, create_file, "TPE1", i);
				write_tags(album, create_file, "TYER", i);
				write_tags(album, create_file, "APIC", i);
			}

			fclose(create_file);
		}
	}
}

int compute_total_size(album_details* album, int song_no) {
	int frame_header_size = 11;	// 4 bytes Frame ID, 4 bytes Size, 2 bytes Flags, 1 byte Text encoding
	int number_frames = 0, total_size = 0;
	char track_no_buffer[8] = { 0 };

	if (!strcmp(album->operation_type, "album")) {
		number_frames = 7;

		// Album Artist
		total_size += strlen(album->album_artist);

		// Album
		total_size += strlen(album->album);

		// Track Number
		sprintf(track_no_buffer, "%d", song_no + 1);
		total_size += strlen(track_no_buffer);
	}
	else {
		number_frames = 4;
	}

	// All headers sizes summed
	total_size += frame_header_size * number_frames;

	// Title
	total_size += strlen(album->song_titles[song_no]);

	// Artist
	total_size += strlen(album->artist);

	// Year
	total_size += strlen(album->year);

	// Image
	total_size += strlen(MIME_TYPE) + 1;
	total_size += 1;	// Picture type byte
	total_size += strlen(DESCRIPTION) + 1;
	total_size += image_size;

	return total_size;
}

void write_tags(album_details* album, FILE* create_file, const char* chosen_tag, int song_no) {
	const char id3_header_tag[4] = "ID3";
	const char text_tags[6][5] = { "TIT2", "TPE1", "TPE2", "TALB", "TYER", "TRCK" };
	const char special_tags[1][5] = { "APIC" };
	char track_no_buffer[8] = { 0 };

	// ID3v2 header data
	unsigned char id3_header[3] = { 0x03, 0x00, 0x00 };
	unsigned char id3_header_size[4] = { 0x00, 0x00, 0x00, 0x00 };
	unsigned char id3_flags_and_encoding[3] = { 0x00, 0x00, 0x00 };

	u_int size = 0;

	// ID3v2 header overview
	// file identifier	"ID3"
	// version			$03 00
	// flags			% abc00000 (tldr 0b00000000)
	// size				4 * %0xxxxxxx
	if (!strcmp(chosen_tag, id3_header_tag)) {
		fprintf(create_file, chosen_tag);
		fwrite(id3_header, sizeof(id3_header), 1, create_file); // Essential tag to identify as an ID3 tagged media file.
		integer_to_four_char(compute_total_size(album, song_no), id3_header_size, 1);
		fwrite(id3_header_size, sizeof(id3_header_size), 1, create_file); // Add size to header.

		return;
	}

	// ID3v2 frame overview
	// 
	// <Header>
	// (*Frames that allow different types of text encoding have a text encoding description byte directly after the frame size. 
	// If ISO-8859-1 is used this byte should be $00, if Unicode is used it should be $01.)
	// Frame ID       $xx xx xx xx (four characters)
	// Size           $xx xx xx xx
	// Flags          $xx xx (tldr $00 $00)
	// *Text Encoding $xx ($00 [ISO-8859-1] /$01 [Unicode])
	// 
	// <Frame Content>
	for (int i = 0; i < sizeof(text_tags) / sizeof(text_tags[0]); i++) {
		if (!strcmp(chosen_tag, text_tags[i])) {
			// Tag Header
			fprintf(create_file, chosen_tag);
			switch (i) {	// Why does switch not work with strings?
			case 0:
				size = strlen(album->song_titles[song_no]);
				break;
			case 1:
				size = strlen(album->artist);
				break;
			case 2:
				size = strlen(album->album_artist);
				break;
			case 3:
				size = strlen(album->album);
				break;
			case 4:
				size = strlen(album->year);
				break;
			case 5:
				sprintf(track_no_buffer, "%d", song_no + 1);
				size = strlen(track_no_buffer);
				break;
			}
			size += 1;	// Text encoding byte
			integer_to_four_char(size, id3_header_size, 0);
			fwrite(id3_header_size, sizeof(id3_header_size), 1, create_file);
			fwrite(id3_flags_and_encoding, sizeof(id3_flags_and_encoding), 1, create_file);

			// Tag Content
			switch (i) {	// Why does switch not work with strings?
			case 0:
				fprintf(create_file, "%s", album->song_titles[song_no]);
				break;
			case 1:
				fprintf(create_file, "%s", album->artist);
				break;
			case 2:
				fprintf(create_file, "%s", album->album_artist);
				break;
			case 3:
				fprintf(create_file, "%s", album->album);
				break;
			case 4:
				fprintf(create_file, "%s", album->year);
				break;
			case 5:
				fprintf(create_file, "%s", track_no_buffer);
				break;
			}

			return;
		}
	}

	for (int i = 0; i < sizeof(special_tags) / sizeof(special_tags[0]); i++) {
		if (!strcmp(chosen_tag, special_tags[i])) {
			// Tag Header
			fprintf(create_file, chosen_tag);
			switch (i) {	// Why does switch not work with strings?
			case 0:
				// <Header for 'Attached picture', ID: "APIC">
				// 	MIME type       <text string> $00
				//	Picture type    $xx
				// 	Description     <text string according to encoding> $00
				//	Picture data    <binary data>

				size = strlen(MIME_TYPE) + 1;		// MIME type
				size += 1;							// Picture type
				size += strlen(DESCRIPTION) + 1;	// Description
				size += image_size;			// Picture data
				break;
			}
			size += 1;	// Text encoding byte
			integer_to_four_char(size, id3_header_size, 0);
			fwrite(id3_header_size, sizeof(id3_header_size), 1, create_file);
			fwrite(id3_flags_and_encoding, sizeof(id3_flags_and_encoding), 1, create_file);

			// Tag Content
			switch (i) {	// Why does switch not work with strings?
			case 0:
				fprintf(create_file, MIME_TYPE); fputc(0x00, create_file);
				fputc(PICTURE_TYPE, create_file);
				fprintf(create_file, DESCRIPTION); fputc(0x00, create_file);

				int a;
				FILE* image_file;
				image_file = fopen(IMAGE_DUMP, "rb");
				if (image_file == NULL) {
					fprintf(stderr, "[!] Could not open file %s.\n", IMAGE_DUMP);
				}
				else {
					while ((a = fgetc(image_file)) != EOF) {
						fputc(a, create_file);
					}

					fclose(image_file);
				}
				break;
			}

			return;
		}
	}
}

void get_image_size() {
	FILE* read_file;
	u_int file_size = 0;

	read_file = fopen(IMAGE_DUMP, "rb");
	if (read_file == NULL) {
		fprintf(stderr, "[!] Could not open file %s.\n", IMAGE_DUMP);
	}
	else {
		fseek(read_file, 0, SEEK_END);
		image_size = ftell(read_file);
		fclose(read_file);
	}
}

void integer_to_four_char(u_int convertee, unsigned char* converted, int is_id3v2_tag) {
	if (is_id3v2_tag) {
		// The ID3v2 tag size is encoded with four bytes where the most significant bit (bit 7) is set to zero in every byte, 
		// making a total of 28 bits. The zeroed bits are ignored, so a 257 bytes long tag is represented as $00 00 02 01.

		// The ID3v2 tag size is the size of the complete tag after unsychronisation, including padding, 
		// excluding the header but not excluding the extended header(total tag size - 10). Only 28 bits(representing up to 256MB) 
		// are used in the size description to avoid the introducuction of 'false syncsignals'.

		// Because of this, we shift bits 3 times.

		u_int temp = 0;

		temp = convertee & 0xFFFFFF80;          // Pull bits 31-7 (last 24)
		temp <<= 1;                             // Left shift
		convertee = convertee & 0x0000007F;     // Zeroise all but first 7 bits
		convertee |= temp;                      // Merge back

		temp = convertee & 0xFFFF8000;          // Pull bits 31-15 (last 16)
		temp <<= 1;                             // Left shift
		convertee = convertee & 0x00007FFF;     // Zeroise all but first 15 bits
		convertee |= temp;                      // Merge back

		temp = convertee & 0xFF800000;          // Pull bits 31-23 (last 8)
		temp <<= 1;                             // Left shift
		convertee = convertee & 0x007FFFFF;     // Zeroise all but first 23 bits
		convertee |= temp;                      // Merge back
	}

	converted[0] = (convertee >> 24) & 0xFF;
	converted[1] = (convertee >> 16) & 0xFF;
	converted[2] = (convertee >> 8) & 0xFF;
	converted[3] = convertee & 0xFF;
}
