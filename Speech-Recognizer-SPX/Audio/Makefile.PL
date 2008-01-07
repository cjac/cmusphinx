use ExtUtils::MakeMaker;
use strict;
use vars qw($PKG_LIBS $PKG_CFLAGS);

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.

# The audio interface has some nasty incompatibility problems, we'll
# deal with them later if we re-add PocketSphinx support.
my $defs = "-DHAVE_ADAPT_RATE -DHAVE_C_LOGFP";

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
	      'NAME'	=> 'Audio::SPX',
	      'VERSION_FROM' => 'SPX.pm', # finds $VERSION
	      LIBS	   => [$PKG_LIBS],
	      INC          => $PKG_CFLAGS,
	      'DEFINE'	=> $defs,
	      'OPTIMIZE'    => '-g -O2',
);
