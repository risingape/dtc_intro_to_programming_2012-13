/*
 * This code accepts portable greymap (P2) images.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define ASCII_ZERO 48
#define MAX_RECURSIONS 100
#define MAX_DIGITS_PER_PIXEL 3

/* Thresholds for tracing an edge with hysteresis
 * We start to trace an edge if the value of the pixel
 * is above EDGE_START and stop if it falls below EDGE_STOP
 */
#define EDGE_START 670
#define EDGE_STOP 670

/* C does not know about MIN and MAX, so we define them as macros.
 * Beware, this code only works for built-in data types like float or int.
 */
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

typedef struct {
    unsigned int** int_image_data;
    unsigned int uint_xres;
    unsigned int uint_yres;
    unsigned int uint_max;
} image;


typedef struct {
    int uint_x;
    int uint_y;
} point;


/*--------------------------
 * GAUSSIAN NOISE FILTERING
 *------------------------*/

/* hard-coded definitions of the filter kernel, gamma = 1.0 */
#define GAUSSIAN_KERNEL_SIZE 5
#define GAUSSIAN_KERNEL_WEIGHT 273.0f
static int int_gaussian_5x5[5][5] = {1, 4, 7, 4, 1,
                                      4, 16, 26, 16, 4, 
                                      7, 26, 41, 26, 7, 
                                      4, 16, 26, 16, 4, 
                                      1, 4, 7, 4, 1};


/* this function should not be called directly.
 * It does not do any boundary checks.
 */
int gaussian_filter(image* the_image, unsigned int uint_x, unsigned int uint_y) {
    int i = 0;
    int j = 0;
    float float_tmp = 0.0f;
    
    for(i = - GAUSSIAN_KERNEL_SIZE / 2; i <= GAUSSIAN_KERNEL_SIZE / 2; i ++) {
         for(j = - GAUSSIAN_KERNEL_SIZE / 2; j <= GAUSSIAN_KERNEL_SIZE / 2; j ++) {        
             float_tmp += int_gaussian_5x5[i + 2][j + 2] * the_image->int_image_data[uint_y + i][uint_x + j];
             //printf("j: %d, i: %d, f: %f\n", j, i, float_tmp);
         }
    }

    return((int)(float_tmp / GAUSSIAN_KERNEL_WEIGHT + 0.5f));
}


/* Linear filter function
 * This code should work with all kind of filter kernels.
 * You only have to change the actual filter function.
 */
void filter_image(image* the_image, image* image_gradient, unsigned int uint_width, unsigned int uint_height, int (filter_pixel)(image*, unsigned int, unsigned int), unsigned int uint_neutral ) {
    int i = 0;
    int j = 0;

    for(i = 0; i < the_image->uint_yres; i ++) {
        for(j = 0; j < the_image->uint_xres; j ++) {

            /* set the border pixels to 255 (neutral for edge detection) */
            if(i <= uint_height / 2 || i >= the_image->uint_yres - uint_height / 2 || j <= uint_width / 2 || j >= the_image->uint_xres - uint_width / 2) {
                image_gradient->int_image_data[i][j] = uint_neutral;
            } else {
                /* change this line to use a different filter */
                image_gradient->int_image_data[i][j] = filter_pixel(the_image, j, i);
            }
        }
    }

    return;
}


/*----------------------
 * CANNY EDGE DETECTION
 *--------------------*/

/*
 * The Canny edge detection process uses the intensity gradient to detect edges.
 * The intensity gradient itself is computed by an edge detection operator like
 * the Sobel, Prewitt or Roberts operators.
 */

/* The sobel operator in x and y direction*/
#define SOBEL_KERNEL_SIZE 3
int sobel_gx(image* the_image, unsigned int uint_x, unsigned int uint_y) {
    int float_tmp = 0;
    
    float_tmp = - the_image->int_image_data[uint_y - 1][uint_x - 1] 
                + the_image->int_image_data[uint_y - 1][uint_x + 1]
                - 2 * the_image->int_image_data[uint_y][uint_x - 1]
                + 2 * the_image->int_image_data[uint_y][uint_x + 1]
                - the_image->int_image_data[uint_y + 1][uint_x - 1]
                + the_image->int_image_data[uint_y + 1][uint_x + 1];

    return(float_tmp);
}

