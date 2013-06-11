/*
 * Function definitions to handle a PGM (P2) file.
 */


/*
 * Pre-processor directives to ensure we include this file only once.
 */
#ifndef __IMAGE_P2__
#define __IMAGE_P2___


/*
 * System level includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
 * C does not know about MIN and MAX, so we define them as macros.
 * Beware, this code only works for built-in data types like float or int.
 */
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))


/*
 * A buffer length of 1024 char seems to be OK.
 * If you want to read larger images, please increase it here.
 */
#define INT_BUFFERLENGTH 1024


/*
 * Define the max. grey level
 */
#define INT_MAX_GREY 255

/* Type definition of a P2 grey level image */
typedef struct {
    unsigned int** int_image_data;
    unsigned int uint_xres;
    unsigned int uint_yres;
    unsigned int uint_max;
} image;


/* 
 * "Public" functions
 * You should use these in your code.
 * All functions are strictly call by reference for
 * anything but primitiv data types. Even though this
 * may look more dificult in the beginning this avoids
 * confusion about lost pointers.
 */
int read_image_p2(char* char_name, image* image_input);
int write_image_p2(char* char_name, image* image_output);
int allocate_image_p2(image* image_p2, unsigned int uint_xres, 
    unsigned int uint_yres, unsigned int uint_greylevel);
void free_image_P2(image* image_p2);
void display_image_p2(image* image_p2);
void clone_image_p2(image* image_parent, image* image_child);


/* 
 * "Private" functions
 * They are for internal use only, so you shouldn't 
 * use them in your code.
 */
int write_image_data_p2(FILE* file_output, image* image_p2);
int read_imagedata_p2(FILE* file_input, image* image_p2);
int allocate_image_data_p2(image* image_p2, 
    unsigned int uint_initialgreylevel);
int read_PBM_header_p2(FILE* file_input, image* image_input);

#endif
