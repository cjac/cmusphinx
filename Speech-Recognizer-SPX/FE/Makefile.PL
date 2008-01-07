use ExtUtils::MakeMaker;
use strict;
use vars qw($PKG_PREFIX);

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
	      'NAME'	=> 'Audio::MFCC',
	      'VERSION_FROM' => 'MFCC.pm', # finds $VERSION
	      LIBS	   => [$PKG_LIBS],
	      INC          => $PKG_CFLAGS,
	      'OPTIMIZE'    => '-g -O2',
);