int sobel_gy(image* the_image, unsigned int uint_x, unsigned int uint_y) {
    int float_tmp = 0;
    
    float_tmp = (- the_image->int_image_data[uint_y - 1][uint_x - 1] 
                - 2 * the_image->int_image_data[uint_y - 1][uint_x]
                - the_image->int_image_data[uint_y - 1][uint_x + 1]
                + the_image->int_image_data[uint_y + 1][uint_x - 1]
                + 2 * the_image->int_image_data[uint_y + 1][uint_x]
                + the_image->int_image_data[uint_y + 1][uint_x + 1]);

    return(float_tmp);
}


void gradient_magnitude(image* image_gradientmagnitude, image* image_gradientx, image* image_gradienty) {
    int i = 0;
    int j = 0;
    float float_tmp = 0.0f;

    for(i = 0; i < image_gradientmagnitude->uint_yres; i ++) {
        for(j = 0; j < image_gradientmagnitude->uint_xres; j ++) {
            float_tmp = sqrtf(image_gradientx->int_image_data[i][j] * image_gradientx->int_image_data[i][j] + image_gradienty->int_image_data[i][j] * image_gradienty->int_image_data[i][j]);
            image_gradientmagnitude->int_image_data[i][j] = (int)(float_tmp + 0.5f);
        }
    }

    return;
}


void gradient_nms(image* image_nms, image* image_gradientx, image* image_gradienty, image* image_gradientmagnitude) {
    int i = 0;
    int j = 0;
    int int_degrees0 = 0;
    int int_degrees45 = 0;
    int int_degrees90 = 0;
    int int_degrees135 = 0;
    float float_direction = 0.0f;
    float float_tmp = 0.0f;

    for(i = 1; i < image_gradientmagnitude->uint_yres - 1; i ++) {
        for(j = 1; j < image_gradientmagnitude->uint_xres - 1; j ++) {
            if(image_gradientmagnitude->int_image_data[i][j] == 0) continue;
           
            float_direction = (fmodf(atan2((float)image_gradienty->int_image_data[i][j], (float)image_gradientx->int_image_data[i][j]) + M_PI, M_PI) / M_PI) * 8.0f;

            /* for readability: compute the non-maximum suppression conditions */
            int_degrees0 = (float_direction <= 1 || float_direction > 7) && image_gradientmagnitude->int_image_data[i][j] >= image_gradientmagnitude->int_image_data[i][j + 1] && image_gradientmagnitude->int_image_data[i][j] > image_gradientmagnitude->int_image_data[i][j - 1];

            int_degrees45 = (float_direction > 1 || float_direction <= 3) && image_gradientmagnitude->int_image_data[i][j] > image_gradientmagnitude->int_image_data[i - 1][j - 1] && image_gradientmagnitude->int_image_data[i][j] > image_gradientmagnitude->int_image_data[i + 1][j + 1];

            int_degrees90 = (float_direction > 3 || float_direction <= 5) && image_gradientmagnitude->int_image_data[i][j] >= image_gradientmagnitude->int_image_data[i + 1][j] && image_gradientmagnitude->int_image_data[i][j] > image_gradientmagnitude->int_image_data[i - 1][j];

            int_degrees135 = (float_direction > 5 || float_direction <= 7) && image_gradientmagnitude->int_image_data[i][j] > image_gradientmagnitude->int_image_data[i - 1][j + 1] && image_gradientmagnitude->int_image_data[i][j] > image_gradientmagnitude->int_image_data[i + 1][j - 1];

            /* if non of it applies delete the edge point */
            if((int_degrees0 || int_degrees45 || int_degrees90 || int_degrees135)) {
               image_nms->int_image_data[i][j] = image_gradientmagnitude->int_image_data[i][j];
            } else { 
               image_nms->int_image_data[i][j] = 0;
            }
        }
    }

    return;
}


