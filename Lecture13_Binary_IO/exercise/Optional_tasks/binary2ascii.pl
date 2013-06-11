#! /usr/bin/perl -w
# Binary file I/O

# magic numbers
$BITS_PER_BYTE = 8;

#open the input file
open(FILE, $ARGV[0]) or die "Can't open file for reading $!";
open(FILEOUT, ">".$ARGV[1]) or die "Can't open file for writing $!";

# read the header in ASCII mode
$line = <FILE>;

# break the line into tokens
@words_in_this_line = split(" ", $line);
if ($words_in_this_line[0] ne "P4") {
    # print error message and exit
}

$line = <FILE>;
@words_in_this_line = split(" ", $line);
while($words_in_this_line[0] eq '#') {
    $line = <FILE>;
    @words_in_this_line = split(" ", $line);
}

# get the resolution 
@words_in_this_line = split(" ", $line);
$int_xres = $words_in_this_line[0];
$int_yres = $words_in_this_line[1];

# sanity check on the resolution
if($int_xres == 0 || $int_yres == 0) {
  # print error message and exit
}

# write the header
print FILEOUT "P1\n";
print FILEOUT "# File ".$ARGV[0]." converted by binary2ascii.pl\n";
print FILEOUT $int_xres." ".$int_yres."\n";

# length of a line in byte
$int_line_length = int($int_xres / $BITS_PER_BYTE + 0.99); # perl does not know about floor or ceil

# just for debug
# print $int_line_length."\n";

# we read the header, so we know the rest is data
# switch to binary mode to read the rest of the file
# in chunks of a line
binmode(FILE);

while(($n = read(FILE, $buffer, $int_line_length)) != 0) {

     #---------------------------------
     # Your conversion code goes here! 
     #---------------------------------

     # you have got a character array full of binary data 
     # your job is to take this data character by character and convert 
     # and print out every bit of it  
     # To do this use the algorithm continually dividing a decimal number by 2 
     # to get it binary representation 

     # then use something like print FILEOUT <parts missing>; to write the data to a file 
}

close(FILE);
close(FILEOUT);
