#!/usr/bin/perl

# History
#     4/14/2005 Modify by Moss: replace chop() with $line=~s/[\n\r\f]//g; when move to cygwin 
#                               comment out the code that disconnect OH_(ONE|TWO ...) (might be specific to only Communicator domain

# Performs CINCINNATI => <city>CINCINNATI</city> based on class file
# Reads in class def file for things to tag. The following get's tagged automatically:
#
# 1) single-classed words
#
# 2) words a member of [month_number] when in
#      [month] [month_number]
#      [day_of_week] THE [month_number]
#      [month] THE [month_number]
#      [month_number] OF [month]
#
# 3) words a member of [hour_number] when in
#      [hour_number] [am_pm]
#      [hour_number] O'CLOCK
#      [hour_number] [minute_number] [am_pm]
#      AROUND [hour_number]
#      BEFORE [hour_number]
#      AFTER [hour_number]
#
# 4) words a member of [minute_number] when in
#      [hour_number] [minute_number]
#
# 5) NEW_YORK,NEWARK,WASHINGTON gets [city] even though all are multi-classed, because
#    they were most likely to be cities within the given transcript context 
#    (Master.june98_may99)
#
# Add by Moss
# 6) words a member of [cardinal_number] when follows the phrase 'MY I D NUMBER IS'
#
# Words can also be a member of [unmarked]. This happens when all instances of word in
# the def file are annotated. This is for words like OH that can be used as a ZERO and 
# also can be used as in 'OH MY GOODNESS'. This is basically to make sure that you have
# processed the word and you have decided it shouldn't belong to any class.
#
# Every other classed word is ambiguous, so we print out previous/next lines for the 
# current line, and then print out the current line with {} around the word in question.
# Then you are prompted for what to do. Your options are:
# [q] quit
#     prints the line you quit at to STDOUT and <remember_file>
#     then exits the class tagger. You can continue where you left off by
#     executing the exact same command as before.
# [n] do nothing
#     skips word in question. This might be useful for skipping words you can't decide
#     what to do about. You can then perform a second pass through the tagger in
#     which case you will only get prompted for the words that weren't tagged on the
#     first pass.
# [r] remember line
#     prints the current line from to <remember_file>. The line is taken from 
#     <transcript> and will not any of the changes that you've made. Useful when
#     you want examples of weird cases, or lines you want to edit later on.
# [n] <class_name>
#     Where n is a number. taggs word with <class_name>.

$debug = 0;

if($#ARGV < 3) {
    die "usage:  class_tagger.pl <class_def> <transcript> <tagged_transcript> <remember_file>\n";
}

#get class info
open(CLASS,"$ARGV[0]") || die "couldn't open $ARGV[0] for reading cause $!\n";

while($line = <CLASS>) {
    #Modify by Moss 4/14/2005 when move to cygwin
    #chop($line);
    $line=~s/[\n\r\f]//g;

    next if($line eq "");

    #start of class
    if($line =~ /^LMCLASS \[(\S+)\]/) {
	$class = $1;
	if($debug) {print "got [$class]\n";}

    #end of a class
    } elsif($line =~ /^END \[(\S+)\]/) {
	if($class ne $1) {die "error parsing class $class\n";}
	if($debug) {print "got [$1] end\n";}
	if($debug) {push(@classes,$class);}

    } else {
	#it's annotated
	if($line =~ /^(\S+):\S+/) {
	    $word = $1;
	    $word =~ tr/a-z/A-Z/;
	    $word2annotCount{$word}++;
	    if($debug) {print "[$word] in [$class] is annotated\n";}

	#its a regular word
	} elsif($line =~ /^(\S+)/) {
	    $word = $1;
	    #print "got $word\n";
	    $word =~ tr/a-z/A-Z/;

	#it's a parse error
	} else {
	    die "bad class line: $line\n";
	}

	#add $class to list of classes for $word
	if($class eq "") {die "error parsing: not inside a class\n";}
	$word2class_list{$word} .= "$class,";
	$word2classCount{$word}++;

	$classed_words{$word} = 1; #just holds unique list of classed words
    }
}

close(CLASS);

#die "done\n";

#if annotCount == classCount then add unmarked class
foreach $word (keys(%classed_words)) {
    if($word2annotCount{$word} == $word2classCount{$word}) {
	#print "$word can be unmarked annot:$word2annotCount{$word} class:$word2classCount{$word}\n";
	$word2class_list{$word} .= "unmarked,";
	$word2classCount{$word}++;

    } elsif($word2classCount{$word} > 1) {
	#print "$word is ambiguous\n";
    }

    #print "$word ==> $word2class_list{$word}\n";
}

if($debug) {

    print "\n\n";
    foreach $word (keys(%word2class_list)) {
	print "$word ==>  $word2class_list{$word}\n";
    }
}


#open tagged file 
open(TAGGED_TRANSCRIPT,"+>>$ARGV[2]") ||
    die "couldn't open/create $ARGV[2] for read/write cause $!\n";