/* recursively follow the edge */
void follow_edge(image* image_edges, image* image_gradientmap, unsigned int uint_tmin, unsigned int x, unsigned int y, unsigned int depth) {
    // check image boundaries
    if(x <= 0 || y <= 0 || x >= image_edges->uint_xres - 1 || y >= image_edges->uint_yres - 1 || depth > MAX_RECURSIONS) return;

    image_edges->int_image_data[y][x] = 255;

    // nw
    if(image_gradientmap->int_image_data[y - 1][x - 1] > uint_tmin && image_edges->int_image_data[y - 1][x - 1] == 0) {
        follow_edge(image_edges, image_gradientmap, uint_tmin, x - 1, y - 1, depth + 1);
    }

    // nn
    if(image_gradientmap->int_image_data[y - 1][x] > uint_tmin && image_edges->int_image_data[y - 1][x] == 0) {
        follow_edge(image_edges, image_gradientmap, uint_tmin, x, y - 1, depth + 1);
    }

    // ne
    if(image_gradientmap->int_image_data[y - 1][x + 1] > uint_tmin && image_edges->int_image_data[y - 1][x + 1] == 0) {
        follow_edge(image_edges, image_gradientmap, uint_tmin, x + 1, y - 1, depth + 1);
    }

    // ee
    if(image_gradientmap->int_image_data[y][x + 1] > uint_tmin && image_edges->int_image_data[y][x + 1] == 0) {
        follow_edge(image_edges, image_gradientmap, uint_tmin, x + 1, y, depth + 1);
    }

    // se
    if(image_gradientmap->int_image_data[y + 1][x + 1] > uint_tmin && image_edges->int_image_data[y + 1][x + 1] == 0) {
        follow_edge(image_edges, image_gradientmap, uint_tmin, x + 1, y + 1, depth + 1);
    }

    // ss
    if(image_gradientmap->int_image_data[y + 1][x] > uint_tmin && image_edges->int_image_data[y + 1][x] == 0) {
        follow_edge(image_edges, image_gradientmap, uint_tmin, x, y + 1, depth + 1);
    }

    // sw
    if(image_gradientmap->int_image_data[y + 1][x - 1] > uint_tmin && image_edges->int_image_data[y + 1][x - 1] == 0) {
        follow_edge(image_edges, image_gradientmap, uint_tmin, x - 1, y + 1, depth + 1);
    }

    // ww
    if(image_gradientmap->int_image_data[y][x - 1] > uint_tmin && image_edges->int_image_data[y][x - 1] == 0) {
        follow_edge(image_edges, image_gradientmap, uint_tmin, x - 1, y, depth + 1);
    }

    return;
}


void trace_edges(image* image_edges, image* image_gradientmap, unsigned int uint_tmin, unsigned int uint_tmax) {
    int i = 0;
    int j = 0;

    for(i = 0; i < image_edges->uint_yres; i ++) {
        for(j = 0; j < image_edges->uint_xres; j ++) {
            /* check if the point is above tmax and not yet part of an edge */
            if(image_gradientmap->int_image_data[i][j] > uint_tmax && image_edges->int_image_data[i][j] == 0) {
                /* follow the edge recursivley */
                follow_edge(image_edges, image_gradientmap, uint_tmin, j, i, 0);
            }
        
        }
    }

    return;
}

/* parameters for tracing edges with hysteresis: 
 * uint_tmin: we mus fall below this to end an edge
 * uint_tmax: we need to be above this to start an edge
 */
