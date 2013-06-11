#! /usr/bin/perl
use strict;

########################################################
#

my $file_buffer = "";

#The number of chars to read at a time
my $buffer_length = 40;

#The filename to be read is a command line argument
my $input_file = $ARGV[0];

my($x_rez, $y_rez, $bit_depth);



#########################################################
#

open(FILE, "<$input_file");

($x_rez, $y_rez, $bit_depth) = parseHeader();
print "This image is $x_rez by $y_rez\n";
print "Each pixel can take one of $bit_depth possible values\n\n";

my $return_val = 0;
do
{ 
	$return_val = read(FILE, $file_buffer, $buffer_length, 0);
	printRawData($file_buffer);
	print "-------\n$return_val\n--------\n";
}while($return_val >= $buffer_length);

close(FILE);


#########################################################
# SUBS

sub parseHeader
{
	#Read a line from the file
	my $string = <FILE>;

	#check to see if the file is P6 format
	if($string =~ /^P6/)
	{
		print "Reading a PPM file, format P6, name $input_file\n";
	}
	else
	{
		print "Not a P6 format PPM file\n";
	}

	#keep reading lines to get ignore comments etc.
	do
	{
		$string = <FILE>;
	}while($string =~ /^#/);

	#$string should now contain the x and y rez of the file	
	chomp $string;
	my($x,$y) = split(/ /, $string);

	#Read the next line to get the bit depth
	$string = <FILE>;
	my $depth = $string;
	chomp $depth;

	return($x, $y, $depth);
}

sub printRawData 
{
	my $buffer = $_[0];
	my $buffer_length = length($buffer);
	my @rgb_colour = (0,0,0);
	my $current_colour = 0;
	my $current_pixel = 0;

	#print "Buffer is $buffer_length elements long\n";
	#print "in prd, buffer = $buffer\n";
	foreach my $byte (split(//, $buffer)) 
	{

#		print "$byte :";
#		print "<cr>" if $byte eq "\n";

		#my $temp = (3 - ($current_colour % 3));
		my $temp = ($current_colour % 3);
		#Clear pixel buffer if we've got a complete pixel
		if($temp == 0)
		{
			print "Clearing buffer\n";
			$current_colour = 0;
			$rgb_colour[0] = 0;
			$rgb_colour[1] = 0;
			$rgb_colour[2] = 0;

		}
		print "temp $temp    --- ";
		$rgb_colour[$temp] = ord($byte);
		#print "=======";
		#print "$temp\n";
		if($byte eq "\n")
		{	
			print "<cr> :" if $byte eq "\n";
		}
		else
		{
			print "$byte :";
		}
		printf("%02x ", ord($byte));
		print("\n");

		print "byte : $current_pixel ($rgb_colour[0],$rgb_colour[1],$rgb_colour[2])\n";
		print "------------------------\n";
		$current_colour++;
		$current_pixel++;
		
	}
}#printRawData
