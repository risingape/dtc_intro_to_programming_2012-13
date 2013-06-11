/*-----------------------------------------
 * Generic netPBM functions
 * General housekeeping code to read and 
 * write a PGM ASCII encoded grey map image.
 * This code only accepts P2 images.
 *---------------------------------------*/


/* Include our routines to handle a PGM file.*/
#include "image_p2.h"


/*
 * "private" function
 *
 * Read the image header.
 * The image header looks like this:
 * 
 * P<type>
 * <size x> <size y>
 * # Comment like the creator of the image
 * <max. grey level>
 */
int read_PBM_header_p2(FILE* file_input, image* image_input) {
    char* char_buffer;
 
    /* allocate some buffer space */
    char_buffer = (char*)malloc(INT_BUFFERLENGTH * sizeof(char) );

    /* check if we got a P2 image */
    fgets(char_buffer, INT_BUFFERLENGTH, file_input);
    if( strncmp(char_buffer, "P2", 2 * sizeof(char)) != 0) {
        perror("Not a P2 image (ASCII encoded portable greymap)\n");
        free(char_buffer);
        return(-1);
    }

    /* get the image resolution */
    fgets(char_buffer, INT_BUFFERLENGTH, file_input);
    while(char_buffer[0] == '#') {
         fgets(char_buffer, INT_BUFFERLENGTH, file_input);
    }
    sscanf(char_buffer, "%d %d", &(image_input->uint_xres), 
        &(image_input->uint_yres));

    /* get the max. grey level */
    fgets(char_buffer, INT_BUFFERLENGTH, file_input);
    while(char_buffer[0] == '#') {
         fgets(char_buffer, INT_BUFFERLENGTH, file_input);
    }
    sscanf(char_buffer, "%d", &(image_input->uint_max));

#ifdef DEBUG
    printf("read image header, max grey level: %d", image_input->uint_max);
#endif

    free(char_buffer);
    return(0);
} 


/* "private" function */
int allocate_image_data_p2(image* image_p2, unsigned int uint_initialgreylevel) {
    int i = 0;
    int j = 0;

    if(image_p2->uint_xres == 0 || image_p2->uint_yres == 0) {
        perror("allocate_image: At least one dimension is zero.");
        return(-1);
    }

    if(uint_initialgreylevel > 255) {
       perror("allocate image: Invalid initial grey level given.\n");
       return(-1);
    }

    image_p2->int_image_data = (unsigned int**)malloc(image_p2->uint_yres *
        sizeof(unsigned int*));
    if(image_p2->int_image_data == NULL) {
        perror("allocate_image: Error allocating storage space.\n");
        return(-1);
    }

    image_p2->int_image_data = (unsigned int**)malloc(image_p2->uint_yres * sizeof(unsigned int*));

    for(i = 0; i < image_p2->uint_yres; i ++) {
        image_p2->int_image_data[i] = (unsigned int*)malloc(
            image_p2->uint_xres * sizeof(unsigned int));
        if(uint_initialgreylevel == 0) {
            memset(image_p2->int_image_data[i], 0, image_p2->uint_xres *
                sizeof(unsigned int));
        } else {
            /* 
             * Why do I have to do this? 
             * Since memset sets bytes but an int has more than one byte.
             */
            for(j = 0; j < image_p2->uint_xres; j ++) {
                image_p2->int_image_data[i][j] = uint_initialgreylevel;
            }
        }
    }

    return(0);
}


/* "public" function */
int allocate_image_p2(image* image_p2, unsigned int uint_xres, 
    unsigned int uint_yres, unsigned int uint_greylevel) {
 
    if(uint_greylevel > 255) {
        perror("allocate_image: the max. allowed grey level is 255.\n");
        return(-1);
    }

    image_p2->uint_xres = uint_xres;
    image_p2->uint_yres = uint_yres;
    image_p2->uint_max = uint_greylevel;    

    return allocate_image_data_p2(image_p2, uint_greylevel);
}


/* "public" function */
void free_image_p2(image* image_p2) {
    int i = 0;

    for(i = 0; i < image_p2->uint_yres; i ++) {
        free(image_p2->int_image_data[i]);
    }

    free(image_p2->int_image_data);

    return;
}


/*
 * "private" function
 *
 * This function reads only the image data.
 */
int read_image_data_p2(FILE* file_input, image* image_p2) {
    int i = 0;
    int j = 0;

    /* check if the iamge has been allocated */
    if(image_p2->uint_xres == 0 || image_p2->uint_yres == 0) {
        perror("Image not allocated.");
        return(-1);
    }

    for(i = 0; i < image_p2->uint_yres; i ++) {
        for(j = 0; j < image_p2->uint_xres; j ++) {
             if( fscanf(file_input, "%d", 
                 &(image_p2->int_image_data[i][j])) == EOF ) {
                 perror("Unexpected end of image data.\n");
                 exit(1);
             }
        }
    }
 
    /* check if we read the entire image */
    if(j != image_p2->uint_xres || i != image_p2->uint_yres ) {
         perror("Unexpected end of PGM file.\n");
         return(-1);
    }

    return(0);
}


