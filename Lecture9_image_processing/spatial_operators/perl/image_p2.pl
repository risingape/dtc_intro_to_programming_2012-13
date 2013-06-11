#---------------------------------
# House keeping functions reading
# and writing a netPBM P2 image as
# well as computing a histogram
# and converying it into an image. 
#---------------------------------
use strict;

#---------------------------------
# Data structures
#---------------------------------

# We use hashes to encapsulate everthing
# that belongs to an image and a histogram, 
# respectively. However, hashes
# may only contain scalar values, so
# we have to place a reference to our data 
# array instead of the array itself.
#%image = (
#    int_xres       => 0,
#    int_yres       => 0,
#    int_max_grey   => 0,
#    int_image_data => (),
#);

#%histogram = (
#    int_number_of_bins => 0,
#    int_histogram_data => (),
#);


#-------------------------------
# Auxiliary function to check if
# a given value is an integer
#-------------------------------

# Parameters:
# val
sub isint {
    my $val = shift;
    return($val =~ m/^\d+$/);
}


#--------------------------
# Read an image from a file
#--------------------------

# Parameters:
# input filename as string
# Reference to the image
sub read_image_p2 {
    #define local variables only visible within this function
    my $file_in;
    my $file_name = $_[0];
    my $image     = $_[1];

    # open the file. If we fail exit and print an error message 
    open($file_in, $file_name) or die "Can't open file ".$file_name.
        " for reading $!. Exit!";

    # read image header
    my $line = <$file_in>;

    # break the line into tokens
    my @words_in_this_line = split(" ", $line);
    if ($words_in_this_line[0] ne "P2") {
        # print error message and exit
        die "Not a P2 image. Exit!";
    }

    $line = <$file_in>;
    @words_in_this_line = split(" ", $line);
    while($words_in_this_line[0] eq '#') {
        $line = <$file_in>;
        @words_in_this_line = split(" ", $line);
    }

    # get the resolution
    @words_in_this_line = split(" ", $line);
    my $int_xres = $words_in_this_line[0];
    my $int_yres = $words_in_this_line[1];

    # sanity check on the resolution
    if(!isint($int_xres) || !isint($int_yres)) {
        # print error message and exit
        die "At least one image dimension is not an integer value. Exit!";
    }

    if($int_xres == 0 || $int_yres == 0) {
        # print error message and exit
        die "At least one image dimension is 0. Exit!";
    }

    # get the max grey level
    $line = <$file_in>;
    @words_in_this_line = split(" ", $line);
    while($words_in_this_line[0] eq '#') {
        $line = <$file_in>;
        @words_in_this_line = split(" ", $line);
    }
    my $int_max_grey = $words_in_this_line[0];
    
    # sanity check on the max grey level
    if(!isint($int_max_grey)) {
        # internal error
        die "The max grey level is not an integer value. Exit!";
    }
    
    if($int_max_grey < 0 || $int_max_grey > 255) {
        # max grey level out of range
        die "The max grey level is out of range. Exit!";
    }
    
    print STDERR "read_image_p2, done read image header, x: ".$int_xres.
        ", y: ".$int_yres.", max grey: ".$int_max_grey."\n";

    # Read image data - we do not need to switch to binary mode

    # for each line we need to split it along the blank characters
    # this code only works if the pixels are not seperated by "\n"

    # the GIMP export filter writes only one pixel per line
    # I think this violates the standard
    # So, read the first line and check its length.

    my @line;
    my @int_array_image_data;
    my @pixels_in_line;
    # for each line
    foreach my $i (0...$int_yres - 1) {
        
        # for each pixel
        foreach my $j (0...$int_xres - 1) {
            @line = split(" ", <$file_in>);
            if(scalar(@line) == $int_xres) {
                @pixels_in_line = @line;
                last; # leave loop immediately, like break in C
            } else {
                push @pixels_in_line, $line[0];
            }
        }
        push @int_array_image_data, [ @pixels_in_line ];
        undef(@pixels_in_line); # to prevent the array from growing we 
                                  # have to empty it
    }
  
    # set the reference in the image hash
    $image->{int_xres}       = $int_xres;
    $image->{int_yres}       = $int_yres;
    $image->{int_max_grey}   = $int_max_grey;
    $image->{int_image_data} = \@int_array_image_data;
    
    close($file_in);
}