#get line count
$startLineIndex = 0;
while(<TAGGED_TRANSCRIPT>) {$startLineIndex++;}
if(!$debug) {print "startLineIndex = $startLineIndex\n";}

#open inputfile at startLineIndex
open(TRANSCRIPT, "$ARGV[1]") || die "couldn't open $ARGV[1] for reading cause $!\n";

@lines = <TRANSCRIPT>;
close(TRANSCRIPT);
$lastLineIndex = $#lines;
if(!$debug) {print "lastLineIndex = $lastLineIndex\n";}

#open remember file for appending
open(REMEMBER,">>$ARGV[3]") || die "couldn't open $ARGV[3] for appending cause $!\n";

#start tagging
#$lineIndex = $startLineIndex;
#while($lineIndex <= $lastLineIndex) {
for($lineIndex = $startLineIndex;$lineIndex <= $lastLineIndex;$lineIndex++) {
    #print "lineIndex = $lineIndex\n";
    $line = $lines[$lineIndex];
    $changed = 0;

    #Modify by Moss 4/14/2005 when move to cygwin
    #chop($line);
    $line=~s/[\n\r\f]//g;

    #comment out, this might be specific to only Communicator domain
    #$line =~ s/ OH_(ONE|TWO|THREE|FOUR|FIVE|SIX|SEVEN|EIGHT|NINE) / OH \1 /g;

    $newline = ""; #line to be modified and printed to TAGGED_TRANSCRIPT
    @words = split(/\s+/,$line);
    #print "\nline: $line\n";
    #print "words: @words\n";
    @tags = ();

    $word_index = 0;
    foreach $word (@words) {

	if($skip_next_word) {$skip_next_word = 0; next;}

	$automatic = 0;
	$classCount = $word2classCount{$word};

	#If the word only has one class, automatically tag.
	if($classCount == 1) {
	    $tmp = $word2class_list{$word};
	    chop($tmp); #chop ','
	    $newline .= "<$tmp>$word</$tmp> ";
	    $tags[$word_index] = $tmp;
	    #$changed = 1;

	#it's a multi-classed word
	} elsif( $classCount > 1) {

	    #automatically tag cases where previous word was a month, current word
	    #can be a month_number and it's the last word within <s> </s>.
	    if(!$automatic && $word2class_list{$word} =~ /month_number,/) {
		if($tags[$word_index-1] eq "month" 
		   #&& $words[$word_index+1] eq "</s>"
		   || (($tags[$word_index-2] eq "day_of_week"
		       || $tags[$word_index-2] eq "month")
		       && $words[$word_index-1] eq "THE")
		   || ($words[$word_index+1] eq "OF" && $word2class_list{$words[$word_index+2]} =~ /month,/)
		   ) {
		    $newline .= "<month_number>$word</month_number> ";
		    $tags[$word_index] = month_number;
		    $changed = 1;
		    $automatic = 1;
		}
	    }

	    #automatically tag cases like TEN in 'TEN [am_pm]' and 'TEN O'CLOCK'
	    if(!$automatic && $word2class_list{$word} =~ /hour_number,/) {
		if($word2class_list{$words[$word_index+1]} =~ /am_pm,/ ||
		   $words[$word_index+1] eq "O'CLOCK" || 
		   ($word2class_list{$words[$word_index+1]} =~ /minute_number,/
		    && $word2class_list{$words[$word_index+2]} =~ /am_pm,/) ||
		   $words[$word_index-1] eq "AROUND" ||
		   $words[$word_index-1] eq "BEFORE" ||
		   $words[$word_index-1] eq "AFTER") {

		    $newline .= "<hour_number>$word</hour_number> ";
		    $tags[$word_index] = hour_number;
		    $changed = 1;
		    $automatic = 1;
		}
	    }

	    if(!$automatic && $word eq "OH"
	       && $tags[$word_index-1] eq "hour_number"
	       && $words[$word_index+1] =~ 
	       /^(ONE|TWO|THREE|FOUR|FIVE|SIX|SEVEN|EIGHT|NINE)$/
	       ) {
		
		$newline .= "<minute_number>OH_$1</minute_number> ";
		$tags[$word_index] = minute_number;
		$tags[$word_index+1] = minute_number;
		$word_index++;
		$skip_next_word = 1;
		$changed = 1;
		$automatic = 1;
	    }

	    if(!$automatic && $word2class_list{$word} =~ /minute_number,/ &&
	       $tags[$word_index-1] eq "hour_number") {
		$newline .= "<minute_number>$word</minute_number> ";
		$tags[$word_index] = minute_number;
		$changed = 1;
		$automatic = 1;
	    }

	    if(!$automatic && $word2class_list{$word} =~ /cardinal_number,/ &&
	       $tags[$word_index-1] eq "cardinal_number" &&
	       $word2class_list{$words[$word_index+1]} =~ /cardinal_number,/) {
		    $newline .= "<cardinal_number>$word</cardinal_number> ";
		    $tags[$word_index] = cardinal_number;
		    $changed = 1;
		    $automatic = 1;		
	    }

	    if(!$automatic && $word eq "NEWARK") {
		if($words[$word_index+1] eq "AIRPORT") {
		    $newline .= "<airport>$word</airport> ";
		    $tags[$word_index] = airport;

		} else {
		    $newline .= "<city>$word</city> ";
		    $tags[$word_index] = city;
		}

		$changed = 1;
		$automatic = 1;		
	    }

	    if(!$automatic && $word =~ /^(NEW_YORK|WASHINGTON)$/) {
		if($word2class_list{$words[$word_index-1]} =~ /city,/) {
		    $newline .= "<state>$word</state> ";
		    $tags[$word_index] = state;

		} else {
		    $newline .= "<city>$word</city> ";
		    $tags[$word_index] = city;
		}

		$changed = 1;
		$automatic = 1;		
	    }

	    #add by Moss March 2001
	    #automatic tags user_id after "MY I_D NUMBER IS" if that word has only 2 classes, user_id and unmarked. All user_id can be unmarked to check for noise
	    if(!$automatic && $word2class_list{$word}=~ /^user_id,unmarked,$/) {
		if($word_index>3
		   &&$words[$word_index-4] eq "MY"
		   && $words[$word_index-3] eq "I_D"
		   && $words[$word_index-2] eq "NUMBER"
		   && $words[$word_index-1] eq "IS"
		   ) {
		    $newline .= "<user_id>$word</user_id> ";
		    $tags[$word_index] = user_id;
		    $changed = 1;
		    $automatic = 1;
		}
	    }

	    #automatic tags cardinal_number after "MY I D NUMBER IS" as cardinal_number 
	    if(!$automatic && $word2class_list{$word}=~ /cardinal_number,/) {
		if($word_index>4
		   &&$words[$word_index-5] eq "MY"
		   && $words[$word_index-4] eq "I"
		   && $words[$word_index-3] eq "D"
		   && $words[$word_index-2] eq "NUMBER"
		   && $words[$word_index-1] eq "IS"
		   ) {
		    $newline .= "<cardinal_number>$word</cardinal_number> ";
		    $tags[$word_index] = cardinal_number;
		    $changed = 1;
		    $automatic = 1;
		}
	    }

	    #not automatic. print the line and surround word with {}.
	    #Then prompt user on what to do.
	    if(!$automatic) {

		&PRINT_SURROUNDING_LINES(6);

		print "\n\n\nline ",$lineIndex+1,": ";

		$i = 0;
		foreach $w (@words) {
		    if($i == $word_index) {
			print "{$w} ";
		    } else {
			print "$w ";
		    }
		    $i++;
		}
		
		print "\n";
		print "replacing {$word} ...\n";
		&TAG();
	    }

	#make no modifications
	} else {
	    $newline .= "$word ";
	}

	$word_index++;
    }
    
    if($newline ne "") {chop($newline);}
    print TAGGED_TRANSCRIPT "$newline\n";
    if($changed) {print "newline: $newline\n";}
    #$lineIndex++;
}

