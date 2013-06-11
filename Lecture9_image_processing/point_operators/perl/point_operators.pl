#!/usr/bin/env perl -w
# image point operators

use strict;

# All the routines to read / write an image from / to a file 
# and to compute its histogram are in the file image_p2.pl.
require "image_p2.pl";

# Read an image from file 
my %image;
my %histogram;
print "main, read file ".$ARGV[0]."\n";
read_image_p2($ARGV[0], \%image);

# Print some header information to screen
print "Done reading image. x: ".$image{int_xres}.", y: ".$image{int_yres}.
    ", max grey level: ".$image{int_max_grey}."\n";

# Compute the image histogram and convert it into an image
compute_histogram(\%image, \%histogram);
histogram_to_image(\%image, \%histogram, 400);

# Write the generated image to file
write_image_p2(\%image, "histogram_example.pgm");

# We are done
print "Done computing histogram. The histogram has been written to histogram_example.pgm.\n"
