#!/usr/bin/perl
# This line tells emacs, that this is a -*-Perl-*- program
# [20080707] (air) some minor error checking

$licence = <<LICENCE;

Copyright (C) 1996 Interactive System Labs and Klaus Ries

LICENCE

$n_default=10;
$break_prob_default = 0;
$leave_out_prob_default = 0.1;
$repeat_prob_default = 0.25;

$n=$n_default;
$break_prob = $break_prob_default;
$leave_out_prob = $leave_out_prob_default;
$repeat_prob = $repeat_prob_default;


while ($ARGV[0] =~ /^-.*$/) {   # PROCESS FLAGS
    $_ = shift;
    if ($_ eq "-h") {
        $help++;
        next;
    }
    if ($_ eq "-n") {
        $n=shift;
        next;
    }
    if ($_ eq "-leave_out") {
        $leave_out_prob=shift;
        next;
    }
    if ($_ eq "-repeat") {
        $repeat_prob=shift;
        next;
    }
    if ($_ eq "-show_break") {
        $show_break++;
        next;
    }
    if ($_ eq "-d") {
        $dir=shift;
        next;
    }
    if ($_ eq "-forms") {
        $forms=shift;
        next;
    }
    if ($_ eq "-warn") {
        $warn++;
        next;
    }
    if ($_ eq "-capital") {
        $capital++;
        next;
    }
    if ($_ eq "-dictionary") {
        $dictionary++;
        next;
    }
    if ($_ eq "-makedict") {
        $makedict++;
        next;
    }
    if ($_ eq "-unkdict") {
        $unkdict++;
        next;
    }
    if ($_ eq "-vocabulary") {
        $vocabulary++;
        next;
    }
    if ($_ eq "-class") {
        $classes++;
        next;
    }
    if ($_ eq "-noclass") {
        $noclasses++;
        next;
    }
    if ($_ eq "-novariants") {
        $novariants++;
        next;
    }
    if ($_ eq "-iclass") {
        $inverse_map++;
        next;
    }
    if ($_ eq "-modifyLM") {
        $modify++;
        next;
    }
    if ($_ eq "-clausi") {
        $clausi++;
        next;
    }
    if ($_ eq "-grammarfile") {
        $grammarfile=shift;
        next; 
    }
    die "Unrecognized command line option $_\nUse generate_random_samples -h for help\n";
}

help() if($help);

if (defined $dir) {
    $currdir=$ENV{PWD};
    die "Cannot change to $dir\n" unless chdir $dir;
    warn "Changed to $dir\n";
}


# First load the forms of that grammer:

$FORMS = "forms";
$FORMS .= ".$forms" if defined $forms;

open FORMS or die "can't open FORMS file $FORMS\n";