close(TAGGED_TRANSCRIPT);
close(REMEMBER);

print "done at line $lineIndex\n";



sub TAG
{
    %option2class = ();

    print "\noptions: [q] quit [n] do nothing [r] remember line\n";
    $class_list = $word2class_list{$word};
    chop($class_list); # get rid of last ','
    @classes = split(/,/,$class_list);

    $optionNumber = 1;

    #set up and print out the class options
    foreach $class (@classes) {
	$option2class{$optionNumber} = $class;
	print "[$optionNumber] $class ";
	$optionNumber++;
    }
    
    #keep asking till get a valid response
    $incorrect = 1;
    while($incorrect) {
	$incorrect = 0;
	print "? ";
	$option = <STDIN>;
	chop($option);
	
	if($option eq "q") {
	    print REMEMBER "quit at line ",$lineIndex+1,"\n";
	    die "quit at line ",$lineIndex+1,"\n";

	#don't change word
	} elsif($option eq "n") {
	    $newline .= "$word ";

	} elsif($option eq "r") {
	    $incorrect = 1;
	    print REMEMBER "line ", $lineIndex+1, ": $line\n";
	    print "remember complete; now what ";

	#check class options
	} else {
	    $class = $option2class{$option};
	    
	    if($class ne "") {
		$changed = 1;
		$newline .=  "<$class>$word</$class> ";
		$tags[$word_index] = $class;

	    #bad option
	    } else {
		$incorrect = 1;
		print "bad option <$option>\ntry again ";
	    }
	}
    }
    print "\n";
}

sub PRINT_SURROUNDING_LINES
{
    local($Nlines) = @_;
    local($i);

    print "\n\n\n";

    foreach $i (($lineIndex - $Nlines) .. ($lineIndex + $Nlines)) {
	if($i == $lineIndex) {print "\n<line in question>\n\n";}
	elsif($i < 0 || $i > $lastLineIndex) {print "<empty>\n";}
	else {
	    print "$lines[$i]";
	}
	
    }
}