#--------------------------
# Write the image to a file
#--------------------------

# Parameters:
# reference to image hash
# output file name as string
sub write_image_p2 {
    # dereference input parameters
    my $image    = $_[0];
    my $filename = $_[1];

    #open output file for reading
    my $file_out;
    if (scalar(@_) == 2) {
        open($file_out, '>', $filename) or die "Can't open file ".
            $filename." for writing $!. Exit!";
    } else {
        $file_out = \*STDOUT;
    }

    # write image header
    print $file_out "P2\n# created by contrast_stretching.pl\n".
        $image->{int_xres}." ".$image->{int_yres}."\n".
        $image->{int_max_grey}."\n";

    foreach my $i (0...$image->{int_yres} - 1) {
        foreach my $j (0...$image->{int_xres} - 2) {
            print $file_out $image->{int_image_data}[$i][$j]." ";
        }
        print $file_out $image->{int_image_data}[$i][$image->{int_xres} - 1];
        print $file_out "\n";
    }


    if(scalar(@_) == 2) {
        close $file_out;
    }
}


#-------------------------
# Compute image histogram
#-------------------------

# Parameters:
# reference to image hash
# reference to histogram hash
sub compute_histogram {
    my $image     = $_[0];
    my $histogram = $_[1];

    print STDERR "compute histogram, xres: ".$image->{int_xres}.", yres: ".
        $image->{int_yres}.", max grey: ".$image->{int_max_grey}."\n";
    
    # initialise the array to 0
    my @int_histogram_bins = (0) x ($image->{int_max_grey} + 1);
    
    foreach my $i (0...$image->{int_yres} - 1) {
        foreach my $j (0...$image->{int_xres} - 1) {
            $int_histogram_bins[ $image->{int_image_data}[$i][$j] ] ++;
        }
    }

    print STDERR "compute histogram, bins:\n";
    foreach my $i (0...$image->{int_max_grey}) {
        print STDERR "bin ".$i." freq: ".$int_histogram_bins[ $i ]."\n";
    }

    # set the number of bins and reference to the histogram 
    # data array in the histogram hash
    $histogram->{int_number_of_bins} = $image->{int_max_grey} + 1;
    $histogram->{int_histogram_data} = \@int_histogram_bins;
}


#----------------------------------
# Convert a histogram into an image 
#----------------------------------

# Parameters:
# reference to image hash
# reference to histogram hash
# desired y resolution of the histogram image
sub histogram_to_image {
    # dereference
    my $image      = $_[0];
    my $histogram = $_[1];
    my $int_yres  = $_[2];

    # find the max frequency
    my @int_array_sorted = sort{$b <=> $a} 
        @{$histogram->{int_histogram_data}};
    my $int_max = $int_array_sorted[0];
    print STDERR "histogram to image, number of bins: ".
        $histogram->{int_number_of_bins}.", max frequency: ".$int_max."\n";

    # precompute the height of each column
    my @height;
    my $float_increment = $int_max / $int_yres;
    foreach my $i (0...$histogram->{int_number_of_bins} - 1) {
        $height[$i] = int( $histogram->{int_histogram_data}[ $i ] /
            $float_increment + 0.5 );
    }

    # prepare a line of the image
    my @int_array_image_data;
    my @int_pixel_line;
    my $height = 0;
    foreach my $j (0...$int_yres) {
        # fill the line of the image
        foreach my $i (0...$histogram->{int_number_of_bins} - 1) {
        
            if( $height[$i] <= $int_yres - $j) {
                $int_pixel_line[$i] = 255;
            } else {
                $int_pixel_line[$i] = 0;
            }
        }
 
        # push the line to the data array
        push @int_array_image_data, [ @int_pixel_line ];
    }

    $image->{int_xres}       = $histogram->{int_number_of_bins};
    $image->{int_yres}       = $int_yres;
    $image->{int_max_grey}   = 255;
    $image->{int_image_data} = \@int_array_image_data;
}

1;

