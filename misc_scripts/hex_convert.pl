#!/usr/bin/perl

#-- Check the input arguments
if ($#ARGV != 2){
  die "Incorrect Syntax.\nMap.pl <Mode: 1~6> <Input File> <Output File>\n";
}


$mode = $ARGV[0];
$infile = $ARGV[1];
$outfile = $ARGV[2];


open (INFILE, $infile) || die "Unable to open input file: $infile\n";
$output = "";

open (OUTPUTFILE, ">$outfile") || die "Cannot open $outfile";
flock (OUTPUTFILE, LOCK_EX);


if($mode == 1){  ### GB -> HEX for lexicon
  while ($infile = <INFILE>){
    chomp($infile);
    @frag = split(/ |\t/, $infile);
    $temphex = HexFn->str2hex(@frag[0])."\t";
    for($i = 1; $i <= $#frag; $i++){
      $temphex .= @frag[$i]." ";
    }
    chop($temphex);
    $output = $temphex."\n";
    print OUTPUTFILE $output;
  }
}elsif($mode == 2){  ### GB -> HEX for transcripts
  while ($infile = <INFILE>){
    chomp($infile);
    if ($infile =~ s/(\([^\)]*\))$//) { $uttid = $1 };
    @frag = split(/ /, $infile);
    $temphex = "";
    foreach $frag(@frag){
      if ($frag =~ m,^</?.*>|\+\+.*\+\+$,) {
	$temphex .= $frag." ";
      }else{
	$temphex .= HexFn->str2hex($frag)." ";
      }
    }
    chop($temphex);
    $output = $temphex.$uttid."\n";
    print OUTPUTFILE $output;
  }
}elsif($mode == 3){   ### GB -> HEX for text 
  # mode=3 indicates direct conversion of unicode to hex code (not in transcript format etc)
  while ($infile = <INFILE>){
    chomp($infile);
    @frag = split(/ /, $infile);
    $temphex = "";
    foreach $frag(@frag){
      $temphex .= HexFn->str2hex($frag)." ";
    }
    chop($temphex);
    $output = $temphex."\n";
    print OUTPUTFILE $output;
  }
}elsif($mode == 4){   ### HEX -> GB for text
  # mode=4 indicates reversed direct conversion of hex code to unicode (not in transcript format etc)
  while ($infile = <INFILE>){
    chomp($infile);
    @frag = split(/ /, $infile);
    $temphex = "";
    foreach $frag(@frag){
      $temphex .= HexFn->hex2str($frag)." ";
    }
    chop($temphex);
    $output = $temphex."\n";
    print OUTPUTFILE $output;
  }
}elsif($mode == 5){   ### HEX -> GB for lexicon
  while ($infile = <INFILE>){
    chomp($infile);
    @frag = split(/ |\t/, $infile);
    $temphex = HexFn->hex2str(@frag[0])."\t";
    for($i = 1; $i <= $#frag; $i++){
      $temphex .= @frag[$i]." ";
    }
    chop($temphex);
    $output = $temphex."\n";
    print OUTPUTFILE $output;
  }
}elsif($mode == 6){   ### HEX -> GB for transcripts
  while ($infile = <INFILE>){
    chomp($infile);
    if ($infile =~ s/(\([^\)]*\))$//) { $uttid = $1 };
    @frag = split(/ /, $infile);
    $temphex = "";
    foreach $frag(@frag){
      if ($frag =~ m,^</?.*>|\+\+.*\+\+$,) {
	$temphex .= $frag." ";
      }else{
	$temphex .= HexFn->hex2str($frag)." ";
      }
    }
    $output = $temphex.$uttid."\n";
    print OUTPUTFILE $output;
  }
}else{
  die "Invalid Mode.\n";
}
close INFILE;


#-- Write output
#open (OUTPUTFILE, ">$outfile") || die "Cannot open $outfile";
#flock (OUTPUTFILE, LOCK_EX);
#print OUTPUTFILE $output;
close (OUTPUTFILE);

package HexFn;

sub chex{
  $number = $_[0];

  if ($number < 0 | $number > 15){
    die "chex: Function argument cannot be smaller than 0 or larger than 15.\n";
  }elsif ($number > 9){
  CASE:{
      return "a" if ($number == 10);
      return "b" if ($number == 11);
      return "c" if ($number == 12);
      return "d" if ($number == 13);
      return "e" if ($number == 14);
      return "f" if ($number == 15);
    }
  }else{
    return $number;
  }
}

sub rchex{
  $hex = $_[0];
  
 CASE:{
    return 10 if ($hex eq "a");
    return 11 if ($hex eq "b");
    return 12 if ($hex eq "c");
    return 13 if ($hex eq "d");
    return 14 if ($hex eq "e");
    return 15 if ($hex eq "f");
    return $hex;
  }
}

sub str2hex{
  $inString = $_[1];
  
  @string = split(//, $inString);
  $outString = "";
  foreach $string(@string){
    $high_hex = chex(ord($string) >> 4);
    $low_hex = chex(ord($string) & 0xf);
    $outString .= $high_hex.$low_hex;
  }
  
  return $outString;
}

sub hex2str{
  $inString = $_[1];
  
  @string = split(//, $inString);
  $outString = "";
  for($i = 0; $i <= $#string; $i+=2){
    $high_hex = @string[$i];
    $low_hex = @string[$i+1];
    $value = (rchex($high_hex) << 4) + rchex($low_hex);
    $outString .= chr($value);
  }

  return $outString;
}

1;