while (<FORMS>) {

    $random_perm++ if /%%randomperm%%/;
    
    $prob=get_prob();
    
    if (s/^\s*FUNCTION:\s*(\S+)//) {
        my @F = ();
        my @G = ();
        $name=$1;
        print STDERR "The name of the function is >>$name<<\n";
        undef $nets;
        push(@top_level,$name);
        push(@top_level_prob,$prob);
        push(@top_level_slots,\@F);
        push(@top_level_probs,\@G);
        push(@top_level_perm,$random_perm);
        undef $random_perm;
        $top_level{$name}++;
    }    
    next unless defined $name;
    
    if (/^\s*NETS:/) {
        $nets++; next;
    } elsif ($nets) {
        1;
    } else {
        next;
    }
    
    if (s/^(\t|\s{2,})(\S+)/$2/) {
        $entry=$2;
        if (defined $top_level{$name,$entry}) {
            warn "Multiple entry for $entry in function $name\n" ;
            next;
        }
        push(@{$top_level_slots[$#top_level_slots]},$entry);
        push(@{$top_level_probs[$#top_level_probs]},$prob);
        $top_level{$name,$entry}++;
        $public{$entry}++;
    }
}
close FORMS;

#warn join($/, @{$top_level_slots[$#top_level_slots]}), $/;

normalize_random_entry(\@top_level_prob);

foreach (@top_level_probs) {
    normalize_random_entry($_);
}


die "No top-level slots" unless () ne keys %top_level;

# Now load the nets themselves

$NETS = "nets";

open NETS;
while (<NETS>) {
    next unless /\S/;
    chomp;
    s/\s+$//;
    
    if (defined $nets{$_} || defined $nets{"[$_]"}) {
        warn "Multiple entry for >>$_<< in nets\n" ;
        next;
    }
    $nets{$_}++;
    
    #if(/[^A-Za-z]/) {
    $_="[$_]";
    #}
    $public{$_}++;
}
close NETS;

$nets{"noises"}++ if -r "noises.gra";

@noise=("SegmentNoise","BeginNoise","EndNoise","BreakNoise","RandomNoise");

if ($nets{"noises"}) {
    foreach (@noise) {  
        $public{$_}++;
    }
}

# Next thing: Load all the grammars

#foreach(keys %nets) {
#    print STDERR "Loading grammar file $_ ............\r";
#    
#    $NAME=$_;
#    load_grammar_file("$NAME.gra");
#}
load_grammar_file($grammarfile);

if ($nets{"noises"}) {
    foreach (@noise) {  
        undef $public{$_}  unless defined $rules{$_};
    }
}

chdir $currdir if defined $currdir;
print STDERR "Finished loading grammar file                                         \n";


# Next thing: Generate sentences

srand;

check_grammar() if $warn;


if ($dictionary || $vocabulary || $makedict || $unkdict) {
    # Print all the defined dictionary/vocabulary entries
    foreach $slot_ref (@top_level_slots) {
        foreach (@{$slot_ref}) {
            $slots{$_}++;
        }
    }
    @netname = keys %slots;
    foreach (@noise) {
        push(@netname,$_) if defined $public{$_};
    }

    build_reachable_dictionary();


    open(OUT,"| sort -u | perl -pe 's/\\(0\\)\\}/\\}/;'");
    if ($vocabulary) {
        foreach (keys %reachable_t) {
            print OUT "$_\n";
        }
        print OUT "<s>\n</s>\n";
    } elsif ($dictionary) {
        foreach (keys %reachable_t) {
            if ($class{$_}) {
                $_=join("\n",print_net_all($_));
            }
            print OUT "$_\n";
        }
    } else {    # Makedict !!
        # or unkdict !!
        # Calculate all words that we need from the dictionary
        # Assign an empty list to all of them to get
        # the effective entries

        foreach (keys %reachable_t) {
            next if $class{$_};
            if (/\266/) {     # In case we have entered that phrase to
                # the dictionary
                next if defined $vocab{$_};
                my @F=();
                $vocab{$_}=\@F;
            }
            if (/\266S$/) {
                my $word2=$_;
                $word2=~s/\266S$/\'S/;
                next if defined $vocab{$word2};
                my @F=();
                $vocab{$word2}=\@F;
            }
            foreach (split(/\266/,$_)) {
                next if defined $vocab{$_};
                my @F=();
                $vocab{$_}=\@F;
            }
        }
        foreach (keys %class) {
            next unless $reachable{$_};
            foreach (print_net_all($_)) {
                ($word,$class)=split(/:/,$_,2);
                if ($word=~/\266/) { # In case we have entered that phrase to
                    # the dictionary
                    next if defined $vocab{$word};
                    my @F=();
                    $vocab{$word}=\@F;
                }
                if ($word=~/\266S$/) {
                    my $word2=$word;
                    $word2=~s/\266S$/\'S/;
                    next if defined $vocab{$word2};
                    my @F=();
                    $vocab{$word2}=\@F;
                }
                foreach (split(/\266/,$word)) {
                    next if defined $vocab{$_};
                    my @F=();
                    $vocab{$_}=\@F;
                }
            }
        }

        # Now selectively read the real dictionary from stdin/commandline
        # Also read pronounciation variants

        while (<>) {
            s/^\{?\s*([^\s\}\(]+)(\([^\)]+\))?\s*\}?\s+//;
            $word=$1;
            $ref = $vocab{$word};           
            s/^\s*\{?\s*//;
            s/\s*\}?\s*$//;         
            foreach $phonem (split) {
                next unless $phonem=~/\S/ && $phonem=~/^[a-z]/i;
                $phonem{$phonem}++;
            }
            next unless defined $ref;
            next if $novariants && @{$ref}>0;
            $pron=$_;
            push(@{$ref},$pron) unless grep($_ eq $pron,@{$ref})>0;
        }


        if ($unkdict) {
            while (($word,$pron) = each %vocab) {
                next if $word=~/\266/;
                print OUT "$word\n" unless @{$pron} ;
            }
        } else {
            @phonem = sort { $phonem{$a} <=> $phonem{$b} }
                grep($phonem{$_}>2 && $_ ne "SIL",keys %phonem);
    
            # Pick a very seldom phonem for the words that are artifically in the dictionary
            # to keep the vocabulary module happy
            # These are specifically the class symbols

            $badphonem=$phonem[0];
            $badphonem.=" $badphonem";$badphonem.=" $badphonem";
            $badphonem.=" $badphonem";$badphonem.=" $badphonem";

            foreach (keys %reachable_t) {
                next if $class{$_};
                print OUT lookup_pronounciation($_,split(/\266/))
            }
            foreach (keys %class) {
                next unless $reachable{$_};
                print OUT "\{$_\} \{$badphonem\}\n";
                foreach $symbol (print_net_all($_)) {
                    ($word,$class)=split(/:/,$symbol,2);
                    print OUT lookup_pronounciation($symbol,split(/\266/,$word));
                }
            }
            print OUT "{\$} {SIL}\n{(} {SIL}\n{)} {SIL}\n";
        }
        close OUT;
    }
} elsif ($classes) {
    foreach $class (keys %class) {
        $_=join(" ",$class,print_net_all($class));
        print "$_\n";
    }
} elsif ($inverse_map) {
    foreach $class (keys %class) {
        foreach $classword (print_net_all($class)) {
            ($word)=split(/:/,$classword);
            $imap=$inversemap{$word};
            if (defined $imap) {
                push(@{$imap},$classword);
            } else {
                my(@F)=($classword);
                $inversemap{$word}=\@F;
            }
        }
    }
    while (($word,$imap)=each %inversemap) {
        print join("\#",@{$imap})." $word\n";
    }
} elsif ($modify) {

    while (<>) {
        if (/^\\subsmodel:/) {
            $skipsmodel++;
            last;
        }
        print;
    }                          
    if ($skipsmodel) {
        while (<>) {
            if (/^\\end\\/) {
                last;
            }
        }       
        print while <>;
    }

    print "\\subsmodel:\n";
    foreach $class (sort keys %class) {
        print "-99.9 $class $class\n";
        @classmembers = print_net_all($class);
        $classpenalty = -log(scalar(@classmembers))/log(10);
        foreach $classmember (sort @classmembers) {
            $_="$classpenalty $classmember $class\n";
            print;
        }
    }
    print "\\end\\\n";
} else {                   # Print random sentences
    foreach ($i=0;$i<$n;$i++) { # Select a top-level frame and generate a sentence for it
        print_random_sentence(select_random_entry(\@top_level_prob,"FORM"));    
    }
}


sub print_random_sentence {

    my($select) = @_;
    my($select_local,$parse,$parse_local);
    
    #warn "print_random_sentence: top_level[select] -> $top_level[$select] >> $/";
    
    if ($top_level_perm[$select]) {
        begin_noise();
        $break=1;
        while ($break) {
            $break=0;
            print_net_random(${$top_level_slots[$select]}[$select_local]);
            if ($break) {
                break_noise();
            } else {
                segment_noise();
            }
        }
        end_noise();
    } else {
        begin_noise();
        #foreach $select_local (@{$top_level_slots[$select]}) {
        # tk - fix 
        # I don't understand, but without this fix every line would be a sucession of each net.
        $select_local = $top_level_slots[$select][int(rand(scalar(@{$top_level_slots[$select]})))];
        $break=1;
        while ($break) {
            $break=0;
            print_net_random($select_local);
            if ($break) {
                break_noise();
            } else {
                segment_noise();
            }
        }
        #}
        end_noise();
    }
    print "\n";
}

    sub begin_noise {
        print_net_random("BeginNoise") if defined $public{"BeginNoise"};
    }    


sub end_noise {
    print_net_random("EndNoise") if defined $public{"EndNoise"};
}    


sub segment_noise {
    print_net_random("SegmentNoise") if defined $public{"SegmentNoise"};
}    

sub break_noise {
    print_net_random("BreakNoise") if defined $public{"BreakNoise"};
}    

sub random_noise {
    print_net_random("RandomNoise") if defined $public{"RandomNoise"};
}    

sub random_break {
    if (1-$break_prob<rand()) {
        $break++;
        print " **<** " if $show_break;
    }
}    


sub select_random_entry {
    my($array,$name) = @_;
    my $prob=0;
    my $random = rand();
    my $return=0;
    my $i=0;
    
    foreach (@$array) {
        $prob+=$_;
        return $i if $random<$prob;
        $i++;
    }
    die "generate_random_samples.pl: $name not a probability distribution: ".join(" ",@{$array})."\n";
}

sub normalize_random_entry {
    my $array = shift;
    my $i;
    my $prob = 0;
    my $zero = 0;

    if (@{$array}==1) {
        $$array[0]=1;
        return;
    }
    foreach my $i (@$array) {
        $prob += $i;
        if ($i == 0) {
            $zero++;
        }
    }
    # Allow for floating point rounding errors
    die "Not a probability distribution $prob > 1\n" if ($prob - 1) > 0.01;
    # Use leftover probability mass on missing entries
    if ($zero > 0) {
        $prob = (1-$prob)/$zero;
	foreach my $i (@$array) {
	    $i = $prob if $i == 0;
	}
    }
    # Normalize the whole darn thing if it doesn't sum to one.
    $prob = 0;
    foreach my $i (@$array) {
        $prob += $i;
    }
    if ($prob < 1.0) {
	foreach my $i (@$array) {
	    $i += (1.0 - $prob) / @$array;
	}
    }
}

sub print_net_random {

    my($netname) = @_;
    my($repeat,$type,$body);
    
    my($text) = "";
    
    unless (defined $rules{$netname}) {
        warn  "Rule for $netname not defined\n" if $warn;
        return "";
    }
    
    my($rule_ref) = ${$rules{$netname}}[select_random_entry($rules_prob{$netname},$netname)];
    
    foreach (@$rule_ref) {
        ($repeat,$type,$body) = split(/,/,$_);
        
        while (1) {
            if ($repeat=~s/\*//g) {
                last if rand()<$leave_out_prob;
                $repeat="1" unless $repeat=~/\S/;
            }
            if ($type eq "T" || $class{$body}) {
                if ($clausi) {
                    print "$body\n"; random_noise();
                } else {
                    print "$body "; random_noise();
                }
            } else {
                print_net_random($body);
                random_break();     
            }
            return if $break;
            last if $repeat eq "1";
            last unless rand()<$repeat_prob;
        }
    }
}



    sub print_net_all {
        local($global_netname) = @_;
        local(%already_visited);

        my(@return) = print_net_all2($global_netname);
        my($ret_length)=scalar(@return);
        @return=grep(/\S/,@return);
        warn "$global_netname could expand to empty, ignored\n"
            unless @return==$ret_length;
        @return=grep(($_.=":$global_netname") || 1,@return);
        return @return;
    }


    sub print_net_all2 {

        my($netname) = shift @_;
        my($repeat,$type,$body,$ruleref);
        my(@returnarray)=();

        unless (defined $rules{$netname}) {
            warn  "Rule for $netname not defined\n" if $warn;
            return ();
        }

        return @_ if $already_visited{$netname}>1;
        $already_visited{$netname}++;
        foreach $ruleref (@{$rules{$netname}}) {
            my(@rulereturn)=@_;
            foreach (@{$ruleref}) {
                ($repeat,$type,$body) = split(/,/,$_);

                my(@localrulereturn)=();
    
                if ($repeat eq "*") {
                    @localrulereturn=@rulereturn;
                    @localrulereturn=("") unless @localrulereturn>0;
                }
                warn "Rule for $netname contains repetition $repeat specification -- ignored"
                    unless $repeat eq "1" || $repeat eq "*";

                if ($type eq "T") {
                    $body = "+$body+" if $body =~ s/^&//;
                    if (@rulereturn) {              
                        for ($i=0;$i<=$#rulereturn;$i++) {
                            if ($rulereturn[$i]=~/\S/) {
                                $rulereturn[$i].="\266$body";
                            } else {
                                $rulereturn[$i]="$body";
                            }
                        }
                    } else {                
                        @rulereturn=($body);
                    }
                } else {                    
                    @rulereturn=print_net_all2($body,@rulereturn);
                }
                push(@rulereturn,@localrulereturn);
            }
            push(@returnarray,@rulereturn);
        }
        $already_visited{$netname}--;
        return @returnarray;
    }    


    sub build_reachable_dictionary {

        my($netname);
        my($repeat,$type,$body,$rule_ref);

        my($text) = "";

        while (@netname) {
            $netname = shift @netname;

            if ($class{$netname}) {
                $reachable_t{$netname}++;
                $reachable{$netname}++;
                next;
            }

            unless (defined $rules{$netname}) {
                warn  "Rule for $netname not defined in build_reachable_dictionary\n" if $warn;
                return "";
            }

            foreach $rule_ref (@{$rules{$netname}}) {
                foreach (@{$rule_ref}) {
                    ($repeat,$type,$body) = split(/,/,$_);
                    if ($type eq "T") {
                        $reachable_t{$body}++;
                    } else {
                        unless (exists $reachable{$body}) {
                            push(@netname,$body);
                            $reachable{$body}++;
                        }
                    }
                }
            }
        }
    }           


sub check_grammar {

    while (($definition) = each %public) {
        warn "Net rule $definition not defined\n" unless defined $rules{$definition};
    }
    while (($definition,$count) = each %definition) {
        for ($i=1;$i<=$count;$i++) {
            warn "$definition:$count not defined" unless defined $rules{"$definition:$count"};
        }
    }
}


# see if it's a comment, if so try to find a delimited prob; otherwise uniform
sub get_prob {

    if (s/[\#;]([\s\S]+)//) {
        my $comment=$1;

        if ($comment=~/%%CLASS%%/) {
            $class=1;
        } else {
            $class=0;
        }

        return $1 if $comment=~/%%(\S+)%%/;
    }
    $class=0;
    return 0;
}


sub load_grammar_file {
    local($GRAMMAR_FILE) = @_;
    local($concept,@rules,@rules_prob);
    local(*GRAMMAR);

    #    print STDERR "Reading $GRAMMAR_FILE of $NAME\n";

    $concept = "";
    @rules   = ();
    $conceptclass=0;

    open(GRAMMAR,$GRAMMAR_FILE);
    while (<GRAMMAR>) {
        if (/^\#include\s+(\S+)/) {
            flush_concept();
            $conceptclass=0;
            load_grammar_file($1,$NAME);
        } else {            
            #print STDERR ">> $_\n";  ###########
            $prob=get_prob();   # Get rid of comments, cacth any probs
            next unless /\S/;   # empty line
            #print STDERR ">> $_\n"; #######

            # Classify line         
            if (/^(\S+)/) {     # It's a concept
                $new_concept=$1; # var passed via global... aargh!
                flush_concept();
                $conceptclass=$class;
                $concept=$new_concept;
            } else {
                #print STDERR ">> $_\n"; ##########
                die "No concept defined" unless $concept=~/\S/;
                die "No begin-parenthesis in $concept rules $_\n" unless s/^\s+\(\s*//;
                die "No end-parenthesis in $concept rules $_\n"   unless s/\s*\)\s*$//;

                push(@rules,$_);
                push(@rules_prob,$prob);
            }
        }
    }
    flush_concept();
    close GRAMMAR;
}


sub select_random_array {
    local($array_ref) =  $_[0];;

    return @$array_ref[int(rand(scalar(@$array_ref)))];
}


sub flush_concept {

    # Determine the symbol-table-entry for the concept defined
    # If it is a top-level entry, it is globally visible.
    # Otherwise, it is only local and may be multiple definded and
    # overwritten

    return unless $concept=~/\S/;

    if ($public{$concept}) {
        $real_name=$concept;
    } else {
        $definition{$NAME,$concept};
        $real_name="$NAME:$concept:".($definition{"$NAME:$concept"}+1);
    }

    die "Multiple definition of $real_name\n" if defined $rules{$real_name};


    my @new_rules = ();

    foreach (@rules) {
        my @rule = ();
        #print STDERR ">>> $_\n";
        foreach $body (split) {
            #print STDERR "\t>>> $body "; ##########
            $repeat = "1";
            $repeat = $1 if $body=~s/^([\+\*]+)//; # remember if */+ used
            $body =~ s/^\s+//; $body =~ s/\s+$//;  # trim end silences
            #print STDERR " => $body\n"; ################
            if ( ($body=~/^[^A-Z_\-]/) && !($body =~ /^\[.*\]$/)) {
                $type="T";
                $body=~s/\+/'/g; # '
                $body=~tr/a-z/A-Z/ if $capital;
                $body = "+$body+" if $body =~ s/^&//;
                $dictionary{$body}++;
            } else {
                $type="N";

                unless ($public{$body}) {
                    $body="$NAME:$body:".($definition{"$NAME:$body"}+1);
                }
            }
            push(@rule,"$repeat,$type,$body");
        }

        push(@new_rules,\@rule);
    }

    $rules{$real_name}=\@new_rules;

    my @new_rules_prob = @rules_prob;
    normalize_random_entry(\@new_rules_prob);
    $rules_prob{$real_name}=\@new_rules_prob;


    unless ($public{$concept}) {
        $definition{"$NAME:$concept"}++;
    }

    @rules = ();
    @rules_prob = ();
    $class{$real_name}++ if $conceptclass && !$noclasses;

    $concept="";

}

sub lookup_pronounciation {

    local($word) = shift @_;

    local($error)="";

    my(@pron);
    my($pronword)=join("\266",@_);

    if (defined $vocab{join("\266",@_)} && @{$vocab{join("\266",@_)}}>0) {
        @pron = allchunk(join("\266",@_));
    } elsif (($pronword=~s/\266S$/\'S/) && (defined $vocab{$pronword}) && @{$vocab{$pronword}}>0) {
        @pron = allchunk($pronword);
    } else {
        @pron = allchunk(@_);
    }
    return $error if $error=~/\S/; # Error case
    my($ret) = "{$word(0)} {$pron[0]}\n";
    for ($i=1;$i<=$#pron;$i++) {
        $ret .= "{$word(".($i+1).")} {$pron[$i]}\n";
    }
    return $ret;
}

sub allchunk {
    my $prefix = shift @_;
    my $dictref = $vocab{$prefix};
    my @ret=();

    unless ((defined $dictref) && (@{$dictref}>0)) {
        $error .= "ERROR: Not all subwords ($prefix) known for $word\n";
        allchunk(@_) unless @_==0;

        return ();
    }

    return @{$dictref} if @_==0;

    my($suff,$pref);

    foreach $suff (allchunk(@_)) {
        foreach $pref (@{$dictref}) {
            push(@ret,"$pref $suff");
        }
    }
    return @ret;        
}
    
        
        


sub help {
    print <<EOT;
generate_random_samples -h -n N -break prob -show_break -d dir -forms ext -noclass -warn -capital
                -clausi -repeat prob -leave_out prob -vocabulary -class -iclass
                -dictionary -modifyLM [LanguageModel]


Generates random sentences from a PHOENIX grammar that is in the
current directory as if it were a stochastic context free grammmar.


-h             --  print this help
-n N           --  print N random sentences ( default $n_default )
-break prob    --  probability of a sentence break ( default $break_prob_default )
-show_break    --  visualizes the occurence of breaks for debugging
-d dir         --  change to directory dir as the PHOENIX grammar dir
-forms ext     --  use forms.ext instead of forms to generate from
-noclass       --  don\'t use classes when generating text, which is more human-readable
-warn          --  only test the grammar
-capital       --  capitalize all words
-clausi        --  use special output format
-repeat prob   --  repetition probability ( default $repeat_prob_default )
-leave_out prob--  leaving out an optional item probability ( default $leave_out_prob_default )
-vocabulary    --  print the vocabulary used in the grammar usable for the LM
-class         --  print the classes used
-iclass        --  print the inverse classes used
-dictionary    --  only print all the words in the grammar for pronounciation
           dictionary construction
-modifyLM      --  takes an LM from stdin or from files after the options
           (possibly with old classes, these will be eliminated)
           and adds classes according to the specification by the grammar 
-makedict      --  takes a dictionary from stdin or from files after the options
           and generates a pronounciation dictionary
-unkdict       --  takes a dictionary from stdin or from files after the options
           and generates a list of unknown words related to that dictionary
           The assumtion is, that all word strings are comosed into subwords

Written by Klaus Ries <ries\@cs.cmu.edu> <kries\@ira.uka.de>
Copyright (C) 1996,1997 Interactive System Labs and Klaus Ries


Detailed description:

All productions are assumed to be equally likely, unless one specifies
probabilities after some of them ( the remain probability mass is
distributed over the other prodcuctions ) in the form   %%0.5%% ,
which is interpreted as a comment by PHOENIX.
A network can have the specification %%CLASS%% in a comment like in

[townname_l]  # %%CLASS%% 
( split )
( bihac )
(banja-luka)
(sarajevo) # %%0.9%% since we are always talk about it
(tuzla)
(drvar)

Elements within one class are assumed to be equiprobable.
The specification of \"+\" is ignored and recursions are broken after
the second visit of a net.

The probability for leaving out an optional event is $leave_out_prob,
the probability for repeating an repeatable event is $repeat_prob.
One can either change these probabilies in the code or can rewrite the
grammar such that one can specify them with corresponding rules.

The generation of noises is possible if a file "noises.gra" is placed in
same directory with the other grammar files, not all slots have to be present.
The file "noises.gra" should look like
______________________________________


# &garbage is expanded to +garbage+ by definition, since "+" at
# the beginning of a tokebn always means repetition
BeginNoise      # called before every utterance
( )                             # %%0.6%%
( &garbage )                    # %%0.25%%
( &garbage &garbage )
( &garbage &garbage &garbage )

EndNoise      # called after every utterance
( )                             # %%0.8%%
( &garbage )                    # %%0.15%%
( &garbage &garbage )
( &garbage &garbage &garbage )

SegmentNoise # called between slots in the forms-file
( )                             # %%0.97%%
( &garbage )                    # %%0.015%%
( &garbage &garbage )           # %%0.01%%
( &garbage &garbage &garbage )

BreakNoise   # called after a break occured instead of SegmentNoise
( )                             # %%0.3%%
( &garbage )                    # %%0.6%%
( &garbage &garbage )

RandomNoise  # Called after every word         
( )                             # %%0.99%%
( &garbage )                    # %%0.008%%
( &garbage &garbage )
______________________________________

A break is a reset of a slot that is entered in the forms file
and simulates restarts resp. corrections.
It may occur at every word and a probabilty for this event can be specified.
Since it is usually marked by discourse markers a separate noise slot is defined for
this event.


EOT
    exit(0);
}