void canny(image* image_input, image* image_edges, unsigned int uint_tmin, unsigned int uint_tmax) {
    int i = 0;
    int j = 0;
    float float_tmp = 0.0f;
    image image_filtered;
    image image_gradientx;
    image image_gradienty;
    image image_gradientmagnitude;
    image image_gradientmap;

    /* Gauusian noise filtering */
    allocate_image_p2(&image_filtered, image_input->uint_xres, image_input->uint_yres, 255);
    filter_image(image_input, &image_filtered, GAUSSIAN_KERNEL_SIZE, GAUSSIAN_KERNEL_SIZE, gaussian_filter, 255);

    /* compute gradients in x and y direction */
    allocate_image_p2(&image_gradientx, image_input->uint_xres, image_input->uint_yres, 255);
    allocate_image_p2(&image_gradienty, image_input->uint_xres, image_input->uint_yres, 255);
    filter_image(&image_filtered, &image_gradientx, SOBEL_KERNEL_SIZE, SOBEL_KERNEL_SIZE, sobel_gx, 0);
    filter_image(&image_filtered, &image_gradienty, SOBEL_KERNEL_SIZE, SOBEL_KERNEL_SIZE, sobel_gy, 0);

    /* compute gradient magnitude and direction and do a non-maximum suppression */
    allocate_image_p2(&image_gradientmagnitude, image_input->uint_xres, image_input->uint_yres, 0);
    allocate_image_p2(&image_gradientmap, image_input->uint_xres, image_input->uint_yres, 0);    
    gradient_magnitude(&image_gradientmagnitude, &image_gradientx, &image_gradienty);
    write_image_p2("edge_gradientx.pgm", &image_gradientx);
    write_image_p2("edge_gradienty.pgm", &image_gradienty);
    write_image_p2("edge_gradient_magnitude.pgm", &image_gradientmagnitude);

    /* suppress non-maxima */
    allocate_image_p2(image_edges, image_input->uint_xres, image_input->uint_yres, 0);
    //clone_image_p2(&image_gradientmagnitude, image_edges);
    image_edges->uint_max = 255;
    gradient_nms(image_edges, &image_gradientx, &image_gradienty, &image_gradientmagnitude);   

    /* trace the edges with hysteresis*/
    trace_edges(image_edges, &image_gradientmagnitude, uint_tmin, uint_tmax);

    /* clean-up temporary storage */
    free_image_p2(&image_filtered);
    free_image_p2(&image_gradientx);
    free_image_p2(&image_gradienty);
    free_image_p2(&image_gradientmagnitude);
    free_image_p2(&image_gradientmap);

    return;
}

/*
 * this function actually render the line into the image
 */
void set_line_pixels(image* image_in, point point_start, point point_end) {
    int i = 0;
    int dx = point_end.uint_x - point_start.uint_x;
    int dy = point_end.uint_y - point_start.uint_y;
    int int_increment = 0;
    float float_slope = 0.0;
    float float_perpendicular_axis = 0.0;

    printf("render line, start: (%d, %d), end: (%d, %d)\n", point_start.uint_x, point_start.uint_y, point_end.uint_x, point_end.uint_y);

    if(point_start.uint_x == point_end.uint_x && point_start.uint_y == point_end.uint_y) return;

    if( abs(dx) > abs(dy) ) {
        /* render in x direction */
        int_increment = dx / abs(dx); // 1 or -1
        float_slope = (float) dy / fabs((float) dx);
        float_perpendicular_axis = (float)point_start.uint_y;
        for(i = point_start.uint_x; i != point_end.uint_x; i += int_increment) {
            /* set pixel i, (int)float_perpendicular_axis */
            image_in->int_image_data[(int)(float_perpendicular_axis + 0.5f)][i] = 0;
            float_perpendicular_axis += float_slope;
        }
    } else {
        /* render in y direction */
        int_increment = dy / abs(dy);
        float_slope = (float) dx / fabs((float) dy);
        
        float_perpendicular_axis = (float)point_start.uint_x;
        printf("inc: %d, slope: %f, x0: %f\n", int_increment, float_slope, float_perpendicular_axis);
        for(i = point_start.uint_y; i != point_end.uint_y; i += int_increment) {
            /* set pixel i, (int)float_perpendicular_axis */
            image_in->int_image_data[i][(int)(float_perpendicular_axis + 0.5f)] = 0;
            float_perpendicular_axis += float_slope;
        }
    }

    return;
}

