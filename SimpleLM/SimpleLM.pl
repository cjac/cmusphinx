#!/usr/local/bin/perl5

# Ricky Houghton, Carnegie Mellon University (Feburary 2nd, 2000)
# This is a based on a version that Alex Hauptmann wrote.

# This script should work for most simple cases, however there are
# many problems/concerns that need to be addressed for larger sets of
# data.
#

# 0.) This is an early version, it could use a bit of cleaning
# up. Hopefully many will find it useful as is.

# 1.) This script can not deal with words not in cmu_dict. A future
# version will create pronunciations for OOV words on the fly. 
# 

# 2.) This script does not merge a smaller corpus with a general
# corpus. This merging step is actually quite important. Even with a
# tight corpus, there is a real benefit to merging with a general
# language model. I will release MergeLM once I find a good general
# text set for merging and we have a pronunciation generator.

# 3.) This script does not deal with text normalization. For hand
# crafted corpora this should not be a problem. However, if you want
# to build an LM for recognizing something like NPR, and would like to
# use text from the WEB as a source of language data, it should be
# normalized. This process would convert the number "100,000", to "one
# hundred thousand", "www.cmu.edu" would become "w w w dot c m u dot e
# d u", or maybe "w w w dot c m u dot ed u". That is, it would attempt
# to convert numbers and symbols to text strings that the recognizer
# might return. (Note, awb has a text normalizer that can be used,
# however I didn't have time to incorporate it into the script
# tonight. Other things popped up.

# 4.) I've made no real effort to allow for parameter passing.
# Anyone interested?

# INPUT: a text file

  # The input text can be just about any text document. It attempts to
  # deal with punctuation and other pieces. It doesn't require special
  # formating.

# OUTPUT: a .dict a .arpabo and a .arpabo.DMP file
  
  # These files are what are needed for SPHINX



$CLEAN_UP = 1;			# Delete misc files when done
$CMU_DICT = "/usr/share/sphinx/cmudict/cmudict.0.5.sphinx"; # Point to your cmudict
$OUTPUT_NAME = "newLM";		# 
$NUM_WORDS_TO_KEEP = 30000;	# Max size of .dict to create.
$INPUT_TEXT = "input.txt";
$BIN_DIR = "/usr/share/sphinx/bin";

$input_text = $INPUT_TEXT unless shift;
$output_name = $OUTPUT_NAME unless shift;


$processing_dir = "/tmp/$OUTPUT_NAME";
mkdir ($processing_dir,0777) unless -d "$processing_dir";

chdir ("$processing_dir") || die "Can't find output directory ($processing_dir)";

system ("unlimit");

if (! -s "ccs.ccs") {
    open CCS,">ccs.ccs";
    print CCS "<s>";
    close CCS;
}


{
    die "${input_txt} was not found, cannot continue." unless -s "${input_text}";
    print "Creating ${input_text}.lc\n";

    # This is the preferred command, however I don't have a textnormal program that is distributable.
    
    #    system ("sed -f $BIN_DIR/punctrem.sed < $input_text | tr \"[A-Z]\" \"[a-z]\" | $BIN_DIR/textnormal | tr \"[A-Z]\" \"[a-z]\" | tr -d \'\\r\' | sed \'s/\\,/ /g\' | sed \'s/\\./ /g\' | gawk \' \$1 != \"\" { print \"<s>\", \$0, \"</s>\"; } ' > $input_text.lc");
    system ("sed -f $BIN_DIR/punctrem.sed < $input_text | tr \"[A-Z]\" \"[a-z]\" | tr -d \'\\r\' | sed \'s/\\,/ /g\' | sed \'s/\\./ /g\' | gawk \' \$1 != \"\" { print \"<s>\", \$0, \"</s>\"; } ' > $input_text.lc");
}



# Create .wfreq
{
    die "${input_txt}.lc was not created, cannot continue" unless -s "${input_text}.lc";
    print "Creating ${input_text}.wfreq\n";
    system ("cat $input_text.lc | $BIN_DIR/text2wfreq | sort -T . > $input_text.wfreq");
}



# Create .$NUM_WORDS_TO_KEEP.vocab
{
    die "${input_txt}.wfreq was not created, cannot continue" unless -s "${input_text}.wfreq";
    print "Creating ${input_text}.$NUM_WORDS_TO_KEEP.vocab\n";
    system ("cat $input_text.wfreq | $BIN_DIR/wfreq2vocab -top $NUM_WORDS_TO_KEEP | tail +5 | grep -i \"[a-z]\" > $output_name.$NUM_WORDS_TO_KEEP.vocab");
}