/* "public" function */
int read_image_p2(char* char_name, image* image_input) {
    FILE* file_input;
    int int_message_length;
    char* char_error_message;

    file_input = fopen(char_name, "r");
    if( file_input == NULL ) {
        /* I am printing a string into 0 allocated bytes.
         * snprintf conviniently reports the number of bytes
         * it is unable to print.
         */
        int_message_length = snprintf(NULL, 0, "Can't open input file: %s\n",
            char_name);
        char_error_message = malloc(int_message_length);
        sprintf(char_error_message, "Can't open input file: %s\n",
            char_name); 
        perror(char_error_message);       
        return(-1);
    }
    
    /* read the header information */
#ifdef DEBUG
    printf("read_image, header of image: %s.\n", char_name);
#endif

    if( read_PBM_header_p2(file_input, image_input) != 0 ) {
        perror("Error reading header of image file.\n");
        fclose(file_input);
        return(-1);
    }

#ifdef DEBUG
    printf("read_image, image resolution x: %d, y: %d, max. grey level: %d\n",
        image_input->uint_xres, image_input->uint_yres, image_input->uint_max);
#endif

    /* allocate image */
    if( allocate_image_p2(image_input, image_input->uint_xres, 
        image_input->uint_yres, image_input->uint_max) != 0 ) {
        perror("Error allocating memory to store image data\n");
        fclose(file_input);
        return(-1);
    }

    /* read image data */
    if( read_image_data_p2(file_input, image_input) != 0 ) {
        perror("Error reading image data\n");
        fclose(file_input);
        return(-1);
    }

    return(0);
}


/* "private" function */
int write_image_data_p2(FILE* file_output, image* image_p2) {
    unsigned int i = 0;
    unsigned int j = 0;

    /* check if the iamge has been allocated */
    if(image_p2->uint_xres == 0 || image_p2->uint_yres == 0) {
        perror("Image not allocated.");
        return(-1);
    }

    /* write header information */
    fprintf(file_output, "P2\n%d %d\n# CREATOR: binary2ascii\n%d\n", 
        image_p2->uint_xres, image_p2->uint_yres, image_p2->uint_max);

    for(i = 0; i < image_p2->uint_yres; i ++) {
        for(j = 0; j < image_p2->uint_xres; j ++) {
            fprintf(file_output, "%d ", image_p2->int_image_data[i][j]);
        }
        fprintf(file_output, "\n");
    }

    return(0);
}


/* public function */
int write_image_p2(char* char_name, image* image_p2) {
    FILE* file_output;
    int int_message_length;
    int int_return_value1;
    int int_return_value2;
    char* char_error_message;

#ifdef DEBUG
    printf("write_image: %s\n", char_name);
#endif

    file_output = fopen(char_name, "w");
    if( file_output == NULL ) {
        /* I am printing a string into 0 allocated bytes.
         * snprintf conviniently reports the number of bytes
         * it is unable to print.
         */
        int_message_length = snprintf(NULL, 0, "Can't open output file: %s\n",
            char_name);
        char_error_message = malloc(int_message_length);
        sprintf(char_error_message, "Can't open output file: %s\n",      
            char_name); 
        perror(char_error_message);       
        return(-1);
    }
    
    /* 
     * Only return 0 if both writing the data and closing 
     * the file are successful. However, we have to close 
     * the file in either case.
     */
    int_return_value1 = write_image_data_p2(file_output, image_p2);
    int_return_value2 = fclose(file_output);

    return(MIN(int_return_value1, int_return_value2));
}


/* "public" function */
void display_image_p2(image* image_p2) {
    unsigned int i = 0;
    unsigned int j = 0;

    printf("display_image: %d, %d, max. grey level: %d\n", 
        image_p2->uint_xres, image_p2->uint_yres, image_p2->uint_max);

    for(i = 0; i < image_p2->uint_yres; i ++) {
        for(j = 0; j < image_p2->uint_xres; j ++) {
            printf("%u ", image_p2->int_image_data[i][j]);
        }
        printf("\n");
    }

    return;
}


/* "public" function */
void clone_image_p2(image* image_parent, image* image_child) {
    int i;
    
    /* allocate the child image */
    allocate_image_p2(image_child, image_parent->uint_xres, 
        image_parent->uint_yres, image_parent->uint_max);

    /* copy the number of grey levels */
    image_child->uint_max = image_parent->uint_max;

    printf("clone image, max grey: %d\n", image_child->uint_max);

    /* deep copy the image data */
    for(i = 0; i < image_child->uint_yres; i ++) {
        memcpy(image_child->int_image_data[i], 
            image_parent->int_image_data[i], 
            image_child->uint_xres * sizeof(int));
    }

    return;
}