void render_line(image* buffer, float float_slope, float b) {
    float x0 = 0.0f;
    float xmax = 0.0f;
    float ymax = 0.0f;
    point point_start;
    point point_end;
 
    /* intersections with the image boundary */
    x0 = - b / float_slope;       // intersect with y = 0
    xmax = (buffer->uint_yres - b) / float_slope; // intersect with y = Y
    
    // y0 = b; we got this as a parameter
    ymax = float_slope * buffer->uint_xres + b;   // intersect with x = X
           
    /* positive slope, line does not intersect with the image */
    if((float_slope >= 0.0f) && (x0 > buffer->uint_xres - 1 || b > buffer->uint_yres - 1)) return;

    /* negative slope, line does not intersect with the image */
    if((float_slope < 0.0f) && (b < 0 || b > buffer->uint_yres - 1 - float_slope * buffer->uint_xres)) return;

    /* at the moment this code is not able to plot vertical lines */
    if(isinf(float_slope) || isinf(float_slope)) return;

    if(float_slope >= 0.0f) {
        /* start point */
        if(b >= 0) {
            point_start.uint_x = 0;
            point_start.uint_y = b;
        } else if(float_slope > 0.0f) {
            point_start.uint_x = x0;
            point_start.uint_y = 0;
        } else {
            return;
        }
                 
        /* end point */
        if(ymax <= buffer->uint_yres) {
            point_end.uint_x = buffer->uint_xres - 1;
            point_end.uint_y = ymax;
        } else {
            point_end.uint_x = xmax;
            point_end.uint_y = buffer->uint_yres - 1;
        }
    } else {
        /* start point */
        if(b <= buffer->uint_yres) {
            point_start.uint_x = 0;
            point_start.uint_y = b;
        } else {
            point_start.uint_x = xmax;
            point_start.uint_y = buffer->uint_yres - 1;
        }
                     
        /* end point */
        if(float_slope * buffer->uint_xres + b >= 0.0f) {
             point_end.uint_x = buffer->uint_xres - 1;
             point_end.uint_y = ymax; 
        } else {
             point_end.uint_x = x0;
             point_end.uint_y = 0;
        }
    }

    set_line_pixels(buffer, point_start, point_end);

    return;
}

void reverse_transform(image* image_foundlines, image* image_houghmap, float float_threshold) {
    int i = 0;
    int j = 0;
    float float_deltarho = 0.0f;
    float float_deltatheta = 0.0f;
    float float_rho = 0.0f;
    float float_theta = 0.0f;
    point point_start;
    point point_end;

    float float_slope = 0.0f;
    float float_offset = 0.0f;

    float_deltatheta = M_PI / image_houghmap->uint_xres;
    float_deltarho = 2.0f * hypotf(image_foundlines->uint_xres, image_foundlines->uint_yres) / image_houghmap->uint_yres;

    /* find all points above the threshold */
    for(i = 0; i < image_houghmap->uint_yres; i ++) {
        for(j = 0; j < image_houghmap->uint_xres; j ++) {
            //if(image_houghmap->int_image_data[i][j] > uint_threshold) {
            if(image_houghmap->int_image_data[i][j] > (int)(float_threshold * image_houghmap->uint_max)) {

                 /* convert bins to r and theta values */
                 float_theta = j *  float_deltatheta;
                 float_rho =  float_deltarho * (image_houghmap->uint_yres / 2.0f - i);
                 
                 if(fabs(sinf(float_theta)) > sqrtf(2.0f)/2.0f ) {
                     /* slope and offset */
                     float_slope = -cosf(float_theta) / sinf(float_theta);
                     float_offset = float_rho / sinf(float_theta);
                 } else {
                     /* slope and offset */
                     //float_slope = -sinf(float_theta) / cosf(float_theta);
                     float_slope = -cosf(float_theta) / sinf(float_theta);
                     //float_offset = float_rho / cosf(float_theta);
                     float_offset = float_rho / sinf(float_theta);
                 } 

                 printf("found line at [%d][%d] threshold: %f, theta: %f, rho: %f, slope: %f, offset: %f\n", i, j, (float_threshold * image_houghmap->uint_max), float_theta, float_rho, float_slope, float_offset);

                 render_line(image_foundlines, float_slope, float_offset);
            }
        }
    }

    return;
}