{
    print "Creating $output_name.$NUM_WORDS_TO_KEEP.dict\n";

    open OUTPUT,">$output_name.$NUM_WORDS_TO_KEEP.dict";

    # Slurp the whole thing, makes life easier...unless you are without enough swap
    open DICT,"$CMU_DICT"|| die "Can not open cmu_dict ($CMU_DICT)\n";
    @dict = <DICT>;
    close DICT;

    for (@dict) {		# Create a hash of the dict entries
	/(\S+)\s+(.*)$/;
	$d{lc($1)} = $2;
    }

    open VOCAB,"$output_name.$NUM_WORDS_TO_KEEP.vocab";
    while (<VOCAB>) {
	chomp;
	if ($d{$_}) {
	    printf OUTPUT "%-30s$d{$_}\n",$_;
	    
	    # Dictionary might contain multiple pronunciations of the same
	    # word. Each version has a (counter) appended to the word e.g. 
	    # WITH                     	W IH DH
	    # WITH(2)                  	W IH TH
	    # WITH(3)                  	W IX DH
	    # WITH(4)                  	W IX TH
	    $i=2;			
	    while ($dup = $d{"$_($i)"}) {
		printf OUTPUT "%-30s$dup\n","$_($i)";
		$i++;
	    }
	}
    }
    close VOCAB;
    close OUTPUT;
}


die "$output_name.$NUM_WORDS_TO_KEEP.dict was not created correctly, cannot continute" 
    unless -s "$output_name.$NUM_WORDS_TO_KEEP.dict";


system ("sed 's/.*\$/&|/' $output_name.$NUM_WORDS_TO_KEEP.dict | tr \'|\' \'\\015\' > $output_name.$NUM_WORDS_TO_KEEP.pc.dict");


system ("cat $input_text.lc | $BIN_DIR/text2wngram -temp ./ > $input_text.wngram") 
    unless -s "$input_text.wngram";

{
    die "${input_text}.wngram was not created correctly." unless -s "${input_text}.wngram";
    
    system ("cat ${input_text}.wngram | $BIN_DIR/wngram2idngram -vocab $output_name.$NUM_WORDS_TO_KEEP.vocab -temp ./ > $output_name.$NUM_WORDS_TO_KEEP.idngram");
}


$voctype = 1;			# Allow OOVs - this version must allow OOVs
$uni_disc = 1;
$bi_disc = 7;
$tri_disc = 7;

$smoothing = "-good_turing";	
#    $smoothing = "-linear";
#    $smoothing = "-absolute";
#    $smoothing = "-witten_bell";

$NumWords = $NUM_WORDS_TO_KEEP;


# Note for large amount of text, idngram2lm sometimes fails. When this
# happens it is often fruitful to attempt different cutoffs. It would
# be possible to add a for ($i=0;...) loop around the program trying
# different cutoffs until some set worked. 

if ($smoothing eq "-good_turing") {
    system ("$BIN_DIR/idngram2lm -vocab $output_name.$NumWords.vocab -idngram $output_name.$NumWords.idngram -arpa $output_name.$NumWords.arpabo -vocab_type $voctype $smoothing -disc_ranges $uni_disc $bi_disc $tri_disc -calc_mem -context ccs.ccs -four_byte_counts -verbosity 1");
} else {
    system ("$BIN_DIR/idngram2lm -vocab $output_name.$NumWords.vocab -idngram $output_name.$NumWords.idngram -arpa $output_name.$NumWords.arpabo -vocab_type $voctype -smoothing -calc_mem -context ccs.ccs -four_byte_counts -verbosity 2");
}


$_ = `wc $output_name.$NumWords.dict`;

($num_lines,$num_words,$num_chars,$foo) = split;

system ("mv $output_name.$NumWords.arpabo $output_name.$num_lines.arpabo");
system ("mv $output_name.$NumWords.dict $output_name.$num_lines.dict");

system ("$BIN_DIR/lm3g2dmp $output_name.$num_lines.arpabo .");

$size = -s "$output_name.$num_lines.arpabo";
print "Size of file: $size\n";
print "NOW TRY $output_name.$num_lines.arpabo\n";

if ($CLEAN_UP) {
    system ("rm *.lc *.ccs *.idngram *.wfreq *.vocab *.wngram *.pc.dict");
}



exit (0);
