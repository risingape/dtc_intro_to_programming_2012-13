/* 
 * This file does not contain any working code. Just ideas how to
 * implement your own filter functions.
 */


/* 
 * This is just a sketch of a generic linear filter function.
 * You have to either make the filter kernel available as
 * global variable or define it inside the filter function.
 */
void filter_image(image* the_image, image* image_gradient, unsigned int uint_width, unsigned int uint_height) {
    int i = 0;
    int j = 0;

    for(i = 0; i < the_image->uint_yres; i ++) {
        for(j = 0; j < the_image->uint_xres; j ++) {

            /* You have to take care of the borders */
            if(i <= uint_height / 2 || i >= the_image->uint_yres - uint_height / 2 || j <= uint_width / 2 || j >= the_image->uint_xres - uint_width / 2) {
                /* this is a border point */
            } else {
               /* filter the pixel */
            }
        }
    }

    return;
}


/*
 * This is a more advanced version of the filter function. The kernel is encapsulated in another function
 * and passed as a pointer to the filter function. This allows you to change the filter kernel "on-the-fly".
 * The prototype of the kernel function is:
 *    int filter_pixel(image* image_in, unsigned int int_x, unsigned int int_y);
 *    (an example is shown below)
 * It simply compute the filtered value of the pixel at position int_x, int_y. Since the filtered value 
 * of a pixel depends on its neighbours, we have to pass a pointer to the image itself.
 */
void filter_image(image* the_image, image* image_gradient, unsigned int uint_width, unsigned int uint_height, int (filter_pixel)(image*, unsigned int, unsigned int)) {
    int i = 0;
    int j = 0;

    for(i = 0; i < the_image->uint_yres; i ++) {
        for(j = 0; j < the_image->uint_xres; j ++) {

            /* You have to take care of the borders */
            if(i <= uint_height / 2 || i >= the_image->uint_yres - uint_height / 2 || j <= uint_width / 2 || j >= the_image->uint_xres - uint_width / 2) {
                /* this is a border point */
            } else {
               /* filter the pixel */
            }
        }
    }

    return;
}


#define KERNEL_SIZE 5 
int filter_pixel(image* the_image, unsigned int uint_x, unsigned int uint_y) {
    int i = 0;
    int j = 0;
    float float_tmp = 0.0f;
    
    for(i = - KERNEL_SIZE / 2; i <= KERNEL_SIZE / 2; i ++) {
         for(j = - KERNEL_SIZE / 2; j <= KERNEL_SIZE / 2; j ++) {        
             /* compute the new pixel value */
         }
    }

    /* return the integer part of the new pixel value */
    return((int)(float_tmp + 0.5f);
}