/*-----------------------
 * HOUGH TRANSFORMATION
 *---------------------*/

/* we are not considering the gradient information yet */
/* advanced task: how to make this loop structure more efficient assuming that
 * there are only few edges in the image
 */
void hough_transform(image* image_edgemap, image* image_houghmap, unsigned int uint_binstheta, unsigned int uint_binsrho) {
    int i = 0;
    int j = 0;
    int k = 0;
    int int_y;
    float float_theta = 0.0f;
    float float_rho = 0.0f;

    /* the inclement in the angle theta */
    float float_deltatheta = M_PI / uint_binstheta;

    /* the maximum value of the radius (diagonal of the image) */
    float float_deltarho = 2.0f * (sqrtf(image_edgemap->uint_xres * image_edgemap->uint_xres + image_edgemap->uint_yres * image_edgemap->uint_yres)) / uint_binsrho;

    /* allocate memory sapce for the hough map
     * and set its size in x and y
     */
    allocate_image_p2(image_houghmap, uint_binstheta, uint_binsrho, 0);

    /* iterate all pixels of the input image */
    for(i = 0; i < image_edgemap->uint_yres; i ++) {
        for(j = 0; j < image_edgemap->uint_xres; j ++) {

            /* check if we are on an edge otherwise do nothing */
            if(image_edgemap->int_image_data[i][j] == 255) continue;

            /* iterate the angle theta */
            for(k = 0; k < image_houghmap->uint_xres; k ++) {
                
                float_theta = k * float_deltatheta;
                float_rho = j * cosf(float_theta) + i * sinf(float_theta);
                int_y = (int)(image_houghmap->uint_yres / 2.0f) - (float_rho / float_deltarho) + 0.5f;

                //printf("i: %d, j: %d, k: %d, int_y: %d\n", i, j, k, int_y);
                /* possible extension: compute the int Hough transform
                 * and compress it into 8 bits
                 */
                //if(image_houghmap->int_image_data[int_y][k] > 0) {
                image_houghmap->int_image_data[int_y][k] ++;
                //}
                image_houghmap->uint_max = MAX(image_houghmap->int_image_data[int_y][k], image_houghmap->uint_max);
            }
        }
    }

    return;
}


/*-----------------------------------------
 * Generic netPBM functions
 *---------------------------------------*/
// read image header
int read_PBM_header(FILE* file_input, image* image_input) {
    char* char_buffer;
 
    char_buffer = (char*)malloc(INT_BUFFERLENGTH * sizeof(char) );

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
    sscanf(char_buffer, "%d %d", &(image_input->uint_xres), &(image_input->uint_yres));

    /* get the max. grey level */
    fgets(char_buffer, INT_BUFFERLENGTH, file_input);
    while(char_buffer[0] == '#') {
         fgets(char_buffer, INT_BUFFERLENGTH, file_input);
    }
    sscanf(char_buffer, "%d", &(image_input->uint_max));

    free(char_buffer);
    return(0);
} 



/*-----------------------------------------
 * functions to deal with a greyscale image
 *---------------------------------------*/
int allocate_image_data(image* image_p2, unsigned int uint_initialgreylevel) {
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

    image_p2->int_image_data = (unsigned int**)malloc(image_p2->uint_yres * sizeof(unsigned int*));
    if(image_p2->int_image_data == NULL) {
        perror("allocate_image: Error allocating storage space.\n");
        return(-1);
    }

    for(i = 0; i < image_p2->uint_yres; i ++) {
        image_p2->int_image_data[i] = (unsigned int*)malloc(image_p2->uint_xres * sizeof(unsigned int));
        if(uint_initialgreylevel == 0) {
            memset(image_p2->int_image_data[i], 0, image_p2->uint_xres * sizeof(unsigned int));
        } else {
            /* why do I have to do this? Since memset sets bytes but an int has more than one byte */
            for(j = 0; j < image_p2->uint_xres; j ++) {
                image_p2->int_image_data[i][j] = uint_initialgreylevel;
            }
        }
    }

    return(0);
}


