#!/usr/bin/perl -w

if(@ARGV<1){
    print "$0 <input>\n";
    exit(-1);
}
use strict;

my ($input)=@ARGV;

open(INPUT,"$input");
my @inputlines=<INPUT>;
close(INPUT);

foreach my $input (@inputlines)
{
    chomp($input);

    $input=filter($input);    
    print "$input\n";

}

sub filter()
{
    my ($line)=@_;
    $line =~ s/[\!\"\&\'\(\)\,\-\:\;\?\[\]_\`]/ /g;
    $line =~ s/\./ <end>/g;
    $line =~ tr/A-Z/a-z/;

    if($line =~ /^VOLUME/){$line ="";}
    if($line =~ /^CHAPTER/){$line ="";}

    return $line;
    
}

