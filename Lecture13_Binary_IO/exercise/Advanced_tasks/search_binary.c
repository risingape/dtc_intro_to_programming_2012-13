/*
 * Load a binary file and search for a specific bit pattern.
 * To explore the difference between buffered and unbuffered I/O
 * compare the timing between buffered and unbuffered reading from disk.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define INT_BUFFER_LENGTH 1024

/*
 * declare and initialise the static look-up tables
 */
char char_mask_left[8] = {0, 0, 0, 0, 0, 0, 0, 0};
char char_mask_middle[8] = {0, 0, 0, 0, 0, 0, 0, 0};
char char_mask_right[8] = {0, 0, 0, 0, 0, 0, 0, 0};


void byte_to_ASCII(char byte) {
    int i = 0;
    char ascii_array[8];

    for(i = 7; i >= 0; i --) {
        ascii_array[i] = 48 + (byte & 1);
        //printf("%c ",48 + (byte & 1));

        byte >>= 1;
    }

    
    for(i = 0; i < 8; i ++) {
       printf("%c ", ascii_array[i]);
    }

    return;
}


void allocate_lookup_tables(char char_left, char char_right) {
    int i = 0;
    uint32_t uint_left_mask = 0xFF000000;
    uint32_t uint_middle_mask = 0x00FF0000;
    uint32_t uint_right_mask = 0x0000FF00;
    uint32_t uint_tmp = 0;
    char* char_tmp;

    printf("input pattern: ");
    byte_to_ASCII(char_left);
    printf(", ");
    byte_to_ASCII(char_right);
    printf("\n");

    /* copy our bit pattern into a 32 bit integer */
    uint32_t uint_pattern = 0;
    uint_pattern |= char_left;
    uint_pattern <<= 8;
    uint_pattern |= char_right;
    uint_pattern <<= 16;

    /* fill the look-up tables */
    for(i = 0; i < 8; i ++ ) {
        printf("%d, ", i);
        uint_tmp = uint_pattern & uint_left_mask;
        char_mask_left[i] = ((unsigned char *)&uint_tmp)[3];
        byte_to_ASCII(char_mask_left[i]);
        
        printf(", ");
        
        uint_tmp = uint_pattern & uint_middle_mask;
        char_mask_middle[i] = ((unsigned char *)&uint_tmp)[2];
        byte_to_ASCII(char_mask_middle[i]);
        printf(", ");

        uint_tmp = uint_pattern & uint_right_mask;
        char_mask_right[i] = ((unsigned char *)&uint_tmp)[1];
        byte_to_ASCII(char_mask_right[i]);
        printf(", ");

        printf("\n");
        uint_pattern >>= 1;
    }

    return;
}


void match_pattern(char* char_buffer, uint32_t uint_length, uint32_t uint_offset) {
    uint32_t i = 0;
    uint32_t j = 0;
    char char_tmp;

    /* we only need to search for a match in the left table */
    for(i = 0; i < uint_length - 2; i ++) {
        for(j = 0; j < 8; j ++) {
          
            /* mask */
            char_buffer[i] <<= j;
            char_buffer[i] >>= j;

            char_tmp = char_buffer[i + 2] >> (8 - j);
            char_tmp <<= (8 - j);

            if(char_buffer[i] == char_mask_left[j]) {
                /* we got a hit, now check the middle and right table */
                if(char_buffer[i + 1] == char_mask_middle[j] && char_tmp == char_mask_right[j]) {
                    printf("found pattern at position %d, offset %d.\n", uint_offset + i, j);
                }
            }
        }
        
    }

    /* check if the last two bytes match the patterm */
    if(char_buffer[uint_length - 2] == char_mask_left[0] && char_buffer[uint_length - 1] == char_mask_middle[0]) {
        printf("found pattern at position %d, offset %d.\n", uint_offset + uint_length - 2, 0);
    }

    return;
}


/*
 * unbuffered read
 */
int main(int argc, char *argv[]) {
    FILE *file_in;
    char* char_buffer;
    uint32_t uint_filelength = 0;
    //char char_left = 0x50;
    //char char_right = 0x34;

    char char_left = 0x78;
    char char_right = 0x7E;

    if( argc != 2 ) {
        perror("Usage: search_binary infilename pattern\n");
        exit(1);
    }

    /* open file read-only */
    file_in = fopen(argv[1], "r");
    if( file_in == NULL ) {
        perror("Can't open input file.\n");
        exit(1);
    }

    /* get the size of the file */
    fseek (file_in , 0 , SEEK_END);
    uint_filelength = ftell(file_in);
    rewind(file_in);

    allocate_lookup_tables(char_left, char_right);

    char_buffer = (char*)malloc(uint_filelength * sizeof(char));
    
    int result = fread (char_buffer, 1, uint_filelength, file_in);
    if(result != uint_filelength){
        fputs("Reading error",stderr);
        exit(1);
    }
    

    /* match the patter */
    match_pattern(char_buffer, uint_filelength, 0);

    fclose(file_in);
    free(char_buffer);
    return 0;
}