int allocate_image_p2(image* image_p2, unsigned int uint_xres, unsigned int uint_yres, unsigned int uint_greylevel) {
 
    if(uint_greylevel > 255) {
        perror("allocate_image: the max. grey level is 255.\n");
        return(-1);
    }

    image_p2->uint_xres = uint_xres;
    image_p2->uint_yres = uint_yres;
    image_p2->uint_max = uint_greylevel;    

    return allocate_image_data(image_p2, uint_greylevel);
}


void free_image_P2(image* image_p2) {
    int i = 0;

    for(i = 0; i < image_p2->uint_yres; i ++) {
        free(image_p2->int_image_data[i]);
    }

    free(image_p2->int_image_data);

    return;
}


/*
 * This function reads only the image data.
 */
int read_imagedata_P2(FILE* file_input, image* image_p2) {
    int i = 0;
    int j = 0;

    /* check if the iamge has been allocated */
    if(image_p2->uint_xres == 0 || image_p2->uint_yres == 0) {
        perror("Image not allocated.");
        return(-1);
    }

    for(i = 0; i < image_p2->uint_yres; i ++) {
        for(j = 0; j < image_p2->uint_xres; j ++) {
             if( fscanf(file_input, "%d", &(image_p2->int_image_data[i][j])) == EOF ) {
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

int read_image(char* char_name, image* image_input) {
    FILE* file_input;
    int int_message_length;
    char* char_error_message;

    file_input = fopen(char_name, "r");
    if( file_input == NULL ) {
        /* I am printing a string into 0 allocated bytes.
         * snprintf conviniently reports the number of bytes
         * it is unable to print.
         */
        int_message_length = snprintf(NULL, 0, "Can't open input file: %s\n", char_name);
        char_error_message = malloc(int_message_length);
        sprintf(char_error_message, "Can't open input file: %s\n", char_name); 
        perror(char_error_message);       
        return(-1);
    }
    
    /* read the header information */
    printf("read_image, header of image: %s.\n", char_name);
    if( read_PBM_header(file_input, image_input) != 0 ) {
        perror("Error reading header of image file.\n");
        fclose(file_input);
        return(-1);
    }

    printf("read_image, image resolution x: %d, y: %d, max. grey level: %d\n", image_input->uint_xres, image_input->uint_yres, image_input->uint_max);

    /* allocate image */
    if( allocate_image_p2(image_input, image_input->uint_xres, image_input->uint_yres, 0) != 0 ) {
        perror("Error allocating memory to store image data\n");
        fclose(file_input);
        return(-1);
    }
    printf("Allocated memory\n");

    /* read image data */
    if( read_imagedata_P2(file_input, image_input) != 0 ) {
        perror("Error reading image data\n");
        fclose(file_input);
        return(-1);
    }

    return(0);
}

int write_image_data(FILE* file_output, image* image_p2) {
    unsigned int i = 0;
    unsigned int j = 0;

    /* check if the iamge has been allocated */
    if(image_p2->uint_xres == 0 || image_p2->uint_yres == 0) {
        perror("Image not allocated.");
        return(-1);
    }

    /* write header information */
    fprintf(file_output, "P2\n%d %d\n# CREATOR: binary2ascii\n%d\n", image_p2->uint_xres, image_p2->uint_yres, image_p2->uint_max);

    for(i = 0; i < image_p2->uint_yres; i ++) {
        for(j = 0; j < image_p2->uint_xres; j ++) {
            fprintf(file_output, "%d ", image_p2->int_image_data[i][j]);
            //fwrite(image_p2->int_image_data[i], sizeof(char), image_p2->uint_xres, file_output);
        }
        fprintf(file_output, "\n");
    }

    printf("write_image_data: end\n");

    return(0);
}


int write_image(char* char_name, image* image_p2) {
    FILE* file_output;
    int int_message_length;
    int int_return_value1;
    int int_return_value2;
    char* char_error_message;

    printf("write_image: %s\n", char_name);

    file_output = fopen(char_name, "w");
    if( file_output == NULL ) {
        /* I am printing a string into 0 allocated bytes.
         * snprintf conviniently reports the number of bytes
         * it is unable to print.
         */
        int_message_length = snprintf(NULL, 0, "Can't open output file: %s\n", char_name);
        char_error_message = malloc(int_message_length);
        sprintf(char_error_message, "Can't open output file: %s\n", char_name); 
        perror(char_error_message);       
        return(-1);
    }
    
    /* Only return 0 if both writing the data and closing the file are successful.
     * However, we have to close the file in either case.
     */
    int_return_value1 = write_image_data(file_output, image_p2);
    int_return_value2 = fclose(file_output);

    return(MIN(int_return_value1, int_return_value2));
}


void display_image_p2(image* image_p2) {
    unsigned int i = 0;
    unsigned int j = 0;

    printf("display_image: %d, %d, max. grey level: %d\n", image_p2->uint_xres, image_p2->uint_yres, image_p2->uint_max);

    for(i = 0; i < image_p2->uint_yres; i ++) {
        for(j = 0; j < image_p2->uint_xres; j ++) {
            printf("%u ", image_p2->int_image_data[i][j]);
        }
        printf("\n");
    }

    return;
}


void clone_image_p2(image* image_parent, image* image_child) {
    int i;
    
    /* allocate the child image */
    allocate_image_p2(image_child, image_parent->uint_xres, image_parent->uint_yres, image_parent->uint_max);

    /* deep copy the image data */
    for(i = 0; i < image_child->uint_yres; i ++) {
        memcpy(image_child->int_image_data[i], image_parent->int_image_data[i], image_child->uint_xres * sizeof(int));
    }

    return;
}

/*----------------------------------------*/

// temporary defines
// should implement these as parameters
#define HOUGH_THETA_BINS 10000
#define HOUGH_RHO_BINS 400
#define INVERSE_HOUGH_THRESHOLD 0.9

/* we expect the input file name in argv[1] 
 * the output file names are generated from 
 * the input file name. 
 */
int main(int argc, char *argv[]) {
    int i = 0;
    FILE *file_input;
    char *char_binary_imageline;
    char *char_ascii_imageline;
    unsigned int uint_imageline_in_bytes = 0;

    image image_input;
    image image_edges;
    image image_houghmap;
    image image_foundlines;

    /* We expect the file name in argv[1],
     * otherwise print a meaningful message
     * and exit with an error number set.
     */
    if( argc != 2 ) {
        perror("Usage: binary2ascii infilename\n");
        exit(1);
    }

    /* Get the length of the input filename.
     * We need it later on to create the 
     * output file name.
     */
    size_t size_t_name_length = strlen(argv[1]);

    /* Read the image into a image data structure.
     * All the file handling and memory allocation
     * is done by read_image().
     */
    read_image(argv[1], &image_input);

    /* Canny edge detection and edge tracing with hysteresis*/
    canny(&image_input, &image_edges, EDGE_STOP, EDGE_START);
    write_image("edgemap.pgm", &image_edges);

    hough_transform(&image_input, &image_houghmap, HOUGH_THETA_BINS, hypot(image_input.uint_xres, image_input.uint_yres));
    printf("Hough map resolution x: %d, y: %d, max. grey level: %d\n", image_houghmap.uint_xres, image_houghmap.uint_yres, image_houghmap.uint_max);

    /* write the Hough map to file */
    write_image("hough.pgm", &image_houghmap);

    /* inverse transform */
    clone_image_p2(&image_input, &image_foundlines);
    reverse_transform(&image_foundlines, &image_houghmap, INVERSE_HOUGH_THRESHOLD);
    write_image("foundlines.pgm", &image_foundlines);

    /* close file */
    free_image_P2(&image_input);
    free_image_P2(&image_edges);
    free_image_P2(&image_houghmap);
    free_image_P2(&image_foundlines);

    return(0);
}

