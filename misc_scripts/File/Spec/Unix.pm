package File::Spec::Unix;

use strict;
use vars qw($VERSION);

$VERSION = '3.2501';

sub canonpath {
    my ($self,$path) = @_;
    
    # Handle POSIX-style node names beginning with double slash (qnx, nto)
    # (POSIX says: "a pathname that begins with two successive slashes
    # may be interpreted in an implementation-defined manner, although
    # more than two leading slashes shall be treated as a single slash.")
    my $node = '';
    my $double_slashes_special = $^O eq 'qnx' || $^O eq 'nto';
    if ( $double_slashes_special && $path =~ s{^(//[^/]+)(?:/|\z)}{/}s ) {
      $node = $1;
    }
    # This used to be
    # $path =~ s|/+|/|g unless ($^O eq 'cygwin');
    # but that made tests 29, 30, 35, 46, and 213 (as of #13272) to fail
    # (Mainly because trailing "" directories didn't get stripped).
    # Why would cygwin avoid collapsing multiple slashes into one? --jhi
    $path =~ s|/{2,}|/|g;                            # xx////xx  -> xx/xx
    $path =~ s{(?:/\.)+(?:/|\z)}{/}g;                # xx/././xx -> xx/xx
    $path =~ s|^(?:\./)+||s unless $path eq "./";    # ./xx      -> xx
    $path =~ s|^/(?:\.\./)+|/|;                      # /../../xx -> xx
    $path =~ s|^/\.\.$|/|;                         # /..       -> /
    $path =~ s|/\z|| unless $path eq "/";          # xx/       -> xx
    return "$node$path";
}

sub catdir {
    my $self = shift;

    $self->canonpath(join('/', @_, '')); # '' because need a trailing '/'
}

sub catfile {
    my $self = shift;
    my $file = $self->canonpath(pop @_);
    return $file unless @_;
    my $dir = $self->catdir(@_);
    $dir .= "/" unless substr($dir,-1) eq "/";
    return $dir.$file;
}

sub curdir () { '.' }

sub devnull () { '/dev/null' }

sub rootdir () { '/' }

my $tmpdir;
sub _tmpdir {
    return $tmpdir if defined $tmpdir;
    my $self = shift;
    my @dirlist = @_;
    {
	no strict 'refs';
	if (${"\cTAINT"}) { # Check for taint mode on perl >= 5.8.0
            require Scalar::Util;
	    @dirlist = grep { ! Scalar::Util::tainted($_) } @dirlist;
	}
    }
    foreach (@dirlist) {
	next unless defined && -d && -w _;
	$tmpdir = $_;
	last;
    }
    $tmpdir = $self->curdir unless defined $tmpdir;
    $tmpdir = defined $tmpdir && $self->canonpath($tmpdir);
    return $tmpdir;
}

sub tmpdir {
    return $tmpdir if defined $tmpdir;
    $tmpdir = $_[0]->_tmpdir( $ENV{TMPDIR}, "/tmp" );
}

sub updir () { '..' }

sub no_upwards {
    my $self = shift;
    return grep(!/^\.{1,2}\z/s, @_);
}

sub case_tolerant () { 0 }

sub file_name_is_absolute {
    my ($self,$file) = @_;
    return scalar($file =~ m:^/:s);
}

sub path {
    return () unless exists $ENV{PATH};
    my @path = split(':', $ENV{PATH});
    foreach (@path) { $_ = '.' if $_ eq '' }
    return @path;
}

sub join {
    my $self = shift;
    return $self->catfile(@_);
}

sub splitpath {
    my ($self,$path, $nofile) = @_;

    my ($volume,$directory,$file) = ('','','');

    if ( $nofile ) {
        $directory = $path;
    }
    else {
        $path =~ m|^ ( (?: .* / (?: \.\.?\z )? )? ) ([^/]*) |xs;
        $directory = $1;
        $file      = $2;
    }

    return ($volume,$directory,$file);
}

sub splitdir {
    return split m|/|, $_[1], -1;  # Preserve trailing fields
}

sub catpath {
    my ($self,$volume,$directory,$file) = @_;

    if ( $directory ne ''                && 
         $file ne ''                     && 
         substr( $directory, -1 ) ne '/' && 
         substr( $file, 0, 1 ) ne '/' 
    ) {
        $directory .= "/$file" ;
    }
    else {
        $directory .= $file ;
    }

    return $directory ;
}

sub abs2rel {
    my($self,$path,$base) = @_;
    $base = $self->_cwd() unless defined $base and length $base;

    ($path, $base) = map $self->canonpath($_), $path, $base;

    if (grep $self->file_name_is_absolute($_), $path, $base) {
	($path, $base) = map $self->rel2abs($_), $path, $base;
    }
    else {
	# save a couple of cwd()s if both paths are relative
	($path, $base) = map $self->catdir('/', $_), $path, $base;
    }

    my ($path_volume) = $self->splitpath($path, 1);
    my ($base_volume) = $self->splitpath($base, 1);

    # Can't relativize across volumes
    return $path unless $path_volume eq $base_volume;

    my $path_directories = ($self->splitpath($path, 1))[1];
    my $base_directories = ($self->splitpath($base, 1))[1];

    # For UNC paths, the user might give a volume like //foo/bar that
    # strictly speaking has no directory portion.  Treat it as if it
    # had the root directory for that volume.
    if (!length($base_directories) and $self->file_name_is_absolute($base)) {
      $base_directories = $self->rootdir;
    }

    # Now, remove all leading components that are the same
    my @pathchunks = $self->splitdir( $path_directories );
    my @basechunks = $self->splitdir( $base_directories );

    if ($base_directories eq $self->rootdir) {
      shift @pathchunks;
      return $self->canonpath( $self->catpath('', $self->catdir( @pathchunks ), '') );
    }

    while (@pathchunks && @basechunks && $self->_same($pathchunks[0], $basechunks[0])) {
        shift @pathchunks ;
        shift @basechunks ;
    }
    return $self->curdir unless @pathchunks || @basechunks;

    # $base now contains the directories the resulting relative path 
    # must ascend out of before it can descend to $path_directory.
    my $result_dirs = $self->catdir( ($self->updir) x @basechunks, @pathchunks );
    return $self->canonpath( $self->catpath('', $result_dirs, '') );
}

sub _same {
  $_[1] eq $_[2];
}

sub rel2abs {
    my ($self,$path,$base ) = @_;

    # Clean up $path
    if ( ! $self->file_name_is_absolute( $path ) ) {
        # Figure out the effective $base and clean it up.
        if ( !defined( $base ) || $base eq '' ) {
	    $base = $self->_cwd();
        }
        elsif ( ! $self->file_name_is_absolute( $base ) ) {
            $base = $self->rel2abs( $base ) ;
        }
        else {
            $base = $self->canonpath( $base ) ;
        }

        # Glom them together
        $path = $self->catdir( $base, $path ) ;
    }

    return $self->canonpath( $path ) ;
}

# Internal routine to File::Spec, no point in making this public since
# it is the standard Cwd interface.  Most of the platform-specific
# File::Spec subclasses use this.
sub _cwd {
    require Cwd;
    Cwd::getcwd();
}

# Internal method to reduce xx\..\yy -> yy
sub _collapse {
    my($fs, $path) = @_;

    my $updir  = $fs->updir;
    my $curdir = $fs->curdir;

    my($vol, $dirs, $file) = $fs->splitpath($path);
    my @dirs = $fs->splitdir($dirs);
    pop @dirs if @dirs && $dirs[-1] eq '';

    my @collapsed;
    foreach my $dir (@dirs) {
        if( $dir eq $updir              and   # if we have an updir
            @collapsed                  and   # and something to collapse
            length $collapsed[-1]       and   # and its not the rootdir
            $collapsed[-1] ne $updir    and   # nor another updir
            $collapsed[-1] ne $curdir         # nor the curdir
          ) 
        {                                     # then
            pop @collapsed;                   # collapse
        }
        else {                                # else
            push @collapsed, $dir;            # just hang onto it
        }
    }

    return $fs->catpath($vol,
                        $fs->catdir(@collapsed),
                        $file
                       );
}

1;
