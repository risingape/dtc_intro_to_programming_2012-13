/*
 * The skeleton for your point operators programme. This is not the
 * solution to any of the exercises. However it should give you an
 * easy start and demonstrate how to use the function in image_p2.h.
 *
 * To compile it use:
 * gcc contrast_stretching.c pgm_routines.c -o point_operators -I . -lm
 */


/* TODO:
 * re-write code to allocate histogram
 */

 
/* system includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


/* include our PGM routines */
#include "image_p2.h"


/* Define a simple histogram data type.
 * We simply assume there are the same number
 * of bins as there are grey levels.
 */
typedef struct {
    unsigned int uint_num_bins;
    unsigned int* uint_bins;
} histogram;


/* 
 * Compute the histogram.
 * For simplicity whe assume that there are as many bins as grey levels.
 */
void compute_histogram(image* image_in, histogram* histogram_data) {
    int i = 0;
    int j = 0;

    histogram_data->uint_num_bins = image_in->uint_max;
   
    for(i = 0; i < image_in->uint_yres; i ++) {
        for(j = 0; j < image_in->uint_xres; j ++) {
                histogram_data->uint_bins[image_in->int_image_data[i][j]] ++;
        }
    }

    return;
}


/*
 * 
 */
void histogram_to_image(histogram* histogram_in, char* char_name, 
    unsigned int uint_yres) {
    int i = 0;
    int j = 0;
    unsigned int uint_max = 0;
    unsigned int uint_height = 0;
    float float_increment = 0.0f;
    image image_out;

    /*
     * Allocate a temporary image data structure.
     * We will destroy it before we leave this function.
     */
    allocate_image_p2(&image_out, histogram_in->uint_num_bins, 
        uint_yres, 255); 

    /* Find the max histogram value. We need this to fit the histogram
     * into the image height uint_yres.
     */
    uint_max = histogram_in->uint_bins[0];
    for(i = 1; i < histogram_in->uint_num_bins; i ++) {
        if(histogram_in->uint_bins[i] > uint_max) {
            uint_max = histogram_in->uint_bins[i];
        }
    }

    float_increment = (float)(uint_max) / uint_yres;

    for(i = 0; i < histogram_in->uint_num_bins; i ++) {
        uint_height = (unsigned int)(histogram_in->uint_bins[i] /
            float_increment + 0.5f);
        for(j = uint_yres - 1; j > uint_yres - uint_height; j --) {
            image_out.int_image_data[j][i] = 0;
        }
    }
    
    write_image_p2(char_name, &image_out);

    /*
     * Free the temporary image. 
     */
    free_image_p2(&image_out);

    return;
}


/*
 * The main entry point.
 */
int main(int argc, char *argv[]) {
    int i = 0;
    image image_in;
    histogram histogram_in;

    /* 
     * We expect the file name in argv[1],
     * otherwise print a meaningful message
     * and exit with an error number set.
     */
    if( argc != 2 ) {
        perror("Usage: histogram <infilename>\n");
        exit(1);
    }
 
    /* 
     * Get the length of the input filename.
     * We need it later on to create the 
     * output file name.
     */
    size_t size_t_name_length = strlen(argv[1]);

    /* 
     * Read the image into a image data structure.
     * All the file handling and memory allocation
     * is done by read_image_p2() to be found in 
     * image_p2.c.
     */
    if( read_image_p2(argv[1], &image_in) ) {
        perror("Unable to open file!\n");
        exit(1);
    }

    /* 
     * Allocate some buffer space for the histogram. 
     * TODO: this should be encapsulated in a function.
     */
    histogram_in.uint_bins = (unsigned int*)malloc(image_in.uint_max *
       sizeof(unsigned int));
    memset(histogram_in.uint_bins, 0 , image_in.uint_max * 
        sizeof(unsigned int));

    /* 
     * Compute the image histogram and render it into an image.
     */
    compute_histogram(&image_in, &histogram_in);
    histogram_to_image(&histogram_in, "histogram_example.pgm", 200);

    /* 
     * clean-up
     */
    free_image_p2(&image_in);

    printf("Done.\n");

    return(0);
}


