#--------------------------------------------
# Sketch af a generic spatial filter function
#--------------------------------------------

Global variables:
$int_sizex; # width of the filter kernel
$int_sizey; # height of the filter kernel

# Parameters (you may have to add some):
# Reference to the input image
# Reference to the filtered image
sub image_filter {
    # dereference
    my %image = %{ $_[0] };

    foreach my $i (0...$image{int_yres} - 1) {
        foreach my $j (0...$image{int_xres} - 1) {

            if( $i <= $int_sizey / 2 || $i >= $image{int_yres} - $int_sizey / 2 || $j <=  $int_sizex / 2 || $j >= $image{int_xres} - $int_sizex / 2) {
                # do something at the border
            } else {
                # filter the pixel
            }

        }
    }

    # make the filtered image complete
    ${ $_[1]}{int_xres} = $image{int_xres};
    ${ $_[1]}{int_yres} = $image{int_yres};
    ${ $_[1]}{int_max_grey} = $image{int_max_grey};
}

