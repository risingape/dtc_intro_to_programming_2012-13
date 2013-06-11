
/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


/* definitions to avoid "magic numbers" */
#define INT_BUFFERLENGTH 1024
#define BITS_PER_BYTE 8
#define ASCII_ZERO 48

/* We expect the input file name in argv[1] 
 * and the outout file name in argv[2] 
 */
int main(int argc, char *argv[]) {
    int i = 0;
    FILE *file_input;
    FILE *file_output;
    char *char_buffer;
    char *char_binary_imageline;
    char *char_ascii_imageline;
    unsigned int uint_xres = 0;
    unsigned int uint_yres = 0;
    unsigned int uint_imageline_in_bytes = 0;

    /* We expect the file name in argv[1],
     * otherwise print a meaningful message
     * and exit with an error number set.
     */
    if( argc != 3 ) {
        perror("Usage: binary2ascii infilename outfilename\n");
        exit(1);
    }

    /* open file read-only */
    file_input = fopen(argv[1], "r");
    if( file_input == NULL ) {
        perror("Can't open input file.\n");
        exit(1);
    }

    /* open output file */
    file_output = fopen(argv[2], "w");
    if( file_output  == NULL ) {
        perror("Can't open output file.\n");
        exit(1);
    }

    /* Get the image type and resolution from the file.
     * We know that the first line gives the image type
     * and the second or third line the dimension.
     */
    printf("read header of binary image: %s.\n", argv[1]);
    char_buffer = (char*)malloc(INT_BUFFERLENGTH * sizeof(char) );


    fgets(char_buffer, INT_BUFFERLENGTH, file_input);
    if( strncmp(char_buffer, "P4", 2 * sizeof(char)) != 0) {
        perror("Not a P4 image (binary encoded portable bitmap)\n");
        exit(1);
    }

    fgets(char_buffer, INT_BUFFERLENGTH, file_input);
    while(char_buffer[0] == '#') {
         fgets(char_buffer, INT_BUFFERLENGTH, file_input);
    }
    sscanf(char_buffer, "%d %d", &uint_xres, &uint_yres);
    
    printf("Image resolution is %d by %d pixels.\n", uint_xres, uint_yres);

    /* How many bytes to we need to store a single line of the image? Consider an 
     * image of size 3 pixels, we still need to store an entire bytes.
     */
    uint_imageline_in_bytes = uint_xres / BITS_PER_BYTE + ((uint_xres % BITS_PER_BYTE) > 0);
    printf("binary image line in bytes: %d\n", uint_imageline_in_bytes);


    /* allocate the memory for a line of the binary and ascii image */
    char_binary_imageline = (char*)malloc( uint_imageline_in_bytes );
    char_ascii_imageline = (char*)malloc(uint_xres * sizeof(char));

    /* write the header information */
    fprintf(file_output, "P1\n%d %d\n# CREATOR: binary2ascii\n", uint_xres, uint_yres);

    for(i = 0; i < uint_yres; i ++) {
        /* read a single line of the binary image */
        printf("read line %d of binary image, ", i);
        fread(char_binary_imageline, sizeof(char), uint_imageline_in_bytes, file_input);
        if (feof(file_input)) {
            perror("Unexpected end of pbm file.\n");
            exit(1);
        }   

        /*---------------------------------*/
        /* Your conversion code goes here! */
        /*---------------------------------*/

        /* you have got a character array full of binary data */
        /* your job is to take this data character by character and convert */
        /* and print out every bit of it  */
        /* To do this use the algorithm continually dividing a decimal number by 2 */
        /* to get it binary representation */ 

        /* append the line to the output file */
        printf("write line of ASCII image.\n");
        fwrite(char_ascii_imageline, sizeof(char), uint_xres, file_output);
        fputc('\n', file_output);
    }
        
    
    /* close file */
    fclose(file_input);
    fclose(file_output);

    /* free the allocated memory */
    free(char_buffer);
    free(char_binary_imageline);
    free(char_ascii_imageline);

    return(0);
}
