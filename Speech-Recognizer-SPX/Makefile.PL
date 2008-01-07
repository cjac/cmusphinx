use ExtUtils::MakeMaker;
use Getopt::Long;
use strict;

use vars qw($PKG_LIBS $PKG_CFLAGS $PKG_PREFIX);

chomp($PKG_CFLAGS = `pkg-config --cflags pocketsphinx`);
if ($?) {
    die <<EOF;
Unable to locate PocketSphinx using pkg-config.  Please make sure that
you have installed pkg-config and PocketSphinx on your system, and
that PKG_CONFIG_PATH is set correctly.
EOF
}
chomp($PKG_LIBS = `pkg-config --libs pocketsphinx sphinxbase`);
chomp($PKG_PREFIX = `pkg-config --variable=prefix pocketsphinx`);

unlink './SPX/Config.pm';
open CONFIG, ">./SPX/Config.pm"
    or die "can't open ./SPX/Config.pm: $!";
print CONFIG <<EOF;
package Speech::Recognizer::SPX;
\$PREFIX = '$PKG_PREFIX';
\$SPHINXDIR = '$PKG_PREFIX/share/pocketsphinx';
1;
EOF
close CONFIG;

# This is necessary for PocketSphinx.  We'll add back support for
# Sphinx-II later.
my @defs = ('-DPOST_0_6_API');

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
	      NAME	   => 'Speech::Recognizer::SPX',
	      VERSION_FROM => 'SPX.pm',
	      DIR          => [ 'FE', 'Audio' ],
	      LIBS	   => [$PKG_LIBS],
	      INC          => $PKG_CFLAGS,
	      DEFINE       => "@defs",
	      OPTIMIZE     => '-g -O2',
	      # For Speech::Recognizer::SPX::Server
	      PREREQ_PM    => { 'Time::HiRes' => 0 },
	      clean        => { FILES => 'SPX/Config.pm' },
);
